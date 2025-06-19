#include "PX_Object_game.h"

PX_OBJECT_UPDATE_FUNCTION(PX_Object_gameUpdate)
{
	PX_Object_game* pgame = PX_ObjectGetDesc(PX_Object_game, pObject);
	PX_GameUpdate();
	PX_VMRun(&pgame->vm, 0xffff, elapsed);
}

PX_OBJECT_RENDER_FUNCTION(PX_Object_gameRender)
{
	px_int i;
	px_int ofti;
	PX_Object_game* pDesc = PX_ObjectGetDesc(PX_Object_game,pObject);
	if (pDesc->background.MP)
	{
		PX_TextureCover(psurface, &pDesc->background, 0, 0, PX_ALIGN_LEFTTOP);
	}

	//render players
	px_int playercount = pDesc->texture_player_profile128.size;
	px_int starty = 100;
	for (i = 0; i < pDesc->texture_player_profile128.size; i++)
	{
		px_texture* profile128 = PX_VECTORAT(px_texture, &pDesc->texture_player_profile128, i);
		px_int startx;
		startx = psurface->width - 212;
		
		px_char payload[32];
		PX_sprintf1(payload, sizeof(payload), "players.player[%1].name", PX_STRINGFORMAT_INT(i));
		const px_char* pplayername = PX_AbiGet_string(&pDesc->game_abi, payload);
		PX_sprintf1(payload, sizeof(payload), "players.player[%1].pp", PX_STRINGFORMAT_INT(i));
		const px_int* ppp = PX_AbiGet_int(&pDesc->game_abi, payload);
		PX_sprintf1(payload, sizeof(payload), "players.player[%1].hp", PX_STRINGFORMAT_INT(i));
		const px_int* php = PX_AbiGet_int(&pDesc->game_abi, payload);
		if (!pplayername || !ppp || !php)
		{
			continue;
		}
		px_char content[128];
		PX_TextureRenderMask(psurface, &pDesc->texture_mask_128, profile128,  startx, starty, PX_ALIGN_CENTER, 0);
		PX_TextureRender(psurface, &pDesc->texture_ring_128, startx, starty, PX_ALIGN_CENTER, 0);
		pDesc->stamps_loser[i]->x = startx*1.f;
		pDesc->stamps_loser[i]->y = starty*1.f;
		PX_ObjectSetAlign(pDesc->stamps_loser[i], PX_ALIGN_CENTER);
		PX_sprintf3(content, sizeof(content), PX_JsonGetString(&pDesc->language,"tag"),PX_STRINGFORMAT_STRING(pplayername),\
			PX_STRINGFORMAT_INT(*ppp),PX_STRINGFORMAT_INT(*php));
		//render name
		PX_FontModuleDrawText(psurface, &pDesc->fm, startx+68, starty-38, PX_ALIGN_LEFTTOP, content, PX_COLOR_RED);

		starty += 168;
	}

	pDesc->printer->x = psurface->width / 2.f;
	pDesc->printer->y = psurface->height/2 - 32.f;
	ofti = 0;
	for (i = 0; i < PX_COUNTOF(pDesc->select_button); i++)
	{
		if (pDesc->select_button[i]->Visible)
		{
			pDesc->select_button[i]->x = psurface->width / 2.f;
			pDesc->select_button[i]->y = psurface->height / 2.f - 128 + ofti * (pDesc->select_button[i]->Height + 8);
			ofti++;
			PX_ObjectSetAlign(pDesc->select_button[i], PX_ALIGN_CENTER);
			
		}
	}
	pDesc->waiting->x= psurface->width / 2.f;
	pDesc->waiting->y = psurface->height - 100.f;

	pDesc->counter_down->x=  100.f;
	pDesc->counter_down->y = 100.f;

}

PX_OBJECT_FREE_FUNCTION(PX_Object_gameFree)
{

}

