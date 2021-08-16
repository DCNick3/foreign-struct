#include "test.h"

#include "union_stringify.h"

#include <foreign/union.h>
#include <foreign/integer.h>

ut::suite suite = [] {
    using namespace ut;

    using test_union = foreign::target_union<
        std::uint16_t,
        std::uint32_t,
        std::uint64_t
    >;

    "from"_test = [] {
        expect(umat_chk(test_union::from<0>(0xf00), "000f000000000000"_hex));
        expect(umat_chk(test_union::from<1>(0x10f00), "000f010000000000"_hex));
        expect(umat_chk(test_union::from<2>(0xfedcba9876543210), "1032547698badcfe"_hex));
    };

    "set"_test = [] {
        expect(acc_chk<test_union>("1032547698badcfe"_hex, [](auto& u) {
            u.template set<0>(1231);
            }, "cf04547698badcfe"_hex));
        expect(acc_chk<test_union>("1032547698badcfe"_hex, [](auto& u) {
            u.template set<1>(0xffeeddcc);
            }, "ccddeeff98badcfe"_hex));
        expect(acc_chk<test_union>("1032547698badcfe"_hex, [](auto& u) {
            u.template set<2>(0x1032547698badcfe);
            }, "fedcba9876543210"_hex));
    };

    "get"_test = [] {
        auto u = foreign::target_materialize<test_union>("1032547698badcfe"_hex);

        expect(eq(u.get<0>(), 0x3210));
        expect(eq(u.get<1>(), 0x76543210));
        expect(eq(u.get<2>(), 0xfedcba9876543210));
    };

    "use"_test = [] {
        expect(acc_chk<test_union>("1032547698badcfe"_hex, [](auto& u) {
                auto t = u.template use<0>();
                *t = 1231;
            }, "cf04547698badcfe"_hex));
        expect(acc_chk<test_union>("1032547698badcfe"_hex, [](auto& u) {
                auto t = u.template use<1>();
                *t = 0xffeeddcc;
            }, "ccddeeff98badcfe"_hex));
        expect(acc_chk<test_union>("1032547698badcfe"_hex, [](auto& u) {
                auto t = u.template use<2>();
                *t = 0x1032547698badcfe;
            }, "fedcba9876543210"_hex));
    };
};
