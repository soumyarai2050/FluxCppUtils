//
// Created by kumar on 12/25/2020.
//

#ifndef TACTICALTRADING_DEV_UTILS_H
#define TACTICALTRADING_DEV_UTILS_H

#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <limits>

class DevUtils {
public:
    [[maybe_unused]] static inline void SleepAndThrow(const char *error, uint8_t sleepInterval = 3) {
        if (sleepInterval < 1 || sleepInterval > 10)
            sleepInterval = 3; //else not required
        sleep(sleepInterval);
        throw std::runtime_error(error);
    }

    [[maybe_unused]] static inline void SleepConsoleErrorAndExit(const char *error, uint8_t sleepInterval = 3) {
        std::cerr << error;
        if (sleepInterval < 1 || sleepInterval > 10)
            sleepInterval = 3; //else not required
        sleep(sleepInterval);
        exit(1);
    }

    static inline int getNextUniqueSequentialReqID() {
        static int reqIDCounter = 0;
        return ++reqIDCounter;
    }

    [[maybe_unused]] static bool IsZero(double zeroVal) {
        if (zeroVal < std::numeric_limits<double>::epsilon() &&
            zeroVal > -std::numeric_limits<double>::epsilon()) {
            return true;
        }
        return false;
    }

    [[maybe_unused]] static bool IsZero(float zeroVal) {
        if (zeroVal < std::numeric_limits<float>::epsilon() &&
            zeroVal > -std::numeric_limits<float>::epsilon()) {
            return true;
        }
        return false;
    }

};

#endif //TACTICALTRADING_DEV_UTILS_H
