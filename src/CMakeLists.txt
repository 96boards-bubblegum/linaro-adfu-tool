find_package(PkgConfig)
pkg_check_modules(USB libusb-1.0)

link_directories(${USB_LIBRARY_DIRS})
include_directories(${USB_INCLUDE_DIRS})

add_executable(linaro-adfu-tool-bg96 linaro-adfu-tool-bg96.c)
target_link_libraries(linaro-adfu-tool-bg96 ${USB_LIBRARIES})

install (TARGETS linaro-adfu-tool-bg96 DESTINATION bin)
