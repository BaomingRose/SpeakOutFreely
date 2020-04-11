#pragma once
#include "LogServer.hpp"
#include "ConnectInfo.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>
using std::string;
using std::unordered_map;
using std::vector;

//用户状态
#define OFFLINE 0
#define REGISTERED 1
#define LOGINED 2
#define ONLINE 3

class UserInfo {
public:
    UserInfo(const string& nickName, const string& career, uint32_t userID, const string& password ) 
        : _nickName(nickName),
          _career(career),
          _UserId(userID),
          _password(password),
          _addrlen(-1), 
          _status(OFFLINE) {
              memset(&_cliAddr, 0, sizeof(_cliAddr));
          }

    void SetUserStatus(int Status) {
        _status = Status;
    }

    int getStatus() {
        return _status;
    }

    string getPassWord() const {
        return _password;
    }

    void setCliAddr(const sockaddr_in& cliaddr) {
        memcpy(&_cliAddr, &cliaddr, sizeof(cliaddr));
    }

    void setCliaddrlen(const socklen_t& len) {
        _addrlen = len;
    }

    sockaddr_in& getCliAddr() {
        return _cliAddr;
    } 

private:
    string _nickName;
    string _career;
    uint32_t _UserId;
    string _password;
    struct sockaddr_in _cliAddr;
    socklen_t _addrlen;

    int _status;
};

class UserManager {
public:
    UserManager() : _prepareUserId(1) {
        pthread_mutex_init(&_lock, NULL);
        pthread_mutex_init(&_userIdlock, NULL);
    }
    ~UserManager() {
        pthread_mutex_destroy(&_lock);
        pthread_mutex_destroy(&_userIdlock);
    }

    int Register(const string& nickName, const string& career, const string& password, uint32_t* userid) {

        if (nickName.size() == 0 || career.size() == 0 || password.size() == 0) {
            return OFFLINE;
        }

        //因为预留账号为临界资源
        pthread_mutex_lock(&_userIdlock);
        UserInfo userInfo(nickName, career, *userid, password);
        //更改用户状态
        userInfo.SetUserStatus(REGISTERED);
        //插入到map
        _UsrMap.insert(std::make_pair(_prepareUserId, userInfo));
        //注册成功则将账号返回，并将预留账号++
        *userid = _prepareUserId;
        ++_prepareUserId;
        pthread_mutex_unlock(&_userIdlock);

        return REGISTERED;
    }

    int Login(uint32_t userId, const string& passwd) {
        pthread_mutex_lock(&_lock);
        for (auto& e : _UsrMap) {
            if (e.first == userId) {
                //调式
                //std::cout << "找到账户的密码为：" << e.second.getPassWord();
                if (e.second.getPassWord() == passwd) {
                    if (e.second.getStatus() != ONLINE) {

                        e.second.SetUserStatus(LOGINED);
                        pthread_mutex_unlock(&_lock);
                        return LOGINED;
                    } else {
                        pthread_mutex_unlock(&_lock);
                        LOG(ERROR, "double log");
                        return ONLINE;
                    }
                } else {
                    //调式
                    //printf("pass is not right\n");
                    LOG(ERROR, "password is not right");

                    pthread_mutex_unlock(&_lock);
                    return REGISTERED;
                 }
            }
        }
        pthread_mutex_unlock(&_lock);
        return OFFLINE;
    }

    int LoginOut() {
        return 0;
    }

    bool isLogin(uint32_t userId, const sockaddr_in& cliaddr, const socklen_t& addrlen) {
        //这个错误可以，因为没写<0，无奈啊
        if (sizeof(cliaddr) < 0 || addrlen < 0)
            return false;

        pthread_mutex_lock(&_lock);
        auto it = _UsrMap.find(userId);
        if (it == _UsrMap.end()) {
            LOG(ERROR, "user not exist");
            pthread_mutex_unlock(&_lock);
            return false;
         }

        if (it->second.getStatus() == OFFLINE || it->second.getStatus() == REGISTERED) {
            LOG(ERROR, "user status error");
            pthread_mutex_unlock(&_lock);
            return false;
        }

        if (it->second.getStatus() == ONLINE) {
            //debug
            //std::cout << "在这true?" << std::endl;
            //忘记解锁了，错误
            pthread_mutex_unlock(&_lock);
            return true;
        }

        if (it->second.getStatus() == LOGINED) {
            it->second.SetUserStatus(ONLINE);
            it->second.setCliAddr(cliaddr);
            it->second.setCliaddrlen(addrlen);

            _OnlineUser.push_back(it->second);
            //debug
            std::cout << "vector.pushback" << std::endl;
        }
        pthread_mutex_unlock(&_lock);
        return true;
    }

    vector<UserInfo>& getVector() {
        return _OnlineUser;
    }

private:
    pthread_mutex_t _lock;
    unordered_map<uint32_t, UserInfo> _UsrMap;
    vector<UserInfo> _OnlineUser;

    uint32_t _prepareUserId;
    pthread_mutex_t _userIdlock;
};
