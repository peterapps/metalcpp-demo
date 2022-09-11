#pragma once

#include "Metal.hpp"

NS::String *nsstr(const char *cstr);
const char *cstr(NS::String *nsstr);
const char *cstr(NS::Error *error);
dispatch_data_t readEmbeddedMetallib();