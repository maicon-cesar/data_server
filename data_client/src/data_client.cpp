//
// Copyright (c) 2022 Maicon Cesar Canales de Oliveira (maicon.cesar@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <string_view>

#include <log.hpp>

using boost::asio::ip::tcp;

namespace fs = std::filesystem;

constexpr auto OUTPUT_DATA_SIZE_BUFFER = 4096;

class Client {
  public:
    Client(boost::asio::io_context &io_context, const std::string &host, const std::string &port)
        : io_context_(io_context), resolver_(io_context), socket_(io_context) {
        endpoints_ = resolver_.resolve(host, port);
    }

    void connect();
    void disconnect();
    void send_file(std::string file_name);
    void send_message();

  private:
    void do_write();

    boost::asio::io_context &io_context_;
    tcp::socket socket_;
    tcp::resolver resolver_;
    tcp::resolver::results_type endpoints_;
    std::queue<std::vector<char>> buffer_;
};

void Client::connect() {
    boost::asio::async_connect(socket_, endpoints_, [this](boost::system::error_code ec, tcp::endpoint) {
        if (ec) {
            log_err("Connect error: " << ec.message());
        }
    });
}

void Client::disconnect() {
    boost::asio::post(io_context_, [this]() { socket_.close(); });
}

void Client::do_write() {

    if (socket_.is_open()) {
        boost::asio::async_write(socket_, boost::asio::buffer(buffer_.front().data(), buffer_.front().size()),
                                 [this](boost::system::error_code ec, std::size_t length) {
                                     if (!ec) {
                                         buffer_.pop();
                                         if (!buffer_.empty()) {
                                             do_write();
                                         } else {
                                             socket_.close();
                                             log("Sent!");
                                         }
                                     } else {
                                         if (ec == boost::asio::error::broken_pipe) {
                                             log("Disconnected from Server");
                                         } else {
                                             log_err("Error: " << ec.message());
                                         }
                                         socket_.close();
                                     }
                                 });
    } else {
        log("Client is disconnected!");
    }
}

void Client::send_file(std::string file_name) {

    if (!fs::exists(file_name)) {
        log_err("File doesnt exist: " << file_name);
        return;
    }

    boost::system::error_code ec{};
    auto file_size = std::filesystem::file_size(file_name, ec);
    if (ec != boost::system::error_code{}) {
        log("Size of file " << file_name << ": " << file_size);
    } else {
        log_err("Error accessing file '" << file_name << "' message: " << ec.message());
        return;
    }

    std::string line;
    std::ifstream file;

    std::vector<char> output_data(OUTPUT_DATA_SIZE_BUFFER);

    try {
        file.open(file_name, std::ifstream::in | std::ifstream::binary);
    } catch (std::exception &e) {
        log_err("Error to open file: " << e.what());
        return;
    }

    if (!file.is_open())
        throw std::runtime_error("Fail file is not open: " + file_name);

    try {
        size_t to_read = file_size;

        while (to_read) {
            if (to_read > OUTPUT_DATA_SIZE_BUFFER) {
                file.read(output_data.data(), OUTPUT_DATA_SIZE_BUFFER);
                to_read -= OUTPUT_DATA_SIZE_BUFFER;
            } else {
                output_data.resize(to_read);
                file.read(output_data.data(), to_read);
                to_read = 0;
            }
            buffer_.push(output_data);
        }

    } catch (std::exception &e) {
        log_err("Error to read file: " << e.what());
        return;
    }

    if (file.is_open())
        file.close();

    boost::asio::post(io_context_, [this]() { do_write(); });
}

void Client::send_message() {
    std::string message = "The quick brown fox jumps over the lazy dog";

    std::vector<char> output(message.begin(), message.end());
    buffer_.push(output);

    boost::asio::post(io_context_, [this]() { do_write(); });
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        log_err("Usage: " << argv[0] << " <host> <port> <file_name>");
        return 1;
    }

    boost::asio::io_context io_context;
    Client client(io_context, argv[1], argv[2]);

    auto work = boost::asio::require(io_context.get_executor(), boost::asio::execution::outstanding_work.tracked);
    boost::thread t(boost::bind(&boost::asio::io_context::run, &io_context));

    while (1) {
        log(std::endl << "1 - Connect");
        log("2 - Disconnect");
        log("3 - Send file");
        log("4 - Send string");
        log("5 - Exit");

        int menu;
        std::cin >> menu;

        switch (menu) {
        case 1:
            client.connect();
            break;
        case 2:
            client.disconnect();
            break;
        case 3:
            client.send_file(argv[3]);
            break;
        case 4:
            client.send_message();
            break;
        case 5:
            client.disconnect();
            io_context.stop();
            t.join();
            exit(EXIT_SUCCESS);
        }
    }

    return EXIT_SUCCESS;
}
