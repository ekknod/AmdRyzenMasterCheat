#include <jni.h>
#include <string>
#include <ctime>
#include <sys/time.h>
#include <sys/resource.h>
#include <cstdint>
#include <android/log.h>
#include <unistd.h>

#include "socket.h"
#include "maths.h"

#define LOG_TAG "dbg"
#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

static double get_time()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
}

void NtSleep(int ms)
{
    usleep(ms*1000);
}


int NtRand()
{
    struct timespec info;
    clock_gettime(4, &info);
    srand(info.tv_nsec);
    return rand();
}


int check_player_pos(const char* buf, int len, int init);
int is_player_valid;
static uint64_t g_system_pml4;
static uint64_t g_system_process;
static uint64_t g_process_pml4;
static uint64_t g_process;
static uint32_t g_client_dll;
static uint32_t g_engine_dll;
static uint32_t g_vstdlib_dll;
static uint32_t g_inputsystem_dll;
static uint32_t vt_client;
static uint32_t vt_entity;
static uint32_t vt_engine;
static uint32_t vt_cvar;
static uint32_t vt_input;
static uint32_t DT_BasePlayer;
static uint32_t DT_BaseEntity;
static uint32_t DT_CSPlayer;
static uint32_t DT_BaseAnimating;
static uint32_t m_vecViewOffset;
static uint32_t m_iHealth;
static uint32_t m_nTickBase;
static uint32_t m_lifeState;
static uint32_t m_vecPunch;
static uint32_t m_vecOrigin;
static uint32_t m_iTeamNum;
static uint32_t m_hActiveWeapon;
static uint32_t m_iCrossHairID;
static uint32_t m_bHasDefuser;
static uint32_t m_bIsDefusing;
static uint32_t m_flFlashDuration;
static uint32_t m_iShotsFired;
static uint32_t m_dwBoneMatrix;
static uint32_t m_dwEntityList;
static uint32_t m_dwClientState;
static uint32_t m_dwGetLocalPlayer;
static uint32_t m_dwGetViewAngles;
static uint32_t m_dwGetMaxClients;
static uint32_t m_dwState;
static uint32_t m_bDormant = 0xED;
static uint32_t m_dwButton;
static uint32_t m_dwAnalog;
static uint32_t m_dwAnalogDelta;
static uint32_t g_local_index;
static uint32_t g_local_address;
static uint32_t g_target_address;
static uint32_t g_target_id;
static uint32_t g_mouse_1, g_mouse_5;
static vec2i    g_mouse_delta;
static vec2i    g_mouse_analog;
static int      g_crosshair_id;
static vec3     g_eyepos;
static vec3     g_vecpunch;
static int      g_shots_fired;
static int      g_current_tick, g_previous_tick;
static vec3     g_viewangles;
static int      g_target_health;
static vec3     g_target_bone;
static float    g_best_fov;
static int      g_has_target;
static bool     g_is_visible;
static int      g_valid_target_count;
static float    g_positions[40];
static int      g_defusing;
static int      esea;
static int      m_dwGetPlayerInfo;


#define JUHO_BUILD
#define JUHO_OLD_GHUB



struct tcp_entry_address {
    struct tcp_entry ehdr;
#if INTPTR_MAX == INT32_MAX
    uint32_t wow64fix;
#endif
    uint64_t address;
};

struct tcp_entry_device {
    struct tcp_entry entry;
#if INTPTR_MAX == INT32_MAX
    uint32_t wow64fix;
#endif
    uint64_t stack_address;
    char name[120];
};

struct tcp_entry_copy64 {
    struct tcp_entry ehdr;
#if INTPTR_MAX == INT32_MAX
    uint32_t wow64fix;
#endif
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
#if INTPTR_MAX == INT32_MAX
    uint32_t wow64fix;
#endif
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
    char button;
	char x;
	char y;
	char wheel;
	char unk1;
};

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

static char impl_upper(char c)
{
    if (c >= 97)
        c -= 32;
    return c;
}

static int impl_strcmp(const char *rcx, const char *rdx)
{
    int eax = 0;
    char r8b;
    while (1) {
        r8b = impl_upper(*(char*)(rcx + eax));
        if (r8b != impl_upper(*(char*)(rdx + eax)))
            break;
        if (r8b == 0)
            break;
        eax = eax + 1;
    }
    r8b -= *(char*)(rdx + eax);
    eax = r8b;
    return eax;
}

static int prev_tick = 0;

/* we dont have wchar_t in linux, we have to implement it */
static int impl_wcscmp(const char *rcx, const char *rdx)
{
    int eax = 0;
    char r8b;
    while (1) {
        r8b = impl_upper(*(char*)(rcx));
        if (r8b != impl_upper(*(char*)(rdx + eax)))
            break;
        if (r8b == 0)
            break;
        rcx = rcx + 1;
        eax = eax + 2;
    }
    r8b -= *(char*)(rdx + eax);
    eax = r8b;
    return eax;
}


