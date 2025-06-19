#ifndef PX_OBJECT_LOGIN_H
#define PX_OBJECT_LOGIN_H
#include	"PainterEngine.h"
#include	"PX_Game.h"

#define PX_OBJECT_LOGIN_EVENT_OK 0x1000

typedef struct
{
	px_texture profile;
	px_texture profile_tiny;
	px_texture profile_mask;
	px_texture profile_ring;
	px_texture background;
	px_texture tex_joingame;
	PX_FontModule fm;
	PX_Object* button_login,*edit_nickname,*image_profile,*messagebox;
	px_bool is_profile_ok;
	PX_Json language;
	PX_SHA256_HASH sha256;
	PX_VM vm;
}PX_Object_login;

PX_Object* PX_Object_loginCreate(PX_Object* pparent);
px_void PX_Object_loginEnable(PX_Object* pObject);
px_void PX_Object_loginDisable(PX_Object* pObject);
#endif // !PX_OBJECT_LOGIN_H
