#pragma once

#include <cstddef>
#include <cstdint>

namespace nusspli_abi {
struct TmdHeader {
    uint8_t padding0[0x18C];
    uint64_t title_id;
    uint32_t type;
    uint16_t group;
    uint8_t padding1[0x3A];
    uint32_t access_rights;
    uint16_t title_version;
};
static_assert(offsetof(TmdHeader, title_id) == 0x18C);
static_assert(offsetof(TmdHeader, title_version) == 0x1DC);
using TitleEntry = void;
using NusDev = int;
} // namespace nusspli_abi