uint32_t device_initialize(void)
{
    unsigned char b_get_proc_address[] = {
            0x65, 0x48, 0x8B, 0x04, 0x25, 0x60, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xD1, 0x4C, 0x8B, 0x40, 0x18, 0x8B, 0x42, 0x08, 0x4D,
            0x8B, 0x48, 0x20, 0x85, 0xC0, 0x74, 0x0E, 0x0F, 0x1F, 0x44, 0x00, 0x00, 0x4D, 0x8B, 0x09, 0x48, 0x83, 0xE8, 0x01, 0x75,
            0xF7, 0x4D, 0x8B, 0x49, 0x20, 0x41, 0x0F, 0xB7, 0x41, 0x3C, 0x42, 0x8B, 0x8C, 0x08, 0x88, 0x00, 0x00, 0x00, 0x8B, 0x42,
            0x0C, 0x42, 0x2B, 0x44, 0x09, 0x10, 0x42, 0x8B, 0x4C, 0x09, 0x1C, 0x49, 0x03, 0xC9, 0x44, 0x8B, 0x04, 0x81, 0x41, 0x0F,
            0xB7, 0x02, 0x4D, 0x03, 0xC1, 0x4E, 0x89, 0x04, 0x10, 0x66, 0x41, 0x83, 0x02, 0x08, 0xC3
    } ;
    unsigned char b_set_system_process[] = {
            0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x41, 0x54, 0x41, 0x55,
            0x41, 0x56, 0x41, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0xBD, 0x20, 0x6A, 0x56, 0x74, 0xFE, 0x7F, 0x00, 0x00, 0x48, 0x8B,
            0xF2, 0x48, 0x8B, 0xF9, 0xFF, 0xD5, 0x33, 0xD2, 0x48, 0x8B, 0xC8, 0x49, 0xBC, 0xC0, 0xDD, 0xA5, 0x74, 0xFE, 0x7F, 0x00,
            0x00, 0x44, 0x8D, 0x42, 0x20, 0x41, 0xFF, 0xD4, 0x41, 0xB8, 0x20, 0x00, 0x00, 0x00, 0x49, 0xBF, 0xF0, 0x80, 0xAD, 0x71,
            0xFE, 0x7F, 0x00, 0x00, 0x4D, 0x8B, 0xCF, 0x48, 0x8B, 0xD0, 0x49, 0xBD, 0xF0, 0x60, 0xAD, 0x74, 0xFE, 0x7F, 0x00, 0x00,
            0x48, 0x8B, 0xD8, 0x41, 0x8D, 0x48, 0xF0, 0x41, 0xFF, 0xD5, 0x49, 0xBE, 0xF0, 0x59, 0x56, 0x74, 0xFE, 0x7F, 0x00, 0x00,
            0x3D, 0x04, 0x00, 0x00, 0xC0, 0x75, 0x6C, 0xA1, 0xF0, 0x80, 0xAD, 0x71, 0xFE, 0x7F, 0x00, 0x00, 0x05, 0x00, 0x20, 0x00,
            0x00, 0xA3, 0xF0, 0x80, 0xAD, 0x71, 0xFE, 0x7F, 0x00, 0x00, 0xFF, 0xD5, 0x48, 0x8B, 0xC8, 0x4C, 0x8B, 0xC3, 0x33, 0xD2,
            0x41, 0xFF, 0xD6, 0x41, 0x8B, 0x1F, 0xFF, 0xD5, 0x48, 0x8B, 0xC8, 0x44, 0x8B, 0xC3, 0x33, 0xD2, 0x41, 0xFF, 0xD4, 0x45,
            0x8B, 0x07, 0x45, 0x33, 0xC9, 0x48, 0x8B, 0xD0, 0x48, 0x8B, 0xD8, 0x41, 0x8D, 0x49, 0x10, 0x41, 0xFF, 0xD5, 0x85, 0xC0,
            0x75, 0x21, 0x48, 0x8B, 0x43, 0x10, 0x48, 0xA3, 0xF0, 0x90, 0xAD, 0x71, 0xFE, 0x7F, 0x00, 0x00, 0xFF, 0xD5, 0x48, 0x8B,
            0xC8, 0x4C, 0x8B, 0xC3, 0x33, 0xD2, 0x41, 0xFF, 0xD6, 0x66, 0x83, 0x07, 0x08, 0xEB, 0x16, 0xFF, 0xD5, 0x48, 0x8B, 0xC8,
            0x4C, 0x8B, 0xC3, 0x33, 0xD2, 0x41, 0xFF, 0xD6, 0x33, 0xC0, 0xC6, 0x47, 0x03, 0x00, 0x66, 0x89, 0x06, 0x48, 0x8B, 0x5C,
            0x24, 0x50, 0x48, 0x8B, 0x6C, 0x24, 0x58, 0x48, 0x8B, 0x74, 0x24, 0x60, 0x48, 0x83, 0xC4, 0x20, 0x41, 0x5F, 0x41, 0x5E,
            0x41, 0x5D, 0x41, 0x5C, 0x5F, 0xC3
    } ;
    unsigned char b_open_device[] = {
            0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20,
            0x41, 0x54, 0x41, 0x56, 0x41, 0x57, 0x48, 0x83, 0xEC, 0x60, 0x66, 0x83, 0x7A, 0x10, 0x00, 0x48, 0x8D, 0x6A, 0x10, 0x4C,
            0x8B, 0xFA, 0x4C, 0x8B, 0xF1, 0x48, 0x8B, 0xDD, 0x74, 0x0A, 0x48, 0x83, 0xC3, 0x02, 0x66, 0x83, 0x3B, 0x00, 0x75, 0xF6,
            0x49, 0xBC, 0x20, 0x6A, 0x56, 0x74, 0xFE, 0x7F, 0x00, 0x00, 0x41, 0xFF, 0xD4, 0x33, 0xD2, 0x48, 0x8B, 0xC8, 0x48, 0xBF,
            0xC0, 0xDD, 0xA5, 0x74, 0xFE, 0x7F, 0x00, 0x00, 0x44, 0x8D, 0x42, 0x10, 0xFF, 0xD7, 0x48, 0x8B, 0xF0, 0x41, 0xFF, 0xD4,
            0x33, 0xD2, 0x48, 0x8B, 0xC8, 0x44, 0x8D, 0x42, 0x30, 0xFF, 0xD7, 0x48, 0x89, 0x6E, 0x08, 0x49, 0x2B, 0xDF, 0x33, 0xED,
            0x48, 0x83, 0xEB, 0x10, 0x89, 0x6C, 0x24, 0x50, 0x48, 0x8B, 0xF8, 0x48, 0x89, 0x6C, 0x24, 0x48, 0x4C, 0x8B, 0xC8, 0xC7,
            0x44, 0x24, 0x40, 0x60, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xC0, 0xC7, 0x44, 0x24, 0x38, 0x03, 0x00, 0x00, 0x00, 0xBA, 0x00,
            0x00, 0x10, 0xC0, 0x48, 0xD1, 0xFB, 0x66, 0x03, 0xDB, 0x89, 0x6C, 0x24, 0x30, 0x66, 0x89, 0x1E, 0x66, 0x83, 0xC3, 0x02,
            0x66, 0x89, 0x5E, 0x02, 0xC7, 0x00, 0x30, 0x00, 0x00, 0x00, 0x48, 0x89, 0x68, 0x08, 0x89, 0x68, 0x18, 0x48, 0x89, 0x70,
            0x10, 0x48, 0x89, 0x68, 0x20, 0x48, 0x89, 0x68, 0x28, 0x48, 0xB8, 0xD0, 0x64, 0x9C, 0x64, 0xF8, 0x7F, 0x00, 0x00, 0x49,
            0x8B, 0x4F, 0x08, 0xC7, 0x44, 0x24, 0x28, 0x80, 0x00, 0x00, 0x00, 0x48, 0x89, 0x6C, 0x24, 0x20, 0xFF, 0xD0, 0x8B, 0xD8,
            0x41, 0xFF, 0xD4, 0x4C, 0x8B, 0xC6, 0x48, 0x8B, 0xC8, 0x48, 0xBE, 0xF0, 0x59, 0x56, 0x74, 0xFE, 0x7F, 0x00, 0x00, 0x33,
            0xD2, 0xFF, 0xD6, 0x41, 0xFF, 0xD4, 0x48, 0x8B, 0xC8, 0x4C, 0x8B, 0xC7, 0x33, 0xD2, 0xFF, 0xD6, 0x85, 0xDB, 0x74, 0x0B,
            0x41, 0xC7, 0x06, 0x04, 0x00, 0x01, 0x00, 0x66, 0x41, 0x89, 0x2F, 0x4C, 0x8D, 0x5C, 0x24, 0x60, 0x49, 0x8B, 0x5B, 0x20,
            0x49, 0x8B, 0x6B, 0x28, 0x49, 0x8B, 0x73, 0x30, 0x49, 0x8B, 0x7B, 0x38, 0x49, 0x8B, 0xE3, 0x41, 0x5F, 0x41, 0x5E, 0x41,
            0x5C, 0xC3
    } ;


/*

    unsigned char b_amd[] = {
		0x5C, 0x00, 0x44, 0x00, 0x65, 0x00, 0x76, 0x00, 0x69, 0x00, 0x63, 0x00, 0x65, 0x00, 0x5C, 0x00, 0x41, 0x00, 0x4D, 0x00,
		0x44, 0x00, 0x52, 0x00, 0x79, 0x00, 0x7A, 0x00, 0x65, 0x00, 0x6E, 0x00, 0x4D, 0x00, 0x61, 0x00, 0x73, 0x00, 0x74, 0x00,
		0x65, 0x00, 0x72, 0x00, 0x44, 0x00, 0x72, 0x00, 0x69, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x56, 0x00, 0x31, 0x00,
		0x33, 0x00, 0x00, 0x00
	} ;

*/

    unsigned char b_amd[] = {
            0x5C, 0x00, 0x44, 0x00, 0x65, 0x00, 0x76, 0x00, 0x69, 0x00, 0x63, 0x00, 0x65, 0x00, 0x5C, 0x00, 0x41, 0x00, 0x4D, 0x00,
            0x44, 0x00, 0x52, 0x00, 0x79, 0x00, 0x7A, 0x00, 0x65, 0x00, 0x6E, 0x00, 0x4D, 0x00, 0x61, 0x00, 0x73, 0x00, 0x74, 0x00,
            0x65, 0x00, 0x72, 0x00, 0x44, 0x00, 0x72, 0x00, 0x69, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x56, 0x00, 0x31, 0x00,
            0x36, 0x00, 0x00, 0x00
    } ;




    unsigned char b_logitech[] = {
            0x5C, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x5C, 0x00, 0x52, 0x00, 0x4F, 0x00, 0x4F, 0x00, 0x54, 0x00, 0x23, 0x00, 0x53, 0x00,
            0x59, 0x00, 0x53, 0x00, 0x54, 0x00, 0x45, 0x00, 0x4D, 0x00, 0x23, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x31, 0x00,
            0x23, 0x00, 0x7B, 0x00, 0x31, 0x00, 0x61, 0x00, 0x62, 0x00, 0x63, 0x00, 0x30, 0x00, 0x35, 0x00, 0x63, 0x00, 0x30, 0x00,
            0x2D, 0x00, 0x63, 0x00, 0x33, 0x00, 0x37, 0x00, 0x38, 0x00, 0x2D, 0x00, 0x34, 0x00, 0x31, 0x00, 0x62, 0x00, 0x39, 0x00,
            0x2D, 0x00, 0x39, 0x00, 0x63, 0x00, 0x65, 0x00, 0x66, 0x00, 0x2D, 0x00, 0x64, 0x00, 0x66, 0x00, 0x31, 0x00, 0x61, 0x00,
            0x62, 0x00, 0x61, 0x00, 0x38, 0x00, 0x32, 0x00, 0x62, 0x00, 0x30, 0x00, 0x31, 0x00, 0x35, 0x00, 0x7D, 0x00, 0x00, 0x00
    } ;




    unsigned char b_copy_pml4[] = {
            0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x50, 0x48, 0xBF, 0xC9, 0xDD, 0xA9,
            0x79, 0xFE, 0x7F, 0x00, 0x00, 0x48, 0xBE, 0x20, 0x6A, 0x56, 0x74, 0xFE, 0x7F, 0x00, 0x00, 0x48, 0x8B, 0x3F, 0xFF, 0xD6,
            0x33, 0xD2, 0x48, 0x8B, 0xC8, 0x48, 0xB8, 0xC0, 0xDD, 0xA5, 0x74, 0xFE, 0x7F, 0x00, 0x00, 0x44, 0x8D, 0x42, 0x14, 0xFF,
            0xD0, 0xC7, 0x44, 0x24, 0x48, 0x14, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xD8, 0x48, 0x89, 0x44, 0x24, 0x40, 0x45, 0x33, 0xC9,
            0xC7, 0x44, 0x24, 0x38, 0x0C, 0x00, 0x00, 0x00, 0x45, 0x33, 0xC0, 0x48, 0x89, 0x44, 0x24, 0x30, 0x33, 0xD2, 0x48, 0xC7,
            0x00, 0xA0, 0x10, 0x00, 0x00, 0x48, 0x8B, 0xCF, 0xC7, 0x40, 0x08, 0x08, 0x00, 0x00, 0x00, 0x48, 0xB8, 0xC8, 0xDD, 0xA9,
            0x79, 0xFE, 0x7F, 0x00, 0x00, 0xC7, 0x44, 0x24, 0x28, 0x08, 0x2F, 0x11, 0x81, 0x48, 0x89, 0x44, 0x24, 0x20, 0x48, 0xB8,
            0x10, 0x5B, 0x9C, 0x64, 0xF8, 0x7F, 0x00, 0x00, 0xFF, 0xD0, 0x48, 0x8B, 0x43, 0x0C, 0x48, 0xA3, 0xC7, 0xDD, 0xA9, 0x79,
            0xFE, 0x7F, 0x00, 0x00, 0xFF, 0xD6, 0x48, 0x8B, 0xC8, 0x4C, 0x8B, 0xC3, 0x48, 0xB8, 0xF0, 0x59, 0x56, 0x74, 0xFE, 0x7F,
            0x00, 0x00, 0x33, 0xD2, 0x48, 0x8B, 0x5C, 0x24, 0x60, 0x48, 0x8B, 0x74, 0x24, 0x68, 0x48, 0x83, 0xC4, 0x50, 0x5F, 0x48,
            0xFF, 0xE0
    } ;
    unsigned char b_copy_memory[] = {
            0x48, 0x83, 0xEC, 0x58, 0x48, 0x8B, 0xC1, 0x48, 0xB9, 0x00, 0x00, 0x38, 0x1F, 0x04, 0x00, 0x00, 0x00, 0x48, 0x3B, 0xC1,
            0x0F, 0x87, 0x83, 0x00, 0x00, 0x00, 0x49, 0xB8, 0xD3, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x48, 0xB9, 0xC9, 0xDD,
            0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x45, 0x33, 0xC9, 0x48, 0x8B, 0x09, 0x48, 0xA3, 0xC7, 0xDD, 0xA9, 0x79, 0xFE, 0x7F,
            0x00, 0x00, 0x8B, 0xC2, 0xA3, 0xCF, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x8D, 0x42, 0x0C, 0x89, 0x44, 0x24, 0x48,
            0x48, 0xB8, 0xC7, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x48, 0x89, 0x44, 0x24, 0x40, 0x8B, 0xD2, 0x49, 0x03, 0xD0,
            0xC7, 0x44, 0x24, 0x38, 0x0C, 0x00, 0x00, 0x00, 0x48, 0x89, 0x44, 0x24, 0x30, 0x45, 0x33, 0xC0, 0xC7, 0x44, 0x24, 0x28,
            0x08, 0x2F, 0x11, 0x81, 0x48, 0xB8, 0x10, 0x5B, 0x9C, 0x64, 0xF8, 0x7F, 0x00, 0x00, 0x48, 0x89, 0x54, 0x24, 0x20, 0x33,
            0xD2, 0xFF, 0xD0, 0x33, 0xC9, 0x85, 0xC0, 0x0F, 0x94, 0xC1, 0x8B, 0xC1, 0x48, 0x83, 0xC4, 0x58, 0xC3, 0x33, 0xC0, 0x48,
            0x83, 0xC4, 0x58, 0xC3
    } ;
    unsigned char b_translate[] = {
            0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20,
            0x48, 0x8B, 0xC2, 0x48, 0x8B, 0xDA, 0x48, 0xC1, 0xE8, 0x27, 0xBA, 0x08, 0x00, 0x00, 0x00, 0x25, 0xFF, 0x01, 0x00, 0x00,
            0x48, 0xBD, 0xC8, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x48, 0x8D, 0x0C, 0xC1, 0xFF, 0xD5, 0x85, 0xC0, 0x0F, 0x84,
            0xC5, 0x00, 0x00, 0x00, 0x48, 0xBF, 0xD3, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x48, 0x8B, 0xCB, 0x48, 0xC1, 0xE9,
            0x1E, 0x48, 0xBE, 0x00, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x81, 0xE1, 0xFF, 0x01, 0x00, 0x00, 0xBA, 0x08, 0x00,
            0x00, 0x00, 0x48, 0x8B, 0x07, 0x48, 0x23, 0xC6, 0x48, 0x8D, 0x0C, 0xC8, 0xFF, 0xD5, 0x85, 0xC0, 0x0F, 0x84, 0x8B, 0x00,
            0x00, 0x00, 0x48, 0x8B, 0x0F, 0x84, 0xC9, 0x79, 0x18, 0x48, 0xB8, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0x0F, 0x00, 0x81,
            0xE3, 0xFF, 0xFF, 0xFF, 0x3F, 0x48, 0x23, 0xC1, 0x48, 0x03, 0xC3, 0xEB, 0x6E, 0x48, 0x23, 0xCE, 0x48, 0x8B, 0xC3, 0x48,
            0xC1, 0xE8, 0x15, 0xBA, 0x08, 0x00, 0x00, 0x00, 0x25, 0xFF, 0x01, 0x00, 0x00, 0x48, 0x8D, 0x0C, 0xC1, 0xFF, 0xD5, 0x85,
            0xC0, 0x74, 0x4E, 0x48, 0x8B, 0x0F, 0x84, 0xC9, 0x79, 0x18, 0x48, 0xB8, 0x00, 0x00, 0xE0, 0xFF, 0xFF, 0xFF, 0x0F, 0x00,
            0x81, 0xE3, 0xFF, 0xFF, 0x1F, 0x00, 0x48, 0x23, 0xC1, 0x48, 0x03, 0xC3, 0xEB, 0x31, 0x48, 0x23, 0xCE, 0x48, 0x8B, 0xC3,
            0x48, 0xC1, 0xE8, 0x0C, 0xBA, 0x08, 0x00, 0x00, 0x00, 0x25, 0xFF, 0x01, 0x00, 0x00, 0x48, 0x8D, 0x0C, 0xC1, 0xFF, 0xD5,
            0x85, 0xC0, 0x74, 0x11, 0x48, 0x8B, 0x07, 0x81, 0xE3, 0xFF, 0x0F, 0x00, 0x00, 0x48, 0x23, 0xC6, 0x48, 0x03, 0xC3, 0xEB,
            0x02, 0x33, 0xC0, 0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48, 0x8B, 0x6C, 0x24, 0x38, 0x48, 0x8B, 0x74, 0x24, 0x40, 0x48, 0x83,
            0xC4, 0x20, 0x5F, 0xC3
    } ;
    unsigned char b_vmcopy[] = {
            0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xFA, 0x48, 0x8B, 0xD9, 0x48, 0x8B, 0x52, 0x10,
            0x48, 0xB8, 0xC7, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x48, 0x8B, 0x4F, 0x08, 0xFF, 0xD0, 0x8B, 0x57, 0x18, 0x48,
            0x8B, 0xC8, 0x48, 0xB8, 0xC8, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0xFF, 0xD0, 0x0F, 0xB7, 0x0B, 0x48, 0xBA, 0xC6,
            0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x88, 0x43, 0x03, 0x48, 0x03, 0xCB, 0x44, 0x8B, 0x47, 0x18, 0x48, 0xB8, 0xC5,
            0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0xFF, 0xD0, 0x0F, 0xB7, 0x47, 0x18, 0x33, 0xD2, 0x66, 0x01, 0x03, 0x48, 0xB9,
            0xC1, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x44, 0x8B, 0x47, 0x18, 0x48, 0xB8, 0xC0, 0xC2, 0x9C, 0x64, 0xF8, 0x7F,
            0x00, 0x00, 0x41, 0x83, 0xC0, 0x0C, 0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48, 0x83, 0xC4, 0x20, 0x5F, 0x48, 0xFF, 0xE0
    } ;
    unsigned char b_vmcopy_p32[] = {
            0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x0F, 0xB7, 0x42, 0x04, 0x48, 0x8B, 0xFA, 0x44, 0x0F, 0xB7,
            0x01, 0x48, 0x8B, 0xD9, 0x4C, 0x2B, 0xC0, 0x41, 0x8B, 0x04, 0x08, 0x48, 0x01, 0x42, 0x10, 0x48, 0xB8, 0xC7, 0xDD, 0xA9,
            0x79, 0xFE, 0x7F, 0x00, 0x00, 0x48, 0x8B, 0x52, 0x10, 0x48, 0x8B, 0x4F, 0x08, 0xFF, 0xD0, 0x8B, 0x57, 0x18, 0x48, 0x8B,
            0xC8, 0x48, 0xB8, 0xC8, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0xFF, 0xD0, 0x0F, 0xB7, 0x0B, 0x48, 0xBA, 0xC6, 0xDD,
            0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x88, 0x43, 0x03, 0x48, 0x03, 0xCB, 0x44, 0x8B, 0x47, 0x18, 0x48, 0xB8, 0xC5, 0xDD,
            0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0xFF, 0xD0, 0x0F, 0xB7, 0x47, 0x18, 0x33, 0xD2, 0x66, 0x01, 0x03, 0x48, 0xB9, 0xC1,
            0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x44, 0x8B, 0x47, 0x18, 0x48, 0xB8, 0xC0, 0xC2, 0x9C, 0x64, 0xF8, 0x7F, 0x00,
            0x00, 0x41, 0x83, 0xC0, 0x0C, 0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48, 0x83, 0xC4, 0x20, 0x5F, 0x48, 0xFF, 0xE0
    } ;
    unsigned char b_vmcopy_p64[] = {
            0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x0F, 0xB7, 0x42, 0x04, 0x48, 0x8B, 0xFA, 0x44, 0x0F, 0xB7,
            0x01, 0x48, 0x8B, 0xD9, 0x4C, 0x2B, 0xC0, 0x49, 0x8B, 0x04, 0x08, 0x48, 0x01, 0x42, 0x10, 0x48, 0xB8, 0xC7, 0xDD, 0xA9,
            0x79, 0xFE, 0x7F, 0x00, 0x00, 0x48, 0x8B, 0x52, 0x10, 0x48, 0x8B, 0x4F, 0x08, 0xFF, 0xD0, 0x8B, 0x57, 0x18, 0x48, 0x8B,
            0xC8, 0x48, 0xB8, 0xC8, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0xFF, 0xD0, 0x0F, 0xB7, 0x0B, 0x48, 0xBA, 0xC6, 0xDD,
            0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x88, 0x43, 0x03, 0x48, 0x03, 0xCB, 0x44, 0x8B, 0x47, 0x18, 0x48, 0xB8, 0xC5, 0xDD,
            0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0xFF, 0xD0, 0x0F, 0xB7, 0x47, 0x18, 0x33, 0xD2, 0x66, 0x01, 0x03, 0x48, 0xB9, 0xC1,
            0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x44, 0x8B, 0x47, 0x18, 0x48, 0xB8, 0xC0, 0xC2, 0x9C, 0x64, 0xF8, 0x7F, 0x00,
            0x00, 0x41, 0x83, 0xC0, 0x0C, 0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48, 0x83, 0xC4, 0x20, 0x5F, 0x48, 0xFF, 0xE0
    } ;
    unsigned char b_vmcopy64[] = {
            0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xC2, 0x48, 0x8B, 0xD9, 0x48, 0x8B, 0x52, 0x10, 0x48, 0x8B, 0x48, 0x08,
            0x48, 0xB8, 0xC7, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0xFF, 0xD0, 0x48, 0x8B, 0xC8, 0xBA, 0x08, 0x00, 0x00, 0x00,
            0x48, 0xB8, 0xC8, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0xFF, 0xD0, 0x0F, 0xB7, 0x0B, 0x48, 0xBA, 0xC6, 0xDD, 0xA9,
            0x79, 0xFE, 0x7F, 0x00, 0x00, 0x88, 0x43, 0x03, 0x48, 0x03, 0xCB, 0x48, 0xB8, 0xC5, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00,
            0x00, 0x41, 0xB8, 0x08, 0x00, 0x00, 0x00, 0xFF, 0xD0, 0x66, 0x83, 0x03, 0x08, 0x33, 0xD2, 0x48, 0xB9, 0xC1, 0xDD, 0xA9,
            0x79, 0xFE, 0x7F, 0x00, 0x00, 0x48, 0xB8, 0xC0, 0xC2, 0x9C, 0x64, 0xF8, 0x7F, 0x00, 0x00, 0x44, 0x8D, 0x42, 0x14, 0x48,
            0x83, 0xC4, 0x20, 0x5B, 0x48, 0xFF, 0xE0
    } ;
    unsigned char b_vmcopy64_p32[] = {
            0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x0F, 0xB7, 0x42, 0x04, 0x4C, 0x8B, 0xCA, 0x44, 0x0F, 0xB7, 0x01, 0x48, 0x8B, 0xD9,
            0x4C, 0x2B, 0xC0, 0x41, 0x8B, 0x04, 0x08, 0x48, 0x01, 0x42, 0x10, 0x48, 0xB8, 0xC7, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00,
            0x00, 0x48, 0x8B, 0x52, 0x10, 0x49, 0x8B, 0x49, 0x08, 0xFF, 0xD0, 0x48, 0x8B, 0xC8, 0xBA, 0x08, 0x00, 0x00, 0x00, 0x48,
            0xB8, 0xC8, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0xFF, 0xD0, 0x0F, 0xB7, 0x0B, 0x48, 0xBA, 0xC6, 0xDD, 0xA9, 0x79,
            0xFE, 0x7F, 0x00, 0x00, 0x88, 0x43, 0x03, 0x48, 0x03, 0xCB, 0x48, 0xB8, 0xC5, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00,
            0x41, 0xB8, 0x08, 0x00, 0x00, 0x00, 0xFF, 0xD0, 0x66, 0x83, 0x03, 0x08, 0x33, 0xD2, 0x48, 0xB9, 0xC1, 0xDD, 0xA9, 0x79,
            0xFE, 0x7F, 0x00, 0x00, 0x48, 0xB8, 0xC0, 0xC2, 0x9C, 0x64, 0xF8, 0x7F, 0x00, 0x00, 0x44, 0x8D, 0x42, 0x14, 0x48, 0x83,
            0xC4, 0x20, 0x5B, 0x48, 0xFF, 0xE0
    } ;
    unsigned char b_vmcopy64_p64[] = {
            0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x0F, 0xB7, 0x42, 0x04, 0x4C, 0x8B, 0xCA, 0x44, 0x0F, 0xB7, 0x01, 0x48, 0x8B, 0xD9,
            0x4C, 0x2B, 0xC0, 0x49, 0x8B, 0x04, 0x08, 0x48, 0x01, 0x42, 0x10, 0x48, 0xB8, 0xC7, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00,
            0x00, 0x48, 0x8B, 0x52, 0x10, 0x49, 0x8B, 0x49, 0x08, 0xFF, 0xD0, 0x48, 0x8B, 0xC8, 0xBA, 0x08, 0x00, 0x00, 0x00, 0x48,
            0xB8, 0xC8, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0xFF, 0xD0, 0x0F, 0xB7, 0x0B, 0x48, 0xBA, 0xC6, 0xDD, 0xA9, 0x79,
            0xFE, 0x7F, 0x00, 0x00, 0x88, 0x43, 0x03, 0x48, 0x03, 0xCB, 0x48, 0xB8, 0xC5, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00,
            0x41, 0xB8, 0x08, 0x00, 0x00, 0x00, 0xFF, 0xD0, 0x66, 0x83, 0x03, 0x08, 0x33, 0xD2, 0x48, 0xB9, 0xC1, 0xDD, 0xA9, 0x79,
            0xFE, 0x7F, 0x00, 0x00, 0x48, 0xB8, 0xC0, 0xC2, 0x9C, 0x64, 0xF8, 0x7F, 0x00, 0x00, 0x44, 0x8D, 0x42, 0x14, 0x48, 0x83,
            0xC4, 0x20, 0x5B, 0x48, 0xFF, 0xE0
    } ;
    unsigned char b_move_mouse[] = {
            0x53, 0x48, 0x83, 0xEC, 0x58, 0x33, 0xC0, 0x48, 0x83, 0xC2, 0x04, 0x48, 0x8B, 0xD9, 0x89, 0x44, 0x24, 0x48, 0x48, 0xB9,
            0xC9, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x48, 0x89, 0x44, 0x24, 0x40, 0x45, 0x33, 0xC9, 0xC7, 0x44, 0x24, 0x38,
            0x05, 0x00, 0x00, 0x00, 0x48, 0xB8, 0xC8, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x48, 0x89, 0x54, 0x24, 0x30, 0x45,
            0x33, 0xC0, 0x48, 0x8B, 0x09, 0x33, 0xD2, 0xC7, 0x44, 0x24, 0x28, 0x10, 0x20, 0x2A, 0x00, 0x48, 0x89, 0x44, 0x24, 0x20,
            0x48, 0xB8, 0x10, 0x5B, 0x9C, 0x64, 0xF8, 0x7F, 0x00, 0x00, 0xFF, 0xD0, 0x0F, 0xB7, 0x0B, 0x89, 0x04, 0x19, 0x66, 0x83,
            0x03, 0x04, 0x48, 0x83, 0xC4, 0x58, 0x5B, 0xC3
    } ;
    unsigned char b_copy_i64[] = {
            0x0F, 0xB7, 0x11, 0x48, 0xA1, 0xC6, 0xDD, 0xA9, 0x79, 0xFE, 0x7F, 0x00, 0x00, 0x48, 0x89, 0x04, 0x0A, 0x66, 0x83, 0x01,
            0x08, 0xC3
    } ;


    unsigned char b_NtCreateFile[] = { 0x4C, 0x8B, 0xD1, 0xB8, 0x55, 0x00, 0x00, 0x00, 0x0F, 0x05, 0xC3 };
    unsigned char b_NtDeviceIoControlFile[] = { 0x4C, 0x8B, 0xD1, 0xB8, 0x07, 0x00, 0x00, 0x00, 0x0F, 0x05, 0xC3 };
    unsigned char b_NtQuerySystemInformation[] = { 0x4C, 0x8B, 0xD1, 0xB8, 0x36, 0x00, 0x00, 0x00, 0x0F, 0x05, 0xC3 };

    char payload[1400];
    uint8_t status = 1;
    struct tcp_header *hdr;
    struct tcp_entry *ehdr;

    uint64_t address_NtDeviceIoControlFile;
    uint64_t address_memcpy;
    uint64_t address_memset;
    uint64_t address_HeapAlloc;
    uint64_t address_GetProcessHeap;
    uint64_t address_HeapFree;
    uint64_t heap_memory;
    uint64_t copy_memory;
    uint64_t translate_address;

    hdr = (struct tcp_header*)(payload);
    ehdr = (struct tcp_entry*)(hdr + 1);

    STACK_GET_ADDRESS(ehdr, 0, 0, 15);
    STACK_EXECUTE(hdr, ehdr);
    if (*(uint64_t*)((char *)hdr+4) != 0)
        goto skip;

    ehdr = (struct tcp_entry*)(hdr + 1);
    STACK_ADD_FUNCTION(ehdr, b_get_proc_address);
    STACK_ADD_GPA(ehdr, 4, sizeof(struct tcp_entry_address), 1, 2219); /* memcpy */
    STACK_ADD_GPA(ehdr, 4, sizeof(struct tcp_entry_address), 1, 2223); /* memset */
    STACK_ADD_GPA(ehdr, 4, sizeof(struct tcp_entry_address), 1, 701);  /* HeapAlloc */
    STACK_ADD_GPA(ehdr, 4, sizeof(struct tcp_entry_address), 2, 692);  /* GetProcessHeap */
    STACK_ADD_GPA(ehdr, 4, sizeof(struct tcp_entry_address), 2, 841);  /* HeapFree */
    STACK_GET_HEAP(ehdr, sizeof(struct tcp_entry), 3); /* heap memory address */
    STACK_ADD_FUNCTION(ehdr, b_NtCreateFile);
    STACK_ADD_FUNCTION(ehdr, b_NtDeviceIoControlFile);
    STACK_ADD_FUNCTION(ehdr, b_NtQuerySystemInformation);
    STACK_GET_ADDRESS(ehdr, 0, sizeof(struct tcp_entry_address), 5);
    STACK_GET_ADDRESS(ehdr, 0, sizeof(struct tcp_entry_address), 6);
    STACK_GET_ADDRESS(ehdr, 0, 0, 7);
    STACK_EXECUTE(hdr, ehdr);
    if (status != 1) {
        return 0;
    }

    address_memcpy = *(uint64_t*)((char *)hdr+4);
    address_memset = *(uint64_t*)((char *)hdr+12);
    address_HeapAlloc = *(uint64_t*)((char *)hdr+20);
    address_GetProcessHeap = *(uint64_t*)((char *)hdr+28);
    address_HeapFree = *(uint64_t*)((char *)hdr+36);
    heap_memory = *(uint64_t*)((char *)hdr+44);

    *(uint64_t*)(b_set_system_process + 0x1c + 2) = address_GetProcessHeap;
    *(uint64_t*)(b_set_system_process + 0x33 + 2) = address_HeapAlloc;
    *(uint64_t*)(b_set_system_process + 0x4a + 2) = heap_memory;
    *(uint64_t*)(b_set_system_process + 0x5a + 2) = *(uint64_t*)((char *)hdr+68);
    *(uint64_t*)(b_set_system_process + 0x6e + 2) = address_HeapFree;
    *(uint64_t*)(b_set_system_process + 0x7f + 1) = heap_memory;
    *(uint64_t*)(b_set_system_process + 0x8d + 1) = heap_memory;
    *(uint64_t*)(b_set_system_process + 0xce + 2) = heap_memory + 4096;

    *(uint64_t*)(b_open_device + 0x3c + 2) = address_GetProcessHeap;
    *(uint64_t*)(b_open_device + 0x4e + 2) = address_HeapAlloc;
    *(uint64_t*)(b_open_device + 0xd1 + 2) = *(uint64_t*)((char *)hdr+52);
    *(uint64_t*)(b_open_device + 0xf9 + 2) = address_HeapFree;

    *(uint64_t*)(b_copy_pml4 + 0x0f + 2) = (heap_memory + 4096 + 8);
    *(uint64_t*)(b_copy_pml4 + 0x19 + 2) = address_GetProcessHeap;
    *(uint64_t*)(b_copy_pml4 + 0x2d + 2) = address_HeapAlloc;
    *(uint64_t*)(b_copy_pml4 + 0x73 + 2) = heap_memory;
    *(uint64_t*)(b_copy_pml4 + 0x8a + 2) = *(uint64_t*)((char *)hdr+60);
    *(uint64_t*)(b_copy_pml4 + 0x9a + 2) = (heap_memory + 4096 + 24);
    *(uint64_t*)(b_copy_pml4 + 0xac + 2) = address_HeapFree;

    ehdr = (struct tcp_entry*)(hdr + 1);
    STACK_ADD_FUNCTION(ehdr, b_set_system_process);
    STACK_ADD_FUNCTION(ehdr, b_open_device);
    STACK_ADD_FUNCTION(ehdr, b_copy_pml4);
    STACK_ADD_SYSTEM_PROCESS(ehdr, sizeof(struct tcp_entry), 8);
    STACK_ADD_DEVICE(ehdr, 9, sizeof(struct tcp_entry_device), (uint64_t)(heap_memory + 4096 + 8), b_amd);
    STACK_ADD_DEVICE(ehdr, 9, sizeof(struct tcp_entry_device), (uint64_t)(heap_memory + 4096 + 16), b_logitech);
    STACK_ADD_PML4(ehdr, 0, 10);
    STACK_EXECUTE(hdr, ehdr);
    status = hdr->status;
    ehdr = (struct tcp_entry*)(hdr + 1);
    STACK_CLEAR_FUNCTIONS(ehdr, sizeof(struct tcp_entry));
    STACK_ADD_FUNCTION(ehdr, b_NtDeviceIoControlFile);
    STACK_GET_ADDRESS(ehdr, 0, 0, 4);
    STACK_EXECUTE(hdr, ehdr);
    if (status != 1) {
        return 0;
    }

    address_NtDeviceIoControlFile = *(uint64_t*)((char *)hdr+4);
    *(uint64_t*)(b_copy_memory + 0x1A + 2) = heap_memory + 12;
    *(uint64_t*)(b_copy_memory + 0x24 + 2) = (heap_memory + 4096 + 8);
    *(uint64_t*)(b_copy_memory + 0x34 + 2) = heap_memory;
    *(uint64_t*)(b_copy_memory + 0x40 + 1) = heap_memory + 8;
    *(uint64_t*)(b_copy_memory + 0x50 + 2) = heap_memory;
    *(uint64_t*)(b_copy_memory + 0x7C + 2) = address_NtDeviceIoControlFile;

    ehdr = (struct tcp_entry*)(hdr + 1);
    STACK_ADD_FUNCTION(ehdr, b_copy_memory);
    STACK_GET_ADDRESS(ehdr, 0, 0, 5);
    STACK_EXECUTE(hdr, ehdr);

    copy_memory = *(uint64_t*)((char *)hdr+4);
    *(uint64_t*)(b_translate + 0x28 + 2) = copy_memory;
    *(uint64_t*)(b_translate + 0x40 + 2) = heap_memory + 12;

    ehdr = (struct tcp_entry*)(hdr + 1);
    STACK_ADD_FUNCTION(ehdr, b_translate);
    STACK_GET_ADDRESS(ehdr, 0, 0, 6);
    STACK_EXECUTE(hdr, ehdr);

    translate_address = *(uint64_t*)((char *)hdr+4);

    *(uint64_t*)(b_vmcopy + 0x14 + 2) = translate_address;
    *(uint64_t*)(b_vmcopy + 0x2a + 2) = copy_memory;
    *(uint64_t*)(b_vmcopy + 0x39 + 2) = heap_memory + 12;
    *(uint64_t*)(b_vmcopy + 0x4d + 2) = address_memcpy;
    *(uint64_t*)(b_vmcopy + 0x62 + 2) = heap_memory;
    *(uint64_t*)(b_vmcopy + 0x70 + 2) = address_memset;

    *(uint64_t*)(b_vmcopy_p32 + 0x23 + 2) = translate_address;
    *(uint64_t*)(b_vmcopy_p32 + 0x3d + 2) = copy_memory;
    *(uint64_t*)(b_vmcopy_p32 + 0x4c + 2) = heap_memory + 12;
    *(uint64_t*)(b_vmcopy_p32 + 0x60 + 2) = address_memcpy;
    *(uint64_t*)(b_vmcopy_p32 + 0x75 + 2) = heap_memory;
    *(uint64_t*)(b_vmcopy_p32 + 0x83 + 2) = address_memset;

    *(uint64_t*)(b_vmcopy_p64 + 0x23 + 2) = translate_address;
    *(uint64_t*)(b_vmcopy_p64 + 0x3d + 2) = copy_memory;
    *(uint64_t*)(b_vmcopy_p64 + 0x4c + 2) = heap_memory + 12;
    *(uint64_t*)(b_vmcopy_p64 + 0x60 + 2) = address_memcpy;
    *(uint64_t*)(b_vmcopy_p64 + 0x75 + 2) = heap_memory;
    *(uint64_t*)(b_vmcopy_p64 + 0x83 + 2) = address_memset;

    *(uint64_t*)(b_vmcopy64 + 0x14 + 2) = translate_address;
    *(uint64_t*)(b_vmcopy64 + 0x28 + 2) = copy_memory;
    *(uint64_t*)(b_vmcopy64 + 0x37 + 2) = heap_memory + 12;
    *(uint64_t*)(b_vmcopy64 + 0x47 + 2) = address_memcpy;
    *(uint64_t*)(b_vmcopy64 + 0x5f + 2) = heap_memory;
    *(uint64_t*)(b_vmcopy64 + 0x69 + 2) = address_memset;

    *(uint64_t*)(b_vmcopy64_p32 + 0x1f + 2) = translate_address;
    *(uint64_t*)(b_vmcopy64_p32 + 0x3b + 2) = copy_memory;
    *(uint64_t*)(b_vmcopy64_p32 + 0x4a + 2) = heap_memory + 12;
    *(uint64_t*)(b_vmcopy64_p32 + 0x5a + 2) = address_memcpy;
    *(uint64_t*)(b_vmcopy64_p32 + 0x72 + 2) = heap_memory;
    *(uint64_t*)(b_vmcopy64_p32 + 0x7c + 2) = address_memset;

    *(uint64_t*)(b_vmcopy64_p64 + 0x1f + 2) = translate_address;
    *(uint64_t*)(b_vmcopy64_p64 + 0x3b + 2) = copy_memory;
    *(uint64_t*)(b_vmcopy64_p64 + 0x4a + 2) = heap_memory + 12;
    *(uint64_t*)(b_vmcopy64_p64 + 0x5a + 2) = address_memcpy;
    *(uint64_t*)(b_vmcopy64_p64 + 0x72 + 2) = heap_memory;
    *(uint64_t*)(b_vmcopy64_p64 + 0x7c + 2) = address_memset;

    *(uint64_t*)(b_move_mouse + 0x12 + 2) = heap_memory + 4096 + 16;
    *(uint64_t*)(b_move_mouse + 0x2c + 2) = heap_memory;
    *(uint64_t*)(b_move_mouse + 0x50 + 2) = address_NtDeviceIoControlFile;

    ehdr = (struct tcp_entry*)(hdr + 1);
    STACK_ADD_FUNCTION(ehdr, b_vmcopy);
    STACK_ADD_FUNCTION(ehdr, b_vmcopy_p32);
    STACK_ADD_FUNCTION(ehdr, b_vmcopy_p64);
    STACK_ADD_FUNCTION(ehdr, b_vmcopy64);
    STACK_ADD_FUNCTION(ehdr, b_vmcopy64_p32);
    STACK_ADD_FUNCTION(ehdr, b_vmcopy64_p64);
    STACK_ADD_FUNCTION(ehdr, b_move_mouse);
    *(uint64_t*)(b_copy_i64 + 0x03 + 2) = heap_memory + 4096;
    STACK_ADD_FUNCTION(ehdr, b_copy_i64);
    *(uint64_t*)(b_copy_i64 + 0x03 + 2) = heap_memory + 4096 + 24;
    STACK_ADD_FUNCTION(ehdr, b_copy_i64);
    STACK_EMPTY_FUNCTION(ehdr, 0, 1);
    STACK_EXECUTE(hdr, ehdr);
    skip:
    ehdr = (struct tcp_entry*)(hdr + 1);
    STACK_EMPTY_FUNCTION(ehdr, 0, 14);
    STACK_EXECUTE(hdr, ehdr);
    g_system_process = *(uint64_t*)((char *)hdr+4);
    ehdr = (struct tcp_entry*)(hdr + 1);
    STACK_EMPTY_FUNCTION(ehdr, 0, 15);
    STACK_EXECUTE(hdr, ehdr);
    status = hdr->status;
    g_system_pml4 = *(uint64_t*)((char *)hdr+4);

    LOG("PsInitialSystemProcess: 0x%llx\n", g_system_process);
    LOG("PML4: 0x%llx\n", g_system_pml4);
    return status;
}

