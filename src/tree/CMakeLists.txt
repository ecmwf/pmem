ecbuild_add_executable(

    TARGET treetool

    SOURCES
        TreeTool.cc
        TreeRoot.h
        TreeRoot.cc
        TreePool.h
        TreePool.cc
        TreeNode.h
        TreeNode.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMOBJ_INCLUDE_DIRS}

    LIBS
        pmemwrapper
        eckit
        ${PMEMOBJ_LIBS} )

