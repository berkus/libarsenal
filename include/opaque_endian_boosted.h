#include <boost/archive/binary_iarchive.hpp>//?
#include <boost/archive/binary_oarchive.hpp>//?
#include "opaque_endian.h"

// Disable versioning for all these classes.
// from http://www.boost.org/doc/libs/1_54_0/libs/serialization/doc/traits.html 
// section Template Serialization Traits
namespace boost {
namespace serialization {

template <typename T, T (*reorder)(const T&)>
struct implementation_level<__endian_conversion<T, reorder>>
{
    typedef mpl::integral_c_tag tag;
    typedef mpl::int_<object_serializable> type;
    BOOST_STATIC_CONSTANT(
        int,
        value = implementation_level::type::value
    );
};

} // serialization namespace
} // boost namespace

    // boost.serialization support
    friend class boost::serialization::access;
    template<class Archive>
    void load(Archive &ar, const unsigned int) {
        ar >> representation;
    }
    template<class Archive>
    void save(Archive &ar, const unsigned int) const {
        ar << representation;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