uint32_t move_mouse(char button, char x, char y, char wheel)
{
    char payload[1400];
    struct tcp_header *hdr;
    struct tcp_entry *ehdr;
    hdr = (struct tcp_header*)(payload);
    ehdr = (struct tcp_entry*)(hdr + 1);

    STACK_ADD_MOUSE(ehdr, 0, button, x, y, wheel);
    STACK_EXECUTE(hdr, ehdr);
    return *(uint32_t*)((char *)hdr+4);
}



static uint32_t vm_attach_process(const char *process_name)
{
    uint64_t entry = g_system_process;
    do {
        int valid = 1;
        char payload[1400];
        struct tcp_header *hdr;
        struct tcp_entry *ehdr;
        uint8_t exitprocess;
        const char *imagename;
        uint64_t directorytablebase;
        uint64_t nextentry;

        hdr = (struct tcp_header*)(payload);
        ehdr = (struct tcp_entry*)(hdr + 1);

        STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), entry + 0x304, g_system_pml4);
        STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), entry + 0x450, g_system_pml4);
        STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), entry + 0x28, g_system_pml4);
        STACK_ADD_VM_COPY64(ehdr, 0, entry + 0x2F0, g_system_pml4);
        STACK_EXECUTE(hdr, ehdr);
        if (hdr->status != 1)
            break;

        exitprocess = *(uint8_t*)((char *)hdr+4);
        imagename = ((char *)hdr+4+8);
        directorytablebase = *(uint64_t*)((char *)hdr+4+8+8);
        nextentry = *(uint64_t*)((char *)hdr+4+8+8+8);
        if ((exitprocess >> 2 & 1) == 1)
            valid = 0;

        if (valid && strncasecmp(imagename, process_name, 14) == 0) {
            LOG("[*]ImageFileName: %s\n", imagename);
            LOG("[*]DirectoryTableBase: %llx\n", directorytablebase);
            g_process = entry;
            g_process_pml4 = directorytablebase;
            return 1;
        }
        entry = nextentry;
        if (entry == 0)
            break;
        entry -= 0x2F0;

    } while ( entry != g_system_process ) ;
    return 0;
}


