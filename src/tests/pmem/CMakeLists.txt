
set( _persistent_tests
    atomic_constructor
    persistent_buffer
    persistent_pod_vector
    persistent_pool
    persistent_ptr
    persistent_string
    persistent_type
    persistent_vector
)

foreach( _test ${_persistent_tests} )

    ecbuild_add_test(
        TARGET test_pmem_${_test}
        INCLUDES ${ECKIT_INCLUDE_DIRS}
        LIBS ${PMEM_LIBRARIES}
        SOURCES
            test_${_test}.cc
            test_persistent_helpers.h
    )

endforeach()
