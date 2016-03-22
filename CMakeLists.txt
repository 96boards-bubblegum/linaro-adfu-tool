cmake_minimum_required (VERSION 2.6)
project (linaro-adfu-tool-bg96)
set (linaro-adfu-tool-bg96_VERSION_MAJOR 0)
set (linaro-adfu-tool-bg96_VERSION_MINOR 1)
subdirs(src)

set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${linaro-adfu-tool-bg96_VERSION_MAJOR}.${linaro-adfu-tool-bg96_VERSION_MINOR}")
set(CPACK_SOURCE_GENERATOR "TXZ")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/.git/;~$;${CPACK_SOURCE_IGNORE_FILES};.*.cmake$;CMakeCache.txt;CMakeFiles;_CPack_Packages;Makefile;.*.tar.xz;install_manifest.txt")
include(CPack)

install (DIRECTORY ${PROJECT_SOURCE_DIR}/firmwares DESTINATION share/linaro-adfu-tool/bg96)
