#include "PX_Object_lobby.h"

PX_OBJECT_UPDATE_FUNCTION(PX_Object_lobbyUpdate)
{
	PX_Object_lobby* pDesc = PX_ObjectGetDesc(PX_Object_lobby, pObject);
	PX_GameUpdate();
	PX_VMRun(&pDesc->vm, 0xffff, elapsed);
}
PX_OBJECT_RENDER_FUNCTION(PX_Object_lobbyRender)
{
	px_int i;
	px_int x=700, y=300;
	px_rect rect = PX_ObjectGetRect(pObject);
	PX_Object_lobby* pDesc = PX_ObjectGetDesc(PX_Object_lobby, pObject);
	if(pDesc->pcurrentbackground)
		PX_TextureCover(psurface, pDesc->pcurrentbackground, (px_int)rect.x, (px_int)rect.y, PX_ALIGN_LEFTTOP);
	
	for ( i = 0; i < PX_Game_GetPlayerCount(); i++)
	{
		PX_Game_Player* player = PX_Game_GetPlayer(i);
		PX_TextureRender(psurface, &pDesc->profile_ring, x, y, PX_ALIGN_CENTER, 0);
		PX_TextureRenderMask(psurface, &pDesc->profile_mask, &player->profile128, x, y, PX_ALIGN_CENTER, 0);
		PX_TextureRender(psurface, &pDesc->profile_ring, x, y, PX_ALIGN_CENTER, 0);
		PX_FontModuleDrawText(psurface, &pDesc->fm, x, y + 80, PX_ALIGN_CENTER, player->name, PX_COLOR_WHITE);

		x += 200;
		if (x>=1180)
		{
			y += 200;
			x = 700;
		}
	}
}

PX_VM_HOST_FUNCTION(PX_Object_lobby_VM_Messagebox)
{
	PX_Object_lobby* pDesc = PX_ObjectGetDescIndex(PX_Object_lobby, (PX_Object*)userptr, 0);
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_STRING)
	{
		return PX_FALSE;
	}
	if (PX_StringLen(&PX_VM_HOSTPARAM(Ins, 0)._string) != 0)
	{
		PX_Object_MessageBoxAlert(pDesc->messagebox, PX_VM_HOSTPARAM(Ins, 0)._string.buffer);
	}
	else
	{
		PX_Object_MessageBoxClose(pDesc->messagebox);
	}
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_Object_lobby_VM_ReadyBackground)
{
	PX_Object_lobby* pDesc = PX_ObjectGetDescIndex(PX_Object_lobby, (PX_Object*)userptr, 0);
	pDesc->pcurrentbackground = &pDesc->background2;
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_Object_lobby_VM_BeginGame)
{
	PX_Object* pObject = (PX_Object*)userptr;
	PX_ObjectExecuteEvent(pObject, PX_OBJECT_BUILD_EVENT(PX_OBJECT_LOBBY_EVENT_BEGIN_GAME));
	return PX_TRUE;
}


PX_Object* PX_Object_lobbyCreate(PX_Object* pparent)
{
	PX_Object* pObject;
	PX_Object_lobby* pDesc;
	pObject = PX_ObjectCreate(mp, pparent, 0, 0, 0, 0, 0, 0);
	if (!pObject)
	{
		return PX_NULL;
	}
	pDesc = (PX_Object_lobby*)PX_ObjectCreateDesc(pObject, 0, 0, PX_Object_lobbyUpdate, PX_Object_lobbyRender, 0, 0, sizeof(PX_Object_lobby));
	if (!pDesc)
	{
		return PX_NULL;
	}

	if (!PX_LoadTextureFromFile(mp_static, &pDesc->background, "assets/lobby_background.png" ))
	{
		PX_LOG("Load login background failed!");
		return PX_NULL;
	}
	if (!PX_LoadTextureFromFile(mp_static, &pDesc->background2, "assets/lobby_ready.png"))
	{
		PX_LOG("Load login background failed!");
		return PX_NULL;
	}

	if (!PX_LoadTextureFromFile(mp_static, &pDesc->profile_ring, "assets/login_profile_128x128.png"))
	{
		PX_LOG("Load login profile ring failed!");
		return PX_NULL;
	}

	if (!PX_FontModuleInitializeTTFFromStaticBuffer(mp_static, \
		&pDesc->fm, \
		PX_FONTMODULE_CODEPAGE_UTF8, \
		38, \
		PX_GameGetFontModuleFileBuffer(), \
		PX_GameGetFontModuleFileSize() \
	))
	{
		return 0;
	}

	if (!PX_JsonInitialize(mp,&pDesc->language))
	{
		PX_LOG("Load language failed!");
		return PX_NULL;
	}
	if (!PX_LoadJsonFromFile(&pDesc->language, "assets/language.json"))
	{
		PX_LOG("Load language failed!");
		return PX_NULL;
	}

	if (!PX_LoadVMFromScriptFile(mp, "assets/lobby.c", &pDesc->vm, "main"))
	{
		PX_LOG("Load login script failed!");
		return PX_NULL;
	}
	PX_Game_VM_LoadStandard(&pDesc->vm, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "MessageBox", PX_Object_lobby_VM_Messagebox, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "BeginGame", PX_Object_lobby_VM_BeginGame, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "ReadyBackground", PX_Object_lobby_VM_ReadyBackground, pObject);

	if (!PX_TextureCreate(mp_static, &pDesc->profile_mask, 128, 128 ))
	{
		PX_LOG("Load profile failed!");
		return PX_NULL;
	}
	PX_GeoDrawSolidCircle(&pDesc->profile_mask, 64, 64, 64, PX_COLOR_WHITE);
	pDesc->messagebox = PX_Object_MessageBoxCreate(mp,pObject, &pDesc->fm);

	PX_VMBeginThreadFunction(&pDesc->vm, 0, "update", 0, 0);
	return pObject;
}


px_void PX_Object_lobbyEnable(PX_Object* pObject)
{
	PX_Object_lobby* pDesc = PX_ObjectGetDescIndex(PX_Object_lobby, pObject, 0);
	PX_ObjectSetVisible(pObject, PX_TRUE);
	PX_ObjectSetEnabled(pObject, PX_TRUE);
	pDesc->pcurrentbackground = &pDesc->background;
	
}

px_void PX_Object_lobbyDisable(PX_Object* pObject)
{
	PX_ObjectSetVisible(pObject, PX_FALSE);
	PX_ObjectSetEnabled(pObject, PX_FALSE);
}
