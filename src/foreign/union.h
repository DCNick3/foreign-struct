
#pragma once

#include "util.h"

namespace foreign {
    template<typename ...Members>
    class target_union;

    template<typename T>
    struct is_target_union : std::false_type {};
    template<typename ...Members>
    struct is_target_union<target_union<Members...>> : std::true_type {};

    template<typename T>
    concept TargetUnion = is_target_union<T>::value;

    // union "materialization"
    template<TargetUnion T>
    struct materializer<T> {
        static auto materialize(span_for_c<T> data) {
            // no bitcast in clang yet :'(
            T result;
            static_assert(std::is_trivially_copyable_v<T>);
            static_assert(std::is_trivially_constructible_v<T>);
            static_assert(sizeof(T) == target_sizeof_v<T>);
            memcpy(&result, data.data(), sizeof(T));
            return result;
        }

        static void unmaterialize(span_for<T> data, T v) {
            static_assert(std::is_trivially_copyable_v<T>);
            static_assert(std::is_trivially_constructible_v<T>);
            static_assert(sizeof(T) == target_sizeof_v<T>);
            memcpy(data.data(), &v, sizeof(T));
        }
    };


    template<TargetUnion T>
    struct target_sizeof<T> {
        static constexpr std::size_t value = T::size;
    };

    template<typename ...Members>
    class target_union {
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

    public:
        static constexpr std::size_t size = comp_size();

        template<std::size_t I>
        auto get() const {
            using MT = std::tuple_element_t<I, member_tuple>;
            return target_materialize<MT>(get_span_for<MT>());
        }

        template<std::size_t I, typename T>
        void set(T v) {
            using MT = std::tuple_element_t<I, member_tuple>;
            return target_unmaterialize<MT>(get_span_for<MT>(), std::move(v));
        }

        template<std::size_t I>
        auto use() {
            using MT = std::tuple_element_t<I, member_tuple>;
            return target_rw_mat_holder<MT>(get_span_for<MT>());
        }

    private:
        std::array<std::uint8_t, size> raw_data;
    };
}
