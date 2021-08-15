#include "test.h"

#include <foreign/enum.h>
#include <foreign/integer.h>

enum plain_u32 : std::uint32_t {
    PU32_ZERO = 0x0,
    PU32_BOM = 0xfeff,
    PU32_FFFFFFFF = 0xffffffff,
};

enum plain_i8 : std::int8_t {
    PI8_ZERO = 0x0,
    PI8_NEG = -1,
    PI8_MAX = 0x7f,
};

enum class scoped_u32 : std::uint32_t {
    ZERO = 0x0,
    BOM = 0xfeff,
    FFFFFFFF = 0xffffffff,
};

enum class scoped_i8 : std::int8_t {
    ZERO = 0x0,
    NEG = -1,
    MAX = 0x7f
};

ut::suite suite = [] {
    using namespace ut;

    "unmaterialize"_test = [] {
        expect(umat_chk(PU32_ZERO, "00000000"_hex));
        expect(umat_chk(PU32_BOM, "fffe0000"_hex));
        expect(umat_chk(PU32_FFFFFFFF, "ffffffff"_hex));

        expect(umat_chk(PI8_ZERO, "00"_hex));
        expect(umat_chk(PI8_NEG, "ff"_hex));
        expect(umat_chk(PI8_MAX, "7f"_hex));

        expect(umat_chk(scoped_u32::ZERO, "00000000"_hex));
        expect(umat_chk(scoped_u32::BOM, "fffe0000"_hex));
        expect(umat_chk(scoped_u32::FFFFFFFF, "ffffffff"_hex));

        expect(umat_chk(scoped_i8::ZERO, "00"_hex));
        expect(umat_chk(scoped_i8::NEG, "ff"_hex));
        expect(umat_chk(scoped_i8::MAX, "7f"_hex));
    };

    "materialize"_test = [] {
        expect(mat_chk("00000000"_hex, PU32_ZERO));
        expect(mat_chk("fffe0000"_hex, PU32_BOM));
        expect(mat_chk("ffffffff"_hex, PU32_FFFFFFFF));

        expect(mat_chk("00"_hex, PI8_ZERO));
        expect(mat_chk("ff"_hex, PI8_NEG));
        expect(mat_chk("7f"_hex, PI8_MAX));

        expect(mat_chk("00000000"_hex, scoped_u32::ZERO));
        expect(mat_chk("fffe0000"_hex, scoped_u32::BOM));
        expect(mat_chk("ffffffff"_hex, scoped_u32::FFFFFFFF));

        expect(mat_chk("00"_hex, scoped_i8::ZERO));
        expect(mat_chk("ff"_hex, scoped_i8::NEG));
        expect(mat_chk("7f"_hex, scoped_i8::MAX));
    };

    "access"_test = [] {
        expect(acc_chk<scoped_i8>("13"_hex, [](scoped_i8& e) { e = scoped_i8::NEG; }, "ff"_hex));
        expect(acc_chk<scoped_u32>("00000000"_hex, [](scoped_u32& e) { e = scoped_u32::BOM; }, "fffe0000"_hex));
    };
};
