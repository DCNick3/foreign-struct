
#include "test.h"

#include <foreign/integer.h>

#include <FixedWidthIntLiterals.h>

using namespace scw::intliterals;

ut::suite integer = [] {
    using namespace ut;

    "unmaterialize"_test = [] {
        expect(umat_chk(0_u8, "00"_hex));
        expect(umat_chk(0_u8, "00"_hex));
        expect(umat_chk(1_u8, "01"_hex));
        expect(umat_chk(0xff_u8, "ff"_hex));

        // little endian here =)
        expect(umat_chk(0_u16, "0000"_hex));
        expect(umat_chk(1_u16, "0100"_hex));
        expect(umat_chk(0xff_u16, "ff00"_hex));
        expect(umat_chk(0xffff_u16, "ffff"_hex));

        expect(umat_chk(1_u32, "01000000"_hex));
        expect(umat_chk(-1_i32, "ffffffff"_hex));
    };

    "materialize"_test = [] {
        expect(mat_chk("00"_hex, 0_u8));
        expect(mat_chk("00"_hex, 0_u8));
        expect(mat_chk("01"_hex, 1_u8));
        expect(mat_chk("ff"_hex, 0xff_u8));

        // little endian here =)
        expect(mat_chk("0000"_hex, 0_u16));
        expect(mat_chk("0100"_hex, 1_u16));
        expect(mat_chk("ff00"_hex, 0xff_u16));
        expect(mat_chk("ffff"_hex, 0xffff_u16));

        expect(mat_chk("01000000"_hex, 1_u32));
        expect(mat_chk("ffffffff"_hex, -1_i32));
    };

    "access"_test = [] {
        const auto inc_u8 = [](std::uint8_t& t) {++t;};

        expect(acc_chk<std::uint8_t>("00"_hex, inc_u8, "01"_hex));
        expect(acc_chk<std::uint8_t>("01"_hex, inc_u8, "02"_hex));
        expect(acc_chk<std::uint8_t>("7f"_hex, inc_u8, "80"_hex));
        expect(acc_chk<std::uint8_t>("80"_hex, inc_u8, "81"_hex));
        expect(acc_chk<std::uint8_t>("ff"_hex, inc_u8, "00"_hex));

        const auto inc_u16 = [](std::uint16_t& t) {++t;};

        expect(acc_chk<std::uint16_t>("0000"_hex, inc_u16, "0100"_hex));
        expect(acc_chk<std::uint16_t>("0100"_hex, inc_u16, "0200"_hex));
        expect(acc_chk<std::uint16_t>("7f00"_hex, inc_u16, "8000"_hex));
        expect(acc_chk<std::uint16_t>("8000"_hex, inc_u16, "8100"_hex));
        expect(acc_chk<std::uint16_t>("ff00"_hex, inc_u16, "0001"_hex));
        expect(acc_chk<std::uint16_t>("ffff"_hex, inc_u16, "0000"_hex));

        const auto inc_u32 = [](std::uint32_t& t) {++t;};

        expect(acc_chk<std::uint32_t>("00000000"_hex, inc_u32, "01000000"_hex));
        expect(acc_chk<std::uint32_t>("01000000"_hex, inc_u32, "02000000"_hex));
        expect(acc_chk<std::uint32_t>("7f000000"_hex, inc_u32, "80000000"_hex));
        expect(acc_chk<std::uint32_t>("80000000"_hex, inc_u32, "81000000"_hex));
        expect(acc_chk<std::uint32_t>("ff000000"_hex, inc_u32, "00010000"_hex));
        expect(acc_chk<std::uint32_t>("ffff0000"_hex, inc_u32, "00000100"_hex));
        expect(acc_chk<std::uint32_t>("ffffffff"_hex, inc_u32, "00000000"_hex));
    };
};