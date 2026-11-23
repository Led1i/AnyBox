anybox_add_system_library(AcpHal src/Exports.cpp AcpHal.def)
target_include_directories(AcpHal PUBLIC include)

anybox_add_exception_test(AcpHal)
