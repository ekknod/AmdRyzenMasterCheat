#ifndef SERVER_H
#define SERVER_H

#include <inttypes.h>
#include <string.h>
#include "socket.h"

struct tcp_header {
	uint16_t size;
	uint8_t  retn, status;
} ;

struct tcp_entry {
	uint16_t next;
	uint16_t id;
} ;

struct tcp_entry_address {
    struct tcp_entry ehdr;
//#if INTPTR_MAX == INT32_MAX
//    uint32_t wow64fix;
//#endif
    uint64_t address;
};

struct tcp_entry_device {
    struct tcp_entry entry;
//#if INTPTR_MAX == INT32_MAX
//    uint32_t wow64fix;
//#endif
    uint64_t stack_address;
    char name[120];
};

struct tcp_entry_copy64 {
    struct tcp_entry ehdr;
//#if INTPTR_MAX == INT32_MAX
//    uint32_t wow64fix;
//#endif
    uint64_t dir_entry;
    uint64_t address;
};

struct tcp_entry_copy64p {
    struct tcp_entry ehdr;
    uint16_t prev;
    uint64_t dir_entry;
    uint64_t address;
};

struct tcp_entry_copy {
    struct tcp_entry ehdr;
//#if INTPTR_MAX == INT32_MAX
//    uint32_t wow64fix;
//#endif
    uint64_t dir_entry;
    uint64_t address;
    uint32_t length;
};

struct tcp_entry_copyp {
    struct tcp_entry ehdr;
    uint16_t prev;
    uint64_t dir_entry;
    uint64_t address;
    uint32_t length;
};

struct tcp_entry_mouse {
    struct tcp_entry ehdr;
    int8_t button;
    int8_t x;
    int8_t y;
    int8_t wheel;
    int8_t unk1;
};

#if PLATFORM != PLATFORM_WINDOWS

#define FIELD_OFFSET(type, field)    ((int)(long)&(((type *)0)->field))

#endif

#define STACK_ADD_FUNCTION(e,d) { \
	(e)->id = 1; \
	(e)->next = 4 + sizeof(d); \
	memcpy(((char *)e + 4), d, sizeof(d)); \
	e = (struct tcp_entry *)(e + 1); \
	e = (struct tcp_entry *)((char *)e + sizeof(d)); \
}

/* we have to use this, if we dont have anything else to call in stack */
#define STACK_EMPTY_FUNCTION(e,n,i) { \
	(e)->id = (i); \
	(e)->next = (n); \
	e = (struct tcp_entry *)(e + 1); \
}

#define STACK_END_FUNCTION STACK_EMPTY_FUNCTION
#define STACK_GET_HEAP STACK_EMPTY_FUNCTION
#define STACK_ADD_SYSTEM_PROCESS STACK_EMPTY_FUNCTION
#define STACK_ADD_PML4 STACK_EMPTY_FUNCTION

#define STACK_CLEAR_FUNCTIONS(e, n) { \
	(e)->id = (2); \
	(e)->next = n; \
	e = (struct tcp_entry *)(e + 1); \
}

#define STACK_ADD_ADDRESS(e,i,n,m,x) { \
	(e)->id = (i); \
	(e)->next = (n); \
	((uint32_t*)&(((struct tcp_entry_address*)e))->address)[0] = (m); \
	((uint32_t*)&(((struct tcp_entry_address*)e))->address)[1] = (x); \
	e = (struct tcp_entry *)((struct tcp_entry_address*)e + 1); \
}

#define STACK_GET_ADDRESS(e,i,n,f) { \
	(e)->id = (i); \
	(e)->next = (n); \
	(((struct tcp_entry_address*)e))->address = (f); \
	e = (struct tcp_entry *)((struct tcp_entry_address*)e + 1); \
}

#define STACK_ADD_DEVICE(e,i,n,a,b) { \
	(e)->id = (i); \
	(e)->next = (n); \
	((struct tcp_entry_device *)e)->stack_address = (a); \
	memcpy(((struct tcp_entry_device *)e)->name, b, sizeof(b)); \
	e = (struct tcp_entry *)((struct tcp_entry_device*)e + 1); \
}

