ecbuild_add_executable(

    TARGET objserver

    SOURCES
        objserver.cc
        layout.h
        persistent_ptr.h


    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMOBJ_INCLUDE_DIRS}

    LIBS
        eckit
        ${PMEMOBJ_LIBS} )


ecbuild_add_executable(

    TARGET dlist

    SOURCES
        dlist.cc
        layout_dlist.h
        persistent_ptr.h


    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${PMEMOBJ_INCLUDE_DIRS}

    LIBS
        eckit
        ${PMEMOBJ_LIBS} )

