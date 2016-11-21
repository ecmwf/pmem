/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016


#ifndef pmem_PersistentBuffer_H
#define pmem_PersistentBuffer_H


#include "pmem/AtomicConstructor.h"
#include "pmem/PersistentType.h"


namespace eckit {
    class Buffer;
}


namespace pmem {

//----------------------------------------------------------------------------------------------------------------------

/*
 * We provide a base type, which gets simply wrapped. This allows the _derived_ types to be PersistentType<T>, so that
 * more than one concrete class can be built on top of the functionality of PersistentBufferBase. See (as a start)
 * both PersistentBuffer and PersistentString.
 */

class PersistentBufferBase {

public: // methods

    PersistentBufferBase(const void* data, size_t length);

    size_t size() const;

    const void* data() const;

    static size_t data_size(size_t length);

private: // members

    size_t length_;

    // Accessor to the data.
    // Data is VARIABLY SIZED following on from this location.
    char data_[0];
};


// ---------------------------------------------------------------------------------------------------------------------

class PersistentBuffer : public PersistentBufferBase
                       , public PersistentType<PersistentBuffer> {

public: // methods

    PersistentBuffer(const void* data, size_t length);
};


template<>
inline size_t AtomicConstructor2Base<PersistentBuffer, const void*, size_t>::size() const {
    return PersistentBufferBase::data_size(x2_);
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace tree

#endif // pmem_PersistentBuffer_H
