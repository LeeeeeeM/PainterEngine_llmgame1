#ifndef PX_OBJECT_GAME_H
#define PX_OBJECT_GAME_H
#include	"PainterEngine.h"
#include	"PX_Game.h"
#include "PX_Object_Message.h"
typedef struct
{
	PX_Object* counter_down;
	PX_Object* printer;
	PX_Object* waiting;
	PX_Object* select_button[8];
	PX_Object* stamps_loser[8];
	px_texture select_button_texture;
	px_texture texture_loser;
	px_texture texture_ring_128;
	px_texture texture_mask_128;
	px_vector texture_player_profile128;
	px_vector texture_player_profile256;
	PX_Json language;
	px_texture background;
	px_int reg_messageIndex;
	px_abi scene_abi;
	px_abi game_abi;
	px_abi players_abi;
	PX_FontModule fm;
	px_bool isSceneEnd;
	px_int choose;
	px_bool game_is_end;
	PX_VM vm;
}PX_Object_game;

PX_Object* PX_Object_gameCreate(PX_Object* pparent);
px_void PX_Object_gameEnable(PX_Object* pObject);
px_void PX_Object_gameDisable(PX_Object* pObject);
#endif
