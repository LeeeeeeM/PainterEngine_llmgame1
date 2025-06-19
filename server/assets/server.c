//server
#name "main"

#define GAME_START_PLAYER_COUNT 3

host int rand();
host void printf(string msg);
host void sleep(int ms);

host int load_scene(string scene);
host int get_login_player_count();

host void game_set_abi_string(string playload,string data);
host void game_set_abi_int(string playload,int data);
host string game_get_abi_string(string playload);
host int game_get_abi_int(string playload);

host void game_reset();
host void game_start();
host void game_synchronous();

int g_player_choose[GAME_START_PLAYER_COUNT];
int g_scene_p;

void game_error(string msg)
{
    printf(msg);
    while(1)
    {
        sleep(1000);
    }
}


int begin_scene(string scene_json)
{
    int i;
    //重置所有玩家选项状态
    for(i=0;i<GAME_START_PLAYER_COUNT;i++)
    {
        game_set_abi_int("game.players.player["+string(i)+"].choose",-1);
    }
    if(!load_scene(scene_json))
    {
      game_error("load scene "+scene_json+" failed");
      return 0;
    }
    printf("load scene "+scene_json+" success");
    return 1;
}

int player_is_dead(int player_index)
{
    string player_state=game_get_abi_string("game.players.player["+string(player_index)+"].state");
    return player_state=="die";
}

void wait_sense()
{
    printf("wait sense");
    int i;
    int all_player_ready=0;
    int duration=game_get_abi_int("scene.duration");
    //重置所有玩家选项
    for(i=0;i<GAME_START_PLAYER_COUNT;i++)
    {
        game_set_abi_int("game.players.player["+string(i)+"].choose",-1);
    }

    printf("get scene duration="+string(duration));
    while(1)
    {
        if(duration<=0)
        {
            printf("scene time up");
            break;
        }
        all_player_ready=1;
        for(i=0;i<GAME_START_PLAYER_COUNT;i++)
        {
            if(!player_is_dead(i))
            {
                int get_player_choose=game_get_abi_int("game.players.player["+string(i)+"].choose");
                if(get_player_choose==-1)
                {
                    all_player_ready=0;
                    break;
                }
            }
            else
            {
                printf("player "+string(i)+" is dead,skip wait sense");
            }
        }

        if(all_player_ready)
        {
            printf("all player ready");
            break;
        }

        sleep(500);
        duration=duration-500;
    }
}

void load_moyu_succeed_scene(int iplayer)
{
    string message;
    if(!begin_scene("assets/scene/moyu_succeed.json"))
    {
        game_error("load scene assets/scene/moyu_succeed.json failed");
    }
    printf("load scene assets/scene/moyu_succeed.json success");
    message=game_get_abi_string("scene.message[0]");
    string player_name=game_get_abi_string("game.players.player["+string(iplayer)+"].name");
    game_set_abi_string("scene.message[0]","太爽了!"+player_name+"在老板眼皮底下摸鱼却没有被抓,摸鱼的感觉真是爽翻了!");
    wait_sense();
}

void load_moyu_failed_scene(int iplayer)
{
    if(!begin_scene("assets/scene/moyu_failed.json"))
    {
        game_error("load scene assets/scene/moyu_failed.json failed");
    }
    printf("load scene assets/scene/moyu_failed.json success");
    string player_name=game_get_abi_string("game.players.player["+string(iplayer)+"].name");
    game_set_abi_string("scene.message[0]",player_name+"上班摸鱼被抓，老板用仿佛欠了他几百万的眼神视奸了他，然后让他认识到了什么叫人心险恶。裁员值下降了");
    wait_sense();
}

void load_work_scene(int iplayer)
{
    if(!begin_scene("assets/scene/work.json"))
    {
        game_error("load scene assets/scene/work.json failed");
    }
    string player_name=game_get_abi_string("game.players.player["+string(iplayer)+"].name");
    game_set_abi_string("scene.message[0]",player_name+"又双叒叕狠狠地享受了996福报,感觉这个人都不好了。健康值下降了");
    wait_sense();
}

