cmake_minimum_required (VERSION 2.8)

# project name
set (PROJECT_NAME scil)
project (${PROJECT_NAME})

MESSAGE( STATUS "Preparing makefiles for SCIL")

# version numbers
set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)

set(DEV_DIR "${CMAKE_SOURCE_DIR}/../dev")
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/../deps")
set(DEPS_COMPILED_DIR "${CMAKE_BINARY_DIR}/deps")

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake ${DEV_DIR}/CMakeModules)

find_package(PkgConfig REQUIRED)

include(CTest)
include(FeatureSummary)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)

option(CODE_COVERAGE "CODE_COVERAGE" OFF)


EXEC_PROGRAM(git ARGS rev-parse HEAD OUTPUT_VARIABLE GIT_VERSION)
# set(CGIT_VERSION "${GIT_VERSION}" CACHE STRING "GIT REVISION" FORCE)

IF (CODE_COVERAGE)
IF ( CMAKE_BUILD_TYPE STREQUAL "Debug")
	IF ( CMAKE_COMPILER_IS_GNUCC )
	    include(CodeCoverage)
	    setup_target_for_coverage(coverage ctest coverage)
	    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
	    set(GCOV_LIBRARIES "-lgcov")
        else()
		MESSAGE (FATAL_ERROR "Error, you need to compile with gcc")
        endif()
else()
	MESSAGE (FATAL_ERROR "Error, you need to compile with Debug mode")
endif()
endif()

set(COMPILER_OPTIONS "-pipe -Wall -Wno-unused-parameter -Wextra -Wfloat-equal -fdata-sections -ffunction-sections -Wl,--gc-sections  -Wl,--version-script=${CMAKE_SOURCE_DIR}/symbols.map")

# compile flags, striping symbols that are not with scil prefix to prevent collisions
set (CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=gnu99 ${COMPILER_OPTIONS}")
set (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${COMPILER_OPTIONS}")
# -Wl,--no-allow-shlib-undefined  does not work with Centos7 / older GCC

if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.6")
else()
	set (NEW_CFLAGS "-Wdouble-promotion -flto -pedantic-errors -pedantic")  #-fdiagnostics-color=always
	set (CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${NEW_CFLAGS}")
	set (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden ${NEW_CFLAGS}")
endif()


set (CMAKE_C_FLAGS_DEBUG   "-O0 -g3 -fvar-tracking -DDEBUG")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")

set (CMAKE_C_FLAGS_RELEASE "-O3")

find_path(LIBZ_INCLUDE_DIRS NAMES zlib.h PATHS ENV ${ADDITIONAL_INC_PATH})
find_library(LIBZ_LIBRARIES NAMES z PATHS ENV ${ADDITIONAL_LIB_PATH})

find_path(LIBGSL_INCLUDE_DIRS NAMES gsl/gsl_errno.h PATHS ENV ${ADDITIONAL_INC_PATH})
find_library(LIBGSL_LIBRARIES NAMES gsl PATHS ENV ${ADDITIONAL_LIB_PATH})

find_path(LIBNETCDF_INCLUDE_DIRS NAMES netcdf.h PATHS "/home/ana/work/install/netcdf/4.4.1.1/include")
find_library(LIBNETCDF_LIBRARIES NAMES netcdf PATHS "/home/ana/work/install/netcdf/4.4.1.1/lib")

#set (LIBNETCDF_INCLUDE_DIRS "/home/ana/work/install/netcdf/4.4.1.1/include")
#set (LIBNETCDF_LIBRARIES "/home/ana/work/install/netcdf/4.4.1.1/lib/netcdf.so")
##### ENDIANESS

set(ENDIANESS "undefined" CACHE STRING "Endianess")

CHECK_C_SOURCE_RUNS("
  #include <assert.h>
  int main(){
    int v = 1;
    char *p = (char*) & v;
    assert(*p == 1);
  }  " LITTLE_ENDIAN)

if( ${LITTLE_ENDIAN} )
  set (CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -D SCIL_LITTLE_ENDIAN")
  set (ENDIANESS "LITTLE" CACHE STRING "Endianess" FORCE)
endif()


CHECK_C_SOURCE_RUNS("
  #include <assert.h>
  int main(){
    int v = 1;
    char *p = (char*) & v;
    assert(*(p + sizeof(int) - 1) == 1);
  }  " BIG_ENDIAN)

if( ${BIG_ENDIAN} )
  set (CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -D SCIL_BIG_ENDIAN")
  set (ENDIANESS "BIG" CACHE STRING "Endianess" FORCE)
endif()

message(STATUS "Endianess: ${ENDIANESS}")

#########################

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
)


# run during configure:
execute_process(COMMAND ${DEPS_DIR}/build-dependencies.sh) # WORKING_DIRECTORY ${DEPS_DIR}
execute_process(COMMAND ${DEV_DIR}/create-datatype-variants.py ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/)

add_custom_target(
    trigger_datatype_variants
    ALL COMMAND ${DEV_DIR}/create-datatype-variants.py ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/)

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


SUBDIRS (base)
SUBDIRS (util)
SUBDIRS (pattern)

SUBDIRS (compression)
SUBDIRS (tools)
SUBDIRS (test)

## Installation
configure_file("${DEV_DIR}/scil.pc.in" "scil.pc" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/scil.pc"  DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")

##
feature_summary(WHAT ALL)
