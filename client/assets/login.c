#name "main"

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
host string getname();
host void messagebox(string title);
host void loginok();
void login()
{
    messagebox("签入中...");
    sleep(1000);
    while(1)
    {
        memory abi,cookie,profile;
        string name;
        abiset_string(&abi, "opcode", "login");
        cookie = getcookie();
        profile = getprofile();
        name = getname();
        abiset_data(&abi, "cookie", &cookie);
        abiset_string(&abi, "name",name);
        abiset_data(&abi, "profile", &profile);
        //return
        memory ret_abi = abicall(&abi);
        string ret= abiget_string(&ret_abi, "return");
        if (ret=="full")
        {
            messagebox("所有房间已满.迟点再来吧");
            Sleep(3000);
            break;
        }
        else if (ret=="ok")
        {
            messagebox("签入成功");
            sleep(500);
            loginok();
            break;
        }
        else if (ret=="error")
        {
             messagebox("未知错误");
             Sleep(3000);
            break;
        }
    }
    messagebox("");
}