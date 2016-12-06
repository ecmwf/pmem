ecbuild_add_library(

    TARGET pmempp

    SOURCES
        AtomicConstructor.h
        AtomicConstructorCast.h
        Exceptions.cc
        Exceptions.h
        PersistentBuffer.cc
        PersistentBuffer.h
        PersistentMutex.h
        PersistentPODVector.h
        PersistentPool.cc
        PersistentPool.h
        PersistentPtr.cc
        PersistentPtr.h
        PersistentString.cc
        PersistentString.h
        PersistentType.h
        PersistentVector.h
        PersistentType.h


    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMIO_INCLUDE_DIRS}

    LIBS
        eckit
        ${PMEMIO_OBJ_LIBRARY} )