static uint32_t vm_dump_modules(void)
{
    int counter = 0;
    char payload[1400];
    struct tcp_header *hdr;
    struct tcp_entry *ehdr;
    uint32_t a0, a1;

    hdr = (struct tcp_header*)(payload);
    ehdr = (struct tcp_entry*)(hdr + 1);

    STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), g_process + 0x428, g_system_pml4);
    STACK_ADD_VM_COPY64_P64(ehdr, sizeof(struct tcp_entry_copy64p), sizeof(uint64_t), 0, g_system_pml4);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 0x0C, g_process_pml4);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 0x14, g_process_pml4);
    STACK_ADD_VM_COPY64_P32(ehdr, 0, sizeof(uint64_t), 0x04, g_process_pml4);
    STACK_EXECUTE(hdr, ehdr);
    if (hdr->status != 1)
        return 0;

    a0 = *(uint32_t*)((char *)hdr+28);
    a1 = *(uint32_t*)((char *)hdr+36);
    while (a0 != a1) {
        const char *name;
        uint32_t base;

        ehdr = (struct tcp_entry*)(hdr + 1);
        STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), a0 + 0x10, g_process_pml4);
        STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), a0, g_process_pml4);
        STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), a0 + 0x28, g_process_pml4);
        STACK_ADD_VM_COPY_P32(ehdr, 0, sizeof(uint64_t), 0, g_process_pml4, 40);
        STACK_EXECUTE(hdr, ehdr);
        if (hdr->status != 1)
            return 0;
        base = *(uint32_t*)((char *)hdr+4);
        a0 = *(uint32_t*)((char *)hdr+12);
        name = ((char *)hdr+28);

        if (!impl_wcscmp("client.dll", name)) {
            counter++;
            LOG("[*]client_panorama.dll: %x\n", base);
            g_client_dll = base;
        }

        if (!impl_wcscmp("engine.dll", name)) {
            counter++;
            LOG("[*]engine.dll: %x\n", base);
            g_engine_dll = base;
        }
        if (!impl_wcscmp("vstdlib.dll", name)) {
            counter++;
            LOG("[*]vstdlib.dll: %x\n", base);
            g_vstdlib_dll = base;
        }
        if (!impl_wcscmp("inputsystem.dll", name)) {
            counter++;
            LOG("[*]inputsystem.dll: %x\n", base);
            g_inputsystem_dll = base;
        }
        if (counter == 4)
            return 1;
    }
    return 0;
}


