#ifndef _SPINLOCK_H_20170410_
#define _SPINLOCK_H_20170410_

#include <atomic>

class spinlock {
public:
    spinlock() { m_lock.clear(); }
    spinlock(const spinlock&) = delete;
    ~spinlock() = default;

    void lock() {
	 while (m_lock.test_and_set(std::memory_order_acquire));
    }
    bool try_lock() {
         return !m_lock.test_and_set(std::memory_order_acquire);
    }
    void unlock() {
        m_lock.clear(std::memory_order_release);
    }
private:
    std::atomic_flag m_lock;
};
#endif//_SPINLOCK_H_20170410_
