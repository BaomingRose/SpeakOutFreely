#pragma once
#include <iostream>
#include <queue>
#include <string>
#include <pthread.h>
using std::queue;
using std::string;

#define MSG_POOL_SIZE 1024

class MsgPool {
public:
    MsgPool() : _capacity(MSG_POOL_SIZE) {
        pthread_mutex_init(&_queLock, NULL);
        pthread_cond_init(&_queSynCom, NULL);
        pthread_cond_init(&_queSynPro, NULL);
    }

    ~MsgPool() {
        pthread_mutex_destroy(&_queLock);
        pthread_cond_destroy(&_queSynCom);
        pthread_cond_destroy(&_queSynPro);
    }

    void PushMsgToPool(const string& msg) {
        pthread_mutex_lock(&_queLock);
        //debug
        //std::cout << "加锁成功" << std::endl;
        while (IsFull()) {
            pthread_cond_wait(&_queSynPro, &_queLock);
        }
        _queMsg.push(msg);
        //debug
        //std::cout << "queue.push 成功 =" << std::endl;
        pthread_mutex_unlock(&_queLock);
        pthread_cond_signal(&_queSynCom);
        //std::cout << "唤醒消费线程" << std::endl;
    }

    void PopMsgFromPool(string& msg) {
        pthread_mutex_lock(&_queLock);
        while (_queMsg.empty()) {
            pthread_cond_wait(&_queSynCom, &_queLock);
        }
        //debug
        //std::cout << "我被唤醒了" << std::endl;
        msg = _queMsg.front();
        _queMsg.pop();
        pthread_mutex_unlock(&_queLock);
        pthread_cond_signal(&_queSynPro);
        //debug
        //std::cout << "我要唤醒生产线程" << std::endl;
    }

    bool IsFull() {
        return _queMsg.size() == _capacity;
    }

private:
    size_t _capacity;
    queue<string> _queMsg;
    pthread_mutex_t _queLock;
    pthread_cond_t _queSynCom;
    pthread_cond_t _queSynPro;
};
