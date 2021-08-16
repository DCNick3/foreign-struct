
#pragma once

#include "util.h"

#include <tuple>

namespace foreign {
    template<typename Type, std::size_t Offset>
    struct target_struct_field;

    template<typename T>
    struct is_target_struct_field : std::false_type {};
    template<typename Type, std::size_t Offset>
    struct is_target_struct_field<target_struct_field<Type, Offset>> : std::true_type {};

    template<typename T>
    concept TargetStructField = is_target_struct_field<T>::value;


    template<std::size_t Size, TargetStructField ...Fields>
    class target_struct_def;

    template<typename T>
    struct is_target_struct_def : std::false_type {};
    template<std::size_t Size, typename ...Args>
    struct is_target_struct_def<target_struct_def<Size, Args...>> : std::true_type {};

    template<typename T>
    concept TargetStructDef = is_target_struct_def<T>::value;

    template<TargetStructDef Def>
    class target_struct_unnamed;

    template<typename T>
    struct is_target_struct_unnamed : std::false_type {};
    template<typename T>
    struct is_target_struct_unnamed<target_struct_unnamed<T>> : std::true_type {};

    template<typename T>
    concept TargetStructUnnamed = is_target_struct_unnamed<T>::value;


    namespace detail {
        // separated for the sake of MSVC incomplete concepts implementation
        template<typename T, std::size_t I>
        concept TargetStructNamedImplHlp = requires(T &t, T const &tc) {
            { t.template get_field<I>() } -> NonConstReference;
            { tc.template get_field<I>() } -> ConstReference;
        };

        template<typename T, std::size_t I>
        struct TargetStructNamedImpl {
            constexpr static bool value = TargetStructNamedImplHlp<T, I> && []() constexpr {
                if constexpr (I > 0)
                    return TargetStructNamedImpl<T, I - 1>::value;
                return true;
            }();
        };
    }
    template<typename T>
    concept TargetStructNamed = requires() {
        typename T::field_accessor;
        typename T::def;
        requires TargetStructDef<typename T::def>;
    } && detail::TargetStructNamedImpl<T, T::def::field_count - 1>::value;

    template<typename T>
    concept TargetStruct = TargetStructNamed<T> || TargetStructUnnamed<T>;

    // structs materialization
    template<TargetStruct T>
    struct materializer<T>
    {
    private:
        using def = typename T::def;

        static_assert(TargetStructDef<def>);

        using field_tuple = typename def::field_tuple;
        using mat_tuple = typename def::mat_tuple;

        using index_seq = std::make_index_sequence<std::tuple_size<field_tuple>::value>;

        constexpr static std::size_t size = def::size;

        using data_span = std::span<std::uint8_t, size>;
        using data_span_c = std::span<const std::uint8_t, size>;

        template<typename Field>
        static inline auto materialize_field(data_span_c data, Field field) {
            auto sp = data.template subspan<Field::offset, Field::size>();
            return target_materialize<typename Field::type>(sp);
        }

        template<typename Field, typename M>
        static inline auto unmaterialize_field(data_span data, Field field, M mat) {
            auto sp = data.template subspan<Field::offset, Field::size>();
            return target_unmaterialize<typename Field::type>(sp, std::move(mat));
        }

        template<std::size_t ...I>
        static inline mat_tuple make_materialized_tuple_impl(
                std::index_sequence<I...> is, data_span_c data) {
            return std::tuple(materialize_field(data, std::get<I>(field_tuple{}))...);
        }

        static inline mat_tuple make_materialized_tuple(data_span_c data) {
            return make_materialized_tuple_impl(index_seq{}, data);
        }

        template<std::size_t Head, std::size_t ...Tail>
        static inline void unmaterialize_impl(data_span data, T &o) {
            unmaterialize_field(data, std::get<Head>(field_tuple{}), std::move(o.template get_field<Head>()));
            if constexpr (sizeof...(Tail) > 0)
                unmaterialize_impl<Tail...>(data, o);
        }

