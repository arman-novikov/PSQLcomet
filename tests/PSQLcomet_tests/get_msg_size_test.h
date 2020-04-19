#pragma once
#include "tst_psqlcomet_common.h"
#include "../../web_socket_crypto.h"
#include "../../ws_server.h"
#include <random>

using namespace testing;
using namespace ws_server_ns;

struct get_msg_size_data_t {
    uint8_t src[16U]; ///< 1 + 1 + 2 + 6 {ctrl + LEN + BIG_LEN + EXTRA_BIG} + unused PADDING(6)
    uint64_t expected;

    friend std::ostream& operator<<(std::ostream& os,
                                    const get_msg_size_data_t& data)
    {
        static_cast<void>(data);
        return os;// << "expected: " << data.expected << std::endl;
    }
} static get_msg_size_data_sets[] = {
    {{static_cast<uint8_t>(rand()), 100, 0},    100},
    {{static_cast<uint8_t>(rand()), 78, 0},     78},
    {{static_cast<uint8_t>(rand()), 1, 0},      1},
    {{static_cast<uint8_t>(rand()), 32, 0},     32},
    {{static_cast<uint8_t>(rand()), 17, 0},     17},

    {{static_cast<uint8_t>(rand()), BIG_FRAME, 136U, 0},   136},
    {{static_cast<uint8_t>(rand()), BIG_FRAME, 151U, 0},   151},
    {{static_cast<uint8_t>(rand()), BIG_FRAME, 200U, 0},   200},
    {{static_cast<uint8_t>(rand()), BIG_FRAME, 216U, 0},   216},
    {{static_cast<uint8_t>(rand()), BIG_FRAME, 255U, 0},   255},
};

struct Get_msg_size_tets:
        PSQLcomet_test,
        WithParamInterface<get_msg_size_data_t> {};

TEST_P(Get_msg_size_tets, get_msg_size_test)
{
    auto params = GetParam();
    EXPECT_EQ(params.expected, ws_get_msg_size(params.src));
}

INSTANTIATE_TEST_CASE_P(Default, Get_msg_size_tets,
                        ValuesIn(get_msg_size_data_sets));

