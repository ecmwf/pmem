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


PersistentBuffer::PersistentBuffer(const void * data, size_t length) :
    PersistentBufferBase(data, length) {}


PersistentBufferBase::PersistentBufferBase(const void *data, size_t length)
    : length_(length) {

    eckit::Log::error() << "Storing" << length << ", " << data << std::endl;
    if (length != 0 && data != 0) {
        eckit::Log::error() << "Storing ..." << std::endl;
        ::memcpy(data_, data, length);
    }
}


size_t PersistentBufferBase::data_size(size_t length) {
    return sizeof(PersistentBufferBase) + length;
}


size_t PersistentBufferBase::size() const {
    return length_;
}


const void * PersistentBufferBase::data () const {
    return data_;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace tree
