#include "PX_Game.h"

#define PX_GAME_MAX_INSTANCE 1
px_mutex   game_lock;
PX_Game game[PX_GAME_MAX_INSTANCE];
PX_SocketHub sockethub;

px_dword PX_Game_CookieToCrc32(const px_byte cookie[32])
{
	return PX_crc32(cookie, 32);
}

px_bool PX_Game_Post(px_memorypool *mp,const px_byte session[16], px_abi *pabi)
{
	px_memory deflate_data;
	PX_MemoryInitialize(mp, &deflate_data);
	if (PX_RFC1951Deflate(PX_AbiGet_Pointer(pabi),PX_AbiGet_Size(pabi), &deflate_data,128))
	{
		PX_SocketHubSend(&sockethub, session, deflate_data.buffer, deflate_data.usedsize);
		PX_MemoryFree(&deflate_data);
		return PX_TRUE;
	}
	PX_MemoryFree(&deflate_data);
	return PX_FALSE;
}

px_int PX_Games_GetClientIndexBySocketSession(PX_Game* pGame, const px_byte socket_session[16])
{
	px_int i;
	for (i = 0; i < pGame->clients.size; i++)
	{
		PX_Game_Client* pClient = (PX_Game_Client*)PX_VECTORAT(PX_Game_Client, &pGame->clients, i);
		if (PX_memequ(pClient->socket_session, socket_session, 16))
		{
			return i;
		}
	}
	return -1;
}

px_int PX_Games_GetClientIndexByCookie(PX_Game* pGame, const px_byte cookie[])
{
	px_int i;
	for (i = 0; i < pGame->clients.size; i++)
	{
		PX_Game_Client* pClient = (PX_Game_Client*)PX_VECTORAT(PX_Game_Client, &pGame->clients, i);
		const px_byte* pcookie = PX_AbiGet_data(&pClient->userinfo, "cookie", NULL);
		if (pcookie && PX_memequ(pcookie, cookie, 32))
		{
			return i;
		}
	}
	return -1;
}

px_void PX_Game_Synchronous(PX_Game* pGame)
{
	px_int *pstate_id = PX_AbiGet_int(&pGame->game, "state_id");
	if (pstate_id ==PX_NULL)
	{
		PX_AbiSet_int(&pGame->game, "state_id",1);
	}
	else
	{
		PX_AbiSet_int(&pGame->game, "state_id", 1+*pstate_id);
	}
	
}

px_int PX_Game_GetClientIndex(PX_Game* pGame, px_abi* pabi)
{
	px_int i;
	px_dword size;
	const px_byte* pcookie = PX_AbiGet_data(pabi, "cookie", &size);
	if (!pcookie || size != 32)
	{
		return PX_NULL;
	}
	for (i = 0; i < pGame->clients.size; i++)
	{
		PX_Game_Client* pClient = (PX_Game_Client*)PX_VECTORAT(PX_Game_Client, &pGame->clients, i);
		if (PX_AbiExist_data(&pClient->userinfo, "cookie", pcookie, size))
		{
			return i;
		}
	}
	return -1;
}

const px_char* PX_Game_GetState(PX_Game* pGame)
{
	return PX_AbiGet_string(&pGame->game, "state");
}

PX_GAME_LOGIN_RETURN PX_Game_Login(PX_Game* pGame,const px_byte socket_session[16], px_abi* pabi)
{
	px_int *pmax_player_count = PX_AbiGet_int(&pGame->game, "max_player_count");
	if (!pmax_player_count)
	{
		return PX_GAME_LOGIN_RETURN_ERROR;
	}
	px_int max_player_count = *pmax_player_count;
	px_int ClientIndex = PX_Game_GetClientIndex(pGame, pabi);

	if (ClientIndex != -1)
	{
		PX_Game_Client* pClient = (PX_Game_Client*)PX_VECTORAT(PX_Game_Client, &pGame->clients, ClientIndex);
		PX_memcpy(pClient->socket_session, socket_session, 16);
		return PX_GAME_LOGIN_RETURN_OK;
	}
	if (pGame->clients.size < max_player_count)
	{
		PX_Game_Client newClient;
		PX_memset(&newClient, 0, sizeof(PX_Game_Client));
		PX_AbiCreate_DynamicWriter(&newClient.userinfo, &pGame->mp);
		PX_memcpy(newClient.socket_session, socket_session, 16);
		if (!PX_AbiCopy_FromAbi(&newClient.userinfo, pabi))
		{
			PX_AbiFree(&newClient.userinfo);
			return PX_GAME_LOGIN_RETURN_ERROR;
		}
		
		PX_VectorPushback(&pGame->clients, &newClient);
		PX_Game_Synchronous(pGame);
		return PX_GAME_LOGIN_RETURN_OK;
	}
	return PX_GAME_LOGIN_RETURN_FULL;
}

