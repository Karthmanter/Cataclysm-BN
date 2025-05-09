#pragma once

#include <cstdint>
#include <functional>

// Support for hashing standard types.
// This is taken almost directly from the boost library code.
namespace cata
{

// Code from boost
// Reciprocal of the golden ratio helps spread entropy
//     and handles duplicates.
// See Mike Seymour in magic-numbers-in-boosthash-combine:
//     http://stackoverflow.com/questions/4948780

template <class T, typename Hash = std::hash<T>>
inline void hash_combine( std::size_t &seed, const T &v, const Hash &hash = std::hash<T>() )
{
    seed ^= hash( v ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
}

namespace tuple_hash_detail
{

// Recursive template code derived from Matthieu M.
template < class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1 >
struct Impl
{
    static void apply( size_t &seed, const Tuple &tuple ) {
        Impl < Tuple, Index - 1 >::apply( seed, tuple );
        hash_combine( seed, std::get<Index>( tuple ) );
    }
};

template <class Tuple>
struct Impl<Tuple, 0> {
    static void apply( size_t &seed, const Tuple &tuple ) {
        hash_combine( seed, std::get<0>( tuple ) );
    }
};

} // namespace tuple_hash_detail

struct tuple_hash {
    template <typename ... TT>
    std::size_t operator()( const std::tuple<TT...> &tt ) const {
        size_t seed = 0;
        tuple_hash_detail::Impl<std::tuple<TT...> >::apply( seed, tt );
        return seed;
    }

    template <class A, class B>
    std::size_t operator()( const std::pair<A, B> &v ) const {
        std::size_t seed = 0;
        hash_combine( seed, v.first );
        hash_combine( seed, v.second );
        return seed;
    }
};

// auto_hash will use std::hash for most types but tuple_hash for pair or
// tuple.
template<typename T>
struct auto_hash : std::hash<T> {};

template<typename T, typename U>
struct auto_hash<std::pair<T, U>> : tuple_hash {};

template<typename... T>
struct auto_hash<std::tuple<T...>> : tuple_hash {};

struct range_hash {
    template<typename Range>
    std::size_t operator()( const Range &range ) const noexcept {
        using value_type = typename Range::value_type;
        using hash_type = auto_hash<value_type>;
        hash_type hash;

        std::size_t seed = range.size();
        for( const auto &value : range ) {
            hash_combine( seed, value, hash );
        }
        return seed;
    }
};

namespace hash64_detail
{

template<typename T>
T maybe_mix_bits( std::uint64_t val )
requires( sizeof( T ) < 8 )
{
    std::uint32_t hi = val >> 32;
    std::uint32_t lo = val;
    T ret = hi;
    hash_combine( ret, lo );
    return ret;
}

template<typename T>
T maybe_mix_bits( std::uint64_t val )
requires( sizeof( T ) >= 8 )
{
    return val;
}

} // namespace hash64_detail

// hash64 hashes a 64-bit integer, either using the identity function (on 64-bit systems)
// or using hash_combine on the two 32-bit halves (on 32-bit systems)
inline std::size_t hash64( std::uint64_t val )
{
    return hash64_detail::maybe_mix_bits<std::size_t>( val );
}

} // namespace cata


