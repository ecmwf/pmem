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

#include <iosfwd>

#include "libpmemobj.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/exception/StaticAssert.h"
#include "eckit/types/IsBaseOf.h"
#include "eckit/types/IsSame.h"

#include "pmem/AtomicConstructor.h"
#include "pmem/PersistentPool.h"
#include "pmem/PersistentType.h"

namespace pmem {

//----------------------------------------------------------------------------------------------------------------------


/// This is a static routine that can be passed to the atomic allocation routines. All the logic
/// should be passed in as the functor AtomicConstructor<T>.
int pmem_constructor(PMEMobjpool * pool, void * obj, void * arg);


// These forward declarations are just to make the templated friend class statement later happy
template <typename T> class PersistentPtr;
template <typename T> bool operator== (const PersistentPtr<T>& lhs, const PersistentPtr<T>& rhs);
template <typename T> bool operator!= (const PersistentPtr<T>& lhs, const PersistentPtr<T>& rhs);
template <typename T> std::ostream& operator<< (std::ostream&, const PersistentPtr<T>&);


//----------------------------------------------------------------------------------------------------------------------


/// Include the non-templated functionality of PersistentPtr, so that it is only compiled in one place!

class PersistentPtrBase {

public: // methods

    PersistentPtrBase();

    /// Deallocate the memory. Note that this atomically frees and sets oid_ == OID_NULL.
    void free();

    /// Is the pointer null?
    bool null() const;

    /// Nullify the persistent pointer
    void nullify();

    /// Give everyone access to the raw oid if they really want
    PMEMoid raw() const;

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

public: // types

    typedef typename T::object_type object_type;

public: // methods

    /// Constructor

    PersistentPtr();

    /// Access the stored object

    object_type& operator*() const;

    object_type* operator->() const;

    object_type* get() const;

    // bool null() const; // Inherited

    bool valid() const;

    /// Modification of pointers

    // void nullify(); // Inherited

    /// @note Allocation and setting of the pointer are atomic. The work of setting up the
    /// object is done inside the functor atomic_constructor<T>.
    void allocate(PMEMobjpool * pool, const AtomicConstructor<object_type>& constructor);
    void allocate(PersistentPool& pool, const AtomicConstructor<object_type>& constructor);

    /// We should be able to allocate directly on an object. If we don't specify the pool, then
    /// it will put the data into the same pool as the pointer is in
    void allocate(const AtomicConstructor<object_type>& constructor);

    /// Allocate functions using argument passthrough
    void allocate();
    template <typename X1> void allocate(const X1& x1);
    template <typename X1, typename X2> void allocate(const X1& x1, const X2& x2);

    /// Atomically replace the existing object with a new one. If anything fails in the chain of
    /// construction, the original object is left unchanged.
    void replace(PMEMobjpool* pool, const AtomicConstructor<object_type>& constructor);
    void replace(PersistentPool& pool, const AtomicConstructor<object_type>& constructor);
    void replace(const AtomicConstructor<object_type>& constructor);

protected: // methods

    void print(std::ostream&) const;

private: // methods

    /// Don't support user-manipulation of the oid directly, but we need to have a way internally.
    PersistentPtr(PMEMoid oid);

    /// Check the validity of the types involved
    void assert_type_validity() const;

private: // friends

    friend bool operator== <> (const PersistentPtr& lhs, const PersistentPtr& rhs);
    friend bool operator!= <> (const PersistentPtr& lhs, const PersistentPtr& rhs);

    friend std::ostream& operator<< <> (std::ostream&, const PersistentPtr&);

