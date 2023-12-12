/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <string>

/*
* This is related to the configuration information of the network library. 
* If it is not necessary, use the default parameters.
*/

namespace bnet { 
struct svr_cfg {
    std::string host = "0.0.0.0";
    std::string port = "10240";
    std::size_t thread_num = 0;
    std::size_t limit_buffer_size = 0;
    bool keepalive = false;
};

struct cli_cfg {
    std::string host = "127.0.0.1";
    std::string port = "10240";
    std::size_t thread_num = 0;
    // number of connection handles
    std::size_t pool_size = 8;
    // connection timeout(ms)
    std::size_t conn_timeout = 3000;
    // reconnection interval(s)
    std::size_t reconn_interval = 3;
    bool is_reconn = false;
    // for http request timeout(s)
    std::size_t request_timout = 3;
    std::size_t limit_buffer_size = 0;
    bool is_async = true;
    bool keepalive = false;
};
}