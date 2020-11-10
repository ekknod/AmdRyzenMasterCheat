#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "socket.h"
#include "server.h"
#include "cs.h"
#include "maths.h"
#include <cmath>

static int tick_ms;

int g_local_index, g_target_id;
uint32_t g_previous_tick;
uint32_t g_local_address;
uint32_t g_target_address;
static vec3 g_eyepos;
static vec3 g_vecpunch;
static uint32_t g_shots_fired;
static uint32_t g_current_tick;
static vec3     g_viewangles;
static int      g_target_health;
static uint32_t g_button_0, g_button_1, g_button_2;
static vec3     g_mouse_delta, g_mouse_analog;
static vec3     g_target_bone;
static int      prev_tick;
static int      g_aimkey_0, g_aimkey_1, g_aimkey_2;
static int      tick_calculate;
char g_steamid[33]={0};
static uint32_t get_best_target(void);
static bool     get_player_information(void);
static vec3     get_target_angle(void);
static void     aimbot(vec3 vangle, vec3 angle, float fov, float smooth, float sensitivity);


static int     headonly;
static int     incross;

#if PLATFORM == PLATFORM_WINDOWS

#include <windows.h>

static int wait_thread;
int thread_args[2];
static void thread_beep()
{
	Beep(thread_args[0], thread_args[1]);
	wait_thread = 0;
}

void BEEP(int freq, int ms)
{
	if (wait_thread == 0) {
		thread_args[0] = freq;
		thread_args[1] = ms;
		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)thread_beep, 0, 0, 0));
		wait_thread = 1;
	}
}


#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
void BEEP(int freq, int ms)
{
}

#include <unistd.h>

#endif





INT64 NtRand()
{
	LARGE_INTEGER counter;
#ifdef _KERNEL_MODE
	_KeQueryPerformanceCounter(&counter);

	
	return _RtlRandomEx((PULONG)&counter.QuadPart);
#else
	QueryPerformanceCounter(&counter);
	srand((unsigned int)counter.QuadPart);
	return rand();
#endif
}




#include <iostream>
#include <string>

#define DEFAULT_KEY "AlpcFreeCompletionListMessage"

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
    return 1;
}
int max_players, aim_time;
extern SOCKET g_socket;
int main(int argc, char **argv)
{





	float fov;
	float smooth;
	float sensitivity;
	std::string ip_address;



	char szPath[260];
	GetCurrentDirectoryA(260, szPath);
	std::string path(szPath + std::string("\\config.cfg"));


	char buffer_config[260] = { 0 };
	GetPrivateProfileStringA("settings", "ip", "192.168.8.106", buffer_config, 260, path.c_str());
	ip_address = buffer_config;
	GetPrivateProfileStringA("settings", "max_server_players", "20", buffer_config, 260, path.c_str());
	max_players = std::atoi(buffer_config);
	GetPrivateProfileStringA("aimbot", "key", "108", buffer_config, 260, path.c_str());
	g_aimkey_0 = std::atoi(buffer_config);
	GetPrivateProfileStringA("aimbot", "key2", "107", buffer_config, 260, path.c_str());
	g_aimkey_2 = std::atoi(buffer_config);
	GetPrivateProfileStringA("aimbot", "fov", "20", buffer_config, 260, path.c_str());
	fov = (float)std::atof(buffer_config) * 100.0f;
	GetPrivateProfileStringA("aimbot", "smooth", "20", buffer_config, 260, path.c_str());
	smooth = (float)std::atof(buffer_config);
	GetPrivateProfileStringA("aimbot", "horizontal_mode", "0", buffer_config, 260, path.c_str());
	headonly = std::atoi(buffer_config);
	GetPrivateProfileStringA("aimbot", "aimtime", "100", buffer_config, 260, path.c_str());
	aim_time = std::atoi(buffer_config);
	GetPrivateProfileStringA("settings", "sensitivity", "2.5", buffer_config, 260, path.c_str());
	sensitivity = (float)std::atof(buffer_config);
	GetPrivateProfileStringA("triggerbot", "key", "111", buffer_config, 260, path.c_str());
	g_aimkey_1 = std::atoi(buffer_config);
	GetPrivateProfileStringA("triggerbot", "incross", "1", buffer_config, 260, path.c_str());
	incross = std::atoi(buffer_config);


	if (!connect_server(ip_address.c_str(), 30609,"ClpWFCeeYomBlQtToDAisHMeJsQgX")) {
		std::cout << "server not found!" << std::endl;
		return 0;
	}


	if (!server_initialize()) {
		std::cout << "failed to initialize server" << std::endl;
		return 0;
	}

	if (!cs_initialize()) {
		std::cout << "failed to initialize game" << std::endl;
		return 0;
	}

	std::cout << "loaded succesfully" << std::endl;

	int prev_state = 0, prev_tick = 0;
	while (1) {
		vec3 target_angle;
		if (g_target_address == 0) {
			uint32_t result = get_best_target();
			if (result == -1)
				goto E0;

			if (result == 0)
				continue;
		}

		if (!get_player_information()) {
			g_target_address = 0;
			tick_calculate = 0;
			continue;
		}

		if (!g_target_health) {
			g_target_address = 0;
			tick_calculate = 0;
			continue;
		}

		target_angle = get_target_angle();
		aimbot(g_viewangles, target_angle, fov, smooth, sensitivity);
	}


E0:
	socket_close();
}


