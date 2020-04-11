#pragma once
#include "LogServer.hpp"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <sstream>
using std::string;

#define UDP_PORT 9090

class UdpSocket {
public:
    UdpSocket() {
        _sockFd = -1;
        _port = UDP_PORT;
    }

    ~UdpSocket() {}

    void Init() {
        _sockFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (_sockFd < 0) {
            LOG(FATAL, "create socket error");
            exit(1);
        }
    }

    void Bind() {
        sockaddr_in sk;
        sk.sin_family = AF_INET;
        sk.sin_port = htons(_port);
        sk.sin_addr.s_addr = inet_addr("0.0.0.0");
        int ret = bind(_sockFd, (sockaddr*)&sk, sizeof(sk));
        if (ret < 0) {
            LOG(FATAL, "bind udp addinfo error");
            exit(2);
        }
        LOG(INFO, "UDP bind success");
    }

    bool Sendto(const string& msg, const sockaddr_in& cliaddr, const socklen_t& len) {
        ssize_t sendsize = sendto(_sockFd, msg.c_str(), msg.size(), 0, (sockaddr*)&cliaddr, len);
        if (sendsize < 0) {
            LOG(ERROR, "sendto msg error");
            return false;
        } 
        std::stringstream ss;
        string tmp;
        ss << "sendto msg success" << "[" << inet_ntoa(cliaddr.sin_addr) << ":" << ntohs(cliaddr.sin_port) << "]" << msg ;
        getline(ss, tmp);
        LOG(INFO, tmp);
        return true;
    }

    bool Recvfrom(string& str, sockaddr_in& cliaddr, socklen_t& cliaddrlen) {
        char buf[10240];
        int recvsize = recvfrom(_sockFd, buf, sizeof(buf) - 1, 0, (sockaddr*)&cliaddr, &cliaddrlen);
        if (recvsize < 0) {
            LOG(ERROR, "recvform msg error");
            //不能因为读数据失败退出掉
            return false;
        } else {
            str.assign(buf, recvsize);
            LOG(INFO, str);
            std::stringstream ss;
            string tmp;
            ss << "recv msg success" << "[" << inet_ntoa(cliaddr.sin_addr) << ":" << ntohs(cliaddr.sin_port) << "]" << str;

            getline(ss, tmp);
            LOG(INFO, tmp);
            return true;
        }
    }

    void Close() {
        close(_sockFd);
    }

private:
    int _sockFd;
    uint16_t  _port;
};
