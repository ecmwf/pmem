ecbuild_add_executable(

    TARGET objserver

    SOURCES
        objserver.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMOBJ_INCLUDE_DIRS}

    LIBS
        eckit
        ${PMEMOBJ_LIBS} )


