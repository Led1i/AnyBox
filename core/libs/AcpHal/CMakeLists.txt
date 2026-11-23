anybox_add_system_library(AcpHal src/Exports.cpp src/memory/Allocation.cpp src/memory/Exports.cpp src/memory/Heap.cpp src/memory/Mapping.cpp AcpHal.def)
target_include_directories(AcpHal PUBLIC include)
target_link_libraries(AcpHal PRIVATE xmem)

anybox_add_exception_test(AcpHal)

if(BUILD_TESTING)
    add_executable(acphal_memory_tests tests/memory/Allocation.cpp tests/memory/Concurrent.cpp tests/memory/Errors.cpp tests/memory/Main.cpp tests/memory/Mapping.cpp)
    target_link_libraries(acphal_memory_tests PRIVATE AcpHal)
    anybox_export_symbols(acphal_memory_tests tests/memory/Hooks.def)
    set_target_properties(acphal_memory_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${ANYBOX_RUNTIME_DIRECTORY}")
    add_test(NAME acphal.memory COMMAND acphal_memory_tests)
    set_tests_properties(acphal.memory PROPERTIES TIMEOUT 15)
endif()
