#include "test.h"

#include "struct_stringify.h"

#include <foreign/struct.h>
#include <foreign/integer.h>

#include "FixedWidthIntLiterals.h"

using namespace scw::intliterals;

template<std::size_t Size, typename ...Fields>
using def = foreign::target_struct_def<Size, Fields...>;

template<typename Type, std::size_t Offset>
using field = foreign::target_struct_field<Type, Offset>;

using def_1 = def<
            8,
            field<std::uint8_t, 0>,
            field<std::uint32_t, 4>
        >;

using def_2 = def<
            8,
            field<std::uint32_t, 0>,
            field<std::uint8_t, 4>
        >;

using unnamed_1 = foreign::target_struct_unnamed<def_1>;
using unnamed_2 = foreign::target_struct_unnamed<def_2>;

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

struct named_2 : foreign::target_struct_base<def_2, named_2> {
    named_2(std::uint32_t a, std::uint8_t b)
    : a(a)
    , b(b) {}

    std::uint32_t a;
    std::uint8_t b;

    using field_accessor = field_accessor_base<
            &named_2::a,
            &named_2::b
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

        expect(umat_chk<unnamed_2>({0, 0}, "0000000000000000"_hex));
        expect(umat_chk<unnamed_2>({1, 1}, "0100000001000000"_hex));
        expect(umat_chk<unnamed_2>({0xffffffff, 0xff}, "ffffffffff000000"_hex));

        expect(umat_chk<named_2>({0, 0}, "0000000000000000"_hex));
        expect(umat_chk<named_2>({1, 1}, "0100000001000000"_hex));
        expect(umat_chk<named_2>({0xffffffff, 0xff}, "ffffffffff000000"_hex));
    };

    "materialize"_test = [] {
        expect(mat_chk<unnamed_1>("0000000000000000"_hex, {0, 0}));
        expect(mat_chk<unnamed_1>("0100000001000000"_hex, {1, 1}));
        expect(mat_chk<unnamed_1>("ff000000ffffffff"_hex, {0xff, 0xffffffff}));

        expect(mat_chk<named_1>("0000000000000000"_hex, {0, 0}));
        expect(mat_chk<named_1>("0100000001000000"_hex, {1, 1}));
        expect(mat_chk<named_1>("ff000000ffffffff"_hex, {0xff, 0xffffffff}));

        expect(mat_chk<unnamed_2>("0000000000000000"_hex, {0, 0}));
        expect(mat_chk<unnamed_2>("0100000001000000"_hex, {1, 1}));
        expect(mat_chk<unnamed_2>("ffffffffff000000"_hex, {0xffffffff, 0xff}));

        expect(mat_chk<named_2>("0000000000000000"_hex, {0, 0}));
        expect(mat_chk<named_2>("0100000001000000"_hex, {1, 1}));
        expect(mat_chk<named_2>("ffffffffff000000"_hex, {0xffffffff, 0xff}));
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

        expect(acc_chk<named_1>("0000000000000000"_hex, [&](auto& t) {
            t.a = 1;
            t.b = 2;
            }, "0100000002000000"_hex));

        expect(acc_chk<named_1>("ffffffffffffffff"_hex, [&](auto& t) {
            t.a = 1;
            t.b = 2;
            }, "01ffffff02000000"_hex));


        expect(acc_chk<unnamed_2>("0000000000000000"_hex, [&](auto& t) {
            std::get<0>(t) = 1;
            t.template get_field<1>() = 2;
            }, "0100000002000000"_hex));

        expect(acc_chk<unnamed_2>("ffffffffffffffff"_hex, [&](auto& t) {
            std::get<0>(t) = 1;
            t.template get_field<1>() = 2;
            }, "0100000002ffffff"_hex));

        expect(acc_chk<named_2>("0000000000000000"_hex, [&](auto& t) {
            t.a = 1;
            t.b = 2;
            }, "0100000002000000"_hex));

        expect(acc_chk<named_2>("ffffffffffffffff"_hex, [&](auto& t) {
            t.a = 1;
            t.b = 2;
            }, "0100000002ffffff"_hex));
    };
};