void load_report_succeed_scene(int i_src_player,int i_target_player)
{
    if(!begin_scene("assets/scene/report_succeed.json"))
    {
        game_error("load scene assets/scene/report.json failed");
    }
    string src_player_name=game_get_abi_string("game.players.player["+string(i_src_player)+"].name");
    string target_player_name=game_get_abi_string("game.players.player["+string(i_target_player)+"].name");
    
    game_set_abi_string("scene.message[0]",src_player_name+"向老板举报了"+target_player_name+"正在摸鱼,"+target_player_name+"被老板抓住了,被狠狠PUA到怀疑人生。"); 
    game_set_abi_string("scene.message[1]",src_player_name+"的裁员值上升了,"+target_player_name+"的裁员值下降了。");
    wait_sense();
}

void load_report_failed_scene(int i_src_player,int i_target_player)
{
    if(!begin_scene("assets/scene/report_failed.json"))
    {
        game_error("load scene assets/scene/report.json failed");
    }
    string src_player_name=game_get_abi_string("game.players.player["+string(i_src_player)+"].name");
    string target_player_name=game_get_abi_string("game.players.player["+string(i_target_player)+"].name");
    game_set_abi_string("scene.message[0]",src_player_name+"恶意举报"+target_player_name+"摸鱼,老板让他有多远滚多远去");
    game_set_abi_string("scene.message[1]",src_player_name+"的裁员值下降了"); 
    wait_sense();
}

void load_fire1_scene(int i_player)
{
    string message;
    if(!begin_scene("assets/scene/fire1.json"))
    {
        game_error("load scene assets/scene/fire.json failed");
    }
    message=game_get_abi_string("scene.message[0]");
    string player_name=game_get_abi_string("game.players.player["+string(i_player)+"].name");
    game_set_abi_string("scene.message[0]",player_name+" "+message);
    wait_sense();
}

void load_fire2_scene(int i_player)
{
    string message;

    if(!begin_scene("assets/scene/fire2.json"))
    {
        game_error("load scene assets/scene/fire2.json failed");
    }
    message=game_get_abi_string("scene.message[0]");
    string player_name=game_get_abi_string("game.players.player["+string(i_player)+"].name");
    game_set_abi_string("scene.message[0]",player_name+" "+message);
    wait_sense();
}

int target_player_is_moyu(int target_player)
{
    int choose_index=g_player_choose[target_player];
    printf("target player "+string(target_player)+" choose index "+string(choose_index));
    return choose_index==0;
}

int target_player_is_breport(int target_player)
{
    int i;
    for(i=0;i<GAME_START_PLAYER_COUNT;i++)
    {
        if(g_player_choose[i]>1)
        {
            int report_index=g_player_choose[i]-2;
            if(report_index==target_player)
            {
                return 1;
            }
        }
    }
    return 0;
}

