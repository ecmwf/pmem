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


#ifndef pmem_PersistentVector_H
#define pmem_PersistentVector_H

#include "eckit/log/Log.h"

#include "pmem/PersistentPtr.h"


/*
 * Modus-operandi:
 *
 * A persistent vector wraps the PersistentPtr functionality. Each change to the vector
 * is a re-allocation of the array contained inside the persistent vector.
 *
 * TODO:
 * N.B. An advantage of the interface, is that this can easily be extended. If we use
 *      fixed sizes of vector, we don't need to re-allocate until we reach a certain
 *      number of elements. We can then point to a "continuation array".
 */


namespace pmem {



//----------------------------------------------------------------------------------------------------------------------

///
/// We separate out the data type and the management type for the PersistentVector. Ultimately the
/// persistent vector is a wrapper around
template <typename T>
class PersistentVectorData : public PersistentType<PersistentVectorData<T> > {

public: // methods

    /// Constructors
    PersistentVectorData(size_t max_size);
    PersistentVectorData(const PersistentVectorData<T>& source, size_t max_size);

    /// The amount of memory that needs to be allocated to store this
    static size_t data_size(size_t max_size);

    /// Number of elements in the list
    size_t size() const;

    /// Returns true if the number of elements is equal to the available space
    bool full() const;

    /// How much space is available
    size_t allocated_size() const;

    /// Append an element to the list.
    PersistentPtr<T> push_back(const AtomicConstructor<T>& constructor);

    /// Return a given element in the list
    const PersistentPtr<T>& operator[] (size_t i) const;

    /// As the nelem_ member is updated after allocation has taken place, and hence non-atomically, we need to
    /// be able to check that its value is correct.
    ///
    /// @note this implementation will be insufficient if we add the capacity to remove elements as well as add them.
    void consistency_check() const;

protected: // methods

    /// Update the number of elements, ensuring that the result is persisted
    void update_nelem(size_t nelem) const;

protected: // members

    // Track the allocated size, and the number of elements used
    mutable size_t nelem_;
    size_t allocatedSize_;

    // The allocator/constructor will make the PersistentVectorData the right size.
    PersistentPtr<T> elements_[1];
};


//----------------------------------------------------------------------------------------------------------------------


template <typename T>
class PersistentVector : public PersistentPtr<PersistentVectorData<T> > {

    typedef PersistentVectorData<T> data_type;

public:

    PersistentPtr<T> push_back_ctr(const AtomicConstructor<T>& constructor);

    PersistentPtr<T> push_back();
    template <typename X1>
    PersistentPtr<T> push_back(const X1& x1);
    template <typename X1, typename X2>
    PersistentPtr<T> push_back(const X1& x1, const X2& x2);

    size_t size() const;

    size_t allocated_size() const;

    const PersistentPtr<T>& operator[] (size_t i) const;

