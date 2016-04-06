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


namespace eckit {
    class Buffer;
}


namespace pmem {

//----------------------------------------------------------------------------------------------------------------------

class PersistentBuffer {

public: // Construction objects

    class Constructor : public AtomicConstructor<PersistentBuffer> {

    public: // methods

        Constructor(const void* data, size_t length);

        virtual void make (PersistentBuffer& object) const;

        virtual size_t size() const;

    private: // members

        const void* data_;
        size_t length_;
    };


public: // methods

    size_t size() const;

    const void* data() const;

private: // members

    size_t length_;

    // Accessor to the data.
    // Data is VARIABLY SIZED following on from this location.
    char data_[0];

private: // friends

    friend class PersistentBuffer::Constructor;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace tree

#endif // pmem_PersistentBuffer_H
