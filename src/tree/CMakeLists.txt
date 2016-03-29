ecbuild_add_library(

    TARGET pmem_tree

    SOURCES
        TreeRoot.h
        TreeRoot.cc
        TreePool.h
        TreePool.cc
        TreeNode.h
        TreeNode.cc
        TreeSchema.h
        TreeSchema.cc
        PersistentBuffer.h
        PersistentBuffer.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMOBJ_INCLUDE_DIRS}

    LIBS
        pmemwrapper
        eckit
        ${PMEMOBJ_LIBS} )


ecbuild_add_executable(

    TARGET treetool

    SOURCES
        TreeTool.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMOBJ_INCLUDE_DIRS}

    LIBS
        eckit
        pmem_tree
        eckit_option )
