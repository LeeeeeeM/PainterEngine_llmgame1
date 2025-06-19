#name "main"

host void sleep(int ms);
host void printf(string title);

host int abiget_int(memory* pabi, string payload);
host float abiget_float(memory* pabi, string payload);
host string abiget_string(memory* pabi, string payload);
host memory abiget_data(memory* pabi, string payload);
host void abiset_int(memory* pabi, string payload, int value);
host void abiset_float(memory* pabi, string payload, float value);
host void abiset_string(memory* pabi, string payload, string value);
host void abiset_data(memory* pabi, string payload, memory *pvalue);
host memory abicall(memory*pabi);
host memory abipost(string opcode,string payload);
host memory getcookie();
host memory getprofile();
host void updateplayerinfo(memory* pabi);
host void messagebox(string title);
host void printf(string title);
host void readybackground();
host void BeginGame();

int state_id;

void game_error()
{
    messagebox("未知错误,请重启游戏!");
    while(1)
    {
        sleep(1000);
    }
}

int query_state_id()
{
    while(1)
    {
        memory ret_abi = abipost("get", "game.state_id");
        if (memlen(ret_abi)!=0)
        {
            return abiget_int(&ret_abi, "state_id");
        }
        else
        {
            sleep(500);
        }
    }
    
}

string query_state()
{
    while(1)
    {
        memory ret_abi = abipost("get", "game.state");
        if (memlen(ret_abi)!=0)
        {
            return abiget_string(&ret_abi, "state");
        }
        else
        {
            sleep(500);
        }
    }
}

void UpdatePlayers()
{
    memory abi;
    abiset_string(&abi, "opcode", "query_players_info");
    memory cookie = getcookie();
    abiset_data(&abi, "cookie", &cookie);
    memory ret_abi = abicall(&abi);
    string ret_string= abiget_string(&ret_abi, "return");
    if (ret_string=="ok")
    {  
        updateplayerinfo(&ret_abi);
    }
    else if (ret_string=="error")
    {
        game_error();
    }
    else
    {
        sleep(500);
    }
}


void update()
{
    while(1)
    {
        int current_state_id = query_state_id();
        if(state_id != current_state_id)
        {
            if(query_state()=="ready")
            {
                readybackground();
            }
            else if(query_state()=="game")
            {
                printf("begin game");
                BeginGame();
                return;
            }
            UpdatePlayers();
            state_id= current_state_id;
        }
        sleep(1000);
    }
    
}