#ifndef CS_H
#define CS_H

#include <inttypes.h>
#include "maths.h"

int32_t cs_initialize(void);

extern uint32_t g_client_dll;
extern uint32_t g_engine_dll;
extern uint32_t g_vstdlib_dll;
extern uint32_t g_inputsystem_dll;

extern uint32_t vt_client;
extern uint32_t vt_entity;
extern uint32_t vt_engine;
extern uint32_t vt_cvar;
extern uint32_t vt_input;


extern uint64_t g_process;
extern uint64_t g_process_pml4;
extern uint32_t m_iHealth;
extern uint32_t m_vecViewOffset;
extern uint32_t m_lifeState;
extern uint32_t m_nTickBase;
extern uint32_t m_vecPunch;
extern uint32_t m_iTeamNum;
extern uint32_t m_vecOrigin;
extern uint32_t m_hActiveWeapon;
extern uint32_t m_iShotsFired;
extern uint32_t m_iCrossHairID;
extern uint32_t m_flFlashDuration;
extern uint32_t m_dwBoneMatrix;
extern uint32_t m_dwEntityList;
extern uint32_t m_dwClientState;
extern uint32_t m_dwGetLocalPlayer;
extern uint32_t m_dwGetViewAngles;
extern uint32_t m_dwGetMaxClients;
extern uint32_t m_dwState;
extern uint32_t m_dwButton;
extern uint32_t m_dwAnalogDelta;
extern uint32_t m_dwAnalog;
extern uint32_t m_bIsDefusing;
extern uint32_t m_bHasDefuser;
extern uint32_t m_dwGetPlayerInfo;


#endif

