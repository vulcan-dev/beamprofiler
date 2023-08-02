#define SDL_MAIN_HANDLED

#include "common/common.h"
#include "beamprofiler.h"
#include "interface.h"
#include "config.h"
#include <cstdio>

// TODO: Add back the ability to have multiple instances once you can change IP & Port

#if defined(BP_PLATFORM_WINDOWS)
HANDLE mutex_handle;
#else
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

const char* semaphore_name = "/beam_profiler";
sem_t* semaphore;
#endif

#define BUFLEN 256

using namespace bp;

void exit_error(BP_SOCKET sock, const char* message, ...)
{
    va_list args;
    va_start(args, message);

#if defined(BP_PLATFORM_WINDOWS)
    char formatted[BUFLEN];
    vsprintf_s(formatted, BUFLEN, message, args);

    int len = MultiByteToWideChar(CP_UTF8, 0, formatted, -1, NULL, 0);
    WCHAR* wide_formatted = (WCHAR*)malloc(len * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, formatted, -1, wide_formatted, len);

    // Get the error message string
    WCHAR error_message[BUFLEN];
    DWORD last_error = GetLastError();
    if (last_error != ERROR_SUCCESS &&
        FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        last_error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        error_message,
        BUFLEN,
        NULL
    ) > 0)
    {
        WCHAR final_message[BUFLEN];
        swprintf(final_message, BUFLEN, L"%s\n\n%s", wide_formatted, error_message);
        MessageBoxW(NULL, final_message, L"BeamProfiler Error", MB_OK | MB_ICONERROR);
    }
    else
    {
        MessageBoxW(NULL, wide_formatted, L"BeamProfiler Error", MB_OK | MB_ICONERROR);
    }

    ReleaseMutex(mutex_handle);
    CloseHandle(mutex_handle);

    free(wide_formatted);
#else
    vfprintf(stderr, message, args);
    fprintf(stderr, ": %s\n", strerror(errno));

    if (semaphore)
        sem_close(semaphore);
    sem_unlink(semaphore_name);
#endif

    va_end(args);

    if (sock > 0)
        net::close(sock);
    net::cleanup();

    exit(-1);
}

BP_SOCKET create_socket(const char* address, const int port)
{
    BP_SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket < 0)
        return -1;

    if (net::set_timeout(sock, 100) < 0)
        return -1;

    if (!net::connect(sock, address, port))
        return 0;

    return sock;
}

#if defined(BP_PLATFORM_WINDOWS)
int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmdline, int cmdshow) {
#if defined(_DEBUG) // Create console if debug build.
    HWND foreground_window = GetForegroundWindow();

    if (AllocConsole())
    {
        FILE* f;
        freopen_s(&f, "conout$", "w", stdout);
        freopen_s(&f, "conout$", "w", stderr);
    } else
    {
        MessageBoxW(NULL, L"AllocConsole Failed! Continuing without console", L"BeamProfiler", MB_OK | MB_ICONERROR);
    }
#endif

    const TCHAR mutex_name[] = "BeamProfiler";
    mutex_handle = CreateMutex(NULL, TRUE, mutex_name);
    if (mutex_handle != NULL && GetLastError() == ERROR_ALREADY_EXISTS) {
        fprintf(stderr, "Another instance is already running");
        MessageBoxW(NULL, L"Another instance is already running", L"BeamProfiler", MB_OK | MB_ICONEXCLAMATION);

        ReleaseMutex(mutex_handle);
        CloseHandle(mutex_handle);
        return 0;
    }

#else
int main(void) {
    semaphore = sem_open(semaphore_namem, O_CREAT | O_EXCL, 0666, 1);
    if (semaphore == SEM_FAILED)
    {
        sem_unlink(semaphore_name);
        fprintf(stderr, "Another instance is already running");
        return 0;
    }
#endif
    if (!net::init())
        exit_error(NULL, "Failed to initialize Winsock! Error Code: %d", net::last_error());

    render_thread_data_t* render_data = new render_thread_data_t;
    render_data->data.bullet_time = 1;
    render_data->can_read = true;

    if (!config_load(&render_data->config))
    {
        delete render_data;
        exit_error(NULL, "Invalid config, please delete it and re-run BeamProfiler");
    }

    BP_SOCKET sock = create_socket(config_get_ip(&render_data->config).c_str(), render_data->config.port);
    if (sock == -1)
    {
        delete render_data;
        exit_error(sock, "Failed to create socket, last error: %d", net::last_error());
    } else if (sock == 0)
    {
        render_data->connected = false;
        render_data->can_read = false; // Everything else was fine, we just couldn't reach the IP
    }

#if defined(_WIN32) && defined(_DEBUG)
    render_data->foreground_window = foreground_window;
#endif

    render_data->request_close = false;
    render_data->requested_connect = false;

    char buf[BUFLEN];
    
    bp::thread render_thread;
    { // Start render thread
        render_thread.start(interface_thread, (void*)render_data);
        render_thread.detach();
    }

    { // Main Thread (Receive Data)
        struct sockaddr_in remote;
        int remote_len = sizeof(remote);

        while (!render_data->request_close)
        {
            // Check if we tried to connect to new address
            if (render_data->requested_connect)
            {
                render_data->requested_connect = false;

                net::close(sock);

                sock = create_socket(config_get_ip(&render_data->config).c_str(), render_data->config.port);
                if (sock == NULL)
                {
                    render_data->can_read = false;
                    render_data->connected = false;
                } else
                {
                    render_data->can_read = true;
                }

                render_data->queue_mtx.lock();
                render_data->queue.push_back(operation_e::op_reset);
                render_data->queue_mtx.unlock();
                continue;
            }

            if (!render_data->can_read)
                continue;

            int recv_len = recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr*)&remote, &remote_len);
            if (recv_len == -1)
            {
                render_data->data.bullet_time = 0; // For pausing. We can't send if we're paused because the vehicle lua also gets paused (obviously)
                render_data->connected = false;
                continue;
            }

            if (!render_data->connected)
                render_data->connected = true;

            if (recv_len == 1)
            {
                const char received_value = *buf - '0';
                render_data->queue_mtx.lock();
                render_data->queue.push_back(received_value);
                render_data->queue_mtx.unlock();
            } else
            {
                memcpy(&render_data->data, buf, sizeof(profiler_t));
            }
        }
    }
    
    render_thread.join();
    delete render_data;

    net::close(sock);
    net::cleanup();

#if defined(BP_PLATFORM_WINDOWS)
    ReleaseMutex(mutex_handle);
    CloseHandle(mutex_handle);

    #if defined(_DEBUG)
        FreeConsole();
    #endif

#else
    sem_close(semaphore);
    sem_unlink(semaphore_name);
#endif

    return 0;
}