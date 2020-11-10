#ifndef SOCKET_H
#define SOCKET_H

#include <inttypes.h>

#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)
typedef int64_t sock_t;
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
typedef int32_t sock_t;
#define PLATFORM PLATFORM_MAC
#else
typedef int32_t sock_t;
#define PLATFORM PLATFORM_UNIX
#endif


uint32_t socket_open(const char *ip, uint16_t port);
uint32_t socket_open_dns(const char* domain_ip, const char* port);
uint32_t socket_domain_exists(const char* domain_ip, const char* port);
void     socket_close(void);
int      socket_recv(void *data, int size);
int      socket_send(void *data, int size);
void     socket_open_client(void);
void     socket_close_client(void);
void     RC4(void *data, int length, unsigned char *key);
extern char decryption_key[30];

#endif

