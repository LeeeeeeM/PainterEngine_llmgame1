#include "PX_Game.h"
PX_Socket game_socket;
px_int game_my_index;
px_byte  game_cookie[32];
px_color game_profile[256 * 256];
px_char user_name[16];
px_memorypool fifo_cache_mp;
px_byte  fifo_cache[1024 * 1024 * 32];
px_byte* fifo_pointers[64];
PX_VM* pCurrentCallVM = PX_NULL;

px_int   fifo_datasize[PX_COUNTOF(fifo_pointers)];
px_int   fifo_wcursor, fifo_rcursor;
px_vector  game_players;


PX_SOCKET_CONNECT_CALLBACK_FUNCTION(px_game_socket_onconnect)
{
	printf("connected\n");
}

PX_SOCKET_RECEIVE_CALLBACK_FUNCTION(px_game_socket_onrecv)
{
	px_memory inflate_data;
	PX_MemoryInitialize(&fifo_cache_mp,&inflate_data);
	if (PX_RFC1951Inflate(data,datasize,&inflate_data))
	{
		if (fifo_pointers[fifo_wcursor] == 0)
		{
			fifo_pointers[fifo_wcursor] = (px_byte*)MP_Malloc(&fifo_cache_mp, inflate_data.usedsize);
			if (fifo_pointers[fifo_wcursor] == PX_NULL)
			{
				return;
			}
			fifo_datasize[fifo_wcursor] = inflate_data.usedsize;
			PX_memcpy(fifo_pointers[fifo_wcursor], inflate_data.buffer, inflate_data.usedsize);
			fifo_wcursor++;
			fifo_wcursor %= PX_COUNTOF(fifo_pointers);
		}
	}
	PX_MemoryFree( &inflate_data);
	
}

PX_SOCKET_SEND_CALLBACK_FUNCTION(px_game_socket_onsend)
{

}

PX_SOCKET_DISCONNECT_CALLBACK_FUNCTION(px_game_socket_ondisconnect)
{
	printf("disconnected\n");
	if (pCurrentCallVM)
	{
		if (pCurrentCallVM->Suspend)
		{
			PX_VM_RET_memory(pCurrentCallVM, PX_NULL, 0);
			PX_VMContinue(pCurrentCallVM);
		}
	}
	
}

#ifdef __EMSCRIPTEN__
#define PX_GAME_HOST "wss://painterengine.com"
#define PX_GAME_PORT 31481
#else
#define PX_GAME_HOST "127.0.0.1"
#define PX_GAME_PORT 31460
#endif


px_bool PX_GameInitialize()
{
	PX_VectorInitialize(mp,&game_players, sizeof(PX_Game_Player), 16);
	if (!PX_SocketInitialize(&game_socket,16*1024*1024, PX_GAME_HOST, PX_GAME_PORT, px_game_socket_onconnect, px_game_socket_onrecv, px_game_socket_onsend, px_game_socket_ondisconnect,0))
	{
		printf("socket initialize failed");
		return PX_FALSE;
	}
	fifo_cache_mp = PX_MemorypoolCreate(fifo_cache, sizeof(fifo_cache));
	//auto reconnect
	PX_SocketConnect(&game_socket, PX_TRUE);
	return PX_TRUE;
}

px_void PX_GameSetCookie(const px_byte cookie[32])
{
	PX_memcpy(game_cookie, cookie, sizeof(px_byte) * 32);
}

px_void PX_GameSetProfile(const px_color profile[])
{
	PX_memcpy(game_profile, profile, sizeof(px_color) * 256 * 256);
}

px_void PX_GameSetName(const px_char name[16])
{
	PX_strcpy(user_name, name, sizeof(user_name));
}

px_bool PX_GameGet(px_abi* pabi)
{
	if (fifo_pointers[fifo_rcursor] != PX_NULL)
	{
		PX_AbiCreate_StaticReader(pabi, fifo_pointers[fifo_rcursor], fifo_datasize[fifo_rcursor]);
		return PX_TRUE;
	}
	return PX_FALSE;
}

px_void PX_GamePop()
{
	if (fifo_pointers[fifo_rcursor] != PX_NULL)
	{
		MP_Free(&fifo_cache_mp, fifo_pointers[fifo_rcursor]);
		fifo_pointers[fifo_rcursor] = PX_NULL;
		fifo_datasize[fifo_rcursor] = 0;
		fifo_rcursor++;
		fifo_rcursor %= PX_COUNTOF(fifo_pointers);
	}
}


