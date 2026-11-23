anybox_add_system_library(xapu src/Exports.cpp xapu.def)
target_include_directories(xapu PUBLIC include)
anybox_add_exception_test(xapu)
