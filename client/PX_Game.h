#ifndef PX_GAME_H
#define PX_GAME_H
#include "PainterEngine.h"
#include "platform/modules/px_socket.h"
#include  "platform/modules/px_request.h"

typedef struct
{
	px_char name[16];
	px_texture profile256;
	px_texture profile128;
}PX_Game_Player;

px_bool PX_GameInitialize();
px_void PX_GameSetCookie(const px_byte cookie[32]);
px_void PX_GameSetProfile(const px_color profile[]);
px_void PX_GameSetName(const px_char name[16]);
px_void PX_GameUpdate();

px_bool PX_Game_VM_LoadStandard(PX_VM* vm,px_void *ptr);
PX_Game_Player* PX_Game_GetPlayer(px_int index);
px_int PX_Game_GetPlayerCount();
px_void PX_Game_SetMyIndex(px_int index);
px_int PX_Game_GetMyIndex();
px_bool PX_GameLoadFontModuleFile(const px_char* filepath);
px_byte* PX_GameGetFontModuleFileBuffer();
px_int PX_GameGetFontModuleFileSize();

#endif // !PX_GAME_H