px_void PX_GameUpdate()
{
	px_abi rabi;
	if (PX_GameGet(&rabi))
	{
		if (pCurrentCallVM)
		{
			PX_VM_RET_memory(pCurrentCallVM, PX_AbiGet_Pointer(&rabi), PX_AbiGet_Size(&rabi));
			PX_VMContinue(pCurrentCallVM);
			pCurrentCallVM = PX_NULL;
		}
		
		PX_GamePop();
	}
}



px_bool PX_GameIsConnected()
{
	if (PX_SocketIsConnecting(&game_socket))
	{
		return PX_TRUE;
	}
	return PX_FALSE;

}



px_bool PX_GamePost(px_abi* pabi)
{
	px_memory deflate_data;
	if (!PX_SocketIsConnecting(&game_socket))
	{
		return PX_FALSE;
	}
	PX_MemoryInitialize(&fifo_cache_mp, &deflate_data);
	if (!PX_RFC1951Deflate(PX_AbiGet_Pointer(pabi), PX_AbiGet_Size(pabi), &deflate_data,256))
	{
		PX_MemoryFree(&deflate_data);
		return PX_FALSE;
	}
	if (!PX_SocketSend(&game_socket, deflate_data.buffer, deflate_data.usedsize))
	{
		PX_MemoryFree(&deflate_data);
		return PX_FALSE;
	}
	PX_MemoryFree( &deflate_data);
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_Game_VM_AbiCall)
{
	px_int ptr_index;
	px_variable* pVar;
	px_abi rabi;
	if (PX_VM_HOSTPARAM(Ins,0).type!=PX_VARIABLE_TYPE_PTR)
	{
		return PX_FALSE;
	}
	ptr_index = PX_VM_HOSTPARAM(Ins, 0)._int;
	if (ptr_index < 0 || ptr_index >Ins->VM_memsize)
	{
		return PX_FALSE;
	}
	pVar = &Ins->_mem[ptr_index];
	PX_AbiCreate_StaticReader(&rabi, pVar->_memory.buffer, pVar->_memory.allocsize);
	pCurrentCallVM = Ins;
	if (!PX_GamePost(&rabi))
	{
		PX_VM_RET_memory(Ins, 0,0);
	}
	else
	{
		PX_VMSuspend(Ins);
	}
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_Game_VM_AbiPost)
{
	px_abi wabi;
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_STRING)//opcode
	{
		return PX_FALSE;
	}
	if (PX_VM_HOSTPARAM(Ins, 1).type != PX_VARIABLE_TYPE_STRING)//payload
	{
		return PX_FALSE;
	}
	PX_AbiCreate_DynamicWriter(&wabi, mp);
	PX_AbiSet_data(&wabi, "cookie", game_cookie, sizeof(game_cookie));
	PX_AbiSet_string(&wabi, "opcode", PX_VM_HOSTPARAM(Ins, 0)._string.buffer);
	PX_AbiSet_string(&wabi, "payload", PX_VM_HOSTPARAM(Ins, 1)._string.buffer);
	pCurrentCallVM = Ins;
	if (!PX_GamePost(&wabi))
	{
		PX_VM_RET_memory(Ins, 0, 0);
	}
	else
	{
		PX_VMSuspend(Ins);
	}
	PX_AbiFree(&wabi);
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_Game_VM_GetCookie)
{
	PX_VM_RET_memory(Ins, game_cookie, sizeof(px_byte) * 32);
	return PX_TRUE;
}
PX_VM_HOST_FUNCTION(PX_Game_VM_GetProfile)
{
	PX_VM_RET_memory(Ins, (const px_byte*)game_profile, sizeof(game_profile));
	return PX_TRUE;
}
PX_VM_HOST_FUNCTION(PX_Game_VM_GetName)
{
	PX_VM_RET_String(Ins, (const px_char*)user_name);
	return PX_TRUE;
}

px_void PX_Game_ClearPlayers()
{
	px_int i;
	for (i = 0; i < game_players.size; i++)
	{
		PX_Game_Player* pplayer = PX_VECTORAT(PX_Game_Player, &game_players, i);
		PX_TextureFree(&pplayer->profile256);
		PX_TextureFree(&pplayer->profile128);
	}
	PX_VectorClear(&game_players);
}

