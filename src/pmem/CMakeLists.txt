ecbuild_add_library(

    TARGET pmem

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
        ${PMEMIO_INCLUDE_DIRS}

    LIBS
        eckit
        ${PMEMIO_OBJ_LIBRARY} )
