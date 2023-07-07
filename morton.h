#pragma once

#include <cstdint>
#include <utility>

namespace detail
{


    template<std::size_t num_bytes>
    struct unsigned_integer;

    template<>
    struct unsigned_integer<1>
    {
        using type = std::uint8_t;
    };

    template<>
    struct unsigned_integer<2>
    {
        using type = std::uint16_t;
    };

    template<>
    struct unsigned_integer<4>
    {
        using type = std::uint32_t;
    };

    template<>
    struct unsigned_integer<8>
    {
        using type = std::uint64_t;
    };

    template<std::size_t num_bytes>
    using unsigned_integer_t = typename unsigned_integer<num_bytes>::type;


} // end detail


template<class UnsignedInteger>
std::pair<
    detail::unsigned_integer_t<sizeof(UnsignedInteger) / 2>,
    detail::unsigned_integer_t<sizeof(UnsignedInteger) / 2>
>
decode_morton_2d(UnsignedInteger m)
{
    using half_width_type = detail::unsigned_integer_t<sizeof(UnsignedInteger) / 2>;

    half_width_type x = 0;
    half_width_type y = 0;

    // for each bit of the half width numbers
#pragma unroll
    for (int i = 0; i < sizeof(half_width_type) * 8; ++i)
    {
        // pick out the ith even bit of the full width integer and assign it to bit i
        x |= (m & UnsignedInteger(1) << 2 * i) >> i;

        // pick out the ith odd bit of the full width integer and assign it to bit i+1
        y |= (m & UnsignedInteger(1) << 2 * i + 1) >> (i + 1);
    }

    return std::make_pair(x, y);
}


template<class UnsignedInteger>
detail::unsigned_integer_t<2 * sizeof(UnsignedInteger)>
encode_morton_2d(UnsignedInteger x, UnsignedInteger y)
{
    using double_width_type = detail::unsigned_integer_t<2 * sizeof(UnsignedInteger)>;

    double_width_type result = 0;

    // for each bit of the double width number
    for (int i = 0; i < sizeof(double_width_type) * 8; ++i)
    {
        result |= (x & double_width_type(1) << i) << i | (y & double_width_type(1) << i) << (i + 1);
    }

    return result;
}


template<class UnsignedInteger>
detail::unsigned_integer_t<2 * sizeof(UnsignedInteger)>
encode_morton_2d(std::pair<UnsignedInteger, UnsignedInteger> xy)
{
    return encode_morton_2d(xy.first, xy.second);
}