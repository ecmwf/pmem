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


#include "eckit/io/Buffer.h"

#include "pmem/PersistentBuffer.h"


using namespace eckit;


namespace pmem {

//----------------------------------------------------------------------------------------------------------------------


PersistentBufferBase::ConstructorBase::ConstructorBase(const void* data, size_t length) :
    data_(data),
    length_(length) {}


size_t PersistentBufferBase::ConstructorBase::size() const {
    return sizeof(PersistentBufferBase) + length_;
}


void PersistentBufferBase::ConstructorBase::make(PersistentBufferBase& object) const {

    object.length_ = length_;

    if (length_ != 0 && data_ != 0)
        ::memcpy(object.data_, data_, length_);
}


//----------------------------------------------------------------------------------------------------------------------

PersistentBuffer::Constructor::Constructor(const void* data, size_t length) :
    ConstructorBase(data, length) {}


void PersistentBuffer::Constructor::make(PersistentBuffer& object) const {
    ConstructorBase::make(object);
}

size_t PersistentBuffer::Constructor::size() const {
    return ConstructorBase::size();
}

//----------------------------------------------------------------------------------------------------------------------


size_t PersistentBufferBase::size() const {
    return length_;
}


const void * PersistentBufferBase::data () const {
    return data_;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace tree
