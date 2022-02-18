#pragma once
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include <memory>
#include <thread>

#include <data.hpp>

constexpr auto OUTPUT_DATA_SIZE_BUFFER = 4096;
constexpr auto TIMEOUT_BWT_FRAMES = 3;

class Connection : public std::enable_shared_from_this<Connection> {

  public:
    Connection(boost::asio::io_context &io_context, tcp::socket &socket, properties::Properties &properties,
               uint16_t cnx_id)
        : io_context_(io_context), properties_(properties), socket_(std::move(socket)), timer_(io_context),
          cnx_id_(cnx_id) {

        timer_.expires_after(boost::asio::chrono::seconds(properties_.timeout));
        timer_.async_wait(boost::bind(&Connection::handle_timeout, this, boost::asio::placeholders::error));

        handle_incoming_data();
    }

    uint16_t get_id() const noexcept { return cnx_id_; }

    void run() { io_context_.run(); }

  private:
    void handle_data() {
        data_ = std::make_unique<Data>(properties_, cnx_id_);
        data_->save(input_data_);
    }

    void handle_timeout(const boost::system::error_code &ec) {
        if (ec != boost::asio::error::operation_aborted) {
            socket_.close();
        }
    }

    void handle_incoming_data() {
        static bool flag_once = true;

        socket_.async_read_some(boost::asio::buffer(buffer_), [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                timer_.expires_after(boost::asio::chrono::seconds(TIMEOUT_BWT_FRAMES));
                timer_.async_wait(boost::bind(&Connection::handle_timeout, this, boost::asio::placeholders::error));

                std::vector<char> incoming_data(length);
                std::copy(buffer_.data(), buffer_.data() + length, incoming_data.begin());

                input_data_.push(std::move(incoming_data));

                if (flag_once) {
                    flag_once = false;
                    data_thread_ = std::thread([&]() { handle_data(); });
                }

                handle_incoming_data();
            } else {
                log("[id:" << std::to_string(cnx_id_) << "] Client disconnected.");

                if (data_thread_.joinable())
                    data_thread_.join();

                if (flag_once == false) {
                    flag_once = true;
                }

                io_context_.stop();
            }
        });
    }

    boost::asio::io_context &io_context_;
    tcp::socket socket_;
    properties::Properties properties_;
    std::unique_ptr<Data> data_;
    std::array<char, OUTPUT_DATA_SIZE_BUFFER> buffer_;
    std::queue<std::vector<char>> input_data_;
    boost::asio::steady_timer timer_;
    uint16_t cnx_id_;
    std::thread data_thread_;
};
