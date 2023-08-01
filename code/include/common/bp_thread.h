#ifndef BP_THREAD_H
#define BP_THREAD_H

#if defined(_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <Windows.h>
#else
    #include <pthread.h>
#endif

namespace bp
{
    class thread
    {
    public:
        thread() = default;
        virtual ~thread() = default;

        void start(void (*fn)(void*), void* udata)
        {
            _udata = udata;

#if defined(_WIN32)
            _handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fn, udata, 0, NULL);
#else
            pthread_create(&_handle, NULL, _thread_fn, udata);
#endif
        }

        void join()
        {
#if defined(_WIN32)
            WaitForSingleObject(_handle, INFINITE);
#else
            pthread_join(_handle, NULL);
#endif
        }

        void detach()
        {
#if defined(_WIN32)
            SetThreadPriority(_handle, THREAD_PRIORITY_NORMAL);
            SetThreadPriorityBoost(_handle, FALSE);
            SetThreadAffinityMask(_handle, 0x0001);
            ResumeThread(_handle);
#else
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            pthread_create(&_handle, &attr, _thread_fn, _udata);
#endif
        }

    private:
        void* _udata;

#if defined(_WIN32)
        static DWORD WINAPI _thread_fn(void* udata);
        HANDLE _handle;
#else
        static void* _thread_fn(void* udata);
        pthread_t _handle;
#endif
    };
}

#endif