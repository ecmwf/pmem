/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016

#ifndef objserver_layout_H
#define objserver_layout_H


#include "libpmemobj.h"
#include "objserver/persistent_ptr.h"

namespace objserver {

// -------------------------------------------------------------------------------------------------

/*
 * Define the layout
 */

class root_obj_atomic;
class list_obj_atomic;

POBJ_LAYOUT_BEGIN(ll_atomic);
POBJ_LAYOUT_ROOT(ll_atomic, root_obj_atomic);
POBJ_LAYOUT_TOID(ll_atomic, list_obj_atomic);
POBJ_LAYOUT_END(ll_atomic);


/*
 * And define the structures
 */

struct root_obj_atomic {
    pmem::persistent_ptr<list_obj_atomic> next;
};


struct list_obj_atomic {
    size_t len;
    pmem::persistent_ptr<list_obj_atomic> next;

    // This element does not store any data directly. It only exists to provide an access
    // point to data of a variable size.
    char buf[sizeof(size_t)];
};


/*
 * Structures used for initialisation of the atomically created objects.
 */

struct list_obj_init_info {
    const void * data;
    size_t len;
};

// -------------------------------------------------------------------------------------------------

}

#endif // objserver_layout_H

