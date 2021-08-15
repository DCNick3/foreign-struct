#include "test.h"

#include <foreign/array.h>
#include <foreign/integer.h>

#include "FixedWidthIntLiterals.h"

using namespace scw::intliterals;

ut::suite suite = [] {
    using namespace ut;

    "unmaterialize"_test = [] {
        expect(umat_chk(std::array{0x0_u8}, "00"_hex));
        expect(umat_chk(std::array{0x0_u8, 0x1_u8}, "0001"_hex));
        expect(umat_chk(std::array{0x100_u16, 0x1_u16}, "00010100"_hex));
    };

    "materialize"_test = [] {
        expect(mat_chk<std::array<std::uint8_t, 1>>("00"_hex, {0x0_u8}));
        expect(mat_chk<std::array<std::uint8_t, 2>>("0001"_hex, {0x0_u8, 0x1_u8}));
        expect(mat_chk<std::array<std::uint16_t, 2>>("00010100"_hex, {0x100_u16, 0x1_u16}));
    };

    "access"_test = [] {
        expect(acc_chk<std::array<std::uint8_t, 3>>("010203"_hex,
                [](auto& t) { t[1] = 4; }, "010403"_hex));

        expect(acc_chk<std::array<std::uint8_t, 3>>("010203"_hex,
                [](auto& t) { t[0] = t[1] = t[2] = 4; }, "040404"_hex));

        expect(acc_chk<std::array<std::uint16_t, 4>>("0001010000010100"_hex,
                [](auto& t) { t[0] = 0xffff; t[3] = 0xeeee; }, "ffff01000001eeee"_hex));

    };
};