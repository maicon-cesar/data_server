//
// Copyright (c) 2022 Maicon Cesar Canales de Oliveira (maicon.cesar@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>

#include <configuration.hpp>
#include <log.hpp>
#include <server.hpp>

using boost::asio::ip::tcp;

bool properties_is_ok(properties::Properties &properties) {
    if ((!properties.file_size) || (!properties.port) || (!properties.timeout))
        return false;

    if (!std::filesystem::exists(properties.save_path))
        return false;

    return true;
}

int main(int argc, char *argv[]) {

    configuration::Configuration config;
    try {
        config.load(properties::PROPERTIES_FILE_PATH);
    } catch (std::exception &e) {
        std::cerr << "Error to get configuration: " << e.what() << std::endl;
        return -1;
    }

    properties::Properties properties;
    properties.port = std::atoi((config.get_value(properties::PROPERTY_PORT)).c_str());
    properties.file_size = std::atol((config.get_value(properties::PROPERTY_FILE_SIZE).c_str()));
    properties.file_name = config.get_value(properties::PROPERTY_FILE_NAME);
    properties.save_path = config.get_value(properties::PROPERTY_SAVE_PATH);
    properties.timeout = std::atol((config.get_value(properties::PROPERTY_TIMEOUT)).c_str());

    if (!properties_is_ok(properties)) {
        log_err("Invalid content in properties file.");
        return -1;
    }

    try {
        boost::asio::io_context io_context;
        Server s(io_context, properties);

        io_context.run();

    } catch (std::exception &e) {
        log_err("Exception error: " << e.what());
    }

    return EXIT_SUCCESS;
}
