#ifndef BP_NET_H
#define BP_NET_H

#if defined(BP_PLATFORM_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #include <WinSock2.h>
    #include <Ws2tcpip.h>
        
    #define BP_SOCKET SOCKET
    #define BP_SOCKET_INVALID (BP_SOCKET)(~0)
        
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <errno.h>
        
    #define BP_SOCKET int
    #define BP_SOCKET_INVALID -1
#endif

namespace bp { namespace net {
    static bool init()
    {
#if defined(BP_PLATFORM_WINDOWS)
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            return false;
#endif

        return true;
    }

    static void cleanup()
    {
#if defined(BP_PLATFORM_WINDOWS)
        WSACleanup();
#endif
    }

    static int close(BP_SOCKET sock)
    {
#if defined(BP_PLATFORM_WINDOWS)
        return closesocket(sock);
#else
        return ::close(sock);
#endif
    }

    static int last_error()
    {
#if defined(BP_PLATFORM_WINDOWS)
        return WSAGetLastError();
#else
        return errno;
#endif
    }

    static int set_timeout(BP_SOCKET sock, int timeout)
    {
        struct timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    }

    static bool connect(BP_SOCKET sock, const char* ip, u_short port)
    {
        struct sockaddr_in local_addr;
        memset(&local_addr, 0, sizeof(local_addr));

        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip, &local_addr.sin_addr) == 0)
            return false;

        if (bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) == BP_SOCKET_INVALID)
            return false;

        return true;
    }
} /* bp */ } /* net */

#endif