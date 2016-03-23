list(APPEND COMMON_FILES
    TreeRoot.h
    TreeRoot.cc
    TreePool.h
    TreePool.cc
    TreeNode.h
    TreeNode.cc
    PersistentBuffer.h
    PersistentBuffer.cc
)


ecbuild_add_executable(

    TARGET treetool

    SOURCES
        TreeTool.cc
        ${COMMON_FILES}

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMOBJ_INCLUDE_DIRS}

    LIBS
        pmemwrapper
        eckit
        ${PMEMOBJ_LIBS}
        eckit_option )

# ecbuild_add_library(

#     TARGET multio_pmem

#     SOURCES
#         TreeMultIO.cc
#         TreeMultIO.h
#         ${COMMON_FILES}

#     PRIVATE_INCLUDES
#         ${MULTIO_INCLUDE_DIRS}
#         ${ECKIT_INCLUDE_DIRS}
#         ${PMEMOBJ_INCLUDE_DIRS}

#     LIBS
#         multio
#         pmemwrapper
#         eckit
#         ${PMEMOBJ_LIBS} )
