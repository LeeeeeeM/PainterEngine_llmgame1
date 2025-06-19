#name "main"

host void sleep(int ms);
host void messagebox(string title);
host void printf(string title);


host int abiget_int(memory* pabi, string payload);
host float abiget_float(memory* pabi, string payload);
host string abiget_string(memory* pabi, string payload);
host memory abiget_data(memory* pabi, string payload);
host void abiset_int(memory* pabi, string payload, int value);
host void abiset_float(memory* pabi, string payload, float value);
host void abiset_string(memory* pabi, string payload, string value);
host void abiset_data(memory* pabi, string payload, memory *pvalue);
host void set_button_text(int index, string text);
host void set_player_index(int index);
host int game_is_end();

host memory abicall(memory*pabi);
host memory abipost(string opcode,string payload);
host memory getcookie();
host memory getprofile();

//game api
host void load_scene(memory* pabi);
host void load_game(memory* pabi);
host void load_players(memory* pabi);
host int  get_current_choose();
host void reset_choose();

//global
int scene_id=0;


void game_error()
{
    messagebox("未知错误,请重启游戏!");
    while(1)
    {
        sleep(1000);
    }
}



int get_scene_id()
{
    memory ret_abi = abipost("get", "scene.id");
    if (memlen(ret_abi)!=0)
    {
        return abiget_int(&ret_abi, "id");
    }
    else
    {
        return 0;
    }
}

int load_players_info()
{
    int i;
    memory ret_abi;
    ret_abi=abipost("get", "myid");
    if(memlen(ret_abi)!=0)
    {
        int player_index=abiget_int(&ret_abi, "player_index");
        printf("player_index:"+string(player_index));
        set_player_index(player_index);       
    }
    else
    {
        printf("load myid error");
        return 0;
    }
    

    ret_abi=abipost("get", "players");
    if(memlen(ret_abi)!=0)
    {
        printf("load players");
        load_players(&ret_abi);
        set_button_text(0,"我要摸鱼!!!");
        set_button_text(1,"我要加班");
        int player_count=abiget_int(&ret_abi, "player_count");
        for (i = 2; i < player_count+2; i++)
        {
            string player_name = abiget_string(&ret_abi, "player[" + string(i - 2) + "].name");
            printf("player name:"+player_name);
            set_button_text(i, "举报 "+player_name+" 在摸鱼");
        }
    }
    else
    {
        printf("load players error");
        return 0;
    }
    printf("load players info success");
    return 1;
}

int new_scene()
{
   //加载游戏数据
   memory ret_abi=abipost("get", "game");
   if(memlen(ret_abi)!=0)
   {
       printf("load game");
       load_game(&ret_abi);
   }
   else
   {
    printf("load game error");
    return 0;
   }
   //加载场景数据
   ret_abi = abipost("get", "scene");
   if(memlen(ret_abi)!=0)
   {
      printf("load scene");
      load_scene(&ret_abi);
   }
   else
   {
    printf("load scene error");
    return 0;
   }
   return 1;
}


int make_choose()
{
    memory abi;
    abiset_string(&abi, "opcode", "make_choose");
    memory cookie = getcookie();
    abiset_data(&abi, "cookie", &cookie);
    abiset_int(&abi, "choose", get_current_choose());
    memory ret_abi = abicall(&abi);
    string ret_string= abiget_string(&ret_abi, "return");
    if (ret_string=="ok")
    {  
        return 1;
    }
    else if (ret_string=="error")
    {
        return 0;
    }
    else
    {
        return 0;
    }
}

int main()
{
    while (1)
    {
        printf("load players info");
        if(load_players_info())
           break;
    }
    
    printf("begin game loop");
    while (1)
    {
        int remote_scene_id=get_scene_id();
        if(remote_scene_id!=0&&scene_id!=remote_scene_id)
        {
            printf("new scene");
            if(new_scene())
            {
                reset_choose();
                scene_id=remote_scene_id;
            }
        }

        if(get_current_choose()!=-1)
        {           
           if(make_choose())
           {
             reset_choose();
           }
        }

        if(game_is_end())
        {
            printf("game is end");
            break;
        }
        sleep(100);
    }
    sleep(10000);
    messagebox("游戏结束,请重启游戏!");
}