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

#ifndef objserver_layout_dlist_H
#define objserver_layout_dlist_H


#include "eckit/types/FixedString.h"


#include "libpmemobj.h"
#include "objserver/persistent_ptr.h"
#include "objserver/persistent_list.h"


struct buffer_type {
    size_t len;
    char buf[sizeof(size_t)];
};


typedef persistent_list<buffer_type>::persistent_list_root buffer_list_root;
typedef persistent_list<buffer_type>::persistent_list_elem buffer_list_elem;

POBJ_LAYOUT_BEGIN(dlist_layout);
    POBJ_CPP_LAYOUT_ROOT(dlist_layout, buffer_list_root);
    POBJ_CPP_LAYOUT_TOID(dlist_layout, buffer_list_elem);
POBJ_LAYOUT_END(dlist_layout);



const eckit::FixedString<8> root_tag = "DLIST666";


#endif // objserver_layout_dlist_H

