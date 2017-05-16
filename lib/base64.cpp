#include "arsenal/base64.h"
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <sstream>

namespace arsenal::encode
{

// From http://stackoverflow.com/questions/7053538/how-do-i-encode-a-string-to-base64-using-only-boost
std::string to_base64(const std::string& in)
{
    using namespace boost::archive::iterators;

    std::stringstream os;
    using base64_text =
        base64_from_binary<    // convert binary values to base64 characters
            transform_width<   // retrieve 6 bit integers from a sequence of 8 bit bytes
                const char *,
                6,
                8
            >
        >; // compose all the above operations in to a new iterator

    std::copy(
        base64_text(in.c_str()),
        base64_text(in.c_str() + in.size()),
        ostream_iterator<char>(os)
    );

    return os.str();
}

std::string from_base64(const std::string& in)
{
    return std::string(); // @todo
}

} // arsenal::encode namespace
