#ifndef __TCPSERVER_HPP_
#define __TCPSERVER_HPP_

#include <iostream>
#include <string>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include "threadpool.hpp"

using namespace std;
class tcpServer
{
private:
    int _listen_sock; 
    int _port; 
    ThreadPool *tp;
    static unordered_map<string, string> dict;
public:
    tcpServer(int port) : _port(port)
    {}

    void Initserver()
    {
        tp = Singleton().GetInstance();
        _listen_sock = socket(AF_INET, SOCK_STREAM, 0);
        if(_listen_sock < 0)
        {
            cerr << "listen socket error" << endl;
            exit(2); 
        }
        sockaddr_in local_addr;
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        local_addr.sin_port = htons(_port);
        if(bind(_listen_sock, (sockaddr *)&local_addr, sizeof(local_addr)) != 0)
        {
            cerr << "bind error" << endl;
            exit(3); 
        }
        if(listen(_listen_sock, 5) != 0) 
        {
            cerr << "listen error" << endl;
            exit(4);
        }
    }

    static void *serverIO(int sock, string ipaddr)
    {
        while(1) 
        {
            char buf[1024];
            ssize_t size = recv(sock, buf, sizeof(buf)-1, 0);
            if(size == 0)
            {
                cout << ipaddr << " client quit." << endl;
                close(sock);
                break; 
            }
            else if(size < 0)
                cerr << "recv error" << endl;
            else
            {
                buf[size] = 0;
                cout << "From " << ipaddr << " recv message: " << buf << endl;
                string ret;
                auto it = dict.find(buf);
                if(it != dict.end())
                    ret = it->second;
                else
                    ret = "Not found!";
                if(send(sock, ret.c_str(), ret.size(), 0) < 0)
                    cout << "send error" << endl;
            }
        }
    }

    void Run()
    {
        while(1)
        {
            sockaddr_in useraddr;
            socklen_t len = sizeof(useraddr);
            int sock = accept(_listen_sock, (sockaddr *)&useraddr, &len);
            if(sock < 0)
            {
                cerr << "accept error" << endl;
                continue;
            }
            cout << "IP: "<< inet_ntoa(useraddr.sin_addr) <<" client coming..." << endl;
            tp->PutTask(Task(sock, inet_ntoa(useraddr.sin_addr), serverIO));
        }
    }

    ~tcpServer()
    {
        if(_listen_sock >= 0)
            close(_listen_sock);
    }
};

unordered_map<string, string> tcpServer::dict = {
            {"apple", "苹果"},
            {"banana", "香蕉"},
            {"orange", "橘子"}
        };

#endif //__TCPSERVER_HPP_
