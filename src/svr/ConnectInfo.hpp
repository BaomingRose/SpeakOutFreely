#pragma once
#include <iostream>

#define REGISTER 0
#define LOGIN 1
#define LOGINOUT 2

struct ReginInfo {
    char _NickName[15];
    char _career[20];
    char _passWord[20];
};

struct LoginInfo {
    uint32_t _userID;
    char _passWord[20];
};

enum USerStatus {
    U_REGFAILED = 0, //注册失败    
    U_REGISTERED,    //注册成功    
    U_LOGINFAILED,   //登录失败    
    U_LOGINED        //登录成功    
};

struct ReplyInfo {
    int _status;
    int _userID;
};

