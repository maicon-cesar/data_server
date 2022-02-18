#include <configuration.hpp>

namespace configuration {

/*****************************************************************************************************************
 DATA
*****************************************************************************************************************/
Data::Data(const std::string &key, const std::string &value) noexcept {
    set_key(key);
    set_value(value);
}

Data::Data(const std::string &parse_string) { this->parse_string(parse_string); }

void Data::set_key(const std::string &key) noexcept { key_ = boost::algorithm::trim_copy(key); }

const std::string &Data::set_key() const noexcept { return key_; }

void Data::set_value(const std::string &value) noexcept { value_ = boost::algorithm::trim_copy(value); }

const std::string &Data::get_value() const noexcept { return value_; }

std::string Data::to_string() const noexcept {
    std::stringstream out;
    out << set_key() << "=" << get_value();
    return out.str();
}

void Data::parse_string(const std::string &str) {
    if (str.find('=') == std::string::npos) {
        throw std::invalid_argument("Invalid property: " + str);
    }
    set_key(str.substr(0, str.find('=')));
    set_value(str.substr(str.find('=') + 1));
}

/*****************************************************************************************************************
 CONFIGURATION
*****************************************************************************************************************/
void Configuration::load(const std::string &path) {
    std::string line;
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("File configuration not found: " + path);
    }

    while (std::getline(file, line)) {
        line = boost::algorithm::trim_copy(line);
        if (line.empty() || begins_with(line, "#")) {
            continue;
        }

        write(Data(line));
    }
}

std::string Configuration::get_value(const std::string &key) {
    const auto index = cache_.find(key);
    if (index == cache_.end()) {
        throw std::invalid_argument("Property not found: " + key);
    }

    return Data(key, index->second).get_value();
}

Data Configuration::read(const std::string &key) {
    const auto index = cache_.find(key);
    if (index == cache_.end()) {
        throw std::invalid_argument("Property not found: " + key);
    }

    return Data(key, index->second);
}

void Configuration::write(const Data &data) { cache_[data.set_key()] = data.get_value(); }

bool Configuration::begins_with(const std::string &source, const std::string &substring) noexcept {
    if (source.size() < substring.size()) {
        return false;
    }

    return memcmp(source.data(), substring.data(), substring.size() * sizeof(substring[0])) == 0;
}

} // namespace configuration
