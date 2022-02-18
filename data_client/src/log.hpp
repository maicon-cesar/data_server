#pragma once
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <string>

static boost::mutex global_stream_lock;

#define lock_stream() global_stream_lock.lock()
#define unlock_stream() global_stream_lock.unlock()

#define log(x)                                                                                                         \
    {                                                                                                                  \
        lock_stream();                                                                                                 \
        std::cout << x << std::endl;                                                                                   \
        unlock_stream();                                                                                               \
    }

#define log_err(x)                                                                                                     \
    {                                                                                                                  \
        lock_stream();                                                                                                 \
        std::cerr << x << std::endl;                                                                                   \
        unlock_stream();                                                                                               \
    }
