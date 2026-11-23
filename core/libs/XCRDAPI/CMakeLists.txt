anybox_add_system_library(XCRDAPI src/Exports.cpp XCRDAPI.def)
target_include_directories(XCRDAPI PUBLIC include)
target_link_libraries(XCRDAPI PUBLIC anybox_common)
anybox_add_exception_test(XCRDAPI)
