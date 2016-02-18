ecbuild_add_executable(

    TARGET treetool

    SOURCES
        TreeTool.cc
        TreeRoot.h
        TreeRoot.cc
        TreePool.h
        TreePool.cc
        TreeNode.h
        TreeBinaryNode.h
        TreeBinaryNode.cc
        TreeLeafNode.h
        TreeLeafNode.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMOBJ_INCLUDE_DIRS}

    LIBS
        pmemwrapper
        eckit
        ${PMEMOBJ_LIBS} )

