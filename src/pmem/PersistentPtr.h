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

    typedef T object_type;

public: // methods

    /// Constructor

    PersistentPtr();

    /// For utility, when building derived types

    template <typename S> PersistentPtr<S> forced_cast() const;

    /// Access the stored object

    object_type& operator*() const;

    object_type* operator->() const;

    object_type* get() const;

    // bool null() const; // Inherited

    bool valid() const;

    uint64_t uuid() const;

    /// For the implementation of simple polymorphism. Will required the PersistentTypes to be
    /// modified to support interconversion
    template <typename S>
    PersistentPtr<S> as() const;

    /// Modification of pointers

    // void nullify(); // Inherited

    /// @note Allocation and setting of the pointer are atomic. The work of setting up the
    /// object is done inside the functor atomic_constructor<T>.
    void allocate_ctr(PMEMobjpool * pool, const AtomicConstructor<object_type>& constructor);
    void allocate_ctr(PersistentPool& pool, const AtomicConstructor<object_type>& constructor);

    /// We should be able to allocate directly on an object. If we don't specify the pool, then
    /// it will put the data into the same pool as the pointer is in
    void allocate_ctr(const AtomicConstructor<object_type>& constructor);

    /// Allocate functions using argument passthrough
    void allocate();
    template <typename X1> void allocate(const X1& x1);
    template <typename X1, typename X2> void allocate(const X1& x1, const X2& x2);
    template <typename X1, typename X2, typename X3> void allocate(const X1& x1, const X2& x2, const X3& x3);

    /// Atomically replace the existing object with a new one. If anything fails in the chain of
    /// construction, the original object is left unchanged.
    void replace_ctr(PMEMobjpool* pool, const AtomicConstructor<object_type>& constructor);
    void replace_ctr(PersistentPool& pool, const AtomicConstructor<object_type>& constructor);
    void replace_ctr(const AtomicConstructor<object_type>& constructor);

    /// Replace functions using argument passthrough
    void replace();
    template <typename X1> void replace(const X1& x1);
    template <typename X1, typename X2> void replace(const X1& x1, const X2& x2);
    template <typename X1, typename X2, typename X3> void replace(const X1& x1, const X2& x2, const X3& x3);

protected: // methods

    void print(std::ostream&) const;

private: // methods

    /// Don't support user-manipulation of the oid directly, but we need to have a way internally.
    PersistentPtr(PMEMoid oid);

private: // friends

    friend bool operator== <> (const PersistentPtr& lhs, const PersistentPtr& rhs);
    friend bool operator!= <> (const PersistentPtr& lhs, const PersistentPtr& rhs);

    friend std::ostream& operator<< <> (std::ostream&, const PersistentPtr&);

    friend class PersistentPool;

    template <typename S> friend class PersistentPtr;
};


//----------------------------------------------------------------------------------------------------------------------

// Templated member functions

template <typename T>
PersistentPtr<T>::PersistentPtr() :
    PersistentPtrBase() {}


template <typename T>
PersistentPtr<T>::PersistentPtr(PMEMoid oid) :
    PersistentPtrBase(oid) {}

template <typename T>
template <typename S>
PersistentPtr<S> PersistentPtr<T>::forced_cast() const {
    return PersistentPtr<S>(oid_);
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
    ASSERT(valid());
    return (object_type*)::pmemobj_direct(oid_);
}


template <typename T>
bool PersistentPtr<T>::valid() const {
    return PersistentType<object_type>::validate_type_id(::pmemobj_type_num(oid_));
}


template <typename T>
uint64_t PersistentPtr<T>::uuid() const {
    return oid_.pool_uuid_lo;
}

/// Convert a PersistentPtr to another type of PersistentPtr. This will almost always
/// fail at runtime on the type check, unless the PersistentType has been overriden
/// to permit this.
template <typename T>
template <typename S>
PersistentPtr<S> PersistentPtr<T>::as() const {
    if (!PersistentType<object_type>::validate_type_id(::pmemobj_type_num(oid_)))
        throw eckit::SeriousBug("Attempting to interconvert between incompatible PersistentPtr types", Here());
    return PersistentPtr<S>(oid_);
}


