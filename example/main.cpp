
#include "foreign/struct.h"
#include "foreign/union.h"
#include "foreign/enum.h"
#include "foreign/integer.h"
#include "foreign/util.h"

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace foreign;

template<std::size_t N>
std::string hex_str(std::array<std::uint8_t, N> data) {
    std::stringstream ss;
    ss << std::hex;

    for (int i(0); i < data.size(); ++i)
        ss << std::setw(2) << std::setfill('0') << (int) data[i];

    return ss.str();
}

// all the target structures are defined here
enum class test_enum : std::uint32_t {
    value1 = 100,
    value2 = 200
};

using test_union = target_union<
        std::uint16_t,
        std::uint32_t,
        std::uint64_t,
        test_enum
>;


// definitions of struct fields
using test_struct_def = target_struct_def<
    target_struct_field<std::int32_t, 0>,
    target_struct_field<std::uint64_t, 8>,
    target_struct_field<test_enum, 16>,
    target_struct_field<test_union, 20>
>;

/*struct test_struct : public target_struct_base<test_struct_def, test_struct> {
    inline test_struct(mat_tuple &&values)
            :
            value1(std::move(std::get<0>(values))),
            value2(std::move(std::get<1>(values))),
            value3(std::move(std::get<2>(values))),
            value4(std::move(std::get<3>(values))) {}

    int32_t value1;
    uint64_t value2;
    test_enum value3;
    test_union value4;

    using field_accessor = target_struct_base::field_accessor_base<
            &test_struct::value1,
            &test_struct::value2,
            &test_struct::value3,
            &test_struct::value4
    >;
};*/

using test_struct = target_struct_unnamed<test_struct_def>;

using wrap_struct_def = target_struct_def<
target_struct_field<test_struct, 0>,
target_struct_field<std::uint32_t, 28>
>;

// a helper class that allows to access fields by their names
// a good place for code generation
struct wrap_struct : public target_struct_base<wrap_struct_def, wrap_struct> {
    inline wrap_struct(mat_tuple &&values)
            :
            test_struct_v(std::move(std::get<0>(values))),
            test_val(std::move(std::get<1>(values))) {}

    test_struct test_struct_v;
    std::uint32_t test_val;

    using field_accessor = target_struct_base::field_accessor_base<
            &wrap_struct::test_struct_v,
            &wrap_struct::test_val
    >;
};

static_assert(TargetStruct<wrap_struct>);

void magic(std::uint8_t *p) {
    // object implicitly created before, safe to alias with unsigned char
    //auto& s = *reinterpret_cast<wrap_struct*>(p);
    auto span = span_for<wrap_struct>(p, target_sizeof_v<wrap_struct>);


    /*{
        auto mat_wrap = target_materialize<wrap_struct>(span);
        auto& mat = std::get<0>(mat_wrap);

        //auto mat = s.materialize();

        //std::cout << "s.size = " << s.size << std::endl;
        //std::cout << "mat[0] = " << std::get<0>(mat) << std::endl;
        //std::cout << "mat[1] = " << std::get<1>(mat) << std::endl;
        //std::cout << "mat[2] = " << std::get<2>(mat) << std::endl;

        //std::cout << "mat_wrap[3] = " << std::get<1>(mat_wrap) << std::endl;

        mat.value1 -= 1;
        mat.value2 += 2;
        mat.value3 = test_enum::value1;

        //std::get<0>(mat) = 1;
        //std::get<1>(mat) = 2;
        //std::get<2>(mat) = 3;
        //mat_wrap.get_field<1>() = 1337;
        //std::get<1>() = 1337;

        //s.unmaterialize(mat);
        target_unmaterialize<wrap_struct>(span, std::move(mat_wrap));
        //s.unmaterialize(mat_wrap);
    }*/
    {
        auto holder = target_rw_mat_holder<wrap_struct>(span);
        holder->test_struct_v.get_field<0>() -= 1;
        holder->test_struct_v.get_field<1>() += 1;
        holder->test_struct_v.get_field<2>() = test_enum::value1;
        holder->test_struct_v.get_field<3>().set<0>(12);

        holder->test_val = 1337;
        holder->get_field<1>() = 1337;
    }

    //std::cout << "data = " << hex_str(s.data) << std::endl;
}


int main() {
    std::array<std::uint8_t, target_sizeof_v<wrap_struct>> a;

    memset(a.data(), 255, a.size());

    {
        //auto mat_wrap = target_materialize<wrap_struct>(a);

        auto holder = target_rw_mat_holder<wrap_struct>(a);
        auto &mat = holder->test_struct_v;
        //holder->test_struct_v.value1 -= 1;
        //holder->test_struct_v.value2 += 1;
        //holder->test_struct_v.value3 = test_enum::value1;
        //

        holder->test_val = 1337;
        holder->get_field<1>() = 1337;


        std::cout << "s.size = " << target_sizeof_v<wrap_struct> << std::endl;
        std::cout << "mat.value1 = " << mat.get_field<0>() << std::endl;
        std::cout << "mat.value2 = " << mat.get_field<1>() << std::endl;
        std::cout << "mat.value3 = " << static_cast<int>(mat.get_field<2>()) << std::endl;

        std::cout << "mat_wrap[1] = " << holder->get_field<1>() << std::endl;

        // can either use the get_field method
        mat.get_field<0>() = 1;
        // or an std::get, as it's a tuple
        std::get<1>(mat) += 3;
        mat.get_field<2>() = test_enum::value1;
        holder->test_struct_v.get_field<3>().set<0>(12);

        //std::get<0>(mat) = 1;
        //std::get<1>(mat) = 2;
        //std::get<2>(mat) = 3;
        holder->get_field<1>() = 1337;
    }

    {
        auto holder = target_rw_mat_holder<wrap_struct>(a);
        auto &mat = holder->test_struct_v;

        std::cout << "s.size = " << target_sizeof_v<wrap_struct> << std::endl;
        std::cout << "mat.value1 = " << mat.get_field<0>() << std::endl;
        std::cout << "mat.value2 = " << mat.get_field<1>() << std::endl;
        std::cout << "mat.value3 = " << static_cast<int>(mat.get_field<2>()) << std::endl;
        std::cout << "mat.value4[uin32_t] = " << mat.get_field<3>().get<1>() << std::endl;

        std::cout << "mat_wrap[1] = " << holder->test_val << std::endl;
    }

    std::cout << "data = " << hex_str(a) << std::endl;

    return 0;
}

