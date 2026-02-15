#pragma once
#include <cstdint>
#include <string_view>

// FNV-1a Hashing
class Hash {
public:
    using Value = uint32_t;

    static constexpr Value OffsetBasis = 0x811c9dc5;
    static constexpr Value Prime       = 0x01000193;

    static constexpr Value Get(const char* str) {
        Value hash = OffsetBasis;
        while (*str) {
            hash ^= static_cast<Value>(*str++);
            hash *= Prime;
        }
        return hash;
    }

    static constexpr Value Get(std::string_view str) {
        Value hash = OffsetBasis;
        for (char c : str) {
            hash ^= static_cast<Value>(c);
            hash *= Prime;
        }
        return hash;
    }
};

constexpr Hash::Value operator "" _h(const char* str, size_t len) {
    Hash::Value hash = Hash::OffsetBasis;
    for (size_t i = 0; i < len; ++i) {
        hash ^= static_cast<Hash::Value>(str[i]);
        hash *= Hash::Prime;
    }
    return hash;
}