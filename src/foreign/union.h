
#pragma once

#include "util.h"

namespace foreign {
    template<std::size_t Size, typename ...Members>
    class target_union;

    // union "materialization"
    template<std::size_t Size, typename ...Members>
    struct materializer<target_union<Size, Members...>> {
        using T = target_union<Size, Members...>;

        static auto materialize(span_for_c<T> data) {
            // no bitcast in clang yet :'(
            T result;
            static_assert(std::is_trivially_copyable_v<T>);
            static_assert(std::is_trivially_constructible_v<T>);
            static_assert(sizeof(T) == target_sizeof_v<T>);
            // target_union has only an std::array of std::uint8_t's as its member, so it's safe to memcpy it
            memcpy(&result, data.data(), sizeof(T));
            return result;
        }

        static void unmaterialize(span_for<T> data, T v) {
            static_assert(std::is_trivially_copyable_v<T>);
            static_assert(std::is_trivially_constructible_v<T>);
            static_assert(sizeof(T) == target_sizeof_v<T>);
            // target_union has only an std::array of std::uint8_t's as its member, so it's safe to memcpy it
            memcpy(data.data(), &v, sizeof(T));
        }
    };


    template<std::size_t Size, typename ...Members>
    struct target_sizeof<target_union<Size, Members...>> {
        static constexpr std::size_t value = target_union<Size, Members...>::size;
    };

    template<std::size_t Size, typename ...Members>
    class target_union {
    private:
        using member_tuple = std::tuple<Members...>;

        static constexpr std::size_t comp_size() {
            std::size_t size = 0;
            util::constexpr_for<0, sizeof...(Members), 1>([&](auto i) constexpr {
                using C = std::tuple_element_t<i, member_tuple>;
                size = std::max(size, target_sizeof_v<C>);
            });
            return size;
        }

        template<typename T>
        span_for<T> get_span_for() {
            return std::span(raw_data).template first<target_sizeof_v<T>>();
        }

        template<typename T>
        span_for_c<T> get_span_for() const {
            return std::span(raw_data).template first<target_sizeof_v<T>>();
        }

        static_assert(comp_size() <= Size);

    public:
        static constexpr std::size_t size = Size;

        template<std::size_t I>
        auto get() const {
            using MT = std::tuple_element_t<I, member_tuple>;
            return target_materialize<MT>(get_span_for<MT>());
        }

        template<std::size_t I, typename T>
        void set(T&& v) {
            using MT = std::tuple_element_t<I, member_tuple>;
            return target_unmaterialize<MT>(get_span_for<MT>(), std::forward<T>(v));
        }

        template<std::size_t I>
        auto use() {
            using MT = std::tuple_element_t<I, member_tuple>;
            return target_rw_mat_holder<MT>(get_span_for<MT>());
        }

        template<std::size_t I, typename T>
        static constexpr auto from(T&& t) {
            target_union res;
            res.raw_data = {};
            res.set<I>(std::forward<T>(t));
            return res;
        }

    private:
        std::array<std::uint8_t, size> raw_data;
    };
}
