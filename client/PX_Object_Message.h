#ifndef PX_OBJECT_RESULT_H
#define  PX_OBJECT_RESULT_H
#include "PainterEngine.h"
#include "PX_Game.h"
typedef struct
{
	PX_Object  *root,*typer;
	px_texture background;
	px_texture render_target;
	px_dword elapsed;
	px_bool isOpen;

	PX_FontModule fm;
}PX_Object_Message;

PX_Object * PX_Object_MessageCreate(PX_Object* parent,px_float x,px_float y);
px_void PX_Object_MessageDisplayOpen(PX_Object* pObject, const px_char message[]);
px_void PX_Object_MessageDisplaySet(PX_Object* pObject, const px_char message[]);
px_void PX_Object_MessageClose(PX_Object* pObject);
px_bool PX_Object_MessageIsEnd(PX_Object* pObject);
px_void PX_Object_MessageSetSpeed(PX_Object* pObject, px_double speed);
#endif