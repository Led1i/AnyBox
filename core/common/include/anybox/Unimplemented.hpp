#ifndef ANYBOX_UNIMPLEMENTED_HPP
#define ANYBOX_UNIMPLEMENTED_HPP

namespace AnyBox {

[[noreturn]] void NotImplemented(const char* libraryName, const char* functionName);

}

#define ANYBOX_NOT_IMPLEMENTED() ::AnyBox::NotImplemented(ANYBOX_LIBRARY_NAME, __func__)

#endif