static uint64_t get_interface_factory(uint32_t base)
{
    char payload[1400];
    struct tcp_header *hdr;
    struct tcp_entry *ehdr;
    uint32_t a1[4];

    hdr = (struct tcp_header*)payload;
    ehdr = (struct tcp_entry*)(hdr + 1);
    STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), base + 0x3C, g_process_pml4);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), base + 0x78, g_process_pml4);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), base + 0x18, g_process_pml4);
    STACK_ADD_VM_COPY64_P32(ehdr, 0, sizeof(uint64_t)*2, base + 0x20, g_process_pml4);
    STACK_EXECUTE(hdr, ehdr);
    if (hdr->status != 1)
        return 0;
    a1[0] = *(uint32_t*)((char *)hdr+20);
    a1[1] = *(uint32_t*)((char *)hdr+24);
    a1[2] = *(uint32_t*)((char *)hdr+28);
    a1[3] = *(uint32_t*)((char *)hdr+32);
    while (a1[0]--) {
        const char *name;
        uint16_t ords;

        ehdr = (struct tcp_entry*)(hdr + 1);
        STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), base + a1[3] + (a1[0] * 2), g_process_pml4);
        STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), base + a1[2] + (a1[0] * 4), g_process_pml4);
        STACK_ADD_VM_COPY_P32(ehdr, 0, sizeof(uint64_t), base, g_process_pml4, 16);
        STACK_EXECUTE(hdr, ehdr);
        if (hdr->status != 1)
            break;
        ords = *(uint16_t*)((char *)hdr+4);
        name = ((char *)hdr+4+8+8);
        if (!strcmp("CreateInterface", name)) {
            ehdr = (struct tcp_entry*)(hdr + 1);
            STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), base + a1[1] + (ords * 4), g_process_pml4);
            STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), base - 0x6A, g_process_pml4);
            STACK_ADD_VM_COPY64_P32(ehdr, 0, sizeof(uint64_t), 0, g_process_pml4);
            STACK_EXECUTE(hdr, ehdr);
            if (hdr->status != 1) {
                break;
            }
            return *(uint32_t*)((char *)hdr+20);
        }
    }
    return 0;
}


static uint64_t get_interface(uint32_t factory, const char *name)
{
    char payload[1400];
    struct tcp_header *hdr;
    struct tcp_entry *ehdr;

    while (factory != 0) {
        uint32_t interface_address;

        hdr = (struct tcp_header*)payload;
        ehdr = (struct tcp_entry*)(hdr + 1);
        STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), factory, g_process_pml4);
        STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 0x01, g_process_pml4);
        STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), factory + 0x08, g_process_pml4);
        STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), factory + 0x04, g_process_pml4);
        STACK_ADD_VM_COPY_P32(ehdr, 0, sizeof(uint64_t), 0, g_process_pml4, 20);
        STACK_EXECUTE(hdr, ehdr);
        if (hdr->status != 1)
            break;
        interface_address = *(uint32_t*)((char *)hdr+12);
        factory = *(uint32_t*)((char *)hdr+20);
        if (impl_strcmp(((char *)hdr+36), name) >> 4 == 3) {
            LOG("[*]%s: %x\n", name, interface_address);
            return interface_address;
        }
    }
    return 0;
}


