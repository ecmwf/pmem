### config headers

ecbuild_generate_config_headers( DESTINATION ${INSTALL_INCLUDE_DIR}/eckit )

configure_file( pmem_config.h.in   pmem_config.h  )
configure_file( pmem_version.h.in  pmem_version.h )

install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/pmem_config.h
            ${CMAKE_CURRENT_BINARY_DIR}/pmem_version.h
        DESTINATION
            ${INSTALL_INCLUDE_DIR}/eckit )

configure_file( pmem_version.cc.in  pmem_version.cc )

### pmem sources

ecbuild_add_library(

    TARGET pmempp

    SOURCES
        pmem_version.cc
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

    GENERATED
        pmem_version.cc

    PRIVATE_INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMIO_INCLUDE_DIRS}

    LIBS
        eckit
        ${PMEMIO_OBJ_LIBRARY} )

add_subdirectory( tree )
