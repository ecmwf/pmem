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

#include "pmem/PersistentString.h"

#include <iostream>
#include <algorithm>


using namespace eckit;


namespace pmem {

//----------------------------------------------------------------------------------------------------------------------


// Construction is easy - we just pass through to the PersistentBuffer we are built from!

PersistentString::Constructor::Constructor(const std::string& value) :
    PersistentBuffer::Constructor(value.c_str(), value.size()+1) {}


size_t PersistentString::Constructor::size() const {
    return PersistentBuffer::Constructor::size();
}


void PersistentString::Constructor::make(PersistentString& object) const {
    PersistentBuffer::Constructor::make(object);

    ASSERT(object.data()[object.size()] == '\0');
}


//----------------------------------------------------------------------------------------------------------------------


size_t PersistentString::size() const {
    return PersistentBuffer::size()-1;
}

size_t PersistentString::length() const {
    return size();
}

const char* PersistentString::c_str() const {
    return static_cast<const char*>(PersistentBuffer::data());
}

const char * PersistentString::data () const {
    return static_cast<const char*>(PersistentBuffer::data());
}


char PersistentString::operator[] (size_t i) const {

    ASSERT(i < size());
    return c_str()[i];
}

bool PersistentString::operator==(const PersistentString& rhs) const {
    return ::strncmp(c_str(), rhs.c_str(), std::min(size(), rhs.size())) == 0;
}

bool PersistentString::operator==(const std::string& rhs) const {
    return c_str() == rhs;
}

bool operator==(const std::string& lhs, const PersistentString& rhs) {
    return lhs == rhs.c_str();
}

std::ostream& operator<< (std::ostream& os, const PersistentString& s) {
    os << s.c_str();
    return os;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace tree
