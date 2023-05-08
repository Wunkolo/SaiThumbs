#pragma once
#include <cstdint>

namespace Globals
{
// Global atomic reference counter used to
// determine when it is safe to unload the entire DLL.
// DllCanUnloadNow() is exported and checks this variable
// to determine when it is safe to unload this DLL without leakage.
// Any time a relavant class or destroyed these values
// get incremented and decremented to signal that the DLL
// is still in use and has floating classes
std::intmax_t ReferenceAdd();
std::intmax_t ReferenceRelease();
std::intmax_t ReferenceGet();
} // namespace Globals