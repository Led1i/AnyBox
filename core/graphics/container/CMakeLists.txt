add_library(anybox_container STATIC src/Dxbc.cpp src/Compression.cpp)
target_link_libraries(anybox_container PUBLIC anybox_options)
target_include_directories(anybox_container PUBLIC include)
