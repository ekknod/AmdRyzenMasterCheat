#include "cs.h"
#include "server.h"
#include <stdio.h>

uint64_t g_process;
uint64_t g_process_pml4;

uint32_t g_client_dll;
uint32_t g_engine_dll;
uint32_t g_vstdlib_dll;
uint32_t g_inputsystem_dll;

uint32_t vt_client;
uint32_t vt_entity;
uint32_t vt_engine;
uint32_t vt_cvar;
uint32_t vt_input;

static uint32_t DT_BasePlayer;
static uint32_t DT_BaseEntity;
static uint32_t DT_CSPlayer;
static uint32_t DT_BaseAnimating;

uint32_t m_iHealth;
uint32_t m_vecViewOffset;
uint32_t m_lifeState;
uint32_t m_nTickBase;
uint32_t m_vecPunch;
uint32_t m_iTeamNum;
uint32_t m_vecOrigin;
uint32_t m_hActiveWeapon;
uint32_t m_iShotsFired;
uint32_t m_iCrossHairID;
uint32_t m_flFlashDuration;
uint32_t m_dwBoneMatrix;

uint32_t m_dwEntityList;
uint32_t m_dwClientState;
uint32_t m_dwGetLocalPlayer;
uint32_t m_dwGetViewAngles;
uint32_t m_dwGetMaxClients;
uint32_t m_dwState;
uint32_t m_dwButton;
uint32_t m_dwAnalogDelta;
uint32_t m_dwAnalog;
uint32_t m_bIsDefusing;
uint32_t m_bHasDefuser;
uint32_t m_dwGetPlayerInfo;


static uint32_t vm_attach_process(const char *process_name);
static uint32_t vm_dump_modules(void);
static uint32_t vm_dump_interfaces(void);
static uint32_t vm_dump_netvar_tables(void);
static uint32_t vm_dump_netvars(void);
static uint32_t vm_dump_offsets(void);


#ifdef _MSC_VER 
#define strncasecmp _strnicmp
#define strcasecmp _stricmp

#else
static double get_time()
{
	struct timeval t;
	struct timezone tzp;
	gettimeofday(&t, &tzp);
	return t.tv_sec + t.tv_usec*1e-6;
}
#endif



int32_t cs_initialize(void)
{


	if (g_process != 0) {
		char payload[1400];
		struct tcp_header *hdr = (struct tcp_header*)(payload);
		struct tcp_entry *ehdr = (struct tcp_entry*)(hdr + 1);
		STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), g_process + 0x304, g_system_pml4);
		STACK_EXECUTE(hdr, ehdr);
		if ((*(uint8_t*)((char *)hdr+4) >> 2 & 1) == 0) {
			goto skip;
		}
	}

	if (!vm_attach_process("csgo.exe")) {
		printf("failed to find target process\n");
		return 0;
	}
	

	if (!vm_dump_modules()) {
		printf("failed to dump modules\n");
		g_process = 0;
		return 0;
	}

	if (!vm_dump_interfaces()) {
		printf("failed to dump interfaces\n");
		g_process = 0;
		return 0;
	}

	if (!vm_dump_netvar_tables()) {
		printf("failed to dump netvar table\n");
		g_process = 0;
		return 0;
	}

	if (!vm_dump_netvars()) {
		printf("failed to dump netvars\n");
		g_process = 0;
		return 0;
	}

	if (!vm_dump_offsets()) {
		printf("failed to dump offsets\n");
		g_process = 0;
		return 0;
	}

skip:
	return 1;
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
			g_process = entry;
			g_process_pml4 = directorytablebase;

			printf("[+] csgo.exe PML4: %llx\n", directorytablebase);
			return 1;
		}
		entry = nextentry;
		if (entry == 0)
			break;
		entry -= 0x2F0;

	} while ( entry != g_system_process ) ;

	return 0;
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
			printf("[+] client.dll: %x\n", base);
			counter++;
			g_client_dll = base;
		}
		if (!impl_wcscmp("engine.dll", name)) {
			printf("[+] engine.dll: %x\n", base);
			counter++;
			g_engine_dll = base;
		}
		if (!impl_wcscmp("vstdlib.dll", name)) {
			printf("[+] vstdlib.dll: %x\n", base);
			counter++;
			g_vstdlib_dll = base;
		}
		if (!impl_wcscmp("inputsystem.dll", name)) {
			printf("[+] inputsystem.dll: %x\n", base);
			counter++;
			g_inputsystem_dll = base;
		}
		if (counter == 4)
			return 1;
	}

	return 0;
}


static uint32_t get_interface_factory(uint32_t base)
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
		if (strcmp("CreateInterface", name))
			continue;

		ehdr = (struct tcp_entry*)(hdr + 1);
		STACK_ADD_VM_COPY64(ehdr, sizeof(struct tcp_entry_copy64), base + a1[1] + (ords * 4), g_process_pml4);
		STACK_ADD_VM_COPY64_P32(ehdr, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), base - 0x6A, g_process_pml4);
		STACK_ADD_VM_COPY64_P32(ehdr, 0, sizeof(uint64_t), 0, g_process_pml4);
		STACK_EXECUTE(hdr, ehdr);
		if (hdr->status != 1)
			break;
			
		return *(uint32_t*)((char *)hdr+20);
	}

	return 0;
}