px_bool PX_Game_RemoveClient(PX_Game* pGame, px_int ClientIndex)
{
	if (ClientIndex != -1)
	{
		PX_Game_Client* pClient;
		pClient = (PX_Game_Client*)PX_VECTORAT(PX_Game_Client, &pGame->clients, ClientIndex);
		PX_AbiFree(&pClient->userinfo);
		PX_VectorErase(&pGame->clients, ClientIndex);
		return PX_TRUE;
	}
	return PX_FALSE;
}

px_int PX_Games_GetGameIndexByClientCookie(const px_byte* pcookie)
{
	px_int i;
	px_dword size;
	for (i = 0; i < PX_GAME_MAX_INSTANCE; i++)
	{
		px_int j;
		PX_Game* pGame = &game[i];
		for (j = 0; j < pGame->clients.size; j++)
		{
			PX_Game_Client* pClient = (PX_Game_Client*)PX_VECTORAT(PX_Game_Client, &pGame->clients, j);
			const px_byte* pcookie2 = PX_AbiGet_data(&pClient->userinfo, "cookie", &size);
			if (pcookie2 && PX_memequ(pcookie, pcookie2, size))
			{
				return i;
			}
		}
	}
	return -1;
}

PX_Game * PX_Games_GetGameByClientCookie(const px_byte* pcookie)
{
	px_int index = PX_Games_GetGameIndexByClientCookie(pcookie);
	if (index != -1)
	{
		return &game[index];
	}
	return PX_NULL;
}

px_void PX_Games_return_error(px_memorypool* mp, const px_byte socket_session[16], const px_char* message)
{
	px_abi wabi;
	PX_AbiCreate_DynamicWriter(&wabi, mp);
	PX_AbiSet_string(&wabi, "return", "error");
	PX_AbiSet_string(&wabi, "message", message);
	PX_Game_Post(mp,socket_session, &wabi);
	PX_AbiFree(&wabi);
	printf("response error: %s\n", message);
}

px_void PX_Games_return_ok(px_memorypool* mp, const px_byte socket_session[16])
{
	px_abi wabi;
	PX_AbiCreate_DynamicWriter(&wabi, mp);
	PX_AbiSet_string(&wabi, "return", "ok");
	PX_Game_Post(mp, socket_session, &wabi);
	PX_AbiFree(&wabi);
}

