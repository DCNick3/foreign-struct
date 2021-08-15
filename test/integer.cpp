
#include "test.h"

#include <foreign/integer.h>

#include <FixedWidthIntLiterals.h>

using namespace scw::intliterals;
using namespace foreign::test;



ut::suite integer = [] {
    using namespace ut;

    "unmaterialize"_test = [] {
        expect(umat(0_u8) == "00"_hex);
        expect(umat(1_u8) == "01"_hex);
        expect(umat(0xff_u8) == "ff"_hex);

        // little endian here =)
        expect(umat(0_u16) == "0000"_hex);
        expect(umat(1_u16) == "0100"_hex);
        expect(umat(0xff_u16) == "ff00"_hex);
        expect(umat(0xffff_u16) == "ffff"_hex);

        expect(umat(1_u32) == "01000000"_hex);
        expect(umat(-1_u32) == "ffffffff"_hex);
    };
};