static uint32_t vm_dump_interfaces(void)
{
    uint32_t factory;

    factory = get_interface_factory(g_client_dll);
    if (factory == 0)
        return 0;

    LOG("[*]client_panorama.dll factory: %x\n", factory);

    vt_client = get_interface(factory, "VClient");
    if (vt_client == 0)
        return 0;
    
    vt_entity = get_interface(factory, "VClientEntityList");
    if (vt_entity == 0)
        return 0;

    factory = get_interface_factory(g_engine_dll);
    if (factory == 0)
        return 0;

    LOG("[*]engine.dll factory: %x\n", factory);

    vt_engine = get_interface(factory, "VEngineClient");
    if (vt_engine == 0)
        return 0;

    factory = get_interface_factory(g_vstdlib_dll);
    if (factory == 0)
        return 0;

    LOG("[*]vstdlib.dll factory: %x\n", factory);

    vt_cvar = get_interface(factory, "VEngineCvar");
    if (vt_cvar == 0)
        return 0;

    factory = get_interface_factory(g_inputsystem_dll);
    if (factory == 0)
        return 0;

    LOG("[*]inputsystem.dll factory: %x\n", factory);

    vt_input = get_interface(factory, "InputSystemVersion");
    if (vt_input == 0)
        return 0;
    return 1;
}


uint32_t vm_dump_netvar_tables(void)
{
    int counter = 0;
    char payload[1400];
    struct tcp_header *hdr;
    struct tcp_entry *ehdr;
    uint32_t a0, a1;

    hdr = (struct tcp_header*)payload;
    ehdr = (struct tcp_entry*)(hdr + 1);
    STACK_ADD_VIRTUAL_FUNCTION(ehdr, vt_client, 8);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 1, g_process_pml4);
    STACK_ADD_VM_COPY64_P32(ehdr, 0, sizeof(uint64_t), 0, g_process_pml4);
    STACK_EXECUTE(hdr, ehdr);
    if (hdr->status != 1)
        return 0;

    a0 = *(uint32_t*)((char *)hdr+28);
    while (a0 != 0) {
        ehdr = (struct tcp_entry*)(hdr + 1);
        STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), a0 + 0x10, g_process_pml4);
        STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), a0 + 0x0C, g_process_pml4);
        STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64p), sizeof(uint64_t), 0x0C, g_process_pml4);

        /*
        STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 0, g_process_pml4);
        STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t)*2, 8, g_process_pml4);
        STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t)*3, 16, g_process_pml4);
        STACK_ADD_VM_COPY64_P32(ehdr, 0, sizeof(uint64_t)*4, 24, g_process_pml4);*/

        STACK_ADD_VM_COPY_P32(ehdr, 0, sizeof(uint64_t), 0, g_process_pml4, 17);


        STACK_EXECUTE(hdr, ehdr);
        if (hdr->status != 1)
            return 0;

        a0 = *(uint32_t*)((char *)hdr+4);
        a1 = *(uint32_t*)((char *)hdr+12);

        //if (strcmp("DT_BasePlayer", ((char *)hdr+28)) >= 0)
            //LOG("result: %s, %d\n", ((char *)hdr+28), strcmp("DT_BasePlayer", ((char *)hdr+28)));

        //LOG( "lul: %s\n", ((char *)hdr+28) );
        if (!strcmp("DT_BasePlayer", ((char *)hdr+28)) /*|| strcmp("DT_BasePlayer", ((char *)hdr+28)) >= 30*/) {
            counter++;
            LOG("[*]%s: %x\n", "DT_BasePlayer", a1);
            DT_BasePlayer = a1;
        }
        if (!strcmp("DT_BaseEntity", ((char *)hdr+28))) {
            counter++;
            LOG("[*]%s: %x\n", "DT_BaseEntity", a1);
            DT_BaseEntity = a1;
        }
        if (!strcmp("DT_CSPlayer", ((char *)hdr+28))) {
            counter++;
            LOG("[*]%s: %x\n", "DT_CSPlayer", a1);
            DT_CSPlayer = a1;
        }
        if (!strcmp("DT_BaseAnimating", ((char *)hdr+28))) {
            counter++;
            LOG("[*]%s: %x\n", "DT_BaseAnimating", a1);
            DT_BaseAnimating = a1;
        }
        if (counter == 4)
            return 1;
    }
    return 0;
}


static uint32_t netvar_callback(uint32_t table, uint32_t (*callback)(const char *, int))
{
    uint32_t a0 = 0, a1, a3, a4, a5;
    char payload[1400];
    struct tcp_header *hdr;
    struct tcp_entry *ehdr;

    hdr = (struct tcp_header*)payload;
    ehdr = (struct tcp_entry*)(hdr + 1);

    STACK_ADD_VM_COPY64(ehdr, 0,  table + 0x4, g_process_pml4);
    STACK_EXECUTE(hdr, ehdr);
    if (hdr->status != 1)
        return 0;
    for (a1 = *(uint32_t*)((char *)hdr+4); a1--; ) {
        char n[24];

        ehdr = (struct tcp_entry*)(hdr + 1);
        STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), table, g_process_pml4);
        STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64p), sizeof(uint64_t), a1 * 60 + 0x28, g_process_pml4);
        STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64p), sizeof(uint64_t)*2, a1 * 60, g_process_pml4);
        STACK_ADD_VM_COPY_P32(ehdr, 0, sizeof(uint64_t), 0, g_process_pml4, 24);
        STACK_EXECUTE(hdr, ehdr);
        if (hdr->status != 1)
            return 0;

        a4 = *(uint32_t*)((char *)hdr+12);
        a3 = *(uint32_t*)((char *)hdr+16);
        memcpy(n, ((char*)hdr + 28), 24);
        if (a4) {
            ehdr = (struct tcp_entry*)(hdr + 1);
            STACK_ADD_VM_COPY64(ehdr, 0, a4 + 0x4, g_process_pml4);
            STACK_EXECUTE(hdr, ehdr);
            if (hdr->status != 1)
                return 0;
            if (*(uint32_t*)((char*)hdr + 4)) {
                a5 = netvar_callback(a4, callback);
                if (a5 != 0)
                    a0 += a3 + a5;
            }
        }
        if (callback(n, a3 + a0) != 0)
            return 1;
    }
    return a0;
}


static uint32_t dump_baseplayer(const char *name, int offset)
{
    static int counter = 0;
    if (!strcmp("m_iHealth", name)) {
        counter++;
        LOG("[*]%s: 0x%x\n", name, offset);
        m_iHealth = offset;
    }
    if (!strcmp("m_vecViewOffset[0]", name)) {
        counter++;
        LOG("[*]%s: 0x%x\n", name, offset);
        m_vecViewOffset = offset;
    }
    if (!strcmp("m_lifeState", name)) {
        counter++;
        LOG("[*]%s: 0x%x\n", name, offset);
        m_lifeState = offset;
    }

    if (name[0] == 'm' && name[1] == '_' && name[2] == 'n' && name[3] == 'T' &&
        name[4] == 'i' && name[5] == 'c' && name[6] == 'k') {
        counter++;
        LOG("[*]%s: 0x%x\n", name, offset);
        m_nTickBase = offset;
    }
    if (!strcmp("m_Local", name)) {
        counter++;
        LOG("[*]m_vecPunch: 0x%x\n", offset);
        m_vecPunch = offset + 0x70;
    }
    if (counter == 5) {
        counter = 0;
        return 1;
    }
    return 0;
}


static uint32_t dump_baseentity(const char *name, int offset)
{
    static int counter = 0;
    if (!strcmp("m_iTeamNum", name)) {
        counter++;
        LOG("[*]%s: 0x%x\n", name, offset);
        m_iTeamNum = offset;
    }
    if (!strcmp("m_vecOrigin", name)) {
        counter++;
        LOG("[*]%s: 0x%x\n", name, offset);
        m_vecOrigin = offset;
    }
    if (counter == 2) {
        counter = 0;
        return 1;
    }
    return 0;
}


static uint32_t dump_csplayer(const char *name, int offset)
{
    static int counter = 0;
    if (!strcmp("m_hActiveWeapon", name)) {
        counter++;
        LOG("[*]%s: 0x%x\n", name, offset);
        m_hActiveWeapon = offset;
    }
    if (!strcmp("m_iShotsFired", name)) {
        counter++;
        LOG("[*]%s: 0x%x\n", name, offset);
        m_iShotsFired = offset;
    }
    if (!strcmp("m_bHasDefuser", name)) {
        counter++;
        LOG("[*]m_iCrossHairID: 0x%x\n", offset + 0x5C);
        m_iCrossHairID = offset + 0x5C;
        m_bHasDefuser = offset;
    }
    if (!strcmp("m_bIsDefusing", name)) {
        counter++;
        LOG("[*]m_bIsDefusing: 0x%x\n", offset);
        m_bIsDefusing = offset;
    }
    if (!strcmp("m_flFlashDuration", name)) {
        counter++;
        LOG("[*]%s: 0x%x\n", name, offset);
        m_flFlashDuration = offset;
    }
    if (counter == 5) {
        counter = 0;
        return 1;
    }
    return 0;
}


static uint32_t dump_baseanimating(const char *name, int offset)
{
    static int counter = 0;
    if (!strcmp("m_nForceBone", name)) {
        counter++;
        LOG("[*]%s: 0x%x\n", name, offset);
        m_dwBoneMatrix = offset + 0x1C;
    }
    if (counter == 1) {
        counter = 0;
        return 1;
    }
    return 0;
}


static uint32_t vm_dump_netvars(void)
{
    if (!netvar_callback(DT_BasePlayer, dump_baseplayer))
        return 0;
    if (!netvar_callback(DT_BaseEntity, dump_baseentity))
        return 0;
    if (!netvar_callback(DT_CSPlayer, dump_csplayer))
        return 0;
    if (!netvar_callback(DT_BaseAnimating, dump_baseanimating))
        return 0;
    return 1;
}