px_void PX_Games_Handle_login(px_memorypool* mp, const px_byte socket_session[16], px_abi* pabi)
{
	px_dword cookie_size,profile_size;
	const px_byte *pcookie = PX_AbiGet_data(pabi, "cookie",&cookie_size);
	px_abi wabi;
	const px_char *pname = PX_AbiGet_string(pabi, "name");
	const px_byte* pprofile = PX_AbiGet_data(pabi, "profile", &profile_size);
	if (!pcookie)
	{
		printf("error: cookie not found\n");
		return;
	}
	if (!pname)
	{
		printf("error: name not found\n");
		return;
	}
	if (!pprofile)
	{
		printf("error: profile not found\n");
		return;
	}
	if (profile_size!=256*256*sizeof(px_color))
	{
		printf("error: profile size not match\n");
		return;
	}
	if (PX_Games_GetGameIndexByClientCookie(pcookie) != -1)
	{
		PX_AbiCreate_DynamicWriter(&wabi, mp);
		PX_AbiSet_string(&wabi, "return", "ok");
		PX_Game_Post(mp,socket_session, &wabi);
		PX_AbiFree(&wabi);
		printf("name:%s cookie:%x already login\n", pname, PX_Game_CookieToCrc32(pcookie));
		//ack ok
	}
	else
	{
		px_int i;
		for (i = 0; i < PX_GAME_MAX_INSTANCE; i++)
		{
			PX_GAME_LOGIN_RETURN ret;
			ret = PX_Game_Login(&game[i], socket_session, pabi);
			if (ret == PX_GAME_LOGIN_RETURN_OK)
			{
				PX_AbiCreate_DynamicWriter(&wabi, mp);
				PX_AbiSet_string(&wabi, "return", "ok");
				PX_Game_Post(mp,socket_session, &wabi);
				PX_AbiFree(&wabi);
				printf("name:%s cookie:%x login ok\n",pname, PX_Game_CookieToCrc32(pcookie));
				return;
			}
			else if (ret == PX_GAME_LOGIN_RETURN_ERROR)
			{
				PX_Games_return_error(mp,socket_session, "Denial of service");
				printf("name:%s cookie:%x login error\n", pname, PX_Game_CookieToCrc32(pcookie));
				return;
			}
		}
		PX_AbiCreate_DynamicWriter(&wabi, mp);
		PX_AbiSet_string(&wabi, "return", "full");
		PX_Game_Post(mp,socket_session, &wabi);
		PX_AbiFree(&wabi);
		printf("name:%s cookie:%x login full\n", pname, PX_Game_CookieToCrc32(pcookie));
	}
}

px_void PX_Games_Handle_abi_get(px_memorypool* mp, const px_byte socket_session[16], px_abi* pabi)
{
	px_dword cookie_size;
	px_byte temp_abi_cache[512];
	px_abi myabi;
	const px_char* pcookie = PX_AbiGet_data(pabi, "cookie", &cookie_size);
	const px_char* payload = PX_AbiGet_string(pabi, "payload");
	px_abi rabi;
	px_int game_index = PX_Games_GetGameIndexByClientCookie(pcookie);
	if (payload&&game_index != -1)
	{
		px_char prefix[128];
		px_char suffix[128];
		px_abi* pabi = PX_NULL;
		px_int n;
		PX_Game* pgame = &game[game_index];
		n = PX_strsub(payload, '.');
		if (!PX_strsubi(payload, prefix, sizeof(prefix), '.', 0))
		{
			PX_Games_return_error(mp, socket_session, "invalid payload");
			printf("cookie:%x get abi error\n", PX_Game_CookieToCrc32(pcookie));
			return;
		}
		if (!PX_strsubx(payload, suffix, sizeof(suffix), '.', 1, n - 1))
		{
			PX_Games_return_error(mp, socket_session, "invalid payload");
			printf("cookie:%x get abi error\n", PX_Game_CookieToCrc32(pcookie));
			return;
		}

		if (PX_strequ(prefix,"game"))
		{
			pabi = &pgame->game;
		}
		else if (PX_strequ(prefix, "players"))
		{
			pabi = &pgame->players;
		}
		else if (PX_strequ(prefix, "scene"))
		{
			pabi = &pgame->scene;
		}
		else if (PX_strequ(prefix, "myid"))
		{
			px_int client_index = PX_Games_GetClientIndexByCookie(pgame, pcookie);
			PX_AbiCreate_StaticWriter(&myabi, temp_abi_cache, sizeof(temp_abi_cache));
			PX_AbiSet_int(&myabi, "player_index", client_index);

			pabi = &myabi;
		}
		else
		{
			PX_Games_return_error(mp, socket_session, "invalid payload");
			printf("cookie:%x get abi error\n", PX_Game_CookieToCrc32(pcookie));
			return;
		}

		PX_ABI_TYPE type;
		px_dword size=0;
		px_byte* pvalue;
		if (suffix[0]!='\0')
		{
			pvalue = PX_AbiGet_PayloadPointer(pabi, &type, &size, suffix);
			if(pvalue)
				size = PX_AbiPointer_GetAbiSize(pvalue);
		}
		else
		{
			pvalue = PX_AbiGet_Pointer(pabi);
			if (pvalue)
				size = PX_AbiGet_Size(pabi);
		}
		
		if (pvalue)
		{
			PX_AbiCreate_StaticReader(&rabi, pvalue, size);
			PX_Game_Post(mp, socket_session, &rabi);
			//printf("cookie:%x get abi return %s:%u\n", PX_Game_CookieToCrc32(pcookie), payload, size);
			return;
		}
		else
		{
			PX_Games_return_error(mp, socket_session, "invalid payload");
			printf("cookie:%x get abi error\n", PX_Game_CookieToCrc32(pcookie));
			return;
		}
	}
	else
	{
		PX_Games_return_error(mp, socket_session, "invalid cookie");
		printf("cookie:%x get abi error\n", PX_Game_CookieToCrc32(pcookie));
	}
}

