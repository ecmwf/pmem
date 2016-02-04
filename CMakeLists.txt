############################################################################################
#
#   Hermes - ECMWF scalable data pre- and post-processing framework
#
############################################################################################

cmake_minimum_required( VERSION 2.8.4 FATAL_ERROR )

project( objserver C CXX )

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild/cmake")

include( ecbuild_system NO_POLICY_SCOPE )

ecbuild_requires_macro_version( 1.9 )

###############################################################################
# project

ecbuild_use_package( PROJECT eckit VERSION  0.10  REQUIRED )

ecbuild_add_option( FEATURE MULTIO
                    DESCRIPTION "Add an interface to pass GRIB files to the object store from MultIO"
                    REQUIRED_PACKAGES
                      "PROJECT multio  VERSION 0.1 REQUIRED"
                    )

# There might be a more magic way to do this. In the long run we should create a FindPMEM.cmake file.
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

###############################################################################
# some variables/options of this project

ecbuild_add_cxx11_flags()
ecbuild_check_cxx11( PRINT )

###############################################################################
# contents

### export package to other ecbuild packages

set( OBJSRV_INCLUDE_DIRS   ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src )
set( OBJSRV_LIBRARIES      ${CMAKE_PROJECT_NAME} )

get_directory_property( COMPILE_DEFINITIONS OBJSRV_DEFINITIONS )

### include directories

include_directories( ${OBJSRV_INCLUDE_DIRS} )

add_subdirectory( src )

### installation

# install( DIRECTORY share DESTINATION ${CMAKE_INSTALL_PREFIX} )

#ecbuild_add_resources( TARGET top_files SOURCES TODO AUTHORS README.md NOTICE LICENSE INSTALL ChangeLog COPYING )

############################################################################################
# packaging and installation

ecbuild_install_project( NAME ${CMAKE_PROJECT_NAME} )

ecbuild_print_summary()
