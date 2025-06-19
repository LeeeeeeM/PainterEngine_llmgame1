
#include "PX_Object_login.h"
#include "PX_Object_lobby.h"
#include "PX_Object_game.h"

PX_VM game_switcher_vm;
PX_Object* pObject_startup;
PX_Object* pObject_lobby;
PX_Object* pObject_game;
PX_Object* pObject_messagebox;
static PX_FontModule fm;
PX_VM_HOST_FUNCTION(PX_vm_llgame_EnterLogin)
{
	PX_Object_loginEnable(pObject_startup);
	PX_Object_lobbyDisable(pObject_lobby);
	PX_Object_gameDisable(pObject_game);
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_vm_llgame_EnterLobby)
{
	PX_Object_loginDisable(pObject_startup);
	PX_Object_lobbyEnable(pObject_lobby);
	PX_Object_gameDisable(pObject_game);
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_vm_llgame_EnterGame)
{
	PX_Object_loginDisable(pObject_startup);
	PX_Object_lobbyDisable(pObject_lobby);
	PX_Object_gameEnable(pObject_game);
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_vm_llgame_messagebox)
{
	if (PX_VM_HOSTPARAM(Ins,0).type!=PX_VARIABLE_TYPE_STRING)
	{
		return PX_FALSE;
	}
	PX_Object_MessageBoxAlertOk(pObject_messagebox, PX_VM_HOSTPARAM(Ins, 0)._string.buffer, PX_NULL, 0);
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_vm_llgame_SwitcherAnimation)
{
	PX_Object_SwitchEffect2Create(mp, root);
	return PX_TRUE;
	px_int rand = PX_rand() % 2;
	switch (rand)
	{
	case 0:
		PX_Object_SwitchEffectCreate(mp, root, PX_COLOR_BLACK);
		break;
	default:
		PX_Object_SwitchEffect2Create(mp, root);
		break;
	}
	return PX_TRUE;
}

PX_OBJECT_UPDATE_FUNCTION(PX_llgame_update)
{
	PX_VMRun(&game_switcher_vm,0xffff, elapsed);
}

PX_OBJECT_EVENT_FUNCTION(PX_llgame_OnSwitch)
{
	if (!PX_VMIsRuning(&game_switcher_vm))
	{
		PX_VMBeginThreadFunction(&game_switcher_vm, 0, "switcher", 0, 0);
	}
}


px_int main()
{
	PainterEngine_Initialize(1280, 720);
	PX_srand(PX_TimeGetTime());
	if (!PX_GameInitialize())
	{
		PX_LOG("Game initialize failed");
		return 0;
	}

	if (!PX_GameLoadFontModuleFile("assets/font.ttf"))
	{
		PX_LOG("Load font module failed");
		return 0;
	}

	if (!PX_FontModuleInitializeTTFFromStaticBuffer(mp_static,\
		&fm,\
		PX_FONTMODULE_CODEPAGE_UTF8,\
		28,\
		PX_GameGetFontModuleFileBuffer(),\
		PX_GameGetFontModuleFileSize() \
	))
	{
		return 0;
	}

	pObject_startup = PX_Object_loginCreate(root);
	if (!pObject_startup)
	{
		return 0;
	}
	PX_Object_loginDisable(pObject_startup);
	PX_ObjectRegisterEvent(pObject_startup, PX_OBJECT_LOGIN_EVENT_OK, PX_llgame_OnSwitch, 0);


	pObject_lobby = PX_Object_lobbyCreate(root);
	PX_Object_lobbyDisable(pObject_lobby);
	PX_ObjectRegisterEvent(pObject_lobby, PX_OBJECT_LOBBY_EVENT_BEGIN_GAME, PX_llgame_OnSwitch, 0);

	pObject_game = PX_Object_gameCreate(root);
	PX_Object_gameDisable(pObject_game);

	pObject_messagebox = PX_Object_MessageBoxCreate(mp,root,&fm);

	if (!PX_LoadVMFromScriptFile(mp, "assets/switcher.c", &game_switcher_vm, "main"))
	{
		return 0;
	}
	PX_Game_VM_LoadStandard(&game_switcher_vm, PX_NULL);
	PX_VMRegisterHostFunction(&game_switcher_vm, "EnterLogin", PX_vm_llgame_EnterLogin,PX_NULL);
	PX_VMRegisterHostFunction(&game_switcher_vm, "EnterLobby", PX_vm_llgame_EnterLobby, PX_NULL);
	PX_VMRegisterHostFunction(&game_switcher_vm, "EnterGame", PX_vm_llgame_EnterGame, PX_NULL);
	PX_VMRegisterHostFunction(&game_switcher_vm, "SwitcherAnimation", PX_vm_llgame_SwitcherAnimation, PX_NULL);
	PX_VMRegisterHostFunction(&game_switcher_vm, "messagebox", PX_vm_llgame_messagebox, PX_NULL);
	PX_VMBeginThreadFunction(&game_switcher_vm, 0, "main", 0, 0);
	PX_ObjectSetUpdateFunction(root, PX_llgame_update, 0);
	
	
	return 0;
}