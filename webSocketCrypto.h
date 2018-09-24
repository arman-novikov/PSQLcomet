#ifndef WEBSOCKETCRYPTO_H_INCLUDED
#define WEBSOCKETCRYPTO_H_INCLUDED

#include <string>
#include <byteswap.h>

#include <boost/uuid/sha1.hpp>
#include <boost/detail/endian.hpp>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

/*!
* \brief function generates (Sec-WebSocket-Accept)'s value
*   \params [in] sec_webSocket_key value of (Sec-WebSocket-Key) field
*   \return value of (Sec-WebSocket-Accept) field
*
*   Acording to WebSocket handshake  protocol
*   client sends Sec-WebSocket-Key and
*   waits for the valid Sec-WebSocket-Accept from server.
*   Processes params using sha1, than processes the result using base64
*/

std::string get_webSocket_accept(const std::string &sec_webSocket_key);

#endif // WEBSOCKETCRYPTO_H_INCLUDED
