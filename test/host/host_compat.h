// Force-included when building the host tests, so firmware sources compile with MSVC.
#pragma once

#ifdef _MSC_VER
#define __attribute__(x)
#endif
