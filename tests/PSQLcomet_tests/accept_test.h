#pragma once
#include "tst_psqlcomet_common.h"
#include "../../web_socket_crypto.h"
#include "../../ws_server.h"

using namespace testing;

struct web_socket_accept_data_t {
    std::string src;
    std::string expected;

    friend std::ostream& operator<<(std::ostream& os,
                                    const web_socket_accept_data_t& data)
    {
        return os << "src: "  << data.src << std::endl
                  << "expected: " << data.expected << std::endl;
    }
} static accept_data_sets[] = {
    {std::string("dGhlIHNhbXBsZSBub25jZQ=="), std::string("s3pPLMBiTxaQ9kYGzzhZRbK+xOo=")},
    {std::string("Iv8io/9s+lYFgZWcXczP8Q=="), std::string("hsBlbuDTkk24srzEOTBUlZAlC2g=")},
};

struct Crypto_test:
       PSQLcomet_test,
       WithParamInterface<web_socket_accept_data_t> {};

TEST_P(Crypto_test, webSocketCrypto)
{
    auto params = GetParam();
    EXPECT_EQ(params.expected, get_webSocket_accept(params.src + ws_server_ns::GUID));
}

INSTANTIATE_TEST_CASE_P(Default, Crypto_test,
                        ValuesIn(accept_data_sets));
