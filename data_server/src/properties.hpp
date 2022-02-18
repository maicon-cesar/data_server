#pragma once
#include <string>

namespace properties {

typedef struct {
    short unsigned port;
    long file_size;
    std::string file_name;
    std::string save_path;
    long timeout;
} Properties;

static const std::string PROPERTIES_FILE_PATH = "configuration.txt";
static const std::string PREFIX_DIR_NAME = "cnx";

constexpr auto PROPERTY_PORT = "port";
constexpr auto PROPERTY_FILE_SIZE = "file_size";
constexpr auto PROPERTY_FILE_NAME = "file_name";
constexpr auto PROPERTY_SAVE_PATH = "save_path";
constexpr auto PROPERTY_TIMEOUT = "timeout";

} // namespace properties
