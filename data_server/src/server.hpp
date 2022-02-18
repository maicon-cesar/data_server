#pragma once
#include <boost/asio.hpp>

#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <connection.hpp>
#include <log.hpp>
#include <properties.hpp>

using boost::asio::ip::tcp;

class Server {
  public:
    Server(boost::asio::io_context &io_context, properties::Properties &properties)
        : io_context_(io_context), signal_(io_context, SIGCHLD), acceptor_(io_context, {tcp::v4(), properties.port}),
          properties_(properties), socket_(io_context) {

        wait_for_signal();
        accept();
    }

  private:
    void wait_for_signal() {
        signal_.async_wait([this](boost::system::error_code ec, int signo) {
            if (acceptor_.is_open()) {
                int status = 0;
                while (waitpid(-1, &status, WNOHANG) > 0) {
                }

                wait_for_signal();
            }
        });
    }

    void accept() {
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket new_socket) {
            log("[id:" << std::to_string(++cnx_id_) << "] Client connected.");

            if (!ec) {
                io_context_.notify_fork(boost::asio::io_context::fork_prepare);

                if (fork() == 0) {
                    io_context_.notify_fork(boost::asio::io_context::fork_child);
                    acceptor_.close();
                    signal_.cancel();

                    connection = std::make_shared<Connection>(io_context_, new_socket, properties_, cnx_id_);
                    connection->run();

                } else {
                    io_context_.notify_fork(boost::asio::io_context::fork_parent);
                    socket_.close();

                    accept();
                }
            } else {
                log_err("Accept error: " << ec.message());
                accept();
            }
        });
    }

    boost::asio::io_context &io_context_;
    boost::asio::signal_set signal_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    properties::Properties properties_;
    uint16_t cnx_id_ = 0;
    std::shared_ptr<Connection> connection = nullptr;
};
