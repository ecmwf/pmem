ecbuild_add_library(

    TARGET pmemwrapper

    SOURCES
        PersistentPtr.h
        PersistentPtr.cc


    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMOBJ_INCLUDE_DIRS}

    LIBS
        eckit
        ${PMEMOBJ_LIBS} )
