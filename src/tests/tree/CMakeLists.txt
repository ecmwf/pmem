ecbuild_add_test( TARGET test_tree_schema
                  SOURCES test_schema.cc
                  INCLUDES ${ECKIT_INCLUDE_DIRS}
                  LIBS pmem_tree )

ecbuild_add_test( TARGET test_tree_node
                  SOURCES test_tree_node.cc
                  INCLUDES ${ECKIT_INCLUDE_DIRS}
                  LIBS pmem_tree )