    void resize(size_t new_size);
};


// ---------------------------------------------------------------------------------------------------------------------

/// Override the determination of the size for each of the two constructors.

template <typename T>
class AtomicConstructor1<PersistentVectorData<T>, size_t> :
        public AtomicConstructor1Base<PersistentVectorData<T>, size_t> {
public:

    AtomicConstructor1(size_t x1) : AtomicConstructor1Base<PersistentVectorData<T>,size_t>(x1) {}

    virtual size_t size() const {
        return PersistentVectorData<T>::data_size(this->x1_);
    }
};


template <typename T>
class AtomicConstructor2<PersistentVectorData<T>, PersistentVectorData<T>, size_t> :
        public AtomicConstructor2Base<PersistentVectorData<T>, PersistentVectorData<T>, size_t> {
public:

    AtomicConstructor2(const PersistentVectorData<T>& x1, size_t x2) :
        AtomicConstructor2Base<PersistentVectorData<T>, PersistentVectorData<T>, size_t>(x1, x2) {}

    virtual size_t size() const {
        return PersistentVectorData<T>::data_size(this->x2_);
    }
};

//----------------------------------------------------------------------------------------------------------------------


/// Normal data constructor
template <typename T>
PersistentVectorData<T>::PersistentVectorData(size_t max_size) :
    nelem_(0),
    allocatedSize_(max_size) {

    for (size_t i = 0; i < allocatedSize_; i++) {
        elements_[i].nullify();
    }
}


/// Copy constructor
template <typename T>
PersistentVectorData<T>::PersistentVectorData(const PersistentVectorData<T>& source, size_t max_size) :
    nelem_(source.size()), // n.b. using size() enforces consistency check)
    allocatedSize_(max_size) {

    ASSERT(allocatedSize_ > nelem_);

    size_t i;
    for (i = 0; i < nelem_; i++) {
        elements_[i] = source.elements_[i];
    }
    for (i = nelem_; i < allocatedSize_; i++) {
        elements_[i].nullify();
    }
}


template <typename T>
size_t PersistentVectorData<T>::data_size(size_t max_size) {
    return sizeof(PersistentVectorData<T>) + (max_size - 1) * sizeof(PersistentPtr<T>);
}


/// Number of elements in the list
template <typename T>
size_t PersistentVectorData<T>::size() const {

    consistency_check();

    ASSERT(nelem_ <= allocatedSize_);
    return nelem_;
}


/// Number of elements in the list
template <typename T>
size_t PersistentVectorData<T>::allocated_size() const {
    return allocatedSize_;
}


/// Returns true if the number of elements is equal to the available space
template <typename T>
bool PersistentVectorData<T>::full() const {

    consistency_check();

    ASSERT(nelem_ <= allocatedSize_);
    return (nelem_ == allocatedSize_);
}


/// Append an element to the list.
template <typename T>
PersistentPtr<T> PersistentVectorData<T>::push_back(const AtomicConstructor<T>& constructor) {

    consistency_check();

    if (nelem_ == allocatedSize_)
        throw eckit::OutOfRange("PersistentVector is full", Here());

    size_t stored_elem = nelem_;
    elements_[stored_elem].allocate_ctr(constructor);

    // n.b. This update is NOT ATOMIC, and therefore creates the requirement to call consistency_check() to ensure
    //      that we haven't had a power-off-power-on incident.
    update_nelem(nelem_ + 1);

    return elements_[stored_elem];
}


/// Return a given element in the list
template<typename T>
const PersistentPtr<T>& PersistentVectorData<T>::operator[] (size_t i) const {
    return elements_[i];
}


/// As the nelem_ member is updated after allocation has taken place, and hence non-atomically, we need to
/// be able to check that its value is correct.
template<typename T>
void PersistentVectorData<T>::consistency_check() const {

    if (nelem_ != 0)
        ASSERT(!elements_[nelem_-1].null());

    // Keep looping until the _next_ element is null (or we reach the end). At that point everything is correct.
    bool updated = false;
    size_t n;
    for (n = nelem_; n < allocatedSize_; n++) {
        if (elements_[n].null())
            break;
        updated = true;
    }

    // If we have modified nelem_, it needs to be persisted.
    if (updated)
        update_nelem(n);
}


/// Update the number of elements, ensuring that the result is persisted
template <typename T>
void PersistentVectorData<T>::update_nelem(size_t nelem) const {

    nelem_ = nelem;
    ::pmemobj_persist(::pmemobj_pool_by_ptr(&nelem_), &nelem_, sizeof(nelem_));
}


//----------------------------------------------------------------------------------------------------------------------


template <typename T>
PersistentPtr<T> PersistentVector<T>::push_back_ctr(const AtomicConstructor<T>& constructor) {

    // TODO: Determine a size at runtime, or set it at compile time, but this is the worst of both worlds.
    if (PersistentPtr<data_type>::null()) {
        PersistentPtr<data_type>::allocate(1);
        ASSERT(size() == 0);
    }

    // If all of the available space is full, then increase the space available (by factor of 2)
    if (PersistentPtr<data_type>::get()->full()) {
        size_t sz = size();
        resize(sz * 2);
    }

    return PersistentPtr<data_type>::get()->push_back(constructor);
}


template <typename T>
PersistentPtr<T> PersistentVector<T>::push_back() {
    AtomicConstructor0<T> ctr;
    return push_back_ctr(ctr);
}


template <typename T>
template <typename X1>
PersistentPtr<T> PersistentVector<T>::push_back(const X1& x1) {
    AtomicConstructor1<T, X1> ctr(x1);
    return push_back_ctr(ctr);
}


template <typename T>
template <typename X1, typename X2>
PersistentPtr<T> PersistentVector<T>::push_back(const X1& x1, const X2& x2) {
    AtomicConstructor2<T, X1, X2> ctr(x1, x2);
    return push_back_ctr(ctr);
}

template <typename T>
size_t PersistentVector<T>::size() const {
    return PersistentPtr<data_type>::null() ? 0 : (*this)->size();
}


template <typename T>
size_t PersistentVector<T>::allocated_size() const {
    return PersistentPtr<data_type>::null() ? 0 : (*this)->allocated_size();
}


template <typename T>
const PersistentPtr<T>& PersistentVector<T>::operator[] (size_t i) const {
    return (*PersistentPtr<data_type>::get())[i];
}


template <typename T>
void PersistentVector<T>::resize(size_t new_size) {

    if (PersistentPtr<data_type>::null()) {

        // Reserve space as specified
        PersistentPtr<data_type>::allocate(new_size);

    } else {

        // Atomically replace the data with a resized copy.
        PersistentPtr<data_type>::replace(**this, new_size);
    }
}


//----------------------------------------------------------------------------------------------------------------------


} // namespace pmem

#endif // pmem_PersistentVector_H