#define STACK_ADD_VM_COPY(e,n,a,d,l) { \
	(e)->id = (7); \
	(e)->next = (n); \
	((struct tcp_entry_copy *)e)->address = (a); \
	((struct tcp_entry_copy *)e)->length = (l); \
	((struct tcp_entry_copy *)e)->dir_entry = (d); \
	e = (struct tcp_entry *)((struct tcp_entry_copy*)e + 1); \
	e = (struct tcp_entry *)((char *)e + l); \
}

#define STACK_ADD_VM_COPY_P32(e,n,p,a,d,l) { \
	(e)->id = (8); \
	(e)->next = (n); \
	((struct tcp_entry_copyp*)e)->prev = (p); \
	((struct tcp_entry_copyp*)e)->address = (a); \
	((struct tcp_entry_copyp*)e)->length = (l); \
	((struct tcp_entry_copyp*)e)->dir_entry = (d); \
	e = (struct tcp_entry *)((struct tcp_entry_copyp*)e + 1); \
}

#define STACK_ADD_VM_COPY_P64(e,n,p,a,d,l) { \
	(e)->id = (9); \
	(e)->next = (n); \
	((struct tcp_entry_copyp*)e)->prev = (p); \
	((struct tcp_entry_copyp*)e)->address = (a); \
	((struct tcp_entry_copyp*)e)->length = (l); \
	((struct tcp_entry_copyp*)e)->dir_entry = (d); \
	e = (struct tcp_entry *)((struct tcp_entry_copyp*)e + 1); \
}

#define STACK_ADD_VM_COPY64(e,n,a,d) { \
	(e)->id = (10); \
	(e)->next = (n); \
	((struct tcp_entry_copy64 *)e)->address = (a); \
	((struct tcp_entry_copy64 *)e)->dir_entry = (d); \
	e = (struct tcp_entry *)((struct tcp_entry_copy64*)e + 1); \
}

#define STACK_ADD_VM_COPY64_P32(e,n,p,a,d) { \
	(e)->id = (11); \
	(e)->next = (n); \
	((struct tcp_entry_copy64p *)e)->prev = (p); \
	((struct tcp_entry_copy64p *)e)->address = (a); \
	((struct tcp_entry_copy64p *)e)->dir_entry = (d); \
	e = (struct tcp_entry *)((struct tcp_entry_copy64p*)e + 1); \
}

#define STACK_ADD_VM_COPY64_P64(e,n,p,a,d) { \
	(e)->id = (12); \
	(e)->next = (n); \
	((struct tcp_entry_copy64p *)e)->prev = (p); \
	((struct tcp_entry_copy64p *)e)->address = (a); \
	((struct tcp_entry_copy64p *)e)->dir_entry = (d); \
	e = (struct tcp_entry *)((struct tcp_entry_copy64p*)e + 1); \
}

#define STACK_ADD_MOUSE(e,n,b,x,y,w) { \
	(e)->id = (13); \
	(e)->next = (n); \
	((struct tcp_entry_mouse*)e)->button = (b); \
	((struct tcp_entry_mouse*)e)->x = (x); \
	((struct tcp_entry_mouse*)e)->y = (y); \
	((struct tcp_entry_mouse*)e)->wheel = (w); \
	((struct tcp_entry_mouse*)e)->unk1 = (0); \
	e = (struct tcp_entry *)((struct tcp_entry_mouse*)e + 1); \
}

#define STACK_ADD_GPA STACK_ADD_ADDRESS

#define STACK_EXECUTE(h, e) { \
	(h)->size = 4; \
	(h)->retn = 1; \
	(h)->status = 1; \
	socket_send(h, (int)((char *)e - (char*)h)); \
	socket_recv(h, 1400); \
	e = (struct tcp_entry*)(h + 1); \
}

#define STACK_EXECUTE_NO_RETURN(h, e) { \
	(h)->size = 4; \
	(h)->retn = 0; \
	socket_send(h, (int)((char *)e - (char*)h)); \
	e = (struct tcp_entry*)(h + 1); \
}


#define STACK_ADD_VIRTUAL_FUNCTION(e, v, i) { \
	STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), v, g_process_pml4); \
	STACK_ADD_VM_COPY64_P32(e, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), i * 4, g_process_pml4); \
}

uint32_t server_initialize(void);
uint32_t server_move_mouse(int8_t button, int8_t x, int8_t y, int8_t wheel);

extern uint64_t g_system_process;
extern uint64_t g_system_pml4;

#endif

