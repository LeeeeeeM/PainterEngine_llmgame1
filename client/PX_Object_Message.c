#include "PX_Object_Message.h"


PX_OBJECT_RENDER_FUNCTION(PX_Object_MessageRender)
{
	PX_Object_Message* pdesc = PX_ObjectGetDesc(PX_Object_Message, pObject);
	px_float x, y;
	px_rect rect;
	PX_TEXTURERENDER_BLEND blend;
	blend.alpha = 0.85f;
	blend.hdr_R = 1;
	blend.hdr_G = 1;
	blend.hdr_B = 1;
	rect = PX_ObjectGetRect(pObject);
	x = rect.x;
	y = rect.y;
	
	PX_TextureClearAll(&pdesc->render_target, PX_COLOR_NONE);;
	PX_TextureRender(&pdesc->render_target, &pdesc->background,0, 0, PX_ALIGN_LEFTTOP, &blend);
	PX_ObjectRender(&pdesc->render_target, pdesc->root, elapsed);
	
	if (pdesc->elapsed<3000)
	{
		px_float schedular1 = pdesc->elapsed / 3000.f*8;
		px_float schedular2 = pdesc->elapsed / 3000.f*2;
		if (schedular1>=1)
		{
			schedular1 = 1;
		}
		if (schedular2 >= 1)
		{
			schedular2 = 1;
		}
		if (!pdesc->isOpen)
		{
			schedular1 = 1 - schedular1;
			schedular2 = 1 - schedular2;
		}

		PX_TextureRenderEx(psurface, &pdesc->render_target, (px_int)(pObject->x), (px_int)(pObject->y), PX_ALIGN_CENTER, 0,schedular1,0);
		pdesc->elapsed += elapsed;
		
	}
	else
	{
		if (pdesc->isOpen)
		{
			PX_TextureRender(psurface, &pdesc->render_target, (px_int)(pObject->x), (px_int)(pObject->y), PX_ALIGN_CENTER, 0);
		}
		else
		{
			PX_ObjectSetVisible(pObject, PX_FALSE);
		}
	}
	
}

PX_Object* PX_Object_MessageCreate(PX_Object* parent, px_float x, px_float y)
{
	PX_Object* pObject = PX_ObjectCreateEx(mp, parent, x, y, 0, 0, 0, 0, 0, 0, PX_Object_MessageRender, 0, 0, sizeof(PX_Object_Message));
	PX_Object_Message* pdesc = PX_ObjectGetDescIndex(PX_Object_Message, pObject, 0);
	if (pObject == PX_NULL)
	{
		return PX_NULL;
	}
	if (!PX_FontModuleInitializeTTFFromStaticBuffer(mp_static, \
		&pdesc->fm, \
		PX_FONTMODULE_CODEPAGE_UTF8, \
		28, \
		PX_GameGetFontModuleFileBuffer(), \
		PX_GameGetFontModuleFileSize() \
	))
	{
		return 0;
	}

	if (!PX_LoadTextureFromFile(mp_static, &pdesc->background, "assets/message.png"))
	{
		PX_ASSERTX("LoadTextureFromFile failed");
		PX_ObjectDelete(pObject);
		return PX_NULL;
	}

	if (!PX_TextureCreate(mp_static, &pdesc->render_target, pdesc->background.width, pdesc->background.height))
	{
		PX_ASSERTX("LoadTextureFromFile failed");
		PX_ObjectDelete(pObject);
		return PX_NULL;
	}

	
	pdesc->root = PX_ObjectCreateRoot(mp);
	pdesc->typer = PX_Object_TyperCreate(mp, pdesc->root, 40, 180, pdesc->background.width-40, &pdesc->fm);
	PX_Object_TyperSetYSpacer(pdesc->typer, 48);
	return pObject;
}

px_void PX_Object_MessageDisplayOpen(PX_Object* pObject, const px_char message[])
{
	PX_Object_Message* pdesc = PX_ObjectGetDescIndex(PX_Object_Message, pObject, 0);
	pdesc->elapsed = 0;
	pdesc->isOpen = PX_TRUE;
	PX_Object_TyperReset(pdesc->typer);
	PX_Object_TyperClear(pdesc->typer);
	PX_Object_TyperPrintPayload(pdesc->typer, message);
	PX_Object_TyperSetSpeed(pdesc->typer, 1);
	PX_ObjectSetVisible(pObject, PX_TRUE);
}

px_void PX_Object_MessageDisplaySet(PX_Object* pObject, const px_char message[])
{
	PX_Object_Message* pdesc = PX_ObjectGetDescIndex(PX_Object_Message, pObject, 0);
	pdesc->isOpen = PX_TRUE;
	PX_Object_TyperReset(pdesc->typer);
	PX_Object_TyperClear(pdesc->typer);
	PX_Object_TyperPrintPayload(pdesc->typer, message);
	PX_ObjectSetVisible(pObject, PX_TRUE);
	
}

px_void PX_Object_MessageClose(PX_Object* pObject)
{
	PX_Object_Message* pdesc = PX_ObjectGetDescIndex(PX_Object_Message, pObject, 0);
	pdesc->elapsed = 0;
	pdesc->isOpen = PX_FALSE;
}

px_bool PX_Object_MessageIsEnd(PX_Object* pObject)
{
	PX_Object_Message* pdesc = PX_ObjectGetDescIndex(PX_Object_Message, pObject, 0);
	return PX_Object_TyperIsEnd(pdesc->typer);
}

px_void PX_Object_MessageSetSpeed(PX_Object* pObject, px_double speed)
{
	PX_Object_Message* pdesc = PX_ObjectGetDescIndex(PX_Object_Message, pObject, 0);
	PX_Object_TyperSetSpeed(pdesc->typer, speed);
}
