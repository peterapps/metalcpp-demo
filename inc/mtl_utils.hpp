#pragma once

#include "Metal.hpp"
#include <tuple>

NS::String *nsstr(const char *cstr);
const char *cstr(NS::String *nsstr);
const char *cstr(NS::Error *error);
dispatch_data_t readEmbeddedMetalLib();