px_void PX_Games_Handle_query_players_info(px_memorypool* mp, const px_byte socket_session[16], px_abi* pabi)
{
	px_dword cookie_size;
	const px_char* pcookie = PX_AbiGet_data(pabi, "cookie", &cookie_size);
	px_abi wabi;
	px_int game_index = PX_Games_GetGameIndexByClientCookie(pcookie);
	if (game_index != -1)
	{
		PX_Game* pgame = &game[game_index];
		px_int i;

		PX_AbiCreate_DynamicWriter(&wabi, mp);
		PX_AbiSet_string(&wabi, "return", "ok");
		for (i = 0; i < pgame->clients.size; i++)
		{
			const px_char* pname=PX_NULL;
			const px_byte* pprofiledata;
			px_char build_payload[32] = {0};
			px_dword profilesize;
			PX_Game_Client* pClient = (PX_Game_Client*)PX_VECTORAT(PX_Game_Client, &pgame->clients, i);
			pname = PX_AbiGet_string(&pClient->userinfo, "name");
			pprofiledata = PX_AbiGet_data(&pClient->userinfo, "profile", &profilesize);
			PX_sprintf1(build_payload, sizeof(build_payload), "player[%1].name", PX_STRINGFORMAT_INT(i));
			PX_AbiSet_int(&wabi, "count", pgame->clients.size);
			if(pname)
				PX_AbiSet_string(&wabi, build_payload, pname);
			else
				PX_AbiSet_string(&wabi, build_payload, "unknown");

			PX_sprintf1(build_payload, sizeof(build_payload), "player[%1].profile", PX_STRINGFORMAT_INT(i));
			if (pprofiledata)
			{
				PX_AbiSet_data(&wabi, build_payload, pprofiledata, profilesize);
			}
		}
		printf("cookie:%x query players info return count %u\n", PX_Game_CookieToCrc32(pcookie), pgame->clients.size);
		PX_Game_Post(mp,socket_session, &wabi);

	}
	else
	{
		PX_Games_return_error(mp,socket_session, "invalid cookie");
		printf("cookie:%x query players info error\n", PX_Game_CookieToCrc32(pcookie));
	}
}

px_void PX_Games_Handle_make_choose(px_memorypool* mp, const px_byte socket_session[16], px_abi* pabi)
{
	px_dword cookie_size;
	const px_char* pcookie = PX_AbiGet_data(pabi, "cookie", &cookie_size);
	px_int* pchoose = PX_AbiGet_int(pabi, "choose");
	px_int game_index = PX_Games_GetGameIndexByClientCookie(pcookie);
	if (pchoose&&game_index != -1)
	{
		px_char payload[128];
		px_int client_index = PX_Games_GetClientIndexBySocketSession(&game[game_index], socket_session);
		if (client_index == -1)
		{
			PX_Games_return_error(mp, socket_session, "invalid session");
			printf("cookie:%x make choose error\n", PX_Game_CookieToCrc32(pcookie));
			return;
		}
		PX_Game* pgame = &game[game_index];
		PX_sprintf1(payload, sizeof(payload), "players.player[%1].choose", PX_STRINGFORMAT_INT(client_index));
		PX_AbiSet_int(&pgame->game, payload, *pchoose);
		PX_Games_return_ok(mp, socket_session);
		PX_Game_Synchronous(pgame);
	}
}

