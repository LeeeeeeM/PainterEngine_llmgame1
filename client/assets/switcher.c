#name "main"

//stdlib
host void sleep(int ms);
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
host void messagebox(string title);
host void printf(string title);
//game
host void EnterLogin();
host void EnterLobby();
host void EnterGame();
host void SwitcherAnimation();



void game_error()
{
    messagebox("未知错误,请重启游戏!");
    while(1)
    {
        sleep(1000);
    }
}

string game_querystate()
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

void switcher()
{
    string state = game_querystate();
    printf("state:"+state);
    if (state == "waiting"||state == "ready")
    {
        printf("switch to lobby");
        SwitcherAnimation();
        sleep(600);
        EnterLobby();
    }
    else if (state == "game")
    {
        printf("switch to game");
        SwitcherAnimation();
        sleep(600);
        EnterGame();
    }
    else{
        game_error();
    }
}



int main()
{
    SwitcherAnimation();
    sleep(800);
    EnterLogin();
    return 1;
}