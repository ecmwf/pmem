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


PersistentBuffer::Constructor::Constructor(const Buffer& buffer) :
    buffer_(buffer) {}


size_t PersistentBuffer::Constructor::size() const {
    return sizeof(PersistentBuffer) - sizeof(char) + buffer_.size();
}


void PersistentBuffer::Constructor::make(PersistentBuffer* object) const {

    object->length_ = buffer_.size();

    ::memcpy(object->data_, buffer_, buffer_.size());
}


// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------

} // namespace treetool