PX_SOCKETHUB_CONNECT_CALLBACK_FUNCTION(PX_llmgame_onconnect) 
{
	px_memorypool *new_mp = (px_memorypool*)malloc(sizeof(px_memorypool));
	*new_mp = MP_Create(malloc(16 * 1024 * 1024), 16 * 1024 * 1024);
	MP_NoCatchError(new_mp);
	PX_SocketHubSetUserPtr(pSocketHub,session, new_mp);
}

PX_SOCKETHUB_SEND_CALLBACK_FUNCTION(PX_llmgame_onsend) {}

PX_SOCKETHUB_RECEIVE_CALLBACK_FUNCTION(PX_llmgame_onreceive)
{
	px_memorypool* mp = (px_memorypool*)PX_SocketHubGetUserPtr(pSocketHub, session);
	px_memory inflate_data;
	PX_MemoryInitialize(mp, &inflate_data);
	if (PX_RFC1951Inflate(data, datasize, &inflate_data))
	{
		const px_char* popcode;
		const px_char* pcookie;
		px_dword size;
		px_abi rabi;
		PX_AbiCreate_StaticReader(&rabi, inflate_data.buffer, inflate_data.usedsize);
		popcode = PX_AbiGet_string(&rabi, "opcode");
		pcookie = PX_AbiGet_data(&rabi, "cookie",&size);
		if (!popcode||!pcookie||size!=32)
		{
			//do nothing
		}
		PX_MutexLock(&game_lock);
		//handle opcode
		if (PX_strequ(popcode, "get"))
		{
			PX_Games_Handle_abi_get(mp, session, &rabi);
		}
		else if (PX_strequ(popcode, "login"))
		{
			PX_Games_Handle_login(mp, session, &rabi);
		}
		else if (PX_strequ(popcode, "query_players_info"))
		{
			PX_Games_Handle_query_players_info(mp, session, &rabi);
		}
		else if (PX_strequ(popcode, "make_choose"))
		{
			PX_Games_Handle_make_choose(mp, session, &rabi);
		}
		else
		{
			printf("error: %s opcode not found\n", popcode);
		}
		PX_MutexUnlock(&game_lock);
	}
	PX_MemoryFree(&inflate_data);
}

PX_SOCKETHUB_DISCONNECT_CALLBACK_FUNCTION(PX_llmgame_ondisconnect)
{
	px_memorypool* mp = (px_memorypool*)PX_SocketHubGetUserPtr(pSocketHub, session);
	px_int i;
	PX_MutexLock(&game_lock);
	for ( i = 0; i < PX_COUNTOF(game); i++)
	{
		PX_Game* pGame = &game[i];
		px_int index=PX_Games_GetClientIndexBySocketSession(pGame, session);
		if (index!=-1)
		{
			if (PX_strequ(PX_Game_GetState(pGame),"waiting"))
			{
				PX_Game_Client* pClient = (PX_Game_Client*)PX_VECTORAT(PX_Game_Client, &pGame->clients, index);
				const px_char* pcookie = PX_AbiGet_data(&pClient->userinfo, "cookie", NULL);
				PX_Game_RemoveClient(pGame, index);
				PX_Game_Synchronous(pGame);
				printf("cookie:%x remove client\n", PX_Game_CookieToCrc32(pcookie));
			}
		}
	}
	free(mp->StartAddr);
	free(mp);
	PX_MutexUnlock(&game_lock);
}

px_bool PX_Games_Initialize()
{
	px_int i;
	PX_srand(((px_uint64)PX_TimeGetTimeUs() << 32) | PX_TimeGetTime());
	if (!PX_SocketHubInitialize(&sockethub, 31460, 31482, 1024 * 1024 * 32, PX_llmgame_onconnect, PX_llmgame_onsend, PX_llmgame_onreceive, PX_llmgame_ondisconnect, 0))
	{
		printf("SocketHubInitialize failed");
		return -1;
	}
	for (i = 0; i < PX_GAME_MAX_INSTANCE; i++)
	{
		if (!PX_Game_Initialize(&game[i]))
		{
			return PX_FALSE;
		}
	}
	return PX_TRUE;
}