px_bool PX_Object_game_player_is_alive(PX_Object* pObject,px_int player)
{
	px_char payload[32];
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, pObject, 0);
	PX_sprintf1(payload, sizeof(payload), "players.player[%1].state", PX_STRINGFORMAT_INT(player));
	const px_char* pstate = PX_AbiGet_string(&pDesc->game_abi, payload);
	if (pstate && PX_strequ(pstate, "alive"))
	{
		return PX_TRUE;
	}
	return PX_FALSE;
	
}

px_void PX_Object_game_DisplayNext(PX_Object* pObject)
{
	px_char payload[32]; 
	px_int i;
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, pObject, 0);

	if (pDesc->reg_messageIndex < 0)
	{
		return;
	}
	if (!pDesc->scene_abi.dynamic.buffer)
	{
		return;
	}
	PX_sprintf1(payload, sizeof(payload), "message[%1]", PX_STRINGFORMAT_INT(pDesc->reg_messageIndex));
	const px_char* pmessage = PX_AbiGet_string(&pDesc->scene_abi, payload);
	if (pmessage)
	{
		if(pDesc->reg_messageIndex==0)
			PX_Object_MessageDisplayOpen(pDesc->printer, pmessage);
		else
			PX_Object_MessageDisplaySet(pDesc->printer, pmessage);
		pDesc->reg_messageIndex++;
		return;
	}
	else
	{
		const px_char* pscene_type = PX_AbiGet_string(&pDesc->scene_abi, "type");
		if (!pscene_type||PX_strequ(pscene_type,"normal"))
		{
			pDesc->choose = 0;
			pDesc->reg_messageIndex = -1;//eof
			PX_ObjectSetVisible(pDesc->waiting, PX_TRUE);
			return;
		}
		else
		{
			px_int playercount = *PX_AbiGet_int(&pDesc->players_abi, "player_count");
			if (!PX_Object_game_player_is_alive(pObject, PX_Game_GetMyIndex()))
			{
				for (i = 0; i < PX_COUNTOF(pDesc->select_button); i++)
				{
					PX_ObjectSetVisible(pDesc->select_button[i], PX_FALSE);
				}
			}
			else
			{
				for (i = 0; i < PX_COUNTOF(pDesc->select_button); i++)
				{
					if (i < 2)
					{
						PX_ObjectSetVisible(pDesc->select_button[i], PX_TRUE);
					}
					else if (i < playercount + 2)
					{
						px_int myindex = PX_Game_GetMyIndex();
						if (i - 2 != myindex && PX_Object_game_player_is_alive(pObject, i - 2))
						{
							PX_ObjectSetVisible(pDesc->select_button[i], PX_TRUE);
						}
						else
						{
							PX_ObjectSetVisible(pDesc->select_button[i], PX_FALSE);
						}
					}
					else
					{
						PX_ObjectSetVisible(pDesc->select_button[i], PX_FALSE);
					}
				}
			}
			
			PX_Object_MessageClose(pDesc->printer);
			pDesc->reg_messageIndex = -1;//eof
		}
		return;
	}
}

PX_OBJECT_EVENT_FUNCTION(PX_Object_gameOnCursorUp)
{
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, (PX_Object *)ptr, 0);
	PX_Object_MessageSetSpeed(pDesc->printer, 3);
}


