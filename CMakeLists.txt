cmake_minimum_required( VERSION 2.8.11 FATAL_ERROR )

project( pmem C CXX )

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild/cmake")

include( ecbuild_system NO_POLICY_SCOPE )

ecbuild_requires_macro_version( 2.0 )



ecbuild_use_package( PROJECT eckit VERSION  0.10  REQUIRED )

find_path(
    PMEMOBJ_INCLUDE_DIRS
    libpmemobj.h
    PATHS ${PMEMOBJ_PATH}
    PATH_SUFFIXES include
)

find_library(
    PMEMOBJ_LIBS
    pmemobj
    PATHS ${PMEMOBJ_PATH}
    PATH_SUFFIXES debug nondebug
)


ecbuild_declare_project()

### export package to other ecbuild packages

set( PMEM_INCLUDE_DIRS   ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src )
set( PMEM_LIBRARIES      pmemwrapper )

get_directory_property( PMEM_DEFINITIONS COMPILE_DEFINITIONS )

### include directories

include_directories( ${PMEM_INCLUDE_DIRS} )

add_subdirectory( src )

### finalize project

ecbuild_add_resources( TARGET top_files SOURCES README.md)
#ecbuild_add_resources( TARGET top_files SOURCES TODO AUTHORS README.md NOTICE LICENSE INSTALL ChangeLog COPYING )

ecbuild_pkgconfig(NAME pmem
                  DESCRIPTION "Use the persistent memory libs"
                  LIBRARIES multio_pmem pmemwrapper)

ecbuild_install_project( NAME ${PROJECT_NAME} )

ecbuild_print_summary()
