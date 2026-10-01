// textkit – a tiny text-statistics library used by Day 29 as if it were a third-party dependency.
#pragma once

#define TEXTKIT_VERSION_MAJOR 1
#define TEXTKIT_VERSION_MINOR 4
#define TEXTKIT_VERSION_PATCH 2

namespace textkit {

/// The version of the *compiled* library – may differ from the headers if the wrong binary is linked.
const char* version();

}  // namespace textkit
