cmake_minimum_required (VERSION 2.8)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# project name
set (PROJECT_NAME scil)
project (${PROJECT_NAME})

MESSAGE( STATUS "Preparing makefiles for SCIL")

# version numbers
set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)

set(DEV_DIR "${CMAKE_SOURCE_DIR}/../dev")
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/../deps")


find_package(PkgConfig REQUIRED)

include(CTest)
include(FeatureSummary)
include(CheckCSourceCompiles)

# compile flags
set (CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -pedantic-errors -std=gnu11 -pedantic -Wall -Wextra -Wdouble-promotion -Wfloat-equal -flto -fdata-sections -ffunction-sections -Wl,--gc-sections  -Wl,--no-allow-shlib-undefined")
# -fdiagnostics-color=always
# -fvisibility=hidden -fvisibility-inlines-hidden -Wl,--print-gc-sections
set ( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wdouble-promotion -Wfloat-equal -flto -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--no-allow-shlib-undefined -Wl,--retain-symbols-file=${CMAKE_SOURCE_DIR}/symbols.txt")

set (CMAKE_C_FLAGS_DEBUG   "-O0 -ggdb -gdwarf-2 -DDEBUG")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")

set (CMAKE_C_FLAGS_RELEASE "-O3")

set (SCIL_ALGO_SOURCE ${CMAKE_SOURCE_DIR}/algo)
set (SCIL_ALGO_BINARY ${CMAKE_BINARY_DIR}/algo)

# run during configure:
execute_process(COMMAND ${DEPS_DIR}/build-dependencies.sh WORKING_DIRECTORY ${DEPS_DIR})
execute_process(COMMAND ${DEV_DIR}/create-datatype-variants.sh ${CMAKE_SOURCE_DIR} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/)



#
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${DEPS_DIR}/openjpeg/lib/pkgconfig/")

pkg_search_module(OPENJPEG REQUIRED libopenjp2)


FILE(GLOB ALGO ${CMAKE_SOURCE_DIR}/algo/*.c ${CMAKE_BINARY_DIR}/algo/*.c)

# MESSAGE( STATUS "Available algorithms: " ${ALGO})
add_library(scil SHARED scil.c scil-util.c scil-errors.c scil-dummy.cpp scil-option.c scil-algo-chooser.c scil-hardware-limits.c ${ALGO} ${DEPS_DIR}/open-simplex-noise-in-c/open-simplex-noise.c)
target_link_libraries(scil m)

# for specific algorithms
include_directories(${CMAKE_BINARY_DIR} ${DEPS_DIR}/open-simplex-noise-in-c ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/pattern ${DEPS_DIR}/include/zfp ${DEPS_DIR}/include/fpzip ${OPENJPEG_INCLUDE_DIRS})

target_link_libraries(scil z)
target_link_libraries(scil
  ${DEPS_DIR}/libfpzip.a
  ${DEPS_DIR}/libzfp.a
  lz4
  ${OPENJPEG_LDFLAGS}
  )

configure_file("${CMAKE_SOURCE_DIR}/scil-config.h.in" "scil-config.h" @ONLY)


## Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )

    find_package( Java )
    # add_feature_info( Java DESCRIPTION "Java is needed in the documentation to run Plantuml" TYPE OPTIONAL)
    if(JAVA_FOUND)
      find_path (PLANTUML_JAR_PATH plantuml.jar HINTS ${DEPS_DIR} DOC "PLANTUML directory" ENV PLANTUML_JAR_PATH)
      if(PLANTUML_JAR_PATH)
        set(PLANTUML_JAR_PATH "${PLANTUML_JAR_PATH}/plantuml.jar")
        set(PLANTUML_ON ON)
      else()
        message(STATUS "Warning: no plantuml.jar found.")
      endif()
    endif(JAVA_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
endif(DOXYGEN_FOUND)

option(plantuml "Plantuml support" ${PLANTUML_ON})
add_feature_info(plantuml plantuml  "PlantUML for documentation" URL http://de.plantuml.com/ TYPE OPTIONAL)
##### END Documentation


SUBDIRS (test)
SUBDIRS (tools)
SUBDIRS (pattern)


## Installation
configure_file("${DEV_DIR}/scil.pc.in" "scil.pc" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/scil.pc"  DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
install(TARGETS scil LIBRARY DESTINATION lib)
install(FILES scil.h DESTINATION include)

##
feature_summary(WHAT ALL)