px_void PX_Game_AddPlayer(px_abi* pabi)
{
	const px_char* name;
	const px_byte* pprofile;
	px_dword size;
	PX_Game_Player player;

	name = PX_AbiGet_string(pabi, "name");
	pprofile = PX_AbiGet_data(pabi, "profile", &size);
	if (!name || !pprofile || size != 256 * 256 * sizeof(px_color))
	{
		return;
	}
	if (!PX_TextureCreate(mp_static, &player.profile256, 256, 256))
	{
		printf("create player profile failed\n");
		return;
	}
	if (!PX_TextureCreate(mp_static, &player.profile128, 128, 128))
	{
		printf("create player profile failed\n");
		PX_TextureFree(&player.profile256);
		return;
	}
	PX_memcpy(player.profile256.surfaceBuffer, pprofile, 256 * 256 * sizeof(px_color));
	PX_TextureScaleToTexture(&player.profile256, &player.profile128);
	PX_strcpy(player.name, name, sizeof(player.name));
	PX_VectorPushback(&game_players, &player);
}
PX_VM_HOST_FUNCTION(PX_Game_VM_printf)
{
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_STRING)
	{
		return PX_FALSE;
	}
	printf("%s\n", PX_VM_HOSTPARAM(Ins, 0)._string.buffer);
	return PX_TRUE;
}


PX_VM_HOST_FUNCTION(PX_Game_VM_updateplayerinfo)
{
	px_int count, * pcount, i;
	px_variable* pVar;
	px_abi rabi;
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_PTR)
	{
		return PX_FALSE;
	}
	pVar = &Ins->_mem[PX_VM_HOSTPARAM(Ins, 0)._int];
	if (pVar->type != PX_VARIABLE_TYPE_MEMORY)
	{
		return PX_FALSE;
	}
	PX_Game_ClearPlayers();
	PX_AbiCreate_StaticReader(&rabi, pVar->_memory.buffer, pVar->_memory.allocsize);
	pcount = PX_AbiGet_int(&rabi, "count");
	if (!pcount)
	{
		return PX_FALSE;
	}
	count = *pcount;
	for (i = 0; i < count; i++)
	{
		px_abi playerabi;
		px_char payload[16];
		PX_sprintf1(payload, sizeof(payload), "player[%1]", PX_STRINGFORMAT_INT(i));
		if (PX_AbiGet_Abi(&rabi, &playerabi, payload))
		{
			PX_Game_AddPlayer(&playerabi);
		}
	}
	return PX_TRUE;
}

PX_Game_Player* PX_Game_GetPlayer(px_int index)
{
	PX_ASSERTIFX(!(index >= 0 && index < game_players.size), "index out of range");
	return PX_VECTORAT(PX_Game_Player, &game_players, index);
}

px_int PX_Game_GetPlayerCount()
{
	return game_players.size;
}

px_void PX_Game_SetMyIndex(px_int index)
{
	game_my_index = index;
}

px_int PX_Game_GetMyIndex()
{
	return game_my_index;
}



px_bool PX_Game_VM_LoadStandard(PX_VM* vm, px_void* ptr)
{
	PX_VMRegisterStandardFunctions(vm, ptr);
	PX_VMRegisterHostFunction(vm, "abicall", PX_Game_VM_AbiCall, ptr);
	PX_VMRegisterHostFunction(vm, "abipost", PX_Game_VM_AbiPost, ptr);
	PX_VMRegisterHostFunction(vm, "getcookie", PX_Game_VM_GetCookie, ptr);
	PX_VMRegisterHostFunction(vm, "getprofile", PX_Game_VM_GetProfile, ptr);
	PX_VMRegisterHostFunction(vm, "getname", PX_Game_VM_GetName, ptr);
	PX_VMRegisterHostFunction(vm, "updateplayerinfo", PX_Game_VM_updateplayerinfo, ptr);
	PX_VMRegisterHostFunction(vm, "printf", PX_Game_VM_printf, ptr);
	return PX_TRUE;
}

px_byte* game_fontmodule_file_buffer = PX_NULL;
px_int game_fontmodule_file_size = 0;
px_bool PX_GameLoadFontModuleFile(const px_char* filepath)
{
	PX_IO_Data io = PX_LoadFileToIOData(filepath);
	if (io.size)
	{
		game_fontmodule_file_buffer = MP_Malloc(mp_static, io.size);
		if (!game_fontmodule_file_buffer)
		{
			PX_FreeIOData(&io);
			return PX_FALSE;
		}
		PX_memcpy(game_fontmodule_file_buffer, io.buffer, io.size);
		game_fontmodule_file_size = io.size;
		PX_FreeIOData(&io);
		return PX_TRUE;
	}
	return PX_FALSE;
}

px_byte* PX_GameGetFontModuleFileBuffer()
{
	return game_fontmodule_file_buffer;
}
px_int PX_GameGetFontModuleFileSize()
{
	return game_fontmodule_file_size;
}