
#pragma once

#include <cstdint>
#include <array>
#include <utility>

// Idea borrowed from here:
// https://stackoverflow.com/questions/25174059/constexpr-conversion-of-hex-chars-to-stdstring/25187296
// or, more specifically:
// http://coliru.stacked-crooked.com/a/31536bd0880a1793


namespace detail {
    template<std::size_t N>
    using bytes_literal = std::array<std::uint8_t, N>;

    template<std::size_t N1, std::size_t N2, std::size_t... Ind1, std::size_t... Ind2>
    constexpr auto add(bytes_literal<N1> lhs, bytes_literal<N2> rhs,
                       std::index_sequence<Ind1...>, std::index_sequence<Ind2...>)
    -> bytes_literal<N1 + N2> {
        return {lhs[Ind1]..., rhs[Ind2]...};
    }

    template<std::size_t N1, std::size_t N2>
    constexpr auto operator+(bytes_literal<N1> lhs, bytes_literal<N2> rhs)
    -> bytes_literal<N1 + N2> {
        using Indices1 = std::make_index_sequence<N1>;
        using Indices2 = std::make_index_sequence<N2>;
        return add(lhs, rhs, Indices1(), Indices2());
    }


    template<char C>
    constexpr std::uint8_t from_hex() {
        static_assert(C >= '0' && C <= '9' || C >= 'a' && C <= 'f');
        if (C >= '0' && C <= '9')
            return C - '0';
        return (C - 'a') + 10;
    }

    template<char C1, char C2>
    constexpr auto process()
    -> bytes_literal<1> {
        return {16 * from_hex<C1>() + from_hex<C2>()};
    }

    template<char C1, char C2, char... Rest,
            typename = std::enable_if_t<(sizeof...(Rest) > 0), void >>
    constexpr auto process()
    -> bytes_literal<sizeof...(Rest) / 2 + 1> {
        return process<C1, C2>() + process<Rest...>();
    }

    template<std::size_t N>
    struct string_literal {
        static constexpr std::size_t length = N;
        std::array<char, N> arr_;

        constexpr string_literal(const char(&in)[N]) : arr_{} {
            std::copy(in, in + N, arr_.begin());
        }
    };

    template<detail::string_literal Str, std::size_t... I>
    auto process(std::index_sequence<I...>)
    -> detail::bytes_literal<Str.length / 2>
    {
        return process<Str.arr_[I]...>();
    }
}

template<detail::string_literal Str>
auto operator "" _hex()
-> detail::bytes_literal<(Str.length - 1) / 2>
{
    return detail::process<Str>(std::make_index_sequence<Str.length - 1>());
}
