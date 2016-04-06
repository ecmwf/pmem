ecbuild_add_test( TARGET test_pmem_persistent_pool
                  BOOST
                  SOURCES test_persistent_pool.cc test_persistent_helpers.h
                  INCLUDES ${ECKIT_INCLUDE_DIRS}
                  LIBS pmem )

ecbuild_add_test( TARGET test_pmem_persistent_buffer
                  BOOST
                  SOURCES test_persistent_buffer.cc
                  INCLUDES ${ECKIT_INCLUDE_DIRS}
                  LIBS pmem )
