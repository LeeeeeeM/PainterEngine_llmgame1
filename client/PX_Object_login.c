#include "PX_Object_login.h"

PX_OBJECT_UPDATE_FUNCTION(PX_Object_loginUpdate)
{
	PX_Object_login* pDesc = PX_ObjectGetDesc(PX_Object_login, pObject);
	
	PX_GameUpdate();

	PX_VMRun(&pDesc->vm, 0xffff, elapsed);
}
PX_OBJECT_RENDER_FUNCTION(PX_Object_loginRender)
{
	px_rect rect = PX_ObjectGetRect(pObject);
	PX_Object_login* pDesc = PX_ObjectGetDesc(PX_Object_login, pObject);
	pDesc->button_login->x = psurface->width / 2.f;
	pDesc->button_login->y = psurface->height - 128.f;
	pDesc->edit_nickname->x = psurface->width / 2.f;
	pDesc->edit_nickname->y = psurface->height - 228.f;
	pDesc->image_profile->x = psurface->width / 2.f;
	pDesc->image_profile->y = psurface->height - 396.f;

	PX_TextureCover(psurface, &pDesc->background, (px_int)rect.x, (px_int)rect.y, PX_ALIGN_LEFTTOP);
	if (!pDesc->is_profile_ok)
	{
		PX_GeoDrawSolidCircle(psurface, (px_int)pDesc->image_profile->x, (px_int)pDesc->image_profile->y, 120, PX_COLOR_WHITE);
		PX_FontModuleDrawText(psurface, &pDesc->fm, (px_int)pDesc->image_profile->x, (px_int)pDesc->image_profile->y, PX_ALIGN_CENTER, PX_JsonGetString(&pDesc->language, "select profile"), PX_COLOR_BLACK);
	}
	else
	{
		PX_TextureRenderMask(psurface, &pDesc->profile_mask, &pDesc->profile_tiny, (px_int)pDesc->image_profile->x, (px_int)pDesc->image_profile->y, PX_ALIGN_CENTER, 0);
	}
}


#ifdef __EMSCRIPTEN__
extern const px_char* PX_InputText(const px_char prompt[], const px_char charset[]);
PX_OBJECT_EVENT_FUNCTION(PX_Object_loginEditInput)
{
	if (PX_ObjectIsCursorInRegion(pObject,e))
	{
		PX_Object_login* pDesc = PX_ObjectGetDescIndex(PX_Object_login,(PX_Object *) ptr, 0);
		const px_char* ptext = PX_InputText(PX_JsonGetString(&pDesc->language, "input your name"),"UTF8");
		if (PX_strlen(ptext)>=16)
		{
			PX_Object_MessageBoxAlert(pDesc->messagebox, PX_JsonGetString(&pDesc->language, "name too long"));
		}
		else
		{
			if (ptext)
			{
				PX_Object_EditSetText(pDesc->edit_nickname, ptext);
				PX_Object_EditSetFocus(pDesc->edit_nickname, PX_FALSE);
			}
			else
			{
				PX_Object_EditSetFocus(pDesc->edit_nickname, PX_FALSE);
			}
		}
	}
}
#endif
px_byte loginSelectProfile_cache[1024 * 1024];

PX_REQUEST_FUNCTION(PX_Object_loginSelectProfile_callback)
{
	PX_Object_login* pDesc = PX_ObjectGetDescIndex(PX_Object_login, (PX_Object *)ptr, 0);
	if (size!=0)
	{
		if (pDesc->profile.MP)
		{
			PX_TextureFree(&pDesc->profile);
			PX_memset(&pDesc->profile, 0, sizeof(px_texture));
		}
		if (PX_TextureCreateFromMemory(mp_static,buffer,size,&pDesc->profile))
		{
			if (PX_TextureScaleToTexture(&pDesc->profile, &pDesc->profile_tiny))
			{
				PX_Sha256Calculate((px_void*)loginSelectProfile_cache, size, &pDesc->sha256);
				pDesc->is_profile_ok = PX_TRUE;
			}
		}
		
	}
	else
	{
		PX_Object_MessageBoxAlertOk(pDesc->messagebox,  PX_JsonGetString(&pDesc->language, "invalid profile"), 0,0);
	}
}

PX_OBJECT_EVENT_FUNCTION(PX_Object_loginSelectProfile)
{
	if (PX_ObjectIsCursorInRegion(pObject,e))
	{
		PX_RequestData("open", loginSelectProfile_cache, sizeof(loginSelectProfile_cache), ptr, PX_Object_loginSelectProfile_callback);
	}
}

PX_OBJECT_EVENT_FUNCTION(PX_Object_loginExecute)
{
	PX_Object_login* pDesc = PX_ObjectGetDescIndex(PX_Object_login, (PX_Object*)ptr, 0);
	const px_char* pnickname;
	if (PX_VMIsRuning(&pDesc->vm))
	{
		return;
	}
	if (!pDesc->is_profile_ok)
	{
		PX_Object_MessageBoxAlertOk(pDesc->messagebox, PX_JsonGetString(&pDesc->language, "please select profile"), 0, 0);
		return;
	}
	pnickname = PX_Object_EditGetText(pDesc->edit_nickname);
	if (!pnickname[0])
	{
		PX_Object_MessageBoxAlertOk(pDesc->messagebox, PX_JsonGetString(&pDesc->language, "please input your name"), 0, 0);
		return;
	}

	PX_GameSetCookie(pDesc->sha256.bytes);
	PX_GameSetProfile(pDesc->profile_tiny.surfaceBuffer);
	PX_GameSetName(pnickname);
	PX_VMBeginThreadFunction(&pDesc->vm, 0, "login", 0, 0);
}

