#include "test.h"

#include <foreign/struct.h>
#include <foreign/integer.h>

#include "FixedWidthIntLiterals.h"

using namespace scw::intliterals;

template<typename ...Fields>
using def = foreign::target_struct_def<Fields...>;

template<typename Type, std::size_t Offset>
using field = foreign::target_struct_field<Type, Offset>;


template<foreign::TargetStruct T>
struct stringifier<T> {
    static auto stringify_value(const T& val) {
        std::stringstream ss;
        ss << '{';
        auto first = true;
        foreign::util::constexpr_for<0, T::def::field_count, 1>([&](auto i) {
            ss << (first ? "" : ", ") << stringify(val.template get_field<i>());
            first = false;
        });
        ss << '}';
        return ss.str();
    }
};


using def_1 = def<
            field<std::uint8_t, 0>,
            field<std::uint32_t, 4>
        >;

using unnamed_1 = foreign::target_struct_unnamed<def_1>;

struct named_1 : foreign::target_struct_base<def_1, named_1> {
    named_1(std::uint8_t a, std::uint32_t b)
        : a(a)
        , b(b) {}

    std::uint8_t a;
    std::uint32_t b;

    using field_accessor = field_accessor_base<
                &named_1::a,
                &named_1::b
            >;
};

static_assert(foreign::TargetStructNamed<named_1>);

ut::suite suite = [] {
    using namespace ut;

    "unmaterialize"_test = [] {
        expect(umat_chk<unnamed_1>({0, 0}, "0000000000000000"_hex));
        expect(umat_chk<unnamed_1>({1, 1}, "0100000001000000"_hex));
        expect(umat_chk<unnamed_1>({0xff, 0xffffffff}, "ff000000ffffffff"_hex));

        expect(umat_chk<named_1>({0, 0}, "0000000000000000"_hex));
        expect(umat_chk<named_1>({1, 1}, "0100000001000000"_hex));
        expect(umat_chk<named_1>({0xff, 0xffffffff}, "ff000000ffffffff"_hex));
    };

    "materialize"_test = [] {
        expect(mat_chk<unnamed_1>("0000000000000000"_hex, {0, 0}));
        expect(mat_chk<unnamed_1>("0100000001000000"_hex, {1, 1}));
        expect(mat_chk<unnamed_1>("ff000000ffffffff"_hex, {0xff, 0xffffffff}));

        expect(mat_chk<named_1>("0000000000000000"_hex, {0, 0}));
        expect(mat_chk<named_1>("0100000001000000"_hex, {1, 1}));
        expect(mat_chk<named_1>("ff000000ffffffff"_hex, {0xff, 0xffffffff}));
    };

    "access"_test = [] {
        expect(acc_chk<unnamed_1>("0000000000000000"_hex, [&](auto& t) {
            std::get<0>(t) = 1;
            t.template get_field<1>() = 2;
        }, "0100000002000000"_hex));

        expect(acc_chk<unnamed_1>("ffffffffffffffff"_hex, [&](auto& t) {
            std::get<0>(t) = 1;
            t.template get_field<1>() = 2;
        }, "01ffffff02000000"_hex));
    };
};