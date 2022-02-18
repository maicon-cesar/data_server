#pragma once
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <queue>
#include <sstream>
#include <string>

#include <properties.hpp>

namespace fs = std::filesystem;

using boost::asio::ip::tcp;

class Data {
  public:
    Data(properties::Properties &properties, uint16_t cnx_id) : properties_(properties), cnx_id_(cnx_id) {
        open_directory();
        open_new_file();
    }

    ~Data() {
        if (file_.is_open()) {
            log("[id:" << std::to_string(cnx_id_) << "] Data successfully received and saved in: " << dir_name_);
            file_.close();
        }
    }

    void save(std::queue<std::vector<char>> &input_data) {
        log("[id:" << std::to_string(cnx_id_) << "] Wait, saving data...");

        while (!input_data.empty()) {
            size_t remaining = 0;
            size_t to_fill = 0;

            if ((current_file_size_ + input_data.front().size()) > properties_.file_size) {
                if (current_file_size_ < properties_.file_size) {
                    to_fill = properties_.file_size - current_file_size_;
                    remaining = (current_file_size_ + input_data.front().size()) - properties_.file_size;
                    file_.write(input_data.front().data(), to_fill);
                } else {
                    open_new_file();
                    file_.write(input_data.front().data(), input_data.front().size());
                    current_file_size_ += input_data.front().size();
                    input_data.pop();
                    continue;
                }
                open_new_file();

                if (remaining) {
                    if (remaining > properties_.file_size) {
                        file_.write(input_data.front().data() + to_fill, properties_.file_size);
                        open_new_file();
                        file_.write(input_data.front().data() + to_fill + properties_.file_size,
                                    remaining - properties_.file_size);
                    } else {
                        file_.write(input_data.front().data() + to_fill, remaining);
                        current_file_size_ = remaining;
                    }
                    remaining = 0;
                }
                input_data.pop();
                continue;
            }

            file_.write(input_data.front().data(), input_data.front().size());
            current_file_size_ += input_data.front().size();
            input_data.pop();
        }
    }

    size_t get_current_file_size() const noexcept { return current_file_size_; }

  private:
    std::string get_current_time() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d%H%M%S");

        return ss.str();
    }

    void open_directory() {
        static size_t dir_index = 1;

        std::string current_time = get_current_time();
        std::string full_name = properties::PREFIX_DIR_NAME + std::to_string(cnx_id_) + "_" + current_time;
        dir_name_ = properties_.save_path / fs::path(full_name);

        if (fs::exists(dir_name_)) {
            std::string rename = std::string(dir_name_) + "_" + std::to_string(dir_index++);
            dir_name_ = fs::path(rename);
        }

        if (!fs::create_directories(dir_name_)) {
            std::string msg_error = "Error to create directory " + std::string(dir_name_);
            throw std::runtime_error(msg_error);
        }
    }

    void open_new_file() {
        static size_t file_index = 1;

        if (file_.is_open())
            file_.close();

        current_file_size_ = 0;

        std::string current_time = get_current_time();
        std::string full_name = properties_.file_name + "_" + current_time;
        file_name_ = dir_name_ / fs::path(full_name);

        if (fs::exists(file_name_)) {
            std::string rename = std::string(file_name_) + "_" + std::to_string(file_index++);
            file_name_ = fs::path(rename);
        } else {
            file_index = 1;
        }

        file_.open(file_name_, std::ios_base::app);
        if (!file_.is_open()) {
            std::string msg_error = "Error to open output file: " + std::string(file_name_);
            throw std::runtime_error(msg_error);
        }
    }

    properties::Properties properties_;
    fs::path dir_name_;
    fs::path file_name_;
    size_t current_file_size_;
    std::ofstream file_;
    uint16_t cnx_id_;
};