static uint32_t get_best_target(void)
{
	float best_fov = 360.0f;
	char buffer[1400];
	struct tcp_entry  *e;
	struct tcp_header *h;
	vec3 va;
	vec3 local_pos;
	vec3 entity_pos;
	

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
	STACK_ADD_VM_COPY64_P32(e, sizeof(struct tcp_entry_copy64), sizeof(uint64_t)*2, m_vecOrigin, g_process_pml4);
	STACK_ADD_VM_COPY64_P32(e, 0, sizeof(uint64_t)*3, m_iHealth, g_process_pml4);
	STACK_EXECUTE(h, e);
	if (h->status != 1)
		return 0;
	
	g_local_address = *(int*)((char*)h + 4);
	int local_team = *(int*)((char*)h + 12);
	*(uint64_t*)&local_pos = *(uint64_t*)((char*)h + 20);

	uint32_t temp_entity = 0;
	for (int i = 0; i < max_players; i++) {
		float angle[3], fov;

		e = (struct tcp_entry*)(h + 1);
		STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), m_dwEntityList + i * 0x10, g_process_pml4);
		STACK_ADD_VM_COPY64_P32(e, sizeof(struct tcp_entry_copy64), sizeof(uint64_t), m_iTeamNum, g_process_pml4);
		STACK_ADD_VM_COPY64_P32(e, sizeof(struct tcp_entry_copy64), sizeof(uint64_t)*2, m_iHealth, g_process_pml4);
		STACK_ADD_VM_COPY64_P32(e, sizeof(struct tcp_entry_copy64), sizeof(uint64_t)*3, m_lifeState, g_process_pml4);
		STACK_ADD_VM_COPY64_P32(e, sizeof(struct tcp_entry_copy64), sizeof(uint64_t)*4, m_vecOrigin, g_process_pml4);
		STACK_ADD_VM_COPY64_P32(e, sizeof(struct tcp_entry_copy64), sizeof(uint64_t)*5, 0xED, g_process_pml4);
		STACK_ADD_VM_COPY64_P32(e, 0, sizeof(uint64_t)*6, m_bIsDefusing, g_process_pml4);
		STACK_EXECUTE(h, e);

		if (h->status != 1)
			continue;
		
		int entity_address = *(int*)((char*)h + 4);
		int entity_team = *(int*)((char*)h + 12);
		int entity_health = *(int*)((char*)h + 20);
		int entity_lifestate = *(int*)((char*)h + 28);
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

		
		local_pos.z = 0;
		entity_pos.z = 0;
		CalcAngle((float*)&local_pos, (float*)&entity_pos, angle);
		vec_clamp((vec3*)&angle);
		fov = get_fov(va, *(vec3*)&angle);
		if (fov < best_fov) {
			best_fov = fov;
			g_target_address = entity_address;
			g_target_id = i;
		}
	}
	
	if (temp_entity != 0) {
		e = (struct tcp_entry*)(h + 1);
		STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), temp_entity + m_bIsDefusing, g_process_pml4);
		STACK_ADD_VM_COPY64(e, 0, temp_entity + m_bHasDefuser, g_process_pml4);
		STACK_EXECUTE(h, e);
		if (*(bool*)((char*)h + 4) == false) {
			goto E0;
		}
		if (*(bool*)((char*)h + 12)) {
		    BEEP(200, 100);
		} else {
		    BEEP(200, 300);
		}
	}
E0:
	return 360.0f != best_fov;
}