px_void PX_Games_Update(px_dword elapsed)
{
	px_int i;
	for (i = 0; i < PX_COUNTOF(game); i++)
	{
		PX_MutexLock(&game_lock);
		PX_VMRun(&game->vm, 0xffff, elapsed);
		PX_MutexUnlock(&game_lock);
	}
	PX_Sleep(5);
}

PX_VM_HOST_FUNCTION(PX_llmgame_printf)
{
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_STRING)
	{
		return PX_FALSE;
	}
	printf("%s\n", PX_VM_HOSTPARAM(Ins, 0)._string.buffer);
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_llmgame_game_set_abi_int)
{
	px_char prefix[128];
	px_char suffix[128];
	px_abi* pabi = PX_NULL;
	px_int n;
	const px_char* payload;
	PX_Game* pgame = (PX_Game*)userptr;
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_STRING)
	{
		return PX_FALSE;
	}
	if (PX_VM_HOSTPARAM(Ins, 1).type != PX_VARIABLE_TYPE_INT)
	{
		return PX_FALSE;
	}
	payload = PX_VM_HOSTPARAM(Ins, 0)._string.buffer;
	n = PX_strsub(payload, '.');
	if (!PX_strsubi(payload, prefix, sizeof(prefix), '.', 0))
	{
		return PX_FALSE;
	}
	if (!PX_strsubx(payload, suffix, sizeof(suffix), '.', 1, n - 1))
	{
		return PX_FALSE;
	}
	if (PX_strequ(prefix, "game"))
	{
		pabi = &pgame->game;
	}
	else if (PX_strequ(prefix, "players"))
	{
		pabi = &pgame->players;
	}
	else if (PX_strequ(prefix, "scene"))
	{
		pabi = &pgame->scene;
	}
	else
	{
		return PX_FALSE;
	}
	PX_AbiSet_int(pabi, suffix, PX_VM_HOSTPARAM(Ins, 1)._int);
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_llmgame_game_set_abi_string)
{
	px_char prefix[128];
	px_char suffix[128];
	px_abi* pabi = PX_NULL;
	px_int n;
	const px_char* payload;
	PX_Game* pgame = (PX_Game*)userptr;
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_STRING)
	{
		return PX_FALSE;
	}
	if (PX_VM_HOSTPARAM(Ins, 1).type != PX_VARIABLE_TYPE_STRING)
	{
		return PX_FALSE;
	}
	payload = PX_VM_HOSTPARAM(Ins, 0)._string.buffer;
	n = PX_strsub(payload, '.');
	if (!PX_strsubi(payload, prefix, sizeof(prefix), '.', 0))
	{
		return PX_FALSE;
	}
	if (!PX_strsubx(payload, suffix, sizeof(suffix), '.', 1, n - 1))
	{
		return PX_FALSE;
	}
	if (PX_strequ(prefix, "game"))
	{
		pabi = &pgame->game;
	}
	else if (PX_strequ(prefix, "players"))
	{
		pabi = &pgame->players;
	}
	else if (PX_strequ(prefix, "scene"))
	{
		pabi = &pgame->scene;
	}
	else
	{
		return PX_FALSE;
	}
	PX_AbiSet_string(pabi, suffix, PX_VM_HOSTPARAM(Ins, 1)._string.buffer);
	return PX_TRUE;
	
}

