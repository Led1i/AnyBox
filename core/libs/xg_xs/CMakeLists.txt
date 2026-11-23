anybox_add_system_library(xg_xs src/Exports.cpp xg_xs.def)
target_include_directories(xg_xs PUBLIC include)
anybox_add_exception_test(xg_xs)
