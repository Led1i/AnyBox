add_library(anybox_common STATIC src/Unimplemented.cpp)
target_include_directories(anybox_common PUBLIC include)
target_link_libraries(anybox_common PUBLIC anybox_options)

if(BUILD_TESTING)
    add_library(anybox_test_support INTERFACE)
    target_include_directories(anybox_test_support INTERFACE tests/include)
    target_link_libraries(anybox_test_support INTERFACE anybox_options)
endif()