PX_VM_HOST_FUNCTION(PX_Object_game_VM_load_scene)
{
	px_int ptr_index;
	px_variable* pVar;
	px_byte* pbackground_data_buffer;
	px_int pbackground_data_size;
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, (PX_Object*)userptr, 0);
	px_abi rabi;
	px_int i,duration;
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_PTR)
	{
		return PX_FALSE;
	}
	ptr_index = PX_VM_HOSTPARAM(Ins, 0)._int;
	if (ptr_index < 0 || ptr_index > Ins->VM_memsize)
	{
		return PX_FALSE;
	}
	pVar = &Ins->_mem[ptr_index];
	if (pVar->type != PX_VARIABLE_TYPE_MEMORY)
	{
		return PX_FALSE;
	}
	//load background
	if (pDesc->background.MP)
	{
		PX_TextureFree(&pDesc->background);
		PX_memset(&pDesc->background, 0, sizeof(px_texture));
	}
	PX_AbiCreate_StaticReader(&rabi, pVar->_memory.buffer, pVar->_memory.allocsize);
	pbackground_data_buffer = PX_AbiGet_data(&rabi, "background", &pbackground_data_size);
	if (!pbackground_data_buffer || pbackground_data_size <= 0)
	{
		return PX_FALSE;
	}
	if (!PX_TextureCreateFromMemory(mp_static, pbackground_data_buffer, pbackground_data_size, &pDesc->background))
	{
		PX_ASSERTX("Load background failed");
		return PX_FALSE;
	}

	PX_AbiCopy_FromBuffer(&pDesc->scene_abi,pVar->_memory.buffer, pVar->_memory.allocsize);
	pDesc->reg_messageIndex = 0;
	pDesc->isSceneEnd = PX_FALSE;
	for (i = 0; i < PX_COUNTOF(pDesc->select_button); i++)
	{
		PX_ObjectSetVisible(pDesc->select_button[i], PX_FALSE);
	}
	PX_ObjectSetVisible(pDesc->waiting, PX_FALSE);
	duration = PX_AbiGet_int(&pDesc->scene_abi, "duration") ? *PX_AbiGet_int(&pDesc->scene_abi, "duration") : 0;
	if(duration>1000)
		PX_Object_CounterDownSetValue(pDesc->counter_down, duration-1000);
	PX_Object_game_DisplayNext((PX_Object*)userptr);
	return PX_TRUE;

}

