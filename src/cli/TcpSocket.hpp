#pragma once
#include "LogServer.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <sstream>
#include <unistd.h>
using std::string;

#define TCP_SVR_PORT 10001

class TcpSocket {
public:
    TcpSocket(void* chatServer): _sockFd(-1), _tcpPort(TCP_SVR_PORT), _chatServer(chatServer) {  }

    void* getPtr() {
        return _chatServer;
    }

    void init() {
        _sockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (_sockFd < 0) {
            LOG(FATAL, "create tcp socket error");
            exit(6);
        }

    }

    void setSockOpt() {
        //端口可以很快复用
        int opt = 1;
        setsockopt(_sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    void Bind() {
        struct sockaddr_in tcpAddr;
        tcpAddr.sin_family = AF_INET;
        tcpAddr.sin_port = htons(_tcpPort);
        tcpAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
        int ret = bind(_sockFd, (sockaddr*)&tcpAddr, sizeof(tcpAddr));
        if (ret < 0) {
            LOG(FATAL, "bind tcp addrInfo error");
            exit(7);
        }
    }

    void Listen() {
        int ret = listen(_sockFd, 5);
        if (ret < 0){
            LOG(FATAL, "tcp listen error");
            exit(8);
        }
        LOG(INFO, "tcp listen 0.0.0.0:10001");
    }

    bool Accept(TcpSocket& cli) {
        sockaddr_in cliaddr; 
        socklen_t len;
        int res = accept(_sockFd, (sockaddr*)&cliaddr, &len);
        if (res < 0) {
            LOG(FATAL, "tcp accept error");
            return false;
        } else {
            cli._sockFd = res;
        }
        return true;
    }

    //客户端发起连接
    bool Connect(const string& srv_ip, const uint16_t& srvport) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(srvport);
        addr.sin_addr.s_addr = inet_addr(srv_ip.c_str());
        socklen_t len = sizeof(struct sockaddr_in);
        int ret = connect(_sockFd, (struct sockaddr*)&addr, len);
        std::stringstream ss;
        string tmp;
        if (ret < 0) {
            std::cout << ret << std::endl;
            ss << "connect error" << " "<< srv_ip << ":" << srvport;
            getline(ss, tmp);
            LOG(FATAL, tmp);
            return false;
        }
        ss << "connect success" << " "<< srv_ip << ":" << srvport;
        getline(ss, tmp);
        LOG(INFO, tmp);
        return true;
    }

    bool Send(const void* data, int sizeofdata) {
        int sendsize = send(_sockFd, data, sizeofdata, 0);
        if (sendsize !=  sizeofdata) {
            LOG(ERROR, "send error");
            sendsize = send(_sockFd, data, sizeofdata, 0);
        }

        LOG(INFO, "tcp send success");

        return true;
    }

    bool Recv(void* buf, int _recvsize) {
        int recvsize = recv(_sockFd, buf, _recvsize, 0);
        if (recvsize < 0) {
            LOG(ERROR, "tcp recv error");
            return false;
        } else if (recvsize == 0){
            LOG(ERROR, "tcp connect  shutdown");
            return false;
        } else if (recvsize != _recvsize) {
            //std::cout << recvsize << std::endl;
            LOG(ERROR, "tcp recv datasize error");
            return false;
        }
        return true;
    }

    void Close() {
        close(_sockFd);
    }

private:
    int _sockFd;
    uint16_t _tcpPort;

    void* _chatServer;
};