PX_VM_HOST_FUNCTION(PX_llmgame_game_get_abi_int)
{
	px_char prefix[128];
	px_char suffix[128];
	px_abi* pabi = PX_NULL;
	px_int n;
	const px_char* payload;
	PX_Game* pgame = (PX_Game*)userptr;
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_STRING)
	{
		return PX_FALSE;
	}
	payload = PX_VM_HOSTPARAM(Ins, 0)._string.buffer;
	n = PX_strsub(payload, '.');
	if (!PX_strsubi(payload, prefix, sizeof(prefix), '.', 0))
	{
		return PX_FALSE;
	}
	if (!PX_strsubx(payload, suffix, sizeof(suffix), '.', 1, n - 1))
	{
		return PX_FALSE;
	}
	if (PX_strequ(prefix, "game"))
	{
		pabi = &pgame->game;
	}
	else if (PX_strequ(prefix, "players"))
	{
		pabi = &pgame->players;
	}
	else if (PX_strequ(prefix, "scene"))
	{
		pabi = &pgame->scene;
	}
	else
	{
		return PX_FALSE;
	}

	px_int *pvalue = PX_AbiGet_int(pabi, suffix);
	if (pvalue)
	{
		PX_VM_RET_int(Ins, *pvalue);
	}
	else
	{
		PX_VM_RET_int(Ins, 0);
	}
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_llmgame_game_get_abi_string)
{
	px_char prefix[128];
	px_char suffix[128];
	px_abi* pabi = PX_NULL;
	px_int n;
	const px_char* payload;
	PX_Game* pgame = (PX_Game*)userptr;
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_STRING)
	{
		return PX_FALSE;
	}
	payload = PX_VM_HOSTPARAM(Ins, 0)._string.buffer;
	n = PX_strsub(payload, '.');
	if (!PX_strsubi(payload, prefix, sizeof(prefix), '.', 0))
	{
		return PX_FALSE;
	}
	if (!PX_strsubx(payload, suffix, sizeof(suffix), '.', 1, n - 1))
	{
		return PX_FALSE;
	}
	if (PX_strequ(prefix, "game"))
	{
		pabi = &pgame->game;
	}
	else if (PX_strequ(prefix, "players"))
	{
		pabi = &pgame->players;
	}
	else if (PX_strequ(prefix, "scene"))
	{
		pabi = &pgame->scene;
	}
	else
	{
		return PX_FALSE;
	}
	const px_char* pvalue = PX_AbiGet_string(pabi, suffix);
	if (pvalue)
	{
		PX_VM_RET_string(Ins, pvalue);
	}
	else
	{
		PX_VM_RET_string(Ins, "");
	}
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_llmgame_game_reset)
{
	px_int i;
	PX_Game* pgame = (PX_Game*)userptr;
	PX_AbiClear(&pgame->game);
	PX_AbiClear(&pgame->players);
	PX_AbiClear(&pgame->scene);
	for (i = 0; i < pgame->clients.size; i++)
	{
		PX_Game_Client* pClient = PX_VECTORAT(PX_Game_Client, &pgame->clients, i);
		PX_AbiFree(&pClient->userinfo);
	}
	PX_VectorClear(&pgame->clients);
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_llmgame_game_start)
{
	px_int i;
	PX_Game* pgame = (PX_Game*)userptr;
	PX_AbiSet_int(&pgame->players, "player_count", pgame->clients.size);
	for (i = 0; i < pgame->clients.size; i++)
	{
		px_char payload[32];
		PX_Game_Client* pClient = PX_VECTORAT(PX_Game_Client, &pgame->clients, i);
		PX_sprintf1(payload, sizeof(payload), "player[%1]", PX_STRINGFORMAT_INT(i));
		PX_AbiSet_Abi(&pgame->players, payload, &pClient->userinfo);
		//remove sensitive data
		PX_sprintf1(payload, sizeof(payload), "player[%1].cookie", PX_STRINGFORMAT_INT(i));
		PX_AbiDelete(&pgame->players, payload);
	}

	PX_Game_Synchronous(pgame);
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_llmgame_game_synchronous)
{
	PX_Game* pgame = (PX_Game*)userptr;
	PX_Game_Synchronous(pgame);
	printf("game_synchronous with id:%d\n",*PX_AbiGet_int(&pgame->game,"state_id"));
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_llmgame_load_scene)
{
	PX_Game* pgame = (PX_Game*)userptr;
	PX_Json scene_json;
	const px_char* scene_file,*background_file;
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_STRING)
	{
		return PX_FALSE;
	}
	scene_file = PX_VM_HOSTPARAM(Ins, 0)._string.buffer;

	PX_AbiClear(&pgame->scene);
	if (!PX_LoadJsonFromJsonFile(&pgame->mp,&scene_json, scene_file))
	{
		PX_VM_RET_int(Ins, 0);
		return PX_TRUE;
	}
	if (!PX_JsonToAbi(&scene_json, &pgame->scene))
	{
		PX_VM_RET_int(Ins, 0);
		PX_JsonFree(&scene_json);
		return PX_TRUE;
	}
	PX_JsonFree(&scene_json);
	//load background
	background_file = PX_AbiGet_string(&pgame->scene, "background");
	if(!background_file)
	{
		PX_VM_RET_int(Ins, 0);
		return PX_TRUE;
	}
	do
	{
		px_char path[260] = "assets/scene/";
		PX_IO_Data io;
		PX_strcat(path,  background_file);
		io = PX_LoadFileToIOData(path);
		if (!io.buffer)
		{
			PX_VM_RET_int(Ins, 0);
			break;
		}
		PX_AbiSet_int(&pgame->scene, "id", pgame->scene_id++);
		PX_AbiSet_data(&pgame->scene, "background", io.buffer, io.size);
		PX_FreeIOData(&io);
	} while (0);

	PX_VM_RET_int(Ins, 1);
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_llmgame_get_login_player_count)
{
	PX_Game* pgame = (PX_Game*)userptr;
	PX_VM_RET_int(Ins, pgame->clients.size);
	return PX_TRUE;
}