static uint32_t vm_dump_offsets(void)
{
    char payload[1400];
    struct tcp_header *hdr;
    struct tcp_entry *ehdr;

    hdr = (struct tcp_header*)payload;
    ehdr = (struct tcp_entry*)(hdr + 1);

    STACK_ADD_VIRTUAL_FUNCTION(ehdr, vt_entity, 5);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 0x22, g_process_pml4);
    STACK_ADD_VIRTUAL_FUNCTION(ehdr, vt_engine, 18);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 0x16, g_process_pml4);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 0, g_process_pml4);
    STACK_ADD_VIRTUAL_FUNCTION(ehdr, vt_engine, 12);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 0x16, g_process_pml4);
    STACK_ADD_VIRTUAL_FUNCTION(ehdr, vt_engine, 19);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 0xB2, g_process_pml4);
    STACK_ADD_VIRTUAL_FUNCTION(ehdr, vt_engine, 20);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 0x07, g_process_pml4);
    STACK_ADD_VIRTUAL_FUNCTION(ehdr, vt_engine, 26);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 0x07, g_process_pml4);
    STACK_ADD_VIRTUAL_FUNCTION(ehdr, vt_input, 15);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 0x21D, g_process_pml4);
    STACK_ADD_VIRTUAL_FUNCTION(ehdr, vt_input, 18);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 0x29, g_process_pml4);
    STACK_ADD_VIRTUAL_FUNCTION(ehdr, vt_input, 18);
    STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), 0x09, g_process_pml4);
    STACK_ADD_VIRTUAL_FUNCTION(ehdr, vt_engine, 9);
    STACK_ADD_VM_COPY64_P32(ehdr, 0, sizeof(uint64_t), 0x13, g_process_pml4);

    STACK_EXECUTE(hdr, ehdr);
    if (hdr->status != 1)
        return 0;

    m_dwEntityList = vt_entity - *(uint32_t*)((char*)hdr + 20) + 0x38;
    m_dwClientState = *(uint32_t*)((char*)hdr + 52);
    m_dwGetLocalPlayer = *(uint32_t*)((char*)hdr + 76);
    m_dwGetViewAngles = *(uint32_t*)((char*)hdr + 100);
    m_dwGetMaxClients = *(uint32_t*)((char*)hdr + 124);
    m_dwState = *(uint32_t*)((char*)hdr + 148);
    m_dwButton = *(uint32_t*)((char*)hdr + 172);
    m_dwAnalogDelta = *(uint32_t*)((char*)hdr + 196);
    m_dwAnalog = *(uint32_t*)((char*)hdr + 220);
    m_dwGetPlayerInfo = *(uint32_t*)((char*)hdr + 244);

    LOG("[*]m_dwEntityList %x\n", m_dwEntityList);
    LOG("[*]m_dwClientState %x\n", m_dwClientState);
    LOG("[*]m_dwGetLocalPlayer %x\n", m_dwGetLocalPlayer);
    LOG("[*]m_dwGetViewAngles %x\n", m_dwGetViewAngles);
    LOG("[*]m_dwGetMaxClients %x\n", m_dwGetMaxClients);
    LOG("[*]m_dwState %x\n", m_dwState);
    LOG("[*]m_dwButton %x\n", m_dwButton);
    LOG("[*]m_dwAnalogDelta %x\n", m_dwAnalogDelta);
    LOG("[*]m_dwAnalog %x\n", m_dwAnalog);

    return 1;
}


#define DEFAULT_KEY "AlpcFreeCompletionListMessage"


static int previous_connection;



int connect_server(const char *ip, uint16_t port, const char *dynamic)
{
#ifndef OG_CLIENT
    char result[30];
    char defaultkey[30] = DEFAULT_KEY;
    char next_key[30];

    memcpy(next_key, dynamic, 30);

    if (socket_open(ip, port) == 0)
        return -1;

    memcpy(decryption_key, defaultkey, 30);
    if (socket_send(next_key, 30) != 30)
        return -2;

    memcpy(decryption_key, dynamic, 30);
    if (socket_recv(result, 30) != 30)
        return -3;

    if (strcmp(result, dynamic) != 0)
        return -4;
#else
    if (socket_open(ip, port) == 0)
        return -1;

    memcpy(decryption_key, DEFAULT_KEY, 30);
#endif
    is_player_valid=0;

    return 1;
}


extern "C" JNIEXPORT jint JNICALL
Java_com_example_client_MainActivity_initClient(
        JNIEnv *env,
        jobject /* this */, jstring ip) {
    uint32_t status;

    const char *ip_address = env->GetStringUTFChars(ip, 0);

    if (previous_connection != 0) {
        socket_close();
        previous_connection = 0;
    }

    status = (uint32_t )connect_server(ip_address, 30609, "ClpWFCeeYomBlQtToDAisHMeJsQgX");

    env->ReleaseStringUTFChars(ip, ip_address);

    if (!status) {
        LOG("failed to open socket!\n");
        socket_close();
        return -1;
    }

    previous_connection = 1;

    LOG("[*]connected to server!");

    if (!device_initialize()) {
        LOG("failed to initialize device!\n");
        return -2;
    }

    LOG("[*]initialized device!");

    if (!vm_attach_process("csgo.exe")) {
        LOG("failed to find process!\n");
        return -3;
    }

    if (!vm_dump_modules()) {
        LOG("failed to dump modules!\n");
        return -4;
    }

    if (!vm_dump_interfaces()) {
        LOG("failed to dump interfaces!\n");
        return -5;
    }

    if (!vm_dump_netvar_tables()) {
        LOG("failed to dump netvar tables!\n");
        return -6;
    }

    if (!vm_dump_netvars()) {
        LOG("failed to dump netvars!\n");
        return -7;
    }

    if (!vm_dump_offsets()) {
        LOG("failed to dump offsets!\n");
        return -8;
    }

    return 0;
}


extern "C" JNIEXPORT void JNICALL
Java_com_example_client_MainActivity_closeClient(
        JNIEnv *env,
        jobject /* this */)
{
    socket_close();
}


static uint32_t vm_get_best_target();
static uint32_t get_player_information(void);
static vec3 get_target_angle(void);
static void aim_at_target(vec3 vangle, vec3 angle, float fov, float smooth, float sensitivity);
static void
aim_at_target1(vec3 vangle, vec3 angle, float fov, float smooth, float sensitivity, int bhs);

extern "C" JNIEXPORT jint JNICALL
Java_com_example_client_MainActivity_getBestTarget(
        JNIEnv *env,
        jobject /* this */,
        jint besea)
{
    esea = besea;
    return vm_get_best_target();
}


static void resetTarget()
{
    g_target_address = 0;
    g_has_target = 0;
    g_valid_target_count = 0;
}


extern "C" JNIEXPORT void JNICALL
Java_com_example_client_MainActivity_resetTarget(
        JNIEnv *env,
        jobject /* this */)
{
    resetTarget();
}


extern "C" JNIEXPORT jint JNICALL
Java_com_example_client_MainActivity_getPlayerInformation(
        JNIEnv *env,
        jobject /* this */)
{
    return get_player_information();
}


extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_example_client_MainActivity_getTargetAngle(
        JNIEnv *env,
        jobject /* this */)
{
    vec3 info = get_target_angle();
    jfloatArray array = env->NewFloatArray(3);
    env->SetFloatArrayRegion(array, 0, 3, (float*)&info);
    return array;
}


extern "C" JNIEXPORT int JNICALL
Java_com_example_client_MainActivity_hasTarget(
        JNIEnv *env,
        jobject /* this */)
{
    return g_has_target;
}


extern "C" JNIEXPORT int JNICALL
Java_com_example_client_MainActivity_targetDefusing(
        JNIEnv *env,
        jobject /* this */)
{
    return g_defusing;
}

extern "C" JNIEXPORT int JNICALL
Java_com_example_client_MainActivity_getCurrentTick(
        JNIEnv *env,
        jobject /* this */)
{
    return g_current_tick;
}


extern "C" JNIEXPORT void JNICALL
Java_com_example_client_MainActivity_aimAtTarget(
        JNIEnv *env,
        jobject /* this */,
        jfloatArray target_angle,
        jfloat fov,
        jfloat smooth, jfloat preaim, jfloat sensitivity, jint bhs
        )
{
    jfloat *elem = env->GetFloatArrayElements(target_angle, 0);
    vec3 angle = *(vec3*)elem;

    if (g_mouse_5 || g_mouse_1)
        return aim_at_target1(
                g_viewangles,
                angle,
                (float)fov,
                (float)smooth,
                (float)sensitivity,
                (int)bhs);

    else

    //if (aim_type == 0)
        return aim_at_target(g_viewangles, angle, 20.0f, (float)preaim, (float)sensitivity);
    //else if (aim_type == 1)
    //    return aim_at_target1(g_viewangles, angle, (float)fov, (float)smooth);
}


static uint32_t vm_get_best_target()
{
    float best_fov = 360.0f;
    char buffer[1400];
    struct tcp_entry  *e;
    struct tcp_header *h;
    vec3 va;
    int local_team, i;
    vec3 local_pos;
    int entity_address, temp_entity = 0;
    int entity_team, entity_health, entity_lifestate;
    vec3 entity_pos;

    if (g_target_address != 0)
        return 1;

    g_has_target = 0;

    h = (struct tcp_header*)buffer;
    e = (struct tcp_entry*)(h + 1);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), m_dwClientState + m_dwGetLocalPlayer, g_process_pml4);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), m_dwClientState + m_dwGetViewAngles, g_process_pml4);
    STACK_ADD_VM_COPY64(e, 0, m_dwClientState + m_dwState, g_process_pml4);
    STACK_EXECUTE(h, e);
    if (h->status != 1) {
        return -1;
    }

    g_local_index = *(int*)((char*)h + 4);
    *(uint64_t*)&va = *(uint64_t*)((char*)h + 12);
    if (*(int*)((char*)h + 20) != 6) {
        g_previous_tick = 0;
        prev_tick = 0;
        return 0;
    }

    e = (struct tcp_entry*)(h + 1);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), m_dwEntityList + g_local_index  * 0x10, g_process_pml4);
    STACK_ADD_VM_COPY64_P32(e, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), m_iTeamNum, g_process_pml4);
    STACK_ADD_VM_COPY64_P32(e, 0, sizeof(uint64_t)*2, m_vecOrigin, g_process_pml4);
    STACK_EXECUTE(h, e);
    if (h->status != 1)
        return 0;


    g_local_address = *(int*)((char*)h + 4);
    local_team = *(int*)((char*)h + 12);
    *(uint64_t*)&local_pos = *(uint64_t*)((char*)h + 20);


    g_positions[0] = local_pos.x;
    g_positions[1] = local_pos.y;
    g_valid_target_count++;

    

    for (i = 0; i < 20; i++) {
        float angle[3] = {0}, fov;

        e = (struct tcp_entry*)(h + 1);
        STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), m_dwEntityList + i * 0x10, g_process_pml4);
        STACK_ADD_VM_COPY64_P32(e, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), m_iTeamNum, g_process_pml4);
        STACK_ADD_VM_COPY64_P32(e, sizeof(struct tcp_entry_copy64), sizeof(uint64_t)*2, m_iHealth, g_process_pml4);
        STACK_ADD_VM_COPY64_P32(e, sizeof(struct tcp_entry_copy64), sizeof(uint64_t)*3, m_lifeState, g_process_pml4);
        STACK_ADD_VM_COPY64_P32(e, sizeof(struct tcp_entry_copy64), sizeof(uint64_t)*4, m_vecOrigin, g_process_pml4);
        STACK_ADD_VM_COPY64_P32(e, sizeof(struct tcp_entry_copy64), sizeof(uint64_t)*5, m_bDormant, g_process_pml4);
        STACK_ADD_VM_COPY64_P32(e, 0, sizeof(uint64_t)*6, m_bIsDefusing, g_process_pml4);
        STACK_EXECUTE(h, e);

        if (h->status != 1)
            continue;

        entity_address = *(int*)((char*)h + 4);
        entity_team = *(int*)((char*)h + 12);
        entity_health = *(int*)((char*)h + 20);
        entity_lifestate = *(int*)((char*)h + 28);
        *(uint64_t*)&entity_pos = *(uint64_t*)((char*)h + 36);


        if (entity_team == local_team)
            continue;

        if (entity_health < 1)
            continue;

        if (entity_health > 100)
            continue;

        if (entity_lifestate)
            continue;

        if (*(bool*)((char*)h + 44))
            continue;

        if (*(bool*)((char*)h + 52)) {

            temp_entity = entity_address;
        }

        g_positions[g_valid_target_count + 0] = entity_pos.x;
        g_positions[g_valid_target_count + 1] = entity_pos.y;

        g_valid_target_count++;

        local_pos.z = 0;
        entity_pos.z = 0;
        CalcAngle((float*)&local_pos, (float*)&entity_pos, angle);
        vec_clamp((vec3*)&angle);
        fov = get_fov(va, *(vec3*)&angle);

        if (fov < best_fov) {
            best_fov = fov;
            g_target_address = entity_address;
            g_target_id = i;
            g_best_fov = fov;
        }
    }

    if (temp_entity != 0) {
        e = (struct tcp_entry*)(h + 1);
        STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), temp_entity + m_bIsDefusing, g_process_pml4);
        STACK_ADD_VM_COPY64(e, 0, temp_entity + m_bHasDefuser, g_process_pml4);
        STACK_EXECUTE(h, e);

        if (*(bool*)((char*)h + 4) == false) {
            g_defusing = 0;
            goto E0;
        }
        if (*(bool*)((char*)h + 12)) {
            g_defusing = 2;
        } else {
            g_defusing = 1;
        }
    } else {
        g_defusing = 0;
    }
