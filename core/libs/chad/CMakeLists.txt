anybox_add_system_library(chad src/Exports.cpp chad.def)
target_include_directories(chad PUBLIC include)
target_link_libraries(chad PUBLIC anybox_common)
anybox_add_exception_test(chad)