static bool get_player_information(void)
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
	STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), vt_input + (((g_aimkey_0 >> 5 ) * 4) + m_dwButton), g_process_pml4);
	STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), vt_input + (((g_aimkey_1 >> 5 ) * 4) + m_dwButton), g_process_pml4);
	STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), vt_input + (((g_aimkey_2 >> 5 ) * 4) + m_dwButton), g_process_pml4);
	STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), g_target_address + 0xED, g_process_pml4);;
	STACK_ADD_VM_COPY64(e, sizeof(struct tcp_entry_copy64), g_target_address + m_dwBoneMatrix, g_process_pml4);
	STACK_ADD_VM_COPY_P32(e, 0, sizeof(uint64_t), (0x30 * 8), g_process_pml4, 48);
	STACK_EXECUTE(h, e);
	if (h->status != 1)
		return 0;
	g_eyepos = *(vec3*)((char*)h + 4);
	g_eyepos.z += *(float*)((char*)h + 16);


	*(uint64_t*)&g_vecpunch = *(uint64_t*)((char*)h + 24);
	g_shots_fired = *(uint32_t*)((char*)h + 32);
	g_current_tick = *(uint32_t*)((char*)h + 40);
	*(uint64_t*)&g_viewangles = *(uint64_t*)((char*)h + 48);
	g_target_health = *(uint32_t*)((char*)h + 56);
	g_button_0 = (*(uint32_t*)((char*)h + 64) >> (g_aimkey_0 & 31)) & 1;
	g_button_1 = (*(uint32_t*)((char*)h + 72) >> (g_aimkey_1 & 31)) & 1;
	g_button_2 = (*(uint32_t*)((char*)h + 80) >> (g_aimkey_2 & 31)) & 1;
	bool tester = *(bool*)((char*)h + 88);
	g_target_bone.x = (*(matrix3x4_t*)((char*)h + 96+8))[0][3];
	g_target_bone.y = (*(matrix3x4_t*)((char*)h + 96+8))[1][3];
	g_target_bone.z = (*(matrix3x4_t*)((char*)h + 96+8))[2][3];

	if(g_target_health < 45) {
		tick_ms = 200;
	} else {
		tick_ms = 400;
	}


	if (g_button_1 && incross) {
		h = (struct tcp_header*)buffer;
		e = (struct tcp_entry*)(h + 1);
		STACK_ADD_VM_COPY64(e, /*sizeof(struct tcp_entry_copy64)*/0, g_local_address + m_iCrossHairID, g_process_pml4);
		STACK_EXECUTE(h, e);
		if (h->status != 1) {
			g_target_address = 0;
			tick_calculate = 0;
			return 0;
		}
		int g_crosshair_id = *(int*)((char*)h + 4);
		if (g_crosshair_id > 0 && g_crosshair_id <= 64 && g_current_tick - prev_tick > 3) {
			prev_tick = g_current_tick;
			server_move_mouse(1, 0, 0, 0);
			#if PLATFORM == PLATFORM_WINDOWS


			Sleep(NtRand() % (100-50 + 1) + 50);
			#else
			usleep(50000);
			#endif
			server_move_mouse(0, 0, 0, 0);
		}
	}
	


	if (tester || g_target_health < 1) {
		g_target_address = 0;
		tick_calculate = 0;
		return 0;
	}


	return 1;
}


static vec3 get_target_angle(void)
{
	vec3 m = g_target_bone;
	vec3 c = g_eyepos;
	c.x = m.x - c.x;
	c.y = m.y - c.y;
	c.z = m.z - c.z;
	vec_normalize(&c);
	vec_angles(c, &c);

	if (g_shots_fired > 1) {
		c.x = c.x - g_vecpunch.x * 2.0f;
		c.y = c.y - g_vecpunch.y * 2.0f;
		c.z = c.z - g_vecpunch.z * 2.0f;
	}

	vec_clamp(&c);
	return c;
}


#define DEG2RAD(x) ((float)(x) * (float)(3.14159265358979323846f / 180.f))





static void aimbot(vec3 vangle, vec3 angle, float fov, float smooth, float sensitivity)
{
	float x, y, sx, sy;
	float dist = vec_distance(g_eyepos, g_target_bone);
	float pitch = (float)(sin(DEG2RAD(vangle.x - angle.x)) * dist );
	float yaw = (float) (sin(DEG2RAD(vangle.y - angle.y)) * dist );
	float dist_x = std::fabs(yaw);
	float dist_y = std::fabs(pitch);
	float tmp_fov = get_fov(vangle, angle);
	float dist_fov = get_fov_distance(vangle, angle, dist);

	if (dist_fov >= 4500.0f) {
		g_target_address = 0;
		tick_calculate = 0;
		return;
	}


	BEEP(400, tick_ms);





	if ( !g_button_0 && !g_button_2 && !g_button_1) {
		tick_calculate = 0;
		return;
	}


	if (dist_fov >= fov)
		return;

	if (aim_time && g_button_0 && dist_fov <= 550.0f)
		return;

	if (dist_fov <= 50.0f)
		return;


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

	if (!headonly) {
		if (pitch <= -20.0f)
			return;
		sy = 0;
	}

	if ((g_button_0 || g_button_1 || g_button_2) && g_current_tick - g_previous_tick > 0) {
		tick_calculate++;
		g_previous_tick = g_current_tick;
		server_move_mouse(0, (char) sx, (char)sy, 0);
	}

	if (!incross && g_button_1) {
		// pitch >= -20.0f
		// dist_x <= 3.5 

		if (dist_x <= 3.5f && pitch >= -5.5f) {

			DWORD required_ticks = NtRand() % (450 - 380 + 1) + 450;
			if (g_current_tick - prev_tick > required_ticks) {
				prev_tick = g_current_tick;
				server_move_mouse(1, 0, 0, 0);
				Sleep(NtRand() % (125-50 + 1) + 125);
				server_move_mouse(0, 0, 0, 0);
			}

		}

		
	}
}


static float clamp(float x, float min, float max)
{
    if (x < min) x = min;
    if (x > max) x = max;
    return x;
}

