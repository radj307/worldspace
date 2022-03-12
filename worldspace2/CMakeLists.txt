# worldspace/worldspace2
cmake_minimum_required(VERSION 3.20)

include(VersionTag)

MAKE_VERSION_HEADER("${CMAKE_CURRENT_SOURCE_DIR}/resources/version.h" "worldspace2" "${WORLDSPACE_VERSION}")

file(GLOB SRCS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"*.c*"
)
file(GLOB HEADERS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"*.h*"
)

add_executable(worldspace2 "${SRCS}" "resources/cpp.hint")

set_property(TARGET worldspace2 PROPERTY CXX_STANDARD 20)
set_property(TARGET worldspace2 PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET worldspace2 PROPERTY POSITION_INDEPENDENT_CODE ON)
if (MSVC)
	target_compile_options(worldspace2 PUBLIC "/Zc:__cplusplus")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # clang: Use -pthread compiler option
	target_compile_options(worldspace2 PUBLIC "-pthread")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU") # gcc: Use -pthread linker option
	target_link_options(worldspace2 PUBLIC "-pthread")
endif()

target_sources(worldspace2 PUBLIC "${HEADERS}")

target_link_libraries(worldspace2 PUBLIC shared TermAPI strlib optlib filelib)

include(PackageInstaller)
INSTALL_EXECUTABLE(worldspace2 "${CMAKE_INSTALL_PREFIX}/bin/")