void begin_player_end_scene(int iplayer)
{
    string player_state=game_get_abi_string("game.players.player["+string(iplayer)+"].state");
    printf(string(iplayer)+":player end scene state=" +player_state);
    if(player_state=="die")
    {
        printf("player "+string(iplayer)+" is dead,skip end scene");
        return;
    }

    int p=g_scene_p;
    int roll=rand()%100+1;
    int hp=game_get_abi_int("game.players.player["+string(iplayer)+"].hp");
    int pp=game_get_abi_int("game.players.player["+string(iplayer)+"].pp");
    int breport=target_player_is_breport(iplayer);

    printf("p="+string(p));
    printf("roll="+string(roll));
    printf("hp="+string(hp));
    printf("pp="+string(pp));
    printf("breport="+string(breport));

    if(g_player_choose[iplayer]>1)
    {
       //举报
       int target_player=g_player_choose[iplayer]-2;
       if(target_player_is_moyu(target_player))
       {
           //举报摸鱼成功
           printf("player "+string(iplayer)+" report player "+string(target_player)+" succeed");
           pp=pp+20;
           if(pp>100)
           {
               pp=100;
           }
           game_set_abi_int("game.players.player["+string(iplayer)+"].pp",pp);
           printf("player "+string(iplayer)+" pp is "+string(pp));
           load_report_succeed_scene(iplayer,target_player);
       }
       else
       {
           //举报摸鱼失败
           printf("player "+string(iplayer)+" report player "+string(target_player)+" failed");
           load_report_failed_scene(iplayer,target_player);
           pp=pp-30;
           if(pp<0)
              pp=0;
           game_set_abi_int("game.players.player["+string(iplayer)+"].pp",pp);
           printf("player "+string(iplayer)+" pp is "+string(pp));
       }
    }
    else if(g_player_choose[iplayer]==0)
    {
        if(breport||roll<=p)//被举报或摸鱼被抓
        {
            if(!breport)
            {
                load_moyu_failed_scene(iplayer);
            }
            pp=pp-50;
            if(pp<=0)
            {
                pp=0;
                game_set_abi_int("game.players.player["+string(iplayer)+"].pp",0);
            }
            else
            {
                game_set_abi_int("game.players.player["+string(iplayer)+"].pp",pp);
                printf("player "+string(iplayer)+" pp is "+string(pp));
            }
        }
        else
        {
            //摸鱼成功
            hp=hp+30;
            if(hp>100)
            {
                hp=100;
            }
            game_set_abi_int("game.players.player["+string(iplayer)+"].hp",hp);
            printf("player "+string(iplayer)+" hp is "+string(hp));
            load_moyu_succeed_scene(iplayer);
        }
    }
    else
    {
        load_work_scene(iplayer);
        hp=hp-30;
        if(hp<=0)
        {
            hp=0;
            game_set_abi_int("game.players.player["+string(iplayer)+"].hp",hp);
        }
        else{
            game_set_abi_int("game.players.player["+string(iplayer)+"].hp",hp);
        }
    }
}

void player_die_scene(int iplayer)
{
    string player_state=game_get_abi_string("game.players.player["+string(iplayer)+"].state");
    if(player_state=="die")
    {
        printf("player "+string(iplayer)+" is dead,skip player scene");
        return;
    }
    int hp=game_get_abi_int("game.players.player["+string(iplayer)+"].hp");
    int pp=game_get_abi_int("game.players.player["+string(iplayer)+"].pp");
    if (hp<=0)
    {
        printf("player "+string(iplayer)+" hp is 0,die");
        game_set_abi_string("game.players.player["+string(iplayer)+"].state","die");
        load_fire2_scene(iplayer);
    }
    else if (pp<=0)
    {
        printf("player "+string(iplayer)+" pp is 0,die");
        game_set_abi_string("game.players.player["+string(iplayer)+"].state","die");
        load_fire1_scene(iplayer);
    }
}

void hanle_player_choose()
{
    int i;
    for(i=0;i<GAME_START_PLAYER_COUNT;i++)
    {
        int choose_index=game_get_abi_int("game.players.player["+string(i)+"].choose");
        printf("player "+string(i)+" choose index "+string(choose_index));
        g_player_choose[i]=choose_index;
    }
}

void end_sense()
{
    int i;
    for(i=0;i<GAME_START_PLAYER_COUNT;i++)
    {
        begin_player_end_scene(i);
    }
    for(i=0;i<GAME_START_PLAYER_COUNT;i++)
    {
        player_die_scene(i);
    }
}

int alive_player_count()
{
    int i;
    int count=0;
    for(i=0;i<GAME_START_PLAYER_COUNT;i++)
    {
        string player_state=game_get_abi_string("game.players.player["+string(i)+"].state");
        if(player_state=="alive")
        {
            count++;
        }
    }
    return count;
}

