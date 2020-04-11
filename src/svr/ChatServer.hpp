#pragma once
#include "UdpSocket.hpp"
#include "MsgPool.hpp"
#include "LogServer.hpp"
#include "ConnectInfo.hpp"
#include "TcpSocket.hpp"
#include "UserManager.hpp"
#include "Message.hpp"
#include <unordered_set>
#include <pthread.h>

#define THREAD_COUNT 2

class ChatServer {
public:
    ChatServer() : _msgPool(nullptr),  _tcpsocket(this),  _userManager(nullptr) {
    }

    ~ChatServer() {
        if (_msgPool) {
            delete  _msgPool;
            _msgPool = nullptr;
        }
        if (_userManager) {
            delete  _userManager;
            _userManager = nullptr;
        }
        _tcpsocket.Close();
        _udpsocket.Close();
    }

    void init() {
        _udpsocket.Init();
        _udpsocket.Bind();
        _msgPool = new MsgPool();
        if (!_msgPool) {
            LOG(FATAL, "create msgPool error");
            exit(6);
        }
        LOG(INFO, "Create MsgPool success");

        _userManager = new UserManager();
        if (!_userManager) {
            LOG(FATAL, "create UserManager, error");
            exit(7);
        }

        _tcpsocket.init();
        _tcpsocket.setSockOpt();
        _tcpsocket.Bind();
        _tcpsocket.Listen();
    }

    void Start() {
        //开启生产者消费者线程，一个接受客户端发来的数据，放到数据池
        //另一个消费者线程，从数据池中拿数据广播给所有在线客户端
        pthread_t tid;
        for (int i = 0; i < THREAD_COUNT; ++i) {
            int ret = pthread_create(&tid, NULL, ProductStart, this);
            if (ret < 0) {
                LOG(FATAL, "pthread_create new thread error");
                exit(5);
            }

            ret = pthread_create(&tid, NULL, CosumerStart, this);
            if (ret < 0) {
                LOG(FATAL, "pthread_create new thread error");
                exit(5);
            }
        }
        LOG(INFO, "Udp Service start success");

        while (1) {
            TcpSocket* cli = new TcpSocket(this);
            if (!_tcpsocket.Accept(*cli)) {
                LOG(FATAL, "tcp accept error");
                continue;
            } else {
                LOG(INFO, "tcp accept success");
            }

            pthread_t tid;
            int ret = pthread_create(&tid, NULL, LoginRegStart, cli);
            if (ret < 0) {
                LOG(ERROR, "create user LoginRegin pthread error");
                continue;
            }
            LOG(INFO, "create tcpconnect thread success");
        }
    }

private:
    static void* ProductStart(void* arg) {
        pthread_detach(pthread_self());
        ChatServer* cs = (ChatServer*)arg;
        while (1) {
            //recvfrom
            string recvMsg;
            sockaddr_in cli;
            socklen_t len = sizeof(cli);
            cs->_udpsocket.Recvfrom(recvMsg, cli, len);

            Message jsonmsg;
            jsonmsg.deserialize(recvMsg);
            //验证是否已登录的用户发送的
            //  判断是否第一次发送
            //这次验证需要userID才能继续, 但是没有userId
            
            //debug
            //std::cout << jsonmsg.getUserId() << std::endl;
            if (cs->_userManager->isLogin(jsonmsg.getUserId(), cli, len)) {
                LOG(INFO, "push msg to MsgPool");

                cs->_msgPool->PushMsgToPool(recvMsg);

                //std::cout << recvMsg << std::endl;
            } else {
                LOG(ERROR, "discarded the msg");
            }
        }

        return NULL;
    }

    static void* CosumerStart(void* arg) {
        pthread_detach(pthread_self());
        ChatServer* cs = (ChatServer*)arg;
        while (1) {
            //Broadcast
            string msgOfPool;
            cs->_msgPool->PopMsgFromPool(msgOfPool);
            cs->Broadcast(msgOfPool);
            //debug
            //std::cout << "Broadcast" << std::endl;
        }

        return NULL;
    }

    static void* LoginRegStart(void* arg) {
        pthread_detach(pthread_self());
        //注册，登录
        TcpSocket* ts = (TcpSocket*)arg;
        char info[1];
        if (!ts->Recv(info, 1)) {
            LOG(ERROR, "recv tagType error");
            ts->Close();
            return NULL;
        } 

        uint32_t userId = -1;
        int userStatus = -1;
        switch(info[0]) {
            case REGISTER : 
            //case '1' : 
                userStatus = dealRegister(ts, &userId);
                break;
            case LOGIN :
            //case '2' :
                userStatus = dealLogin(ts, &userId);
                if (userStatus != LOGINED)
                    LOG(ERROR, "client login error");
                break;
            case LOGINOUT :
            //case '3' :
                userStatus = dealLoginOut(ts, &userId);
                break;
            default :
                userStatus = OFFLINE;
                LOG(ERROR, "recv request type not effectice value");
                break;
        }

        ReplyInfo ri;
        ri._status = userStatus;
        ri._userID = userId;

        if (!ts->Send(&ri, sizeof(ri))) {
            LOG(ERROR, "Reply Client ERROR");
        }
        LOG(INFO, "Reply Client success");

        ts->Close();
        delete ts;

        return NULL;
    }

    static int dealRegister(TcpSocket* ts, uint32_t* userId) {
        ReginInfo ri;
        if (!ts->Recv(&ri, sizeof(ri))) {
            return OFFLINE;
        }
        ChatServer* cs = (ChatServer*)(ts->getPtr());
        return cs->_userManager->Register(ri._NickName, ri._career, ri._passWord, userId);
    }

    static int dealLogin(TcpSocket* ts, uint32_t* userId) {
        LoginInfo li;
        if (!ts->Recv(&li, sizeof(li))) {
            return OFFLINE;
        }

        //debug
        //printf("li.password: %s\n", li._passWord);

        *userId = li._userID;
        ChatServer* cs = (ChatServer*)ts->getPtr();
        return cs->_userManager->Login(li._userID, li._passWord);
    }

    static int dealLoginOut(TcpSocket* ts, uint32_t* userId) {
        ts = NULL;
        userId = NULL;
        return 0;
    }

    void Broadcast(const string& msg) {
        vector<UserInfo>& vu = _userManager->getVector();
        for (auto& e : vu) {
            //std::cout << "进循环了" << std::endl;
            _udpsocket.Sendto(msg, e.getCliAddr(), sizeof(e.getCliAddr()));
            //debug
            //std::cout << "发送消息" << std::endl;
        }
    }

private:
    UdpSocket _udpsocket;
    MsgPool* _msgPool;
    TcpSocket _tcpsocket;
    UserManager* _userManager;
};
