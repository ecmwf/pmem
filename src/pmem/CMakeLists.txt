ecbuild_add_library(

    TARGET pmemwrapper

    SOURCES
        PersistentPtr.h
        PersistentPtr.cc
        PersistentPool.h
        PersistentPool.cc
        Exceptions.h
        Exceptions.cc
        AtomicConstructor.h
        PersistentVector.h


    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMOBJ_INCLUDE_DIRS}

    LIBS
        eckit
        ${PMEMOBJ_LIBS} )
