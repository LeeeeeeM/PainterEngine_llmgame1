#ifndef PX_OBJECT_LOBBY_H
#define PX_OBJECT_LOBBY_H
#include	"PainterEngine.h"
#include	"PX_Game.h"

#define PX_OBJECT_LOBBY_EVENT_BEGIN_GAME 0x1000

typedef struct
{
	px_texture profile_mask;
	px_texture profile_ring;
	px_texture background;
	px_texture background2;
	px_texture* pcurrentbackground;
	PX_FontModule fm;
	PX_Object*messagebox;
	PX_Json language;
	PX_VM vm;
}PX_Object_lobby;

PX_Object* PX_Object_lobbyCreate(PX_Object* pparent);
px_void PX_Object_lobbyEnable(PX_Object* pObject);
px_void PX_Object_lobbyDisable(PX_Object* pObject);
#endif 
