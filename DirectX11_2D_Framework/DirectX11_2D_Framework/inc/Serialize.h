#include <cereal/cereal.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

static constexpr int SceneFileIndex = 's';

#define SERIALIZE_OUTPUT cereal::JSONOutputArchive
#define SERIALIZE_INPUT cereal::JSONInputArchive

#define REGISTER_TYPE_WITH_CEREAL(base, derived)         \
    CEREAL_REGISTER_TYPE(derived);                       \
    CEREAL_REGISTER_POLYMORPHIC_RELATION(base, derived)


#define COUNT_ARGS_IMPL( \
     _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, _10, \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
    _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, \
    _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, \
    N, ...) N
#define COUNT_ARGS(...) EXPAND(COUNT_ARGS_IMPL(__VA_ARGS__, \
    50, 49, 48, 47, 46, 45, 44, 43, 42, 41, \
    40, 39, 38, 37, 36, 35, 34, 33, 32, 31, \
    30, 29, 28, 27, 26, 25, 24, 23, 22, 21, \
    20, 19, 18, 17, 16, 15, 14, 13, 12, 11, \
    10,  9,  8,  7,  6,  5,  4,  3,  2,  1, 0))

// Handle empty argument lists correctly
#define HAS_ARGS(...) EXPAND(COUNT_ARGS(__VA_ARGS__, dummy)) // 'dummy' ensures we can distinguish empty from one-arg case
#define COUNT(...) (HAS_ARGS(__VA_ARGS__) == 2 ? 0 : COUNT_ARGS(__VA_ARGS__))


// Helper macros to expand and wrap each argument
#define EXPAND(x) x
#define CEREAL_WRAP(arg) CEREAL_NVP(arg)

// Recursive macro to process each argument
#define SERIALIZE_ARGS_1(arg1) CEREAL_WRAP(arg1)
#define SERIALIZE_ARGS_2(arg1, arg2) CEREAL_WRAP(arg1), SERIALIZE_ARGS_1(arg2)
#define SERIALIZE_ARGS_3(arg1, arg2, arg3) CEREAL_WRAP(arg1), SERIALIZE_ARGS_2(arg2, arg3)
#define SERIALIZE_ARGS_4(arg1, arg2, arg3, arg4) CEREAL_WRAP(arg1), SERIALIZE_ARGS_3(arg2, arg3, arg4)
#define SERIALIZE_ARGS_5(arg1, arg2, arg3, arg4, arg5) CEREAL_WRAP(arg1), SERIALIZE_ARGS_4(arg2, arg3, arg4, arg5)
#define SERIALIZE_ARGS_6(arg1, arg2, arg3, arg4, arg5, arg6) CEREAL_WRAP(arg1), SERIALIZE_ARGS_5(arg2, arg3, arg4, arg5, arg6)
#define SERIALIZE_ARGS_7(arg1, arg2, arg3, arg4, arg5, arg6, arg7) CEREAL_WRAP(arg1), SERIALIZE_ARGS_6(arg2, arg3, arg4, arg5, arg6, arg7)
// Extend as needed for more arguments

// Dispatcher macro to choose the correct expansion
#define GET_SERIALIZE_ARGS_MACRO(_1, _2, _3, _4, _5, _6, _7, NAME, ...) NAME
#define SERIALIZE_ARGS(...) EXPAND(GET_SERIALIZE_ARGS_MACRO(__VA_ARGS__, SERIALIZE_ARGS_7, SERIALIZE_ARGS_6, SERIALIZE_ARGS_5, SERIALIZE_ARGS_4, SERIALIZE_ARGS_3, SERIALIZE_ARGS_2, SERIALIZE_ARGS_1)(__VA_ARGS__))

// Define the GENERATE_SERIALIZE macro
#define GENERATE_SERIALIZE(...)                             \
friend class cereal::access;                                \
template <class Archive>                                    \
void serialize(Archive& ar) {                               \
    if constexpr (COUNT_ARGS( __VA_ARGS__ ) <= 7) {         \
        ar(SERIALIZE_ARGS(__VA_ARGS__));                    \
    } else {                                                \
        ar(__VA_ARGS__);                                    \
    }                                                       \
}

namespace cereal
{
    template<class Archive>
    void serialize(Archive& ar, DirectX::XMFLOAT3& vec)
    {
        ar(CEREAL_NVP(vec.x), CEREAL_NVP(vec.y), CEREAL_NVP(vec.z));
    }

    template<class Archive>
    void serialize(Archive& ar, DirectX::XMFLOAT4& f)
    {
        ar(CEREAL_NVP(f.x), CEREAL_NVP(f.y), CEREAL_NVP(f.z), CEREAL_NVP(f.w));
    }

    template <class Archive>
    void save(Archive& archive, const std::wstring& wstr) {
        // Convert std::wstring to std::string
        std::string str(wstr.begin(), wstr.end());
        archive(str); // Serialize as a std::string
    }

    template <class Archive>
    void load(Archive& archive, std::wstring& wstr) {
        // Deserialize as a std::string and convert back to std::wstring
        std::string str;
        archive(str);
        wstr.assign(str.begin(), str.end());
    }
}

