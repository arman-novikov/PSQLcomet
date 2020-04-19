#include "web_socket_crypto.h"

/// \union sha1_container_t
/// \brief ontains 20 bytes of sha1 algorithm result one extra char as a terminating one ('\0')
union sha1_container_t {
    static constexpr size_t PADDING = 4U;
    unsigned int nums[5];                             ///< to input int values into union memory
    char chars[sizeof(unsigned int) * 5 + PADDING];   ///< to get simple char values from union memory
};

/*!
* \brief returns result of sha1 algorithm processing of the parameter
* \param [in] string containing bytes to work on with sha1
* \return string containing 20 bytes of sha1 processing result
*/
static std::string get_sha1(const std::string& sha1_hash)
{
    union sha1_container_t sha1_container{};
    boost::uuids::detail::sha1 sha1;

    sha1.process_bytes(sha1_hash.data(), sha1_hash.size());
    sha1.get_digest(sha1_container.nums);

#ifdef BOOST_LITTLE_ENDIAN
    for (int i = 0; i < 5; ++i) /// 5 unsigned int values according to the sha1 algorithm
        sha1_container.nums[i] = bswap_32(sha1_container.nums[i]);
#endif
    return std::string(sha1_container.chars);
}

/*!
* \brief returns result of base algorithm processing of the parameter
* \param [in] string containing bytes to work on with base64
* \return string containing 20 base64 processing result
*/
static std::string encode64(const std::string &web_accept_key) {
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
    auto tmp = std::string(It(std::begin(web_accept_key)), It(std::end(web_accept_key)));
    return tmp.append((3 - web_accept_key.size() % 3) % 3, '=');
}

std::string get_webSocket_accept(const std::string &sec_webSocket_key)
{
    return encode64(get_sha1(sec_webSocket_key));///just uses static funcs declared above
}
