#ifndef TT_SPIN_READ_WRITE_LOCK
#define TT_SPIN_READ_WRITE_LOCK

#include <atomic>
#include <sched.h>


#define ATOMIC_INIT_VAL -1
#define ATOMIC_LOCK_VAL 0

//Linux 64 bit
inline bool ATOMIC_CAS_32(volatile uint32_t *p, uint32_t oldVal, uint32_t newVal) {
    return (__sync_bool_compare_and_swap(p, oldVal, newVal));
}

#define ATOMIC_SOFT_BARRIER __asm__ ("": : :"memory")

class SpinReadWriteLock
{
	enum {
		UNLOCK_VAL = 1
	};
private:
	uint32_t lockVal;
public:
	SpinReadWriteLock() {
        lockVal = UNLOCK_VAL;
    }

    inline bool WriterTryLock() {
        return ATOMIC_CAS_32(&lockVal, UNLOCK_VAL, ATOMIC_LOCK_VAL);
    }

    inline bool WriterLock() {
        while (!(ATOMIC_CAS_32(&lockVal, UNLOCK_VAL, ATOMIC_LOCK_VAL)));
        return true;
    }

    //causes the calling thread to relinquish the CPU.  The thread is moved to
    //the end of the queue for its static priority and a new thread gets to run
    [[maybe_unused]] inline bool WriterRelinquishLock() {
        while (!(ATOMIC_CAS_32(&lockVal, UNLOCK_VAL, ATOMIC_LOCK_VAL))) {
            sched_yield();
        }
        return true;
    }

    inline bool WriterUnlock() {
        return (ATOMIC_CAS_32(&lockVal, ATOMIC_LOCK_VAL, UNLOCK_VAL));
    }

	inline bool ReaderTryLock() {
		do {
			ATOMIC_SOFT_BARRIER;
			uint32_t curLockVal = lockVal;
			if( ATOMIC_LOCK_VAL != curLockVal) {
                if (ATOMIC_CAS_32(&lockVal, curLockVal, curLockVal + 1)) {
                    return true;
                }
                //else not required, some other reader lock or did unlock this time, simply retry.
            }
			else {
				return false; //Writer has lock
			}
		}
		while(ATOMIC_LOCK_VAL != lockVal);
		return false;
	}

	inline bool ReaderRelinquishLock() {
        do {
            ATOMIC_SOFT_BARRIER;
            uint32_t curLockVal = lockVal;
            if (ATOMIC_LOCK_VAL != curLockVal) {
                if (ATOMIC_CAS_32(&lockVal, curLockVal, curLockVal + 1)) {
                    return true;
                }
                //else not required, some other reader lock or did unlock this time, simply retry.
            } else {
                //Locked by Writer just relinquish
                sched_yield();
            }
        } while (true);
	}

    inline bool ReaderUnlock() {
		do {
            ATOMIC_SOFT_BARRIER;
            uint32_t curLockVal = lockVal;
            if (UNLOCK_VAL < curLockVal) {
                if (ATOMIC_CAS_32(&lockVal, curLockVal, curLockVal - 1)) {
                    return true;
                }
                //else not required, some other reader lock or did unlock this time, simply retry.
            } else {
                return false;
            }
        } while (true);
    }

    [[maybe_unused]] void lock() { WriterLock(); }

    [[maybe_unused]] bool try_lock() { return WriterTryLock(); }

    [[maybe_unused]] void unlock() { WriterUnlock(); }

    [[maybe_unused]] void lock_shared() { ReaderRelinquishLock(); }

    [[maybe_unused]] bool try_lock_shared() { return ReaderTryLock(); }

    [[maybe_unused]] void unlock_shared() { ReaderUnlock(); }
};

class SharedSpinLockGuard {
public:
    inline explicit SharedSpinLockGuard(SpinReadWriteLock &sL) : rwSpinLock(&sL) {
        rwSpinLock->ReaderRelinquishLock();
    }

    inline ~SharedSpinLockGuard() {
        rwSpinLock->ReaderRelinquishLock();
    }

    [[maybe_unused]] inline void lock() {
        rwSpinLock->ReaderRelinquishLock();
    }

    [[maybe_unused]] inline void unlock() {
        rwSpinLock->ReaderUnlock();
    }

    SharedSpinLockGuard() = delete;

    SharedSpinLockGuard(SharedSpinLockGuard &m) = delete;

    SharedSpinLockGuard &operator=(const SharedSpinLockGuard &rhs) = delete;

protected:
    SpinReadWriteLock *rwSpinLock;
};

class UniqueSpinLockGuard {
public:
    inline explicit UniqueSpinLockGuard(SpinReadWriteLock &sL) : rwSpinLock(&sL) {
        rwSpinLock->WriterLock();
    }

    inline ~UniqueSpinLockGuard() {
        rwSpinLock->WriterUnlock();
    }

    [[maybe_unused]] inline void lock() {
        rwSpinLock->WriterLock();
    }

    [[maybe_unused]] inline void unlock() {
        rwSpinLock->WriterUnlock();
    }

    UniqueSpinLockGuard() = delete;

    [[maybe_unused]] UniqueSpinLockGuard(SharedSpinLockGuard &m) = delete;

    [[maybe_unused]] UniqueSpinLockGuard &operator=(const SharedSpinLockGuard &rhs) = delete;

protected:
    SpinReadWriteLock *rwSpinLock;
};

//Usage: SpinReadWriteLock l1;
//For reading: UniqueSpinLockGuard guard(l1)
//For writing: SharedSpinLockGuard guard(l1)

#endif//TT_SPIN_READ_WRITE_LOCK
