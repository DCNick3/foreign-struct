
#pragma once

#include "util.h"

namespace foreign {
    namespace detail {
        template<std::integral T>
        void integral_to_bytes_le(std::span<std::uint8_t, sizeof(T)> data, T value) {
            static_assert(std::endian::native == std::endian::little);
            memcpy(data.data(), &value, sizeof(T));
        }

        template<std::integral T>
        auto bytes_to_integral_le(std::span<const std::uint8_t, sizeof(T)> data) {
            static_assert(std::endian::native == std::endian::little);
            T value;
            memcpy(&value, data.data(), sizeof(T));
            return value;
        }
    }

    // integral types materialization
    template<std::integral T>
    struct materializer<T> {
        static auto materialize(span_for_c<T> data) {
            return detail::bytes_to_integral_le<T>(data);
        }

        static void unmaterialize(span_for<T> data, T v) {
            detail::integral_to_bytes_le(data, v);
        }
    };

    template<std::integral T>
    struct target_sizeof<T> {
        static constexpr std::size_t value = sizeof(T);
    };
}