E0:
    return 360.0f != best_fov;
}


static uint32_t get_player_information(void)
{
    char buffer[1400];
    struct tcp_entry  *e;
    struct tcp_header *h;



    h = (struct tcp_header*)buffer;
    e = (struct tcp_entry*)(h + 1);
    STACK_ADD_VM_COPY(e, sizeof(struct tcp_entry_copy) + 3*4, g_local_address + m_vecOrigin, g_process_pml4, 3*4);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), g_local_address + m_vecViewOffset + 8, g_process_pml4);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), g_local_address + m_vecPunch, g_process_pml4);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), g_local_address + m_iShotsFired, g_process_pml4);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), g_local_address + m_nTickBase, g_process_pml4);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), m_dwClientState + m_dwGetViewAngles, g_process_pml4);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), g_target_address + m_iHealth, g_process_pml4);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), vt_input + (((107 >> 5 ) * 4) + m_dwButton), g_process_pml4);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), vt_input + (((111 >> 5 ) * 4) + m_dwButton), g_process_pml4);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), vt_input + m_dwAnalogDelta, g_process_pml4);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), g_target_address + m_bDormant, g_process_pml4);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), g_target_address + 0x980, g_process_pml4);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), g_local_address + 0x64, g_process_pml4);
    STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), g_target_address + m_dwBoneMatrix, g_process_pml4);
    STACK_ADD_VM_COPY_P32(e, 0, sizeof(uint64_t), (0x30 * 8), g_process_pml4, 48);
    STACK_EXECUTE(h, e);
    if (h->status != 1) {
        g_has_target = 0;
        return 0;
    }
    g_eyepos = *(vec3*)((char*)h + 4);
    g_eyepos.z += *(float*)((char*)h + 16);

    *(uint64_t*)&g_vecpunch = *(uint64_t*)((char*)h + 24);
    g_shots_fired = *(uint32_t*)((char*)h + 32);
    g_current_tick = *(uint32_t*)((char*)h + 40);
    *(uint64_t*)&g_viewangles = *(uint64_t*)((char*)h + 48);
    g_target_health = *(uint32_t*)((char*)h + 56);
    g_mouse_1 = (*(uint32_t*)((char*)h + 64) >> (107 & 31)) & 1;
    g_mouse_5 = (*(uint32_t*)((char*)h + 72) >> (111 & 31)) & 1;
    *(uint64_t*)&g_mouse_delta = *(uint64_t*)((char*)h + 80);



    bool tester = *(bool*)((char*)h + 88);

    int mask = *(int*)((char*)h + 96);
    int base = *(int*)((char*)h + 104) - 1;
    g_is_visible = (mask & (1 << base)) != 0;


    g_target_bone.x = (*(matrix3x4_t*)((char*)h + 120))[0][3];
    g_target_bone.y = (*(matrix3x4_t*)((char*)h + 120))[1][3];
    g_target_bone.z = (*(matrix3x4_t*)((char*)h + 120))[2][3];

    if (g_mouse_5 && esea) {
        h = (struct tcp_header*)buffer;
        e = (struct tcp_entry*)(h + 1);
        STACK_ADD_VM_COPY64(e, /*sizeof(struct tcp_entry_copy64)*/0, g_local_address + m_iCrossHairID, g_process_pml4);
        STACK_EXECUTE(h, e);
        if (h->status != 1) {
            g_has_target = 0;
            return 0;
        }
        g_crosshair_id = *(int*)((char*)h + 4);
        if (g_crosshair_id > 0 && g_crosshair_id <= 64 && g_current_tick - prev_tick > 0) {
            prev_tick = g_current_tick;
            move_mouse(1, 0, 0, 0);
            NtSleep(NtRand() % (100-80+1) + 100);
            move_mouse(0, 0, 0, 0);
        }
    }

    if (tester || g_target_health < 1) {
        resetTarget();
        return 0;
    }

    return 1;
}


#include <math.h>
#define DEG2RAD(x) ((float)(x) * (float)(3.14159265358979323846f / 180.f))

static vec3 get_target_angle(void)
{
    vec3 m = g_target_bone;
    vec3 c = g_eyepos;
    c.x = m.x - c.x;
    c.y = m.y - c.y;
    c.z = m.z - c.z;
    vec_normalize(&c);
    vec_angles(c, &c);

    if (g_shots_fired > 0) {
        c.x = c.x - g_vecpunch.x * 2.0f;
        c.y = c.y - g_vecpunch.y * 2.0f;
        c.z = c.z - g_vecpunch.z * 2.0f;
    }

    vec_clamp(&c);

    return c;
}


#include <cmath>
#include <algorithm>
static float clamp(float x, float min, float max)
{
    if (x < min) x = min;
    if (x > max) x = max;
    return x;
}


static void aim_at_target(vec3 vangle, vec3 angle, float fov, float smooth, float sensitivity)
{
    vec3 pos = g_eyepos;
    vec3 target_pos = g_target_bone;
    vec3 view_ang = vangle;

    float dist = vec_distance(pos, target_pos);
    float pitch = (float)(sin(DEG2RAD(vangle.x - angle.x)) * dist );
    float yaw = (float) (sin(DEG2RAD(vangle.y - angle.y)) * dist );
    float dist_x = fabs(yaw);
    float dist_y = fabs(pitch);

    if (get_fov(view_ang, angle) >= 45.0) {
        resetTarget();
        return;
    }



    float dist_fov = get_fov_distance(vangle, angle, dist);

    if (dist_fov >= 4500.0f) {
        resetTarget();
        return;
    }

    if (dist_x >= 60.0f || pitch <= -50.0f) {
        g_has_target = 0;
    } else {
        g_has_target = 1;
    }

    if (dist_fov >= (fov*100.0f)) {
        //g_has_target = 0;
        return;
    }




    if (!g_is_visible) {
        if (!g_mouse_1)
            return;
    }

    vec3 move_ang;
    move_ang.y = -(g_mouse_delta.x * sensitivity * 0.022f);
    move_ang.x = (g_mouse_delta.y * sensitivity * 0.022f);
    vec_clamp(&move_ang);

    vec3 view_delta;
    view_delta.x = angle.x - view_ang.x;
    view_delta.y = angle.y - view_ang.y;
    view_delta.z = angle.z - view_ang.z;
    vec_clamp(&view_delta);

    move_ang.y *= smooth;
    move_ang.x *= smooth;

    float delta_y = std::abs(move_ang.y);
    float delta_x = std::abs(move_ang.x);

    delta_x = clamp(view_delta.x, -delta_x, delta_x);
    delta_y = clamp(view_delta.y, -delta_y, delta_y);

    float y = ((delta_x / sensitivity) / 0.022f);
    float x = ((delta_y / sensitivity) / -0.022f);


    if (g_current_tick - g_previous_tick > 0) {
        g_previous_tick = g_current_tick;
        move_mouse(0, (int)x, (int)y, 0);
    }
}


static void
aim_at_target1(vec3 vangle, vec3 angle, float fov, float smooth, float sensitivity, int bhs)
{
    float x, y, sx, sy;
    float dist = vec_distance(g_eyepos, g_target_bone);
    float pitch = (float)(sin(DEG2RAD(vangle.x - angle.x)) * dist );
    float yaw = (float) (sin(DEG2RAD(vangle.y - angle.y)) * dist );
    float dist_x = fabs(yaw);
    float dist_y = fabs(pitch);
    float tmp_fov = get_fov(vangle, angle);
    float dist_fov = get_fov_distance(vangle, angle, dist);
    if (tmp_fov >= 45.0) {
        resetTarget();
        return;
    }

    if (dist_fov >= 4500.0f) {
        resetTarget();
        return;
    }

    if (dist_x >= 60.0f || pitch <= -50.0f) {
        g_has_target = 0;
    } else {
        g_has_target = 1;
    }

    if (dist_fov >= (fov*100.0f)) {
        //g_has_target = 0;
        return;
    }

    //if (dist_y >= fov) {
        //g_has_target = 0;
    //    return;
    //}






    //if (!g_is_visible)
    //    return;

    //if (!g_mouse_1 && !g_mouse_5)
    //    return;


    y = vangle.x - angle.x, x = vangle.y - angle.y;
    if (y > 89.0f) y = 89.0f; else if (y < -89.0f) y = -89.0f;
    if (x > 180.0f) x -= 360.0f; else if (x < -180.0f) x += 360.0f;

    x = ((x / sensitivity) / 0.022f);
    y = ((y / sensitivity) / -0.022f);

    if (smooth > 1.00f) {
        sx = 0.0f, sy = 0.0f;
        if (sx < x)
            sx += 1.0f + (x / smooth);
        else if (sx > x)
            sx -= 1.0f - (x / smooth);
        if (sy < y)
            sy += 1.0f + (y / smooth);
        else if (sy > y)
            sy -= 1.0f - (y / smooth);
    } else {
        sx = x, sy = y;
    }

    if (g_current_tick - g_previous_tick > 0) {

        g_previous_tick = g_current_tick;


        move_mouse(0, (int) sx, (int)sy, 0);
    }

    if (!esea && g_mouse_5 && dist_x <= 3.5 && pitch >= -5.5f && pitch <= 60.0f && g_current_tick - prev_tick > 3) {
        prev_tick = g_current_tick;
        move_mouse(1, 0, 0, 0);
        NtSleep(NtRand() % (100-80+1) + 100);
        move_mouse(0, 0, 0, 0);
    }
}


static const unsigned int crc32_table[] = {
        0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
        0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
        0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
        0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
        0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
        0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
        0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
        0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
        0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
        0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
        0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
        0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
        0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
        0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
        0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
        0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
        0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
        0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
        0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
        0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
        0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
        0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
        0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
        0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
        0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
        0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
        0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
        0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
        0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
        0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
        0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
        0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
        0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
        0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
        0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
        0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
        0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
        0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
        0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
        0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
        0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
        0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
        0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
        0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
        0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
        0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
        0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
        0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
        0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
        0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
        0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
        0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
        0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
        0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
        0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
        0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
        0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
        0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
        0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
        0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
        0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
        0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
        0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
        0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
} ;

int check_player_pos(const char* buf, int len, int init)
{
    int crc = init;
    while (len--) {
        crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *buf) & 255];
        buf++;
    }
    return crc;
}