        template<std::size_t ...I>
        static inline void unmaterialize_impl(data_span data, std::index_sequence<I...> is, T &o) {
            unmaterialize_impl<I...>(data, o);
        }


    public:
        static inline T materialize(data_span_c data) {
            auto matuple = make_materialized_tuple(data);

            return std::make_from_tuple<T>(std::move(matuple));
        }

        static inline void unmaterialize(data_span data, T &o) {
            unmaterialize_impl(data, index_seq{}, o);
        }

        static inline void unmaterialize(data_span data, T &&o) {
            T storage(std::move(o));
            unmaterialize_impl(data, index_seq{}, static_cast<T&>(storage));
        }
    };


    template<TargetStruct T>
    struct target_sizeof<T> {
        static constexpr std::size_t value = T::size;
    };

    template<TargetStructDef Def>
    class target_struct_unnamed final
        : public Def::mat_tuple {
    private:
        using mat_tuple = typename Def::mat_tuple;

    public:
        using def = Def;
        static constexpr std::size_t size = Def::size;

        template<typename ...Args>
        inline target_struct_unnamed(Args&& ... args)
                : mat_tuple(std::forward<Args>(args)...) {}

        target_struct_unnamed(target_struct_unnamed &other) = delete;

        target_struct_unnamed &operator=(target_struct_unnamed &other) = delete;

        target_struct_unnamed &operator=(target_struct_unnamed &&other) = delete;

        template<std::size_t I>
        constexpr auto const &get_field() const {
            return std::get<I>(*this);
        }

        template<std::size_t I>
        constexpr auto &get_field() {
            return std::get<I>(*this);
        }
    };


    template<typename Type, std::size_t Offset>
    struct target_struct_field {
        using type = Type;
        static constexpr std::size_t offset = Offset;
        static constexpr std::size_t size = target_sizeof_v<Type>;
    };

    template<std::size_t Size, TargetStructField ...Fields>
    class target_struct_def {
    private:
        template<typename Field>
        using materialized_field_type = typename Field::type;

    public:
        using field_tuple = std::tuple<Fields...>;
        using mat_tuple = std::tuple<materialized_field_type<Fields>...>;
        static constexpr std::size_t field_count = sizeof...(Fields);
        static constexpr std::size_t size = Size;

    private:
        constexpr static std::size_t comp_size() {
            std::size_t res = 0;
            util::constexpr_for<0, field_count, 1>([&](auto i) constexpr {
                using C = std::tuple_element_t<i, field_tuple>;
                res = std::max(res, C::offset + C::size);
            });
            return res;
        }

        static_assert(comp_size() <= Size);
    };

    template<TargetStructDef Def, typename Mat>
    /* not using TargetStructMatCustom concept here, as it is not satisfied __yet__ */
    struct target_struct_base {
    private:
        static constexpr std::size_t field_count = Def::field_count;

    public:
        using def = Def;
        static constexpr std::size_t size = Def::size;

    protected:
        using mat_tuple = typename Def::mat_tuple;
        inline target_struct_base() {}

    public:
        template<std::size_t I>
        constexpr auto const &get_field() const {
            return const_cast<Mat *>(static_cast<Mat const *>(this))->template get_field<I>();
        }

        template<std::size_t I>
        constexpr auto &get_field() {
            // MSVC WTF
            return Mat::field_accessor::template get_field<I>(*static_cast<Mat *>(this));
        }

        bool operator==(target_struct_base const& o) const {
            bool result = true;
            util::constexpr_for<0, Def::field_count, 1>([&](auto i) {
                result &= get_field<i>() == o.template get_field<i>();
            });
            return result;
        }
        bool operator !=(target_struct_base const& o) const {
            return !operator==(o);
        }

    protected:
        template<auto ...Args>
        struct field_accessor_base {
            template<std::size_t I>
            // MSVC WTF
            static auto &get_field(Mat &mat) {
                static_assert(I >= 0 && I < field_count);
                auto p = std::get<I>(std::make_tuple(Args...));
                return mat.*p;
            }
        };
    };
}