template <typename T>
void PersistentPtr<T>::allocate_ctr(PMEMobjpool * pool, const AtomicConstructor<object_type>& constructor) {

    ASSERT(null());

    // We don't want to assert(null()). We may be updating, say, pointers in a chain of
    // objects, with atomic rearrangement. That is fine.
    /// @note ::pmemobj_alloc does not modify the contents of the final argument, which is passed through
    ///       untouched. It is only non-const to permit workflows that we don't use.
    if (::pmemobj_alloc(pool,
                        &oid_,
                        constructor.size(),
                        constructor.type_id(),
                        &pmem_constructor,
                        const_cast<void*>(reinterpret_cast<const void*>(&constructor))) != 0) {
        throw AtomicConstructorBase::AllocationError("Persistent allocation failed");
    }
}


template <typename T>
void PersistentPtr<T>::allocate_ctr(PersistentPool& pool, const AtomicConstructor<object_type>& constructor) {
    allocate_ctr(pool.raw_pool(), constructor);
}


template <typename T>
void PersistentPtr<T>::allocate_ctr(const AtomicConstructor<object_type>& constructor) {

    // For allocating directly on an existing persistent object, we don't have to specify the pool manually. Get the
    // pool by using the same one as the current persistent object.
    PMEMobjpool * pool = ::pmemobj_pool_by_ptr(this);

    if (pool == 0)
        throw eckit::SeriousBug("Allocating persistent memory to non-persistent pointer", Here());

    allocate_ctr(pool, constructor);
}


template <typename T>
void PersistentPtr<T>::allocate() {

    AtomicConstructor0<T> ctr;
    allocate_ctr(ctr);
}


template <typename T>
template <typename X1>
void PersistentPtr<T>::allocate(const X1& x1) {

    AtomicConstructor1<T, X1> ctr(x1);
    allocate_ctr(ctr);
}


template <typename T>
template <typename X1, typename X2>
void PersistentPtr<T>::allocate(const X1& x1, const X2& x2) {

    AtomicConstructor2<T, X1, X2> ctr(x1, x2);
    allocate_ctr(ctr);
}


template <typename T>
template <typename X1, typename X2, typename X3>
void PersistentPtr<T>::allocate(const X1& x1, const X2& x2, const X3& x3) {

    AtomicConstructor3<T, X1, X2, X3> ctr(x1, x2, x3);
    allocate_ctr(ctr);
}

template <typename T>
void PersistentPtr<T>::replace_ctr(PMEMobjpool* pool, const AtomicConstructor<object_type> &constructor) {

    ASSERT(!null());
    PMEMoid oid_tmp = oid_;

    if (::pmemobj_alloc(pool,
                        &oid_,
                        constructor.size(),
                        constructor.type_id(),
                        &pmem_constructor,
                        const_cast<void*>(reinterpret_cast<const void*>(&constructor))) != 0) {
        ASSERT(OID_EQUALS(oid_, oid_tmp));
        throw AtomicConstructorBase::AllocationError("Persistent allocation failed");
    } else {
        ::pmemobj_free(&oid_tmp);
    }
}


template <typename T>
void PersistentPtr<T>::replace_ctr(PersistentPool& pool, const AtomicConstructor<object_type> &constructor) {
    replace_ctr(pool.raw_pool(), constructor);
}


template <typename T>
void PersistentPtr<T>::replace_ctr(const AtomicConstructor<object_type> &constructor) {

    PMEMobjpool* pool = ::pmemobj_pool_by_ptr(this);

    if (pool == 0)
        throw eckit::SeriousBug("Replacing persistent memory alloction in non-persistent memory", Here());

    replace_ctr(pool, constructor);
}


template <typename T>
void PersistentPtr<T>::replace() {

    AtomicConstructor0<T> ctr;
    replace_ctr(ctr);
}


template <typename T>
template <typename X1>
void PersistentPtr<T>::replace(const X1& x1) {

    AtomicConstructor1<T, X1> ctr(x1);
    replace_ctr(ctr);
}

template <typename T>
template <typename X1, typename X2>
void PersistentPtr<T>::replace(const X1& x1, const X2& x2) {

    AtomicConstructor2<T, X1, X2> ctr(x1, x2);
    replace_ctr(ctr);
}


template <typename T>
template <typename X1, typename X2, typename X3>
void PersistentPtr<T>::replace(const X1& x1, const X2& x2, const X3& x3) {

    AtomicConstructor3<T, X1, X2, X3> ctr(x1, x2, x3);
    replace_ctr(ctr);
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