PX_VM_HOST_FUNCTION(PX_Object_game_VM_load_players)
{
	px_int ptr_index;
	px_variable* pVar;
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, (PX_Object*)userptr, 0);
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_PTR)
	{
		return PX_FALSE;
	}
	ptr_index = PX_VM_HOSTPARAM(Ins, 0)._int;
	if (ptr_index < 0 || ptr_index > Ins->VM_memsize)
	{
		return PX_FALSE;
	}
	pVar = &Ins->_mem[ptr_index];
	if (pVar->type != PX_VARIABLE_TYPE_MEMORY)
	{
		return PX_FALSE;
	}
	PX_AbiCopy_FromBuffer(&pDesc->players_abi, pVar->_memory.buffer, pVar->_memory.allocsize);

	//load player profile
	px_int* pplayercount = PX_AbiGet_int(&pDesc->players_abi, "player_count");
	if (pplayercount)
	{
		px_int i;
		for (i = 0; i < *pplayercount; i++)
		{
			px_texture profile128, profile256;
			PX_TextureCreate(mp_static, &profile128, 128, 128);
			PX_TextureCreate(mp_static, &profile256, 256, 256);
			px_char payload[32];
			px_dword datasize;
			PX_sprintf1(payload, sizeof(payload), "player[%1].profile", PX_STRINGFORMAT_INT(i));
			px_byte* profiledata = PX_AbiGet_data(&pDesc->players_abi, payload, &datasize);
			if (datasize != 256 * 256 * 4)
			{
				break;
			}
			PX_memcpy(profile256.surfaceBuffer, profiledata, 256 * 256 * 4);
			PX_TextureScaleToTexture(&profile256, &profile128);
			PX_VectorPushback(&pDesc->texture_player_profile128, &profile128);
			PX_VectorPushback(&pDesc->texture_player_profile256, &profile256);
		}
	}
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_Object_game_VM_load_game)
{
	px_int ptr_index;
	px_int i;
	px_int player_count;
	const px_char* pgame_state;
	px_variable* pVar;
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, (PX_Object*)userptr, 0);
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_PTR)
	{
		return PX_FALSE;
	}
	ptr_index = PX_VM_HOSTPARAM(Ins, 0)._int;
	if (ptr_index < 0 || ptr_index > Ins->VM_memsize)
	{
		return PX_FALSE;
	}
	pVar = &Ins->_mem[ptr_index];
	if (pVar->type != PX_VARIABLE_TYPE_MEMORY)
	{
		return PX_FALSE;
	}
	PX_AbiCopy_FromBuffer(&pDesc->game_abi, pVar->_memory.buffer, pVar->_memory.allocsize);
	pgame_state = PX_AbiGet_string(&pDesc->game_abi, "state");
	if (pgame_state &&PX_strequ(pgame_state, "end"))
	{
		pDesc->game_is_end = PX_TRUE;
	}

	player_count = *PX_AbiGet_int(&pDesc->game_abi, "max_player_count");
	for ( i = 0; i < player_count; i++)
	{
		const px_char* pstate;
		px_char payload[32];
		PX_sprintf1(payload, sizeof(payload), "players.player[%1].state", PX_STRINGFORMAT_INT(i));
		pstate = PX_AbiGet_string(&pDesc->game_abi, payload);
		if (pstate && PX_strequ(pstate, "die"))
		{
			if (!pDesc->stamps_loser[i]->Visible)
			{
				PX_Object_StampPresent(pDesc->stamps_loser[i], &pDesc->texture_loser);
			}
			
		}
		
	}


	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_Object_game_VM_get_scene_message)
{
	PX_Object_game *pDesc= PX_ObjectGetDescIndex(PX_Object_game, (PX_Object*)userptr, 0);
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_INT)
	{
		return PX_FALSE;
	}
	px_char payload[32];
	PX_sprintf1(payload, sizeof(payload), "message[%1]", PX_STRINGFORMAT_INT(PX_VM_HOSTPARAM(Ins, 0)._int));
	const px_char* pmessage = PX_AbiGet_string(&pDesc->scene_abi, "message");
	if (pmessage)
	{
		PX_VM_RET_string(Ins, pmessage);
	}
	else
	{
		PX_VM_RET_string(Ins, "0");
	}
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_Object_game_VM_message)
{
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, (PX_Object*)userptr, 0);
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_STRING)
	{
		return PX_FALSE;
	}
	if (PX_StringLen(&PX_VM_HOSTPARAM(Ins, 0)._string) != 0)
	{
		PX_Object_MessageDisplayOpen(pDesc->printer, PX_VM_HOSTPARAM(Ins, 0)._string.buffer);
	}
	else
	{
		PX_Object_MessageBoxClose(pDesc->printer);
	}
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_Object_game_VM_messageIsEnd)
{
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, (PX_Object*)userptr, 0);
	PX_VM_RET_int(Ins, PX_Object_TyperIsEnd(pDesc->printer));
	return PX_TRUE;
}

PX_OBJECT_EVENT_FUNCTION(PX_Object_gameOnCursorDown)
{
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, (PX_Object*)ptr, 0);
	if (PX_Object_MessageIsEnd(pDesc->printer))
	{
		//next
		PX_Object_game_DisplayNext((PX_Object*)ptr);
	}
	else
	{
		PX_Object_MessageSetSpeed(pDesc->printer, 10);
	}
}

PX_VM_HOST_FUNCTION(PX_Object_game_VM_get_current_choose)
{
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, (PX_Object*)userptr, 0);
	PX_VM_RET_int(Ins, pDesc->choose);
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_Object_game_VM_reset_choose)
{
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, (PX_Object*)userptr, 0);
	pDesc->choose = -1;
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_Object_game_VM_close_choose_buttons)
{
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, (PX_Object*)userptr, 0);
	px_int i;
	for (i = 0; i < PX_COUNTOF(pDesc->select_button); i++)
	{
		PX_ObjectSetVisible(pDesc->select_button[i], PX_FALSE);
	}
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_Object_game_VM_game_is_end)
{
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, (PX_Object*)userptr, 0);
	if (pDesc->game_is_end)
	{
		PX_VM_RET_int(Ins, PX_TRUE);
	}
	else
	{
		PX_VM_RET_int(Ins, PX_FALSE);
	}
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_Object_game_VM_set_button_text)
{
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, (PX_Object*)userptr, 0);
	px_int index;
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_INT)
	{
		return PX_FALSE;
	}
	index = PX_VM_HOSTPARAM(Ins, 0)._int;
	if (index < 0 || index >= PX_COUNTOF(pDesc->select_button))
	{
		return PX_FALSE;
	}
	if (PX_VM_HOSTPARAM(Ins, 1).type != PX_VARIABLE_TYPE_STRING)
	{
		return PX_FALSE;
	}
	PX_Object_PushButtonSetText(pDesc->select_button[index], PX_VM_HOSTPARAM(Ins, 1)._string.buffer);
	return PX_TRUE;
}

