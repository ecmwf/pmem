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

/*
 * Here we provide an interface for accessing the ATOMIC functionality of
 * libpmemobj.h.
 *
 * Note that this does NOT provide any functionality for accessing the
 * transaction based api.
 */

//   i) Declare layout
//  ii) Define construction routines, with appropriate data accepted.
// iii) Store the oid directly inside the object, not outside
//  iv) Note that this should support c++97, not just c++11...

#ifndef objserver_persistent_ptr_H
#define objserver_persistent_ptr_H


#include "libpmemobj.h"

namespace pmem {

// -------------------------------------------------------------------------------------------------

// Usage notes:
//
// i) We must declare the types using POBJ_LAYOUT_BEGIN, POBJ_LAYOUT_ROOT, POBJ_LAYOUt_TOID, POBJ_LAYOUT_END


template <typename T>
class persistent_ptr {

private: // members

    PMEMoid oid_;
};


// -------------------------------------------------------------------------------------------------

}

#endif // objserver_persistent_ptr_H