void winner()
{
    int i;
    int winner_index=-1;
    for(i=0;i<GAME_START_PLAYER_COUNT;i++)
    {
        string player_state=game_get_abi_string("game.players.player["+string(i)+"].state");
        if(player_state=="alive")
        {
            winner_index=i;
            break;
        }
    }
    if(winner_index==-1)
    {
        game_error("no winner");
    }
    printf("winner is "+string(winner_index));
    begin_scene("assets/scene/winner.json");
    string message=game_get_abi_string("scene.message[0]");
    string player_name=game_get_abi_string("game.players.player["+string(winner_index)+"].name");
    game_set_abi_string("scene.message[0]",player_name+" "+message);
    wait_sense();
    printf("player win");
}

void lose()
{
    begin_scene("assets/scene/lose.json");
    wait_sense();
    printf("all player lose");
}

void restart_game()
{
    int i;
    int last_player_online_count=0;
    //重置游戏状态
    game_reset();
    printf("PainterEngine llmgame framework");
    printf("vm start");
    //设置游戏场景
    game_set_abi_string("game.state","waiting");
    game_set_abi_int("game.max_player_count",GAME_START_PLAYER_COUNT);
    printf("max_player_count="+string(GAME_START_PLAYER_COUNT));
    while (1)
    {
        int player_count=get_login_player_count();
        if (player_count!=last_player_online_count)
        {
            printf("player count changed "+string(player_count));
            last_player_online_count=player_count;
        }
        if(player_count>=GAME_START_PLAYER_COUNT)
        {
            break;
        }
        sleep(0);
    }
   
    //所有玩家就绪,设置游戏场景,等待一段时间
    printf("all player ready....");
    printf("game.state=ready");
    game_set_abi_string("game.state","ready");//设置游戏状态为ready
    game_synchronous();//同步状态数据
    printf("game start");
    game_start();//开始游戏,准备玩家数据,延迟一段时间加载数据
    sleep(3000);

    //初始化玩家数据
    printf("initialize player data");
    for (i = 0; i < GAME_START_PLAYER_COUNT; i++)
    {
        string player_name = game_get_abi_string("players.player[" + string(i) + "].name");//转写玩家名称
        game_set_abi_string("game.players.player["+string(i)+"].name",player_name);//玩家ID
        game_set_abi_int("game.players.player["+string(i)+"].hp",100);//生命值
        game_set_abi_int("game.players.player["+string(i)+"].pp",100); //裁员值
        game_set_abi_string("game.players.player["+string(i)+"].state","alive");//当前状态
        game_set_abi_int("game.players.player["+string(i)+"].choose",-1);//选项
    }
}

void load_choose_scene_data()
{
    g_scene_p=game_get_abi_int("scene.p");
}


void main()
{
    while(1)
    {
        //重置游戏,并初始化玩家基本数据
        restart_game();

        //游戏开始,先进入初始引导画面
        begin_scene("assets/scene/begin.json");//加载初始引导游戏画面
        printf("game.state=game");
        game_set_abi_string("game.state","game");
        game_synchronous();//同步数据
        wait_sense();//等待玩家选择,直到所有玩家都选择了选项或者时间到

        while(1)
        {
            int rand_scene=rand()%11; //随机选择场景
            begin_scene("assets/scene/"+string(rand_scene)+".json");
            string scene_type=game_get_abi_string("scene.type");
            printf("scene type="+scene_type);
            wait_sense();//等待玩家选择,直到所有玩家都选择了选项或者时间到
            if(scene_type!="normal")
            {
                load_choose_scene_data();
                hanle_player_choose();//处理玩家选择结果
                end_sense();//结束玩家选择,开始处理玩家选择结果
            }

            if(alive_player_count()==1)
            {
                winner();
                break;
            }
            else if(alive_player_count()==0)
            {
                
                lose();
                break;
            }
        }
        game_set_abi_string("game.state","end");//设置游戏状态为end
        printf("game end,wait for restart");
        sleep(10000); //等待10秒
    }
}