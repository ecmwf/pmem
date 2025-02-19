/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/*
 * This software was developed as part of the EC H2020 funded project NextGenIO
 * (Project ID: 671951) www.nextgenio.eu
 */

/// @author Simon Smart
/// @date   Feb 2016

#ifndef pmem_PersistentType_H
#define pmem_PersistentType_H

#include <stdint.h>

namespace pmem {

//----------------------------------------------------------------------------------------------------------------------

template <typename T>
class PersistentType {

public: // types

    typedef T object_type;

    static uint64_t type_id;

    static bool validate_type_id(uint64_t id);
};

//----------------------------------------------------------------------------------------------------------------------

template <typename T>
bool PersistentType<T>::validate_type_id(uint64_t id) {

    return type_id == id;
}

//----------------------------------------------------------------------------------------------------------------------

}

#endif // pmem_PersistentType_H