static uint32_t get_interface(uint32_t factory, const char *name)
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
			printf("[+] %s %x\n", name, interface_address);
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

	printf("[+] client.dll factory %lx\n", factory);

	vt_client = get_interface(factory, "VClient");
	if (vt_client == 0)
		return 0;

	vt_entity = get_interface(factory, "VClientEntityList");
	if (vt_entity == 0)
		return 0;
	
	factory = get_interface_factory(g_engine_dll);
	if (factory == 0)
		return 0;
	printf("[+] engine.dll factory %lx\n", factory);

	vt_engine = get_interface(factory, "VEngineClient");
	if (vt_engine == 0)
		return 0;

	factory = get_interface_factory(g_vstdlib_dll);
	if (factory == 0)
		return 0;

	printf("[+] vstdlib.dll factory %lx\n", factory);
	
	vt_cvar = get_interface(factory, "VEngineCvar");
	if (vt_cvar == 0)
		return 0;
		
	factory = get_interface_factory(g_inputsystem_dll);
	if (factory == 0)
		return 0;

	printf("[+] inputsystem.dll factory %lx\n", factory);
	
	vt_input = get_interface(factory, "InputSystemVersion");
	if (vt_input == 0)
		return 0;


	return 1;
}


static uint32_t vm_dump_netvar_tables(void)
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
		STACK_ADD_VM_COPY_P32(ehdr, 0, sizeof(uint64_t), 0, g_process_pml4, 17);
		STACK_EXECUTE(hdr, ehdr);
		if (hdr->status != 1)
			return 0;
		a0 = *(uint32_t*)((char *)hdr+4);
		a1 = *(uint32_t*)((char *)hdr+12);

		if (!strcmp("DT_BasePlayer", ((char *)hdr+28))) {
			printf("[+] DT_BasePlayer %lx\n", a1);
			counter++;
			DT_BasePlayer = a1;
		}
		if (!strcmp("DT_BaseEntity", ((char *)hdr+28))) {
			printf("[+] DT_BaseEntity %lx\n", a1);
			counter++;
			DT_BaseEntity = a1;
		}
		if (!strcmp("DT_CSPlayer", ((char *)hdr+28))) {
			printf("[+] DT_CSPlayer %lx\n", a1);
			counter++;
			DT_CSPlayer = a1;
		}
		if (!strcmp("DT_BaseAnimating", ((char *)hdr+28))) {
			printf("[+] DT_BaseAnimating %lx\n", a1);
			counter++;
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
		printf("[+] m_iHealth %lx\n", offset);
		counter++;
		m_iHealth = offset;
	}
	if (!strcmp("m_vecViewOffset[0]", name)) {
		printf("[+] m_vecViewOffset %lx\n", offset);
		counter++;
		m_vecViewOffset = offset;
	}
	if (!strcmp("m_lifeState", name)) {
		printf("[+] m_lifeState %lx\n", offset);
		counter++;
		m_lifeState = offset;
	}

	if (!strcmp("m_nTickBase", name) || strcmp("m_nTickBase", name) < -30) {
		printf("[+] m_nTickBase %lx\n", offset);
		counter++;
		m_nTickBase = offset;
	}
	if (!strcmp("m_Local", name)) {
		printf("[+] m_vecPunch %lx\n", offset + 0x70);
		counter++;
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
		printf("[+] m_iTeamNum %lx\n", offset);
		counter++;
		m_iTeamNum = offset;
	}
	if (!strcmp("m_vecOrigin", name)) {
		printf("[+] m_vecOrigin %lx\n", offset);
		counter++;
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
		printf("[+] m_hActiveWeapon %lx\n", offset);
		counter++;
		m_hActiveWeapon = offset;
	}
	if (!strcmp("m_iShotsFired", name)) {
		printf("[+] m_iShotsFired %lx\n", offset);
		counter++;
		m_iShotsFired = offset;
	}
	if (!strcmp("m_bHasDefuser", name)) {
		printf("[+] m_bHasDefuser %lx\n", offset);
		counter++;
		m_bHasDefuser = offset;
		m_iCrossHairID = offset + 0x5C;
	}
	if (!strcmp("m_bIsDefusing", name)) {
		printf("[+] m_bIsDefusing %lx\n", offset);
		counter++;
		m_bIsDefusing = offset;
	}
	if (!strcmp("m_flFlashDuration", name)) {
		printf("[+] m_flFlashDuration %lx\n", offset);
		counter++;
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
		printf("[+] m_dwBoneMatrix %lx\n", offset + 0x1C);
		counter++;
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


	printf("[+] m_dwEntityList %lx\n", m_dwEntityList);
	printf("[+] m_dwClientState %lx\n", m_dwClientState);
	printf("[+] m_dwGetLocalPlayer %lx\n", m_dwGetLocalPlayer);
	printf("[+] m_dwGetViewAngles %lx\n", m_dwGetViewAngles);
	printf("[+] m_dwGetMaxClients %lx\n", m_dwGetMaxClients);
	printf("[+] m_dwState %lx\n", m_dwState);
	printf("[+] m_dwButton %lx\n", m_dwButton);

	return 1;
}

