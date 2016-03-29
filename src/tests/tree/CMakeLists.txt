ecbuild_add_test( TARGET tree_test_schema
                  BOOST
                  SOURCES test_schema.cc
                  INCLUDES ${ECKIT_INCLUDE_DIRS}
                  LIBS pmem_tree )