px_bool PX_Game_Initialize(PX_Game* pGame)
{
	px_void* ptr;


	ptr = malloc(1024 * 1024 * 32);
	if (!ptr)
	{
		return PX_FALSE;
	}
	PX_memset(pGame, 0, sizeof(PX_Game));
	pGame->mp = PX_MemorypoolCreate(ptr, 1024 * 1024 * 32);
	if(!PX_VectorInitialize(&pGame->mp ,&pGame->clients, sizeof(PX_Game_Client),32))
	{
		printf("error: initialize clients vector failed\n");
		free(ptr);
		return PX_FALSE;
	}

	pGame->scene_id = 1;

	PX_AbiCreate_DynamicWriter(&pGame->game, &pGame->mp);
	PX_AbiCreate_DynamicWriter(&pGame->players, &pGame->mp);
	PX_AbiCreate_DynamicWriter(&pGame->scene, &pGame->mp);

	
	PX_MutexInitialize(&game_lock);

	if (!PX_LoadVMFromScriptFile(&pGame->mp,"assets/server.c",&pGame->vm,"main"))
	{
		printf("error: initialize clients vector failed\n");
		free(ptr);
		return PX_FALSE;
	}

	PX_VMRegisterStandardFunctions(&pGame->vm,pGame);
	PX_VMRegisterHostFunction(&pGame->vm, "printf", PX_llmgame_printf, pGame);
	PX_VMRegisterHostFunction(&pGame->vm, "game_set_abi_string", PX_llmgame_game_set_abi_string, pGame);
	PX_VMRegisterHostFunction(&pGame->vm, "game_set_abi_int", PX_llmgame_game_set_abi_int, pGame);
	PX_VMRegisterHostFunction(&pGame->vm, "game_get_abi_string", PX_llmgame_game_get_abi_string, pGame);
	PX_VMRegisterHostFunction(&pGame->vm, "game_get_abi_int", PX_llmgame_game_get_abi_int, pGame);

	PX_VMRegisterHostFunction(&pGame->vm, "game_reset", PX_llmgame_game_reset, pGame);
	PX_VMRegisterHostFunction(&pGame->vm, "game_start", PX_llmgame_game_start, pGame);
	PX_VMRegisterHostFunction(&pGame->vm, "game_synchronous", PX_llmgame_game_synchronous, pGame);
	PX_VMRegisterHostFunction(&pGame->vm, "load_scene", PX_llmgame_load_scene, pGame);
	PX_VMRegisterHostFunction(&pGame->vm, "get_login_player_count", PX_llmgame_get_login_player_count, pGame);

	PX_VMBeginThreadFunction(&pGame->vm, 0, "main", 0, 0);

	return PX_TRUE;
}



