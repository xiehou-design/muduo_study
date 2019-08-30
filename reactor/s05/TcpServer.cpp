//
// Created by jxq on 19-8-29.
//

#include "TcpServer.h"
#include <boost/bind.hpp>
#include "../../base/Logging.h"
#include "../s04/SocketsOps.h"

using namespace muduo;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      name_(listenAddr.toHostPort()),   // 地址：端口号
      acceptor_(new Acceptor(loop, listenAddr)),
      started_(false),
      nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(
            boost::bind(&TcpServer::newConnection, this, _1, _2)
            );
}

TcpServer::~TcpServer()
{

}

void TcpServer::start()
{
    if (!started_)
    {
        started_ = true;
    }

    if (!acceptor_->listenning())
    {
        loop_->runInLoop(
                boost::bind(&Acceptor::listen, get_pointer(acceptor_)));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_
             << "] - new connection [" << connName
             << "] from " << peerAddr.toHostPort();
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connectio
    TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->connectEstablished();
}