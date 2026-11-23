anybox_add_system_library(xmem src/Exports.cpp xmem.def)
target_include_directories(xmem PUBLIC include)
anybox_add_exception_test(xmem)
