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


#include "tree/PersistentBuffer.h"


#include "eckit/io/Buffer.h"


using namespace eckit;


namespace treetool {

// -------------------------------------------------------------------------------------------------


PersistentBuffer::Constructor::Constructor(const void* data, size_t length) :
    data_(data),
    length_(length) {}


size_t PersistentBuffer::Constructor::size() const {
    return sizeof(PersistentBuffer) - sizeof(char) + length_;
}


void PersistentBuffer::Constructor::make(PersistentBuffer* object) const {

    object->length_ = length_;

    ::memcpy(object->data_, data_, length_);
}


// -------------------------------------------------------------------------------------------------


size_t PersistentBuffer::size() const {
    return length_;
}


const void * PersistentBuffer::data () const {
    return data_;
}

// -------------------------------------------------------------------------------------------------

} // namespace treetool
