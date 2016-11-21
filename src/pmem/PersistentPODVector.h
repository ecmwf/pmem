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


#ifndef pmem_PersistentPODVector_H
#define pmem_PersistentPODVector_H


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
class PersistentPODVectorData : public PersistentType<PersistentPODVectorData<T> > {

public: // Constructors

    /// Constructors
    PersistentPODVectorData(size_t max_size);
    PersistentPODVectorData(const PersistentPODVector<T>& source, size_t max_size);

    /// The amount of memory that needs to be allocated to store this
    static size_t data_size(size_t max_size);

    /// Number of elements in the list
    size_t size() const;

    /// Returns true if the number of elements is equal to the available space
    bool full() const;

    /// How much space is available
    size_t allocated_size() const;

    /// Append an element to the list.
    void push_back(const T& value);

    /// Return a given element in the list
    const T& operator[] (size_t i) const;

protected: // methods

    /// Update the number of elements, ensuring that the result is persisted
    void update_nelem(size_t nelem) const;

protected: // members

    // Track the allocated size, and the number of elements used
    mutable size_t nelem_;
    size_t allocatedSize_;

    // The allocator/constructor will make the PersistentVectorData the right size.
    T elements_[1];
};


//----------------------------------------------------------------------------------------------------------------------


template <typename T>
class PersistentPODVector : public PersistentPtr<PersistentPODVectorData<T> > {

public: // types

    typedef PersistentPODVectorData<T> data_type;

public:

    void push_back(const T& constructor);

    size_t size() const;

    size_t allocated_size() const;

    const T& operator[] (size_t i) const;

    void resize(size_t new_size);
};


//----------------------------------------------------------------------------------------------------------------------


PersistentPODVectorData<T>::PersistentPODVectorData(size_t max_size) :
    allocatedSize_(max_size),
    nelem_(0) {

}


PersistentPODVectorData<T>::PersistentPODVectorData(const PersistentPODVector<T> &source,
                                                    size_t max_size) :
    allocatedSize_(max_size),
    nelem_(source_.size()) { // n.b. using size() enforces consistency check.

    ASSERT(allocatedSize_ > nelem_);

    for (size_t i = 0; i < nelem_; i++) {
        elements_[i] = source_.elements_[i];
    }
}


template <typename T>
size_t PersistentPODVectorData<T>::data_size(size_t max_size) {
    return sizeof(PersistentPODVectorData<T>) + (max_size - 1) * sizeof(T);
}

// Specify the size of the PersistentPODVectorData
template<T>
inline size_t AtomicConstructor1<PersistentPODVectorData<T>, std::string>::size() const {
    return PersistentPODVectorData<T>::data_size(x1_);
}

template<T>
inline size_t AtomicConstructor1<PersistentPODVectorData<T>, std::string>::size() const {
    return PersistentPODVectorData<T>::data_size(x2_);
}


/// Number of elements in the list
template <typename T>
size_t PersistentPODVectorData<T>::size() const {
    ASSERT(nelem_ <= allocatedSize_);
    return nelem_;
}


/// Number of elements in the list
template <typename T>
size_t PersistentPODVectorData<T>::allocated_size() const {
    return allocatedSize_;
}


/// Returns true if the number of elements is equal to the available space
template <typename T>
bool PersistentPODVectorData<T>::full() const {

    ASSERT(nelem_ <= allocatedSize_);
    return (nelem_ == allocatedSize_);
}


/// Append an element to the list.
/// The last thing that is done is to update the nelem_ count. Up to that point, it is considered not to have
/// happened yet. That's ok.
template<typename T>
void PersistentPODVectorData<T>::push_back(const T& value) {

    ASSERT(nelem_ <= allocatedSize_);
    if (nelem_ == allocatedSize_)
        throw eckit::OutOfRange("PersistentVector is full", Here());

    // Store the data first, and then update the counter. If persistence is lost imbetween, that is fine, as we
    // still retain an object in a reasonable state.
    // TODO: Add a method to the Constructor to make expanding the PersistentPODVectorData<> one-step.

    elements_[nelem_] = value;
    ::pmemobj_persist(::pmemobj_pool_by_ptr(&elements_[nelem_]), &elements_[nelem_], sizeof(T));

    ++nelem_;
    ::pmemobj_persist(::pmemobj_pool_by_ptr(&nelem_), &nelem_, sizeof(nelem_));
}


/// Return a given element in the list
template<typename T>
const T& PersistentPODVectorData<T>::operator[] (size_t i) const {
    return elements_[i];
}


//----------------------------------------------------------------------------------------------------------------------


template <typename T>
void PersistentPODVector<T>::push_back(const T& value) {

    // TODO: Add a construtor that includes an element, or a new element, to avoid two stage allocate, push_back
    // TODO: A better sizing that using 1 as the default.

    if (PersistentPtr<data_type>::null()) {
        PersistentPtr<data_type>::allocate(size_t(1));
        ASSERT(size() == 0);
    }

    // If all of the available space is full, then increase the space available (by factor of 2)
    if (PersistentPtr<data_type>::get()->full()) {
        size_t sz = size();
        eckit::Log::info() << "Resizing POD vector from " << sz << " elements to " << 2*sz << std::endl;
        resize(sz * 2);
    }

    PersistentPtr<data_type>::get()->push_back(value);
}

template <typename T>
size_t PersistentPODVector<T>::size() const {
    return PersistentPtr<data_type>::null() ? 0 : (*this)->size();
}


template <typename T>
size_t PersistentPODVector<T>::allocated_size() const {
    return PersistentPtr<data_type>::null() ? 0 : (*this)->allocated_size();
}


template <typename T>
const T& PersistentPODVector<T>::operator[] (size_t i) const {
    return (*PersistentPtr<data_type>::get())[i];
}


template <typename T>
void PersistentPODVector<T>::resize(size_t new_size) {

    if (PersistentPtr<data_type>::null()) {

        // Reserve space as specified
        PersistentPtr<data_type>::allocate(new_size);

    } else {

        // Atomically replace the data with a resized copy.
        typename data_type::Constructor ctr(**this, new_size);
        PersistentPtr<data_type>::replace(ctr);
    }
}


//----------------------------------------------------------------------------------------------------------------------


} // namespace pmem

#endif // pmem_PersistentPODVector_H

