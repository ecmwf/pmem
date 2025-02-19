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

#include <cstring>

#include "eckit/io/Buffer.h"

#include "pmem/PersistentBuffer.h"


using namespace eckit;


namespace pmem {

//----------------------------------------------------------------------------------------------------------------------


PersistentBuffer::PersistentBuffer(const void *data, size_t length)
    : length_(length) {

    if (length != 0 && data != 0)
        ::memcpy(data_, data, length);
}


size_t PersistentBuffer::data_size(size_t length) {
    return sizeof(PersistentBuffer) + length;
}


size_t PersistentBuffer::size() const {
    return length_;
}


const void * PersistentBuffer::data () const {
    return data_;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace tree
