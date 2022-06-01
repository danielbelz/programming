cmake_minimum_required (VERSION 3.12)

# Uncomment this to enable debugging for this CMake file
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(LIBGLIB2 "glib-2.0")


# Begin find glib

if (GLIB_PKG_FOUND)
    find_path(GLIB_INCLUDE_DIR  NAMES glib.h PATH_SUFFIXES glib-2.0
       PATHS
       ${GLIB_PKG_INCLUDE_DIRS}
       ${CMAKE_SYSROOT}/usr/include/glib-2.0
       ${CMAKE_SYSROOT}/usr/include
       ${CMAKE_SYSROOT}/usr/local/include
    )
    find_path(GLIB_CONFIG_INCLUDE_DIR NAMES glibconfig.h PATHS ${GLIB_PKG_LIBDIR} PATH_SUFFIXES glib-2.0/include)

    find_library(GLIB_LIBRARIES NAMES glib-2.0
       PATHS
       ${GLIB_PKG_LIBRARY_DIRS}
       ${CMAKE_SYSROOT}/usr/lib
       ${CMAKE_SYSROOT}/usr/local/lib
    )

else (GLIB_PKG_FOUND)
    # Find Glib even if pkg-config is not working (eg. cross compiling to Windows)
    find_library(GLIB_LIBRARIES NAMES glib-2.0)
    string (REGEX REPLACE "/[^/]*$" "" GLIB_LIBRARIES_DIR ${GLIB_LIBRARIES})

    find_path(GLIB_INCLUDE_DIR NAMES glib.h PATH_SUFFIXES glib-2.0)
    find_path(GLIB_CONFIG_INCLUDE_DIR NAMES glibconfig.h PATHS ${GLIB_LIBRARIES_DIR} PATH_SUFFIXES glib-2.0/include)

endif (GLIB_PKG_FOUND)

if (GLIB_INCLUDE_DIR AND GLIB_CONFIG_INCLUDE_DIR AND GLIB_LIBRARIES)
    set(GLIB_INCLUDE_DIRS ${GLIB_INCLUDE_DIR} ${GLIB_CONFIG_INCLUDE_DIR})
endif (GLIB_INCLUDE_DIR AND GLIB_CONFIG_INCLUDE_DIR AND GLIB_LIBRARIES)

if(GLIB_INCLUDE_DIRS AND GLIB_LIBRARIES)
   set(GLIB_FOUND TRUE CACHE INTERNAL "glib-2.0 found")
   message(STATUS "Found glib-2.0: ${GLIB_INCLUDE_DIR}, ${GLIB_LIBRARIES}")
else(GLIB_INCLUDE_DIRS AND GLIB_LIBRARIES)
   set(GLIB_FOUND FALSE CACHE INTERNAL "glib-2.0 found")
   message(STATUS "glib-2.0 not found.")
endif(GLIB_INCLUDE_DIRS AND GLIB_LIBRARIES)

mark_as_advanced(GLIB_INCLUDE_DIR GLIB_CONFIG_INCLUDE_DIR GLIB_INCLUDE_DIRS GLIB_LIBRARIES)

# End find glib

set(test_app_targets
    load_parse_config
    split_str
    #add other source applications here
    )

foreach(TARGETAPP ${test_app_targets})
    # Source files
    add_executable(${TARGETAPP} "${TARGETAPP}.c")
    target_compile_options(${TARGETAPP} PRIVATE -g3 -Wall -Werror  ${COMPILER_OPTS})

    # include folders
    target_include_directories (${TARGETAPP} PRIVATE ${CMAKE_SYSROOT}/usr/include ${GLIB_INCLUDE_DIRS} ${H_FILES})
    target_link_libraries(${TARGETAPP} PRIVATE c pthread rt m ${LIBGLIB2} )

    set_target_properties(
        ${TARGETAPP}
        PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO )
endforeach(TARGETAPP)