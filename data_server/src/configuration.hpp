#pragma once
#include <boost/algorithm/string/trim.hpp>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace configuration {

class Data {
  public:
    explicit Data(const std::string &key, const std::string &value) noexcept;
    explicit Data(const std::string &parse_string);

    void set_key(const std::string &key) noexcept;
    const std::string &set_key() const noexcept;
    void set_value(const std::string &value) noexcept;
    const std::string &get_value() const noexcept;

    std::string to_string() const noexcept;
    void parse_string(const std::string &str);

  private:
    std::string key_;
    std::string value_;
};

class Configuration {
  public:
    void load(const std::string &path);

    std::string get_value(const std::string &key);

    void write(const Data &data);
    Data read(const std::string &key);

  private:
    bool begins_with(const std::string &source, const std::string &substring) noexcept;

    std::map<std::string, std::string> cache_;
};

} // namespace configuration
