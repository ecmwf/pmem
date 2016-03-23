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
        Constructor(size_t max_size) :
            maxSize_(max_size),
            sourceVector_(0) {}

        /// Copy constructor
        Constructor(const PersistentVectorData<T>& source, size_t max_size) :
            maxSize_(max_size),
            sourceVector_(&source) {

            ASSERT(max_size > source.nelem_);
        }

        virtual void make(PersistentVectorData<T>& object) const {

            object.allocatedSize_ = maxSize_;

            // If we are copying an existing vector, then transfer the data across
            size_t i = 0;
            if (sourceVector_) {
                object.nelem_ = sourceVector_->nelem_;
                for (; i < sourceVector_->nelem_; i++)
                    object.elements_[i] = sourceVector_->elements_[i];
            } else {
                object.nelem_ = 0;
            }

            // And nullify the remaining elements.
            for (; i < maxSize_; i++)
                object.elements_[i].nullify();
        }

        /// Return the size in bytes. This is variable depending on the number of elements
        /// stored in the vector.
        virtual size_t size() const {
            return sizeof(PersistentVectorData<T>) + (maxSize_ - 1) * sizeof(PersistentPtr<T>);
        }

    private: // members

        size_t maxSize_;
        const PersistentVectorData<T>* sourceVector_;
    };

public: // methods

    /// Number of elements in the list
    size_t size() const {
        consistency_check();
        return nelem_;
    }

    /// Append an element to the list.
    void push_back(const AtomicConstructor<T>& constructor) {
        consistency_check();
        if (nelem_ == allocatedSize_)
            throw eckit::OutOfRange("PersistentVector is full", Here());

        elements_[nelem_].allocate(constructor);

        // n.b. This update is NOT ATOMIC, and therefore creates the requirement to call consistency_check() to ensure
        //      that we haven't had a power-off-power-on incident.
        update_nelem(nelem_ + 1);
    }

    /// Return a given element in the list
    const T& operator[] (size_t i) const {
        return *elements_[i];
    }

    /// As the nelem_ member is updated after allocation has taken place, and hence non-atomically, we need to
    /// be able to check that its value is correct.
    void consistency_check() const {

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

protected: // methods

    /// Update the number of elements, ensuring that the result is persisted
    void update_nelem(size_t nelem) const {

        nelem_ = nelem;
        ::pmemobj_persist(::pmemobj_pool_by_ptr(&nelem_), &nelem_, sizeof(nelem_));
    }

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

    void push_back(const AtomicConstructor<T>& constructor) {

        // TODO: Determine a size at runtime, or set it at compile time, but this is the worst of both worlds.
        if (items_.null()) {
            typename data_type::Constructor ctr(123);
            items_.allocate(ctr);
            ASSERT(size() == 0);
        }

        items_->push_back(constructor);
    }

    size_t size() const {
        return items_.null() ? 0 : items_->size();
    }

    const T& operator[] (size_t i) const {
        return (*items_)[i];
    }

    void resize(size_t new_size) {
        ASSERT(!items_.null());

        // Atomically replace the data with a resized copy.
        typename data_type::Constructor ctr(*items_, new_size);
        items_.replace(ctr);
    }

protected:

    PersistentPtr<data_type> items_;
};


//----------------------------------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------------------------------


} // namespace pmem

#endif // pmem_PersistentVector_H

