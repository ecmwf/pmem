ecbuild_add_test( TARGET test_pmem_persistent_pool
                  BOOST
                  SOURCES test_persistent_pool.cc
                  INCLUDES ${ECKIT_INCLUDE_DIRS}
                  LIBS pmem )
