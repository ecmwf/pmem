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
class PersistentVectorData {

public: // Constructors

    class Constructor : public AtomicConstructor<PersistentVectorData<T> > {

    public: // methods

        /// Normal constructor
        Constructor(size_t max_size);

        /// Copy constructor
        Constructor(const PersistentVectorData<T>& source, size_t max_size);

        virtual void make(PersistentVectorData<T>& object) const;

        /// Return the size in bytes. This is variable depending on the number of elements
        /// stored in the vector.
        virtual size_t size() const;

    private: // members

        size_t maxSize_;
        const PersistentVectorData<T>* sourceVector_;
    };

public: // methods

    /// Number of elements in the list
    size_t size() const;

    /// Returns true if the number of elements is equal to the available space
    bool full() const;

    /// How much space is available
    size_t allocated_size() const;

    /// Append an element to the list.
    void push_back(const AtomicConstructor<T>& constructor);

    /// Return a given element in the list
    const PersistentPtr<T>& operator[] (size_t i) const;

    /// As the nelem_ member is updated after allocation has taken place, and hence non-atomically, we need to
    /// be able to check that its value is correct.
    void consistency_check() const;

protected: // methods

    /// Update the number of elements, ensuring that the result is persisted
    void update_nelem(size_t nelem) const;

private: // members

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

    void push_back(const AtomicConstructor<T>& constructor);

    size_t size() const;

    size_t allocated_size() const;

    const PersistentPtr<T>& operator[] (size_t i) const;

    void resize(size_t new_size);
};


//----------------------------------------------------------------------------------------------------------------------


/// Normal data constructor
template <typename T>
PersistentVectorData<T>::Constructor::Constructor(size_t max_size) :
    maxSize_(max_size),
    sourceVector_(0) {}


/// Copy constructor
template <typename T>
PersistentVectorData<T>::Constructor::Constructor(const PersistentVectorData<T>& source, size_t max_size) :
    maxSize_(max_size),
    sourceVector_(&source) {

    ASSERT(max_size > source.nelem_);
}


template <typename T>
void PersistentVectorData<T>::Constructor::make(PersistentVectorData<T>& object) const {

    object.allocatedSize_ = maxSize_;

    // If we are copying an existing vector, then transfer the data across
    size_t i = 0;
    if (sourceVector_) {
        object.nelem_ = sourceVector_->size();  // n.b. enforces consistency check.
        for (; i < sourceVector_->nelem_; i++) {
            object.elements_[i] = sourceVector_->elements_[i];
        }
    } else {
        object.nelem_ = 0;
    }

    // And nullify the remaining elements.
    for (; i < maxSize_; i++) {
        object.elements_[i].nullify();
    }
}


template <typename T>
size_t PersistentVectorData<T>::Constructor::size() const {
    return sizeof(PersistentVectorData<T>) + (maxSize_ - 1) * sizeof(PersistentPtr<T>);
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
template<typename T>
void PersistentVectorData<T>::push_back(const AtomicConstructor<T>& constructor) {

    consistency_check();

    if (nelem_ == allocatedSize_)
        throw eckit::OutOfRange("PersistentVector is full", Here());

    elements_[nelem_].allocate(constructor);

    // n.b. This update is NOT ATOMIC, and therefore creates the requirement to call consistency_check() to ensure
    //      that we haven't had a power-off-power-on incident.
    update_nelem(nelem_ + 1);
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
void PersistentVector<T>::push_back(const AtomicConstructor<T>& constructor) {

    // TODO: Determine a size at runtime, or set it at compile time, but this is the worst of both worlds.
    if (PersistentPtr<data_type>::null()) {
        typename data_type::Constructor ctr(1);
        PersistentPtr<data_type>::allocate(ctr);
        ASSERT(size() == 0);
    }

    // If all of the available space is full, then increase the space available (by factor of 2)
    if (PersistentPtr<data_type>::get()->full()) {
        size_t sz = size();
        eckit::Log::info() << "Resizing vector from " << sz << " elements to " << 2*sz << std::endl;
        resize(sz * 2);
    }

    PersistentPtr<data_type>::get()->push_back(constructor);
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
        typename data_type::Constructor ctr(new_size);
        PersistentPtr<data_type>::allocate(ctr);

    } else {

        // Atomically replace the data with a resized copy.
        typename data_type::Constructor ctr(**this, new_size);
        PersistentPtr<data_type>::replace(ctr);
    }
}


//----------------------------------------------------------------------------------------------------------------------


} // namespace pmem

#endif // pmem_PersistentVector_H

