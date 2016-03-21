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

#ifndef pmem_PersistentPtr_H
#define pmem_PersistentPtr_H

#include "libpmemobj.h"

#include "eckit/exception/Exceptions.h"

#include "pmem/AtomicConstructor.h"
#include "pmem/PersistentPool.h"

namespace pmem {

//----------------------------------------------------------------------------------------------------------------------


/// This is a static routine that can be passed to the atomic allocation routines. All the logic
/// should be passed in as the functor AtomicConstructor<T>.
int pmem_constructor(PMEMobjpool * pool, void * obj, void * arg);


// These forward declarations are just to make the templated friend class statement later happy
template <typename T> class PersistentPtr;
template <typename T> bool operator== (const PersistentPtr<T>& lhs, const PersistentPtr<T>& rhs);


//----------------------------------------------------------------------------------------------------------------------


/// Include the non-templated functionality of PersistentPtr, so that it is only compiled in one place!

class PersistentPtrBase {

public: // methods

    PersistentPtrBase();

    /// Obtain a persistent pointer given an actual pointer to an object in virtual memory.
    PersistentPtrBase(void* object);

    /// Deallocate the memory. Note that this atomically frees and sets oid_ == OID_NULL.
    void free();

    /// Is the pointer null?
    bool null() const;

    /// Nullify the persistent pointer
    void nullify();

protected: // methods

    /// Don't support user-manipulation of the oid directly, but we need to have a way internally.
    PersistentPtrBase(PMEMoid oid);

protected: // members

    /*
     * N.B. There is only ONE data member here. This is a PMEMoid, and is the SAME data that
     *      is held in the TOID() macro-enhanced data type in the C-bindings. (In C this is
     *      actually a named union, with the only data being a PMEMoid). This means that PMEMoid,
     *      TOID and PersistentPtr can be used interchangeably in the data format.
     *
     * N.B. 2 - Because we want to ensure the data stays the same, PersistentPtr MUST NOT be
     *          a virtual class.
     *
     * TODO: Can we use boost::is_polymorphic / std::is_polymorphic?
     */

    PMEMoid oid_;

};


//----------------------------------------------------------------------------------------------------------------------


template <typename T>
class PersistentPtr : public PersistentPtrBase {

public: // members

    /// This static is used to perform type id lookups.
    /// Needs to be defined GLOBALLY.

    static int type_id;

public: // methods

    /*
     * Constructors
     */
    PersistentPtr();

    /// Obtain a persistent pointer given an actual pointer to an object in virtual memory.
    PersistentPtr(void * object);

    /*
     * Access the stored object
     */

    T& operator*() const;

    T* operator->() const;

    T* get() const;

    /*
     * Status testing?
     */

    // bool null() const; // Inherited

    bool valid() const;

    /*
     * Modification
     */

    // void nullify(); // Inherited

    /// @note Allocation and setting of the pointer are atomic. The work of setting up the
    /// object is done inside the functor atomic_constructor<T>.
    void allocate(PMEMobjpool * pool, AtomicConstructor<T>& constructor);

    /// We should be able to allocate directly on an object. If we don't specify the pool, then
    /// it will put the data into the same pool as the pointer is in
    void allocate(AtomicConstructor<T>& constructor);


private: // methods

    /// Don't support user-manipulation of the oid directly, but we need to have a way internally.
    PersistentPtr(PMEMoid oid);

    friend bool operator== <> (const PersistentPtr& lhs, const PersistentPtr& rhs);

private: // friends

    friend class PersistentPool;
};


//----------------------------------------------------------------------------------------------------------------------

// Templated member functions

template <typename T>
PersistentPtr<T>::PersistentPtr() :
    PersistentPtrBase() {}


template <typename T>
PersistentPtr<T>::PersistentPtr(void* object) :
    PersistentPtrBase(object) {}


template <typename T>
PersistentPtr<T>::PersistentPtr(PMEMoid oid) :
    PersistentPtrBase(oid) {}


template <typename T>
T& PersistentPtr<T>::operator*() const {
    return *get();
}


template <typename T>
T* PersistentPtr<T>::operator->() const {
    return get();
}


template <typename T>
T* PersistentPtr<T>::get() const {
    ASSERT(valid());
    return (T*)::pmemobj_direct(oid_);
}


template <typename T>
bool PersistentPtr<T>::valid() const {
    return ::pmemobj_type_num(oid_) == type_id;
}


template <typename T>
void PersistentPtr<T>::allocate(PMEMobjpool * pool, AtomicConstructor<T>& constructor) {

    // We don't want to assert(null()). We may be updating, say, pointers in a chain of
    // objects, with atomic rearrangement. That is fine.
    if (::pmemobj_alloc(pool, &oid_, constructor.size(), type_id, &pmem_constructor, &constructor) != 0)
        throw AtomicConstructorBase::AllocationError("Persistent allocation failed");
}


template <typename T>
void PersistentPtr<T>::allocate(AtomicConstructor<T>& constructor) {

    // For allocating directly on an existing persistent object, we don't have to specify the pool manually. Get the
    // pool by using the same one as the current persistent object.
    PMEMobjpool * pool = ::pmemobj_pool_by_ptr(this);

    if (pool == 0)
        throw eckit::SeriousBug("Allocating persistent memory to non-persistent pointer", Here());

    allocate(pool, constructor);
}


template <typename T>
bool operator== (const PersistentPtr<T>& lhs, const PersistentPtr<T>& rhs) {
    return ((lhs.oid_.off == rhs.oid_.off) && (lhs.oid_.pool_uuid_lo == rhs.oid_.pool_uuid_lo));
}

//----------------------------------------------------------------------------------------------------------------------

}

#endif // pmem_PersistentPtr_H
