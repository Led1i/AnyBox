anybox_add_system_library(d3d12_xs src/Exports.cpp d3d12_xs.def)
target_include_directories(d3d12_xs PUBLIC include)
target_link_libraries(d3d12_xs PUBLIC anybox_common)
anybox_add_exception_test(d3d12_xs)
