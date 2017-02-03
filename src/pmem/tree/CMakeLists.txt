ecbuild_add_library(

    TARGET pmem_tree

    SOURCES
        TreeNode.cc
        TreeNode.h
        TreePool.cc
        TreePool.h
        TreeRoot.cc
        TreeRoot.h
        TreeSchema.cc
        TreeSchema.h

    PRIVATE_INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMIO_INCLUDE_DIRS}

    LIBS
        pmempp
        eckit )


ecbuild_add_executable(

    TARGET treetool

    SOURCES
        TreeTool.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMIO_INCLUDE_DIRS}

    LIBS
        eckit
        pmem_tree
        eckit_option )
