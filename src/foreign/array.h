
#pragma once

#include "util.h"

namespace foreign {
    // enum types materialization
    template<typename ElementType, std::size_t Length>
    struct materializer<std::array<ElementType, Length>> {
    private:
        using T = std::array<ElementType, Length>;
        static constexpr auto element_size = target_sizeof_v<ElementType>;

        template<std::size_t I>
        static auto materialize_element(span_for_c<T> data) {
            return target_materialize<ElementType>(data.template subspan<I * element_size, element_size>());
        }

        template<std::size_t ...I>
        static auto materialize(span_for_c<T> data, std::index_sequence<I...>) {
            return std::array<ElementType, Length>{materialize_element<I>(data)...};
        }

        template<std::size_t I>
        static void unmaterialize_element(span_for<T> data, const T& v) {
            target_unmaterialize(data.template subspan<I * element_size, element_size>(), v[I]);
        }


    public:
        static auto materialize(span_for_c<T> data) {
            return materialize(data, std::make_index_sequence<Length>{});
        }

        static void unmaterialize(span_for<T> data, const T& v) {
            util::constexpr_for<0, Length, 1>([&](auto i) {
                unmaterialize_element<i>(data, v);
            });
        }
    };

    template<typename T, std::size_t S>
    struct target_sizeof<std::array<T, S>> {
        constexpr static auto value = target_sizeof_v<T> * S;
    };
}