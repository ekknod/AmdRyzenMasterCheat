#include "socket.h"
#include <string.h>

#if PLATFORM == PLATFORM_WINDOWS
#include <winsock2.h>
#pragma comment( lib, "wsock32.lib" )
typedef int64_t sock_t;
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
typedef int32_t sock_t;
#endif


static unsigned int inet_pton4(const char* src);



static sock_t g_socket;
static sock_t g_client;
static sock_t get_socket_0(void) { return g_socket; }
static sock_t get_socket_1(void) { return g_client; }
static sock_t(*get_socket)(void);
char decryption_key[30];

int socket_open(const char* ip, uint16_t port)
{
	int no_delay = 1;
	struct sockaddr_in address;

#if PLATFORM == PLATFORM_WINDOWS
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
#endif

	g_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	address.sin_family = AF_INET;
	address.sin_port = port;
	if (ip == 0) {
		address.sin_addr.s_addr = 0;
		bind(g_socket, (struct sockaddr*) & address, sizeof(address));
		listen(g_socket, 1);
		get_socket = get_socket_1;
	}
	else {
		address.sin_addr.s_addr = inet_pton4(ip);
		connect(g_socket, (struct sockaddr*) & address, sizeof(address));
		get_socket = get_socket_0;
	}
	setsockopt(g_socket, IPPROTO_TCP, 1, (void*)&no_delay, sizeof(no_delay));

	return 1;
}


void socket_close(void)
{
#if PLATFORM == PLATFORM_WINDOWS
	closesocket(g_socket);
	WSACleanup();
#else
	close(g_socket);
#endif
}


int socket_recv(void* data, int size)
{
	int len = recv(get_socket(), data, size, 0);
	RC4((char*)data, len, (unsigned char*)decryption_key);
	return len;
}


int socket_send(void* data, int size)
{
	RC4((char*)data, size, (unsigned char*)decryption_key);
	return send(get_socket(), data, size, 0);
}


void socket_open_client(void)
{
	int len = sizeof(struct sockaddr_in);
	struct sockaddr_in address;
	g_client = accept(g_socket, (struct sockaddr*) & address, &len);
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

void RC4(void* data, int length, unsigned char* key)
{
	unsigned char T[128];
	unsigned char S[128];
	unsigned char tmp;
	int i, j = 0, x, t = 0;

	for (i = 0; i < 128; i++) {
		S[i] = i;
		T[i] = key[i % 30];
	}
	for (i = 0; i < 128; i++) {
		j = (j + S[i] + T[i]) % 128;
		tmp = S[j];
		S[j] = S[i];
		S[i] = tmp;
	}
	j = 0;
	for (x = 0; x < length; x++) {
		i = (i + 1) % 128;
		j = (j + S[i]) % 128;
		tmp = S[j];
		S[j] = S[i];
		S[i] = tmp;
		t = (S[i] + S[j]) % 128;
		((unsigned char*)data)[x] = ((unsigned char*)data)[x] ^ S[t];
	}
}

