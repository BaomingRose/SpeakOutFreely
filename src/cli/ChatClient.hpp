#pragma once
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <unordered_set>
#include "TcpSocket.hpp"
#include "udpsocket.hpp"
#include "ConnectInfo.hpp"
#include "LogServer.hpp"
#include "ConnectInfo.hpp"
#include "UserManager.hpp"
#include "CliWindow.hpp"
using std::string;

#define MESSAGE_MAX_SIZE 1024

struct Myself {
    string _nickname;
    string _career;
    string _passWord;
    uint32_t _userId;
};

class ChatClient {
public:
    ChatClient(string svrip = "127.0.0.1") : _tcpsock(NULL), _udpsock(), _svrIp(svrip) {
    }

    ~ChatClient() {
        _tcpsock.Close();
        _udpsock.Close();
    }

    void Init() {
        _udpsock.Init();
        _tcpsock.init();
    }

    bool connectToServer() {
        return _tcpsock.Connect(_svrIp, TCP_SVR_PORT);
    }

    bool regiser() {
        if (!connectToServer()) {
            return false;
        }

        char type = REGISTER;
        _tcpsock.Send(&type, 1);

        ReginInfo ri;
        std::cout << "please input your nickname:";
        std::cin >> ri._NickName;
        std::cout << "please input your career:";
        std::cin >> ri._career;

        while (1) {
            std::cout << "please input your password:";
            string password;
            std::cin >> password;
            std::cout << "please input your password again:";
            string passAgain;
            std::cin >> passAgain;
            if (password == passAgain) {
                strcpy(ri._passWord, password.c_str());
                break;
            } else {
                std::cout << "password Two input inconsistencies" << std::endl;
            }
        }

        _tcpsock.Send(&ri, sizeof(ri));
        ReplyInfo rep;
        if (_tcpsock.Recv(&rep, sizeof(rep))) {
            if (rep._status != REGISTERED) {
                printf("注册失败\n");
                _tcpsock.Close();
                return false;
            } else {
                printf("注册成功, userId = %d\n", rep._userID);
                _me._nickname = ri._NickName;
                _me._career = ri._career;
                //没有修改自身密码导致登录错误
                _me._passWord = ri._passWord;
                _me._userId = rep._userID;
                _tcpsock.Close();
                return true;
            }
        }
        _tcpsock.Close();
        return false;
    }

    bool login() {
        if (!connectToServer()) {
            return false;
        }

        char type = LOGIN;
        _tcpsock.Send(&type, 1);
        LoginInfo li;
        li._userID = _me._userId;
        strcpy(li._passWord, _me._passWord.c_str());

        //调式
        //printf("password : %s\n", li._passWord);

        _tcpsock.Send(&li, sizeof(li));

        ReplyInfo rep;
        if (_tcpsock.Recv(&rep, sizeof(rep))) {
            if (rep._status != LOGINED) {
                printf("Login statu: %d\n", rep._status);
                printf("登录失败\n");
                _tcpsock.Close();
                return false;
            } else {
                _tcpsock.Close();
                printf("登录成功\n");
                return true;
            }
        }
        _tcpsock.Close();
        return false;
    }

    bool Send(const std::string& msg) {
        sockaddr_in peeraddr;
        peeraddr.sin_family = AF_INET;
        peeraddr.sin_port = htons(UDP_PORT);
        peeraddr.sin_addr.s_addr = inet_addr(_svrIp.c_str());
        if (_udpsock.Sendto(msg, peeraddr, sizeof(peeraddr))) {
            return true;
        }
        return false;
    }

    bool Recv(string& msg) {
        sockaddr_in peeraddr;
        socklen_t len = sizeof(peeraddr);
        if (_udpsock.Recvfrom(msg, peeraddr, len)) {
            return true;
        }
        return false;
    }

    Myself& getMyself() {
        return _me;
    }

    void pushUser(const string& nick_career) {
        _onlineUser.insert(nick_career);
    }

    std::unordered_set<std::string>& getonlineUser() {
        return _onlineUser;
    }

private:
    TcpSocket _tcpsock;
    UdpSocket _udpsock;

    //保存服务端的ip
    string _svrIp;

    //客户端自己的信息
    Myself _me;

    //保存在线用户
    std::unordered_set<std::string> _onlineUser;
};
