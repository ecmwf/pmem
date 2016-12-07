ecbuild_add_test( TARGET test_pmem_persistent_pool
                  BOOST
                  SOURCES test_persistent_pool.cc test_persistent_helpers.h
                  INCLUDES ${ECKIT_INCLUDE_DIRS}
                  LIBS ${PMEM_LIBRARIES} )

ecbuild_add_test( TARGET test_pmem_persistent_buffer
                  BOOST
                  SOURCES test_persistent_buffer.cc
                  INCLUDES ${ECKIT_INCLUDE_DIRS}
                  LIBS ${PMEM_LIBRARIES} )

ecbuild_add_test( TARGET test_pmem_persistent_ptr
                  BOOST
                  SOURCES test_persistent_ptr.cc
                  INCLUDES ${ECKIT_INCLUDE_DIRS}
                  LIBS ${PMEM_LIBRARIES} )

ecbuild_add_test( TARGET test_pmem_persistent_pod_vector
                  BOOST
                  SOURCES test_persistent_pod_vector.cc
                  INCLUDES ${ECKIT_INCLUDE_DIRS}
                  LIBS ${PMEM_LIBRARIES} )

ecbuild_add_test( TARGET test_pmem_persistent_vector
                  BOOST
                  SOURCES test_persistent_vector.cc
                  INCLUDES ${ECKIT_INCLUDE_DIRS}
                  LIBS ${PMEM_LIBRARIES} )

ecbuild_add_test( TARGET test_pmem_atomic_constructor
                  BOOST
                  SOURCES test_atomic_constructor.cc
                  INCLUDES ${ECKIT_INCLUDE_DIRS}
                  LIBS ${PMEM_LIBRARIES} )

ecbuild_add_test( TARGET test_pmem_persistent_string
                  BOOST
                  SOURCES test_persistent_string.cc
                  INCLUDES ${ECKIT_INCLUDE_DIRS}
                  LIBS ${PMEM_LIBRARIES} )

ecbuild_add_test( TARGET test_pmem_persistent_type
                  BOOST
                  SOURCES test_persistent_type.cc
                  INCLUDES ${ECKIT_INCLUDE_DIRS}
                  LIBS ${PMEM_LIBRARIES} )
