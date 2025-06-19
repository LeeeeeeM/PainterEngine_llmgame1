#ifndef PX_GAME_H
#define PX_GAME_H
#include "PainterEngine.h"
#include "platform/modules/px_sysmemory.h"
#include "platform/modules/px_thread.h"
#include "platform/modules/px_sockethub.h"

typedef struct
{
	px_byte socket_session[16];
	px_bool online;
	px_abi  userinfo;
}PX_Game_Client;

typedef struct
{
	px_memorypool mp;
	px_abi        game;
	px_abi        scene;
	px_int        scene_id;
	px_abi        players;
	px_abi        resources;
	px_vector     clients;
	PX_VM         vm;
}PX_Game;

typedef enum
{
	PX_GAME_LOGIN_RETURN_OK,	
	PX_GAME_LOGIN_RETURN_FULL,
	PX_GAME_LOGIN_RETURN_ERROR,
}PX_GAME_LOGIN_RETURN;

px_bool PX_Games_Initialize();
px_void PX_Games_Update(px_dword elapsed);

px_bool PX_Game_Initialize(PX_Game* pGame);
px_int PX_Game_GetClientIndex(PX_Game* pGame, px_abi* pabi);
PX_GAME_LOGIN_RETURN PX_Game_Login(PX_Game* pGame, const px_byte socket_session[16], px_abi* pabi);
px_bool  PX_Game_RemoveClient(PX_Game* pGame, px_int Index);


#endif // !PX_GAME_H