PX_VM_HOST_FUNCTION(PX_Object_login_VM_Messagebox)
{
	PX_Object_login* pDesc = PX_ObjectGetDescIndex(PX_Object_login, (PX_Object*)userptr, 0);
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
PX_VM_HOST_FUNCTION(PX_Object_login_VM_loginok)
{
	PX_Object* pObject = (PX_Object*)userptr;
	PX_ObjectExecuteEvent(pObject, PX_OBJECT_BUILD_EVENT(PX_OBJECT_LOGIN_EVENT_OK));
	return PX_TRUE;
}
PX_Object* PX_Object_loginCreate(PX_Object* pparent)
{
	PX_Object* pObject;
	PX_Object_login* pDesc;
	pObject = PX_ObjectCreate(mp, pparent, 0, 0, 0, 0, 0, 0);
	if (!pObject)
	{
		return PX_NULL;
	}
	pDesc = (PX_Object_login*)PX_ObjectCreateDesc(pObject, 0, 0, PX_Object_loginUpdate, PX_Object_loginRender, 0, 0, sizeof(PX_Object_login));
	if (!pDesc)
	{
		return PX_NULL;
	}

	if (!PX_LoadTextureFromFile(mp_static, &pDesc->background, "assets/login_background.png" ))
	{
		PX_LOG("Load login background failed!");
		return PX_NULL;
	}

	if (!PX_LoadTextureFromFile(mp_static, &pDesc->profile_ring, "assets/login_profile_ring.png"))
	{
		PX_LOG("Load login profile ring failed!");
		return PX_NULL;
	}

	if (!PX_LoadTextureFromFile(mp_static, &pDesc->tex_joingame, "assets/button.png"))
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

	if (!PX_LoadVMFromScriptFile(mp, "assets/login.c", &pDesc->vm, "main"))
	{
		PX_LOG("Load login script failed!");
		return PX_NULL;
	}
	PX_Game_VM_LoadStandard(&pDesc->vm, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "MessageBox", PX_Object_login_VM_Messagebox, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "LoginOk", PX_Object_login_VM_loginok, pObject);

	pDesc->button_login = PX_Object_PushButtonCreate(mp, pObject, 0, 0, pDesc->tex_joingame.width, pDesc->tex_joingame.height, PX_JsonGetString(&pDesc->language, "join game"), &pDesc->fm);
	PX_Object_PushButtonSetTexture(pDesc->button_login, &pDesc->tex_joingame);
	PX_Object_PushButtonSetBorder(pDesc->button_login, PX_FALSE);
	PX_ObjectSetAlign(pDesc->button_login, PX_ALIGN_CENTER);
	PX_ObjectRegisterEvent(pDesc->button_login, PX_OBJECT_EVENT_EXECUTE, PX_Object_loginExecute, pObject);

	pDesc->edit_nickname = PX_Object_EditCreate(mp, pObject, 0, 0, 256, 46, &pDesc->fm);
	PX_ObjectSetAlign(pDesc->edit_nickname, PX_ALIGN_CENTER);
	PX_Object_EditSetTips(pDesc->edit_nickname, PX_JsonGetString(&pDesc->language, "input your name"));
	PX_Object_EditSetMaxTextLength(pDesc->edit_nickname, 16);
	PX_Object_EditSetBackgroundColor(pDesc->edit_nickname, PX_COLOR_WHITE);
	PX_ObjectSetVisible(pObject, PX_FALSE);

	if (!PX_TextureCreate(mp_static, &pDesc->profile_tiny, 256, 256))
	{
		PX_LOG("Load profile failed!");
		return PX_NULL;
	}

	if (!PX_TextureCreate(mp_static, &pDesc->profile_mask, 256, 256 ))
	{
		PX_LOG("Load profile failed!");
		return PX_NULL;
	}
	PX_GeoDrawSolidCircle(&pDesc->profile_mask, 128, 128, 128, PX_COLOR_WHITE);

#ifdef __EMSCRIPTEN__
	PX_ObjectRegisterEvent(pDesc->edit_nickname, PX_OBJECT_EVENT_CURSORDOWN, PX_Object_loginEditInput, pObject);
#endif

	pDesc->image_profile = PX_Object_ImageCreate(mp, pObject, 0, 0, pDesc->profile_ring.width, pDesc->profile_ring.height,&pDesc->profile_ring);
	PX_ObjectSetAlign(pDesc->image_profile, PX_ALIGN_CENTER);
	PX_ObjectRegisterEvent(pDesc->image_profile, PX_OBJECT_EVENT_CURSORDOWN, PX_Object_loginSelectProfile, pObject);

	pDesc->messagebox = PX_Object_MessageBoxCreate(mp,pObject, &pDesc->fm);
	return pObject;
}

px_void PX_Object_loginEnable(PX_Object* pObject)
{
	PX_ObjectSetVisible(pObject, PX_TRUE);
	PX_ObjectSetEnabled(pObject, PX_TRUE);
}

px_void PX_Object_loginDisable(PX_Object* pObject)
{
	PX_ObjectSetVisible(pObject, PX_FALSE);
	PX_ObjectSetEnabled(pObject, PX_FALSE);
}
