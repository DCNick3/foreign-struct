//
// Created by dcnick3 on 8/10/21.
//

#pragma once

#include <climits>
#include <cstdint>
#include <cstring>
#include <cassert>

#include <type_traits>
#include <concepts>
#include <span>
#include <bit>

namespace foreign {
    // are we on a conventional system?
    static_assert(CHAR_BIT == 8);
    static_assert(std::is_same_v<std::uint8_t, unsigned char>);
    static_assert(sizeof(std::uint8_t) == 1);
    static_assert(sizeof(std::uint16_t) == 2);
    static_assert(sizeof(std::uint32_t) == 4);
    static_assert(sizeof(std::uint64_t) == 8);
    static_assert(sizeof(std::int8_t) == 1);
    static_assert(sizeof(std::int16_t) == 2);
    static_assert(sizeof(std::int32_t) == 4);
    static_assert(sizeof(std::int64_t) == 8);

    namespace util {
        template<auto Start, auto End, auto Inc, class F>
        constexpr void constexpr_for(F &&f) {
            if constexpr (Start < End) {
                f(std::integral_constant<decltype(Start), Start>());
                constexpr_for<Start + Inc, End, Inc>(f);
            }
        }
    }

    // traits

    // should have a single std::size_t constexpr member - value
    // defines the size of an object in the target address space
    template<typename TargetType>
    struct target_sizeof;

    // provides static two functions:
    // R materialize(span_for_c<TargetType> data)
    // *** materializes TargetType from the data
    //
    // template<typename Mat>
    // void unmaterialize(span_for<TargetType> data, Mat&& mat)
    // *** unmaterializes TargetType from materialized form Mat to data
    template<typename TargetType>
    struct materializer;

    // trait helpers
    template<typename T>
    inline constexpr std::size_t target_sizeof_v = target_sizeof<std::remove_reference_t<T>>::value;

    template<typename TargetType>
    using span_for = std::span<std::uint8_t, target_sizeof_v<TargetType>>;

    template<typename TargetType>
    using span_for_c = std::span<const std::uint8_t, target_sizeof_v<TargetType>>;

    template<typename TargetType>
    using array_for = std::array<std::uint8_t, target_sizeof_v<TargetType>>;

    template<typename TargetType>
    auto target_materialize(span_for_c<TargetType> data) {
        return materializer<TargetType>::materialize(data);
    }

    template<typename TargetType>
    void target_unmaterialize(span_for<TargetType> data, TargetType&& materialized) {
        return materializer<std::remove_reference_t<TargetType>>::unmaterialize(data, std::forward<TargetType>(materialized));
    }

    template<typename TargetType>
    auto target_unmaterialize(TargetType&& materialized) {
        std::array<std::uint8_t, target_sizeof_v<TargetType>> buffer;
        target_unmaterialize(buffer, std::forward<TargetType>(materialized));
        return buffer;
    }

    // concepts!
    // some of them are missing from std, so here they are!
    template<typename T>
    concept Enum = std::is_enum_v<T>;

    template<typename T>
    concept NonConstReference = std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>;

    template<typename T>
    concept ConstReference = std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>;

    static_assert(NonConstReference<int &>);
    static_assert(!ConstReference<int &>);
    static_assert(ConstReference<int const &>);
    static_assert(!NonConstReference<int const &>);

    // RAII holder type
    // materializes the target value on construction, unmaterializes on destruction
    // supports move, but not copy
    // you should not access it's data after the object was moved from
    template<typename T>
    class target_rw_mat_holder {
        bool valid = true;
        span_for<T> data_span;
        T storage;

    public:
        target_rw_mat_holder(span_for<T> data_span)
                :
                data_span(data_span),
                storage(target_materialize<T>(data_span)) {}

        target_rw_mat_holder(target_rw_mat_holder &&o)
                :
                data_span(std::move(o.data_span)),
                storage(std::move(o.storage)),
                valid(o.valid) {
            o.valid = false;
        }

        target_rw_mat_holder &operator=(target_rw_mat_holder &&o) {
            if (this != &o) {
                storage = std::move(o.storage);
                data_span = std::move(data_span);
                valid = o.valid;
                o.valid = false;
            }
            return *this;
        }

        ~target_rw_mat_holder() {
            if (valid) {
                target_unmaterialize<T>(data_span, std::move(storage));
                valid = false;
            }
        }

        target_rw_mat_holder(target_rw_mat_holder const &) = delete;

        target_rw_mat_holder &operator=(target_rw_mat_holder const &) = delete;


        T &operator*() {
            assert(valid);
            return storage;
        }

        T const &operator*() const {
            assert(valid);
            return storage;
        }

        T *operator->() {
            assert(valid);
            return &storage;
        }

        T const &operator->() const {
            assert(valid);
            return &storage;
        }
    };
}