PX_VM_HOST_FUNCTION(PX_Object_game_VM_set_player_index)
{
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, (PX_Object*)userptr, 0);
	px_int index;
	if (PX_VM_HOSTPARAM(Ins, 0).type != PX_VARIABLE_TYPE_INT)
	{
		return PX_FALSE;
	}
	index = PX_VM_HOSTPARAM(Ins, 0)._int;
	
	PX_Game_SetMyIndex(index);
	return PX_TRUE;
}

PX_OBJECT_EVENT_FUNCTION(PX_Object_game_event_MakeChoose)
{
	PX_Object_game* pDesc = PX_ObjectGetDescIndex(PX_Object_game, (PX_Object*)ptr, 0);
	px_int i;
	PX_ObjectSetVisible(pDesc->waiting, PX_TRUE);
	for (i = 0; i < PX_COUNTOF(pDesc->select_button); i++)
	{
		PX_ObjectSetVisible(pDesc->select_button[i], PX_FALSE);
	}
	for (i = 0; i < PX_COUNTOF(pDesc->select_button); i++)
	{
		if (pDesc->select_button[i] == pObject)
		{
			pDesc->choose = i;
			return;
		}
	}
}

PX_Object* PX_Object_gameCreate(PX_Object* pparent)
{
	PX_Object_game* pDesc;
	px_int i;
	PX_Object* pObject = PX_ObjectCreateEx(mp, pparent, 0, 0, 0, 0, 0, 0, 0, PX_Object_gameUpdate, PX_Object_gameRender, PX_Object_gameFree, 0, sizeof(PX_Object_game));
	pDesc = PX_ObjectGetDescIndex(PX_Object_game, pObject, 0);
	if (!PX_LoadVMFromScriptFile(mp,"assets/game.c",&pDesc->vm,"main"))
	{
		PX_ASSERTX( "Load game script failed");
		return PX_NULL;
	}
	if (!PX_FontModuleInitializeTTFFromStaticBuffer(mp_static, \
		&pDesc->fm, \
		PX_FONTMODULE_CODEPAGE_UTF8, \
		26, \
		PX_GameGetFontModuleFileBuffer(), \
		PX_GameGetFontModuleFileSize() \
	))
	{
		return 0;
	}
	
	if (!PX_LoadTextureFromFile(mp_static, &pDesc->select_button_texture, "assets/select_button.png"))
	{
		PX_ASSERTX("Load game button texture failed");
		return PX_NULL;
	}

	if (!PX_LoadTextureFromFile(mp_static, &pDesc->texture_loser, "assets/loser.png"))
	{
		PX_ASSERTX("Load game loser texture failed");
		return PX_NULL;
	}


	if (!PX_LoadTextureFromFile(mp_static, &pDesc->texture_ring_128, "assets/login_profile_128x128.png"))
	{
		PX_ASSERTX("Load game button texture failed");
		return PX_NULL;
	}

	if (!PX_TextureCreate(mp_static, &pDesc->texture_mask_128, 128, 128))
	{
		PX_ASSERTX("Load game button texture failed");
		return PX_NULL;
	}

	if (!PX_LoadJsonFromJsonFile(mp_static,&pDesc->language,"assets/language.json"))
	{
		PX_ASSERTX("Load game language failed");
		return PX_NULL;
	}

	PX_GeoDrawSolidCircle(&pDesc->texture_mask_128, 64, 64, 64, PX_COLOR_WHITE);



	pDesc->printer = PX_Object_MessageCreate(pObject, 0, 0);
	PX_ObjectSetVisible(pDesc->printer, PX_FALSE);

	pDesc->counter_down = PX_Object_CounterDownCreate(mp, pparent, 0, 0, 32, &pDesc->fm);
	for (i = 0; i < PX_COUNTOF(pDesc->select_button); i++)
	{
		pDesc->select_button[i] = PX_Object_PushButtonCreate(mp, pparent, 0, 0, pDesc->select_button_texture.width, pDesc->select_button_texture.height, "", &pDesc->fm);
		PX_ObjectSetVisible(pDesc->select_button[i], PX_FALSE);
		PX_Object_PushButtonSetBorder(pDesc->select_button[i],  PX_FALSE);
		PX_Object_PushButtonSetTexture(pDesc->select_button[i], &pDesc->select_button_texture);
		PX_ObjectRegisterEvent(pDesc->select_button[i], PX_OBJECT_EVENT_EXECUTE, PX_Object_game_event_MakeChoose, pObject);
	}

	for (i = 0; i < PX_COUNTOF(pDesc->stamps_loser); i++)
	{
		pDesc->stamps_loser[i] = PX_Object_StampCreate(mp, pObject, 0, 0);
		PX_ObjectSetVisible(pDesc->stamps_loser[i], PX_FALSE);
	}

	pDesc->waiting = PX_Object_WaitingCreate(mp, pObject, 0, 0,64, 8, 0.5f, PX_COLOR_WHITE);
	PX_Object_WaitingSetText(pDesc->waiting, PX_JsonGetString(&pDesc->language, "wait"), &pDesc->fm);
	PX_ObjectSetVisible(pDesc->waiting, PX_FALSE);

	PX_AbiCreate_DynamicWriter(&pDesc->scene_abi, mp);
	PX_AbiCreate_DynamicWriter(&pDesc->game_abi, mp);
	PX_AbiCreate_DynamicWriter(&pDesc->players_abi, mp);

	PX_VectorInitialize(mp,&pDesc->texture_player_profile128, sizeof(px_texture), 8);
	PX_VectorInitialize(mp,&pDesc->texture_player_profile256, sizeof(px_texture), 8);

	PX_Game_VM_LoadStandard(&pDesc->vm, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "load_scene", PX_Object_game_VM_load_scene, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "load_players", PX_Object_game_VM_load_players, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "load_game", PX_Object_game_VM_load_game, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "get_scene_message", PX_Object_game_VM_get_scene_message, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "message", PX_Object_game_VM_message, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "messageIsEnd", PX_Object_game_VM_messageIsEnd, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "set_button_text", PX_Object_game_VM_set_button_text, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "set_player_index", PX_Object_game_VM_set_player_index, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "get_current_choose", PX_Object_game_VM_get_current_choose, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "reset_choose", PX_Object_game_VM_reset_choose, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "close_choose_buttons", PX_Object_game_VM_close_choose_buttons, pObject);
	PX_VMRegisterHostFunction(&pDesc->vm, "game_is_end", PX_Object_game_VM_game_is_end, pObject);
	PX_VMBeginThreadFunction(&pDesc->vm, 0, "main", 0, 0);
	PX_Object_MessageSetSpeed(pDesc->printer, 3);

	PX_ObjectRegisterEvent(pObject, PX_OBJECT_EVENT_CURSORDOWN, PX_Object_gameOnCursorDown,pObject);
	PX_ObjectRegisterEvent(pObject, PX_OBJECT_EVENT_CURSORUP, PX_Object_gameOnCursorUp, pObject);

	return pObject;
}

px_void PX_Object_gameEnable(PX_Object* pObject)
{
	PX_ObjectSetVisible(pObject, PX_TRUE);
	PX_ObjectSetEnabled(pObject, PX_TRUE);
}

px_void PX_Object_gameDisable(PX_Object* pObject)
{
	PX_ObjectSetVisible(pObject, PX_FALSE);
	PX_ObjectSetEnabled(pObject, PX_FALSE);
}
