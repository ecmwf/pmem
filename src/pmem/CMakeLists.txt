ecbuild_add_library(

    TARGET pmem

    SOURCES
        AtomicConstructor.h
        Exceptions.cc
        Exceptions.h
        PersistentBuffer.cc
        PersistentBuffer.h
        PersistentPool.cc
        PersistentPool.h
        PersistentPtr.cc
        PersistentPtr.h
        PersistentVector.h
        PersistentMutex.h


    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMIO_INCLUDE_DIRS}

    LIBS
        eckit
        ${PMEMIO_OBJ_LIBRARY} )
