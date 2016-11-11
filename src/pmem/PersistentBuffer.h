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

public: // Construction objects

    class ConstructorBase {

    public: // methods

        ConstructorBase(const void* data, size_t length);

        virtual void make (PersistentBufferBase& object) const;

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
};


// ---------------------------------------------------------------------------------------------------------------------

class PersistentBuffer : public PersistentBufferBase
                       , public PersistentType<PersistentBuffer> {

public: // Construction objects

    struct Constructor : public AtomicConstructor<PersistentBuffer>
                       , public ConstructorBase {

        Constructor(const void* data, size_t length);
        virtual void make(PersistentBuffer& object) const;
        virtual size_t size() const;
    };

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace tree

#endif // pmem_PersistentBuffer_H