    friend class PersistentPool;
};


//----------------------------------------------------------------------------------------------------------------------

// Templated member functions

template <typename T>
void PersistentPtr<T>::assert_type_validity() const {

    eckit::StaticAssert<eckit::IsBaseOf<PersistentType<T>, T>::value ||
                        eckit::IsSame<PersistentType<typename T::object_type>, T>::value>();
}

template <typename T>
PersistentPtr<T>::PersistentPtr() :
    PersistentPtrBase() {
    assert_type_validity();
}


template <typename T>
PersistentPtr<T>::PersistentPtr(PMEMoid oid) :
    PersistentPtrBase(oid) {
    assert_type_validity();
}


template <typename T>
typename PersistentPtr<T>::object_type& PersistentPtr<T>::operator*() const {
    return *get();
}


template <typename T>
typename PersistentPtr<T>::object_type* PersistentPtr<T>::operator->() const {
    return get();
}


template <typename T>
typename PersistentPtr<T>::object_type* PersistentPtr<T>::get() const {
    assert_type_validity();
    ASSERT(valid());
    return (typename PersistentPtr<T>::object_type*)::pmemobj_direct(oid_);
}


template <typename T>
bool PersistentPtr<T>::valid() const {
    return PersistentType<typename T::object_type>::validate_type_id(::pmemobj_type_num(oid_));
}


template <typename T>
void PersistentPtr<T>::allocate(PMEMobjpool * pool, const AtomicConstructor<object_type>& constructor) {

    assert_type_validity();
    ASSERT(null());

    // We don't want to assert(null()). We may be updating, say, pointers in a chain of
    // objects, with atomic rearrangement. That is fine.
    /// @note ::pmemobj_alloc does not modify the contents of the final argument, which is passed through
    ///       untouched. It is only non-const to permit workflows that we don't use.
    if (::pmemobj_alloc(pool,
                        &oid_,
                        constructor.size(),
                        T::type_id,
                        &pmem_constructor,
                        const_cast<void*>(reinterpret_cast<const void*>(&constructor))) != 0)
        throw AtomicConstructorBase::AllocationError("Persistent allocation failed");
}


template <typename T>
void PersistentPtr<T>::allocate(PersistentPool& pool, const AtomicConstructor<object_type>& constructor) {
    allocate(pool.raw_pool(), constructor);
}


template <typename T>
void PersistentPtr<T>::allocate(const AtomicConstructor<object_type>& constructor) {

    // For allocating directly on an existing persistent object, we don't have to specify the pool manually. Get the
    // pool by using the same one as the current persistent object.
    PMEMobjpool * pool = ::pmemobj_pool_by_ptr(this);

    if (pool == 0)
        throw eckit::SeriousBug("Allocating persistent memory to non-persistent pointer", Here());

    allocate(pool, constructor);
}


template <typename T>
void PersistentPtr<T>::allocate() {

    AtomicConstructor0<T> ctr;
    allocate(ctr);
}


template <typename T>
template <typename X1>
void PersistentPtr<T>::allocate(const X1& x1) {

    AtomicConstructor1<T, X1> ctr(x1);
    allocate(ctr);
}


template <typename T>
template <typename X1, typename X2>
void PersistentPtr<T>::allocate(const X1& x1, const X2& x2) {

    AtomicConstructor2<T, X1, X2> ctr(x1, x2);
    allocate(ctr);
}

template <typename T>
void PersistentPtr<T>::replace(PMEMobjpool* pool, const AtomicConstructor<object_type> &constructor) {

    assert_type_validity();
    ASSERT(!null());
    PMEMoid oid_tmp = oid_;

    if (::pmemobj_alloc(pool,
                        &oid_,
                        constructor.size(),
                        T::type_id,
                        &pmem_constructor,
                        const_cast<void*>(reinterpret_cast<const void*>(&constructor))) != 0) {
        ASSERT(OID_EQUALS(oid_, oid_tmp));
        throw AtomicConstructorBase::AllocationError("Persistent allocation failed");
    } else {
        ::pmemobj_free(&oid_tmp);
    }
}


template <typename T>
void PersistentPtr<T>::replace(PersistentPool& pool, const AtomicConstructor<object_type> &constructor) {
    replace(pool.raw_pool(), constructor);
}


template <typename T>
void PersistentPtr<T>::replace(const AtomicConstructor<object_type> &constructor) {

    PMEMobjpool* pool = ::pmemobj_pool_by_ptr(this);

    if (pool == 0)
        throw eckit::SeriousBug("Replacing persistent memory alloction in non-persistent memory", Here());

    replace(pool, constructor);
}


template <typename T>
void PersistentPtr<T>::print(std::ostream& os) const {
    os << "PersistentPtr(" << std::hex << oid_.pool_uuid_lo << ":" << oid_.off << ")";
}


template <typename T>
bool operator== (const PersistentPtr<T>& lhs, const PersistentPtr<T>& rhs) {
    return ((lhs.oid_.off == rhs.oid_.off) && (lhs.oid_.pool_uuid_lo == rhs.oid_.pool_uuid_lo));
}

template <typename T>
bool operator!= (const PersistentPtr<T>& lhs, const PersistentPtr<T>& rhs) {
    return ((lhs.oid_.off != rhs.oid_.off) || (lhs.oid_.pool_uuid_lo != rhs.oid_.pool_uuid_lo));
}

template <typename T>
std::ostream& operator<< (std::ostream& os, const PersistentPtr<T>& p) {
    p.print(os);
    return os;
}

//----------------------------------------------------------------------------------------------------------------------

}

#endif // pmem_PersistentPtr_H
