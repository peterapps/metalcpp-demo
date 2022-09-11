#define __BLOCKS__ 1 // dispatch_data_create

#include "mtl_utils.hpp"

extern char metallib_start __asm("section$start$metallib$metallib");
extern char metallib_end __asm("section$end$metallib$metallib");

NS::String *nsstr(const char *cstr) {
    return NS::String::string(cstr, NS::ASCIIStringEncoding);
}

const char *cstr(NS::String *nsstr) {
    return nsstr->cString(NS::ASCIIStringEncoding);
}

const char *cstr(NS::Error *error) {
    return error->description()->cString(NS::ASCIIStringEncoding);
}

dispatch_data_t readEmbeddedMetallib() {
    char *data = &metallib_start;
    size_t dataSize = &metallib_end - &metallib_start;
    return dispatch_data_create(data, dataSize, NULL, DISPATCH_DATA_DESTRUCTOR_FREE);
}