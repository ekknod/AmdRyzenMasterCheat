#include "socket.h"
#include <string.h>

#if PLATFORM == PLATFORM_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment( lib, "Ws2_32.lib")
typedef int64_t sock_t;
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
typedef int32_t sock_t;
#endif


static unsigned int inet_pton4(const char* src);
void RC4(void *data, int length, unsigned char *key);

char decryption_key[30];

SOCKET g_socket = INVALID_SOCKET;
static SOCKET g_client;
static SOCKET get_socket_0(void) { return g_socket; }
static SOCKET get_socket_1(void) { return g_client; }
static SOCKET (*get_socket)(void);

uint32_t socket_open(const char *ip, uint16_t port)
{
	SOCKET sock;
	int no_delay = 1;
	struct sockaddr_in address;

	if (g_socket != INVALID_SOCKET)
		return 1;

#if PLATFORM == PLATFORM_WINDOWS
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
#endif

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) {
		return 0;
	}

	address.sin_family = AF_INET;
	address.sin_port = port;
	if (ip == 0) {
		address.sin_addr.s_addr = 0;
		if (bind(sock, (struct sockaddr*)&address, sizeof(address)) == -1)
			return 0;
		if (listen(sock, 1) == 0)
			return 0;
		get_socket = get_socket_1;
	} else {
		address.sin_addr.s_addr = inet_pton4(ip);
		if (connect(sock, (struct sockaddr*)&address, sizeof(address)) == -1)
			return 0;
		get_socket = get_socket_0;
	}
	if (setsockopt(sock, IPPROTO_TCP, 1, (const char*)&no_delay, sizeof(no_delay)) == -1)
		return 0;

	g_socket = sock;

	return 1;
}


uint32_t socket_domain_exists(const char* domain_ip, const char* port)
{
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
#endif
	struct addrinfo hints = { 0 }, * addrs;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	const int status = getaddrinfo(domain_ip, port, &hints, &addrs);
	return status == 0;
}
#include <stdio.h>

uint32_t socket_open_dns(const char* domain_ip, const char *port)
{
	int no_delay = 1;

#if PLATFORM == PLATFORM_WINDOWS
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
#endif

	struct addrinfo hints = { 0 }, * addrs;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (g_socket != INVALID_SOCKET)
		return 1;

	const int status = getaddrinfo(domain_ip, port, &hints, &addrs);
	if (status != 0)
		return 0;

	for (struct addrinfo* addr = addrs; addr != NULL; addr = addr->ai_next) {

		SOCKET sock = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
		if (sock == -1)
			continue;

		if (connect(sock, addr->ai_addr, (int)addr->ai_addrlen) == -1) {
			closesocket(sock);
			continue;
		}
		get_socket = get_socket_0;
		g_socket = sock;
		break;
	}
	freeaddrinfo(addrs);

	if (g_socket == INVALID_SOCKET)
		return 0;

	if (setsockopt(g_socket, IPPROTO_TCP, 1, (const char*)&no_delay, sizeof(no_delay)) == -1)
		return 0;
	return 1;
}


void socket_close(void)
{
	if (g_socket != 0) {
#if PLATFORM == PLATFORM_WINDOWS
		closesocket(g_socket);
		WSACleanup();
#else
		close(g_socket);
#endif
		g_socket = INVALID_SOCKET;
	}

	if (g_client != 0) {
#if PLATFORM == PLATFORM_WINDOWS
		closesocket(g_client);
		WSACleanup();
#else
		close(g_client);
#endif
		g_client = INVALID_SOCKET;
	}
}


int socket_recv(void *data, int size)
{
	int len = recv(get_socket(), (char *)data, size, 0);
	RC4((char *)data, len, (unsigned char *)decryption_key);
	return len;
}


int socket_send(void *data, int size)
{
	RC4((char*)data, size, (unsigned char *)decryption_key);
	return send(get_socket(), (const char *)data, size, 0);
}


void socket_open_client(void)
{
	int len = sizeof(struct sockaddr_in);
	struct sockaddr_in address;
	g_client = accept(g_socket, (struct sockaddr *)&address, &len);
}


void socket_close_client(void)
{
#if PLATFORM == PLATFORM_WINDOWS
	closesocket(g_client);
#else
	close(g_client);
#endif
}


static unsigned int inet_pton4(const char* src)
{
	static const char digits[] = "0123456789";
	int saw_digit, octets, ch;
#define NS_INADDRSZ	4
	unsigned char tmp[NS_INADDRSZ], * tp;

	saw_digit = 0;
	octets = 0;
	*(tp = tmp) = 0;
	while ((ch = *src++) != '\0') {
		const char* pch;

		if ((pch = strchr(digits, ch)) != NULL) {
			unsigned int n = *tp * 10 + (int)(pch - digits);

			if (saw_digit && *tp == 0)
				return (0);
			if (n > 255)
				return (0);
			*tp = n;
			if (!saw_digit) {
				if (++octets > 4)
					return (0);
				saw_digit = 1;
			}
		}
		else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return (0);
			*++tp = 0;
			saw_digit = 0;
		}
		else
			return (0);
	}
	if (octets < 4)
		return (0);
	return *(unsigned int*)&tmp;
}


void RC4(void *data, int length, unsigned char *key)
{
	unsigned char T[128];
	unsigned char S[128];
	unsigned char tmp;
	int i, j = 0, x, t = 0;

	for (i = 0; i < 128; i++) {
		S[i]=i;
		T[i]= key[i % 30];
	}
	for(i = 0 ; i < 128; i++) {
		j = ( j + S[i] + T[i] ) % 128;
		tmp = S[j];
		S[j]= S[i];
		S[i] = tmp;
	}
	j = 0;
	for(x = 0 ; x < length; x++) {
		i = (i+1) % 128;
		j = (j + S[i]) % 128;
		tmp = S[j];
		S[j]= S[i];
		S[i] = tmp;
		t = (S[i] + S[j]) % 128;
		((unsigned char *)data)[x] = ((unsigned char *)data)[x]^S[t];
	}
}

