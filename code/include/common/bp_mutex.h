#ifndef BP_MUTEX_H
#define BP_MUTEX_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <ptrhead.h>
#endif

namespace bp
{
    // Windows
    //------------------------------------------------------------------------
#ifdef _WIN32
    class mutex
    {
    public:
        mutex() {
            InitializeCriticalSection(&_cs);
        }

        ~mutex() {
            DeleteCriticalSection(&_cs);
        }

        void lock() {
            EnterCriticalSection(&_cs);
        }

        void unlock() {
            LeaveCriticalSection(&_cs);
        }

    private:
        CRITICAL_SECTION _cs;
    };
#else
    // Other (pthread)
    //------------------------------------------------------------------------
    class mutex {
    public:
        mutex() {
            pthread_mutex_init(&_mutex, nullptr);
        }

        ~mutex() {
            pthread_mutex_destroy(&_mutex);
        }

        void lock() {
            pthread_mutex_lock(&_mutex);
        }

        void unlock() {
            pthread_mutex_unlock(&_mutex);
        }

    private:
        pthread_mutex_t _mutex;
    };
#endif
}

#endif