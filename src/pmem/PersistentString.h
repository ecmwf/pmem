/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
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


#ifndef pmem_PersistentString_H
#define pmem_PersistentString_H


#include "pmem/AtomicConstructor.h"

#include "pmem/PersistentBuffer.h"

#include <iosfwd>
#include <string>


namespace pmem {

//----------------------------------------------------------------------------------------------------------------------

/// @note We store the null character, so that data() and c_str() can be implemented O(1) according to the std.

class PersistentString : private PersistentBuffer {

public: // methods

    PersistentString(const std::string& str);

    size_t size() const;
    size_t length() const;

    // matching the std::string implementation, these return null-terminated strings.
    const char* c_str() const;
    const char* data() const;

    bool operator==(const PersistentString& rhs) const;
    bool operator==(const std::string& rhs) const;

    bool operator!=(const PersistentString& rhs) const;
    bool operator!=(const std::string& rhs) const;

    char operator[](size_t i) const;

private: // friends

    friend bool operator==(const std::string&, const PersistentString&);
    friend bool operator!=(const std::string&, const PersistentString&);
    friend std::ostream& operator<< (std::ostream&, const PersistentString&);
};

//----------------------------------------------------------------------------------------------------------------------

template<>
inline size_t AtomicConstructor1Base<PersistentString, std::string>::size() const {
    return PersistentBuffer::data_size(x1_.size() + 1);
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace tree

#endif // pmem_PersistentBuffer_H
