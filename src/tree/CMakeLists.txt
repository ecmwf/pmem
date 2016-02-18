ecbuild_add_executable(

    TARGET treetool

    SOURCES
        TreeTool.cc
        Layout.h
        Layout.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMOBJ_INCLUDE_DIRS}

    LIBS
        pmemwrapper
        eckit
        ${PMEMOBJ_LIBS} )

