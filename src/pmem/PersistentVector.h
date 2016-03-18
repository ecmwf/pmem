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


#include "persistent/PersistentPtr.h"


#ifndef tree_PersistentVector_H
#define tree_PersistentVector_H

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


// -------------------------------------------------------------------------------------------------


template <typename T>
class PersistentVector {

public: // types

    /*
     * The data stored in a persistent vector is stored inside a single data type, that
     * is contained within a PersistentPtr
     *
     * --> In terms of memory layout, using a persistent vector is entirely equivalent to
     *     using a PersistentPtr<data_type>, with some wrapper functionality.
     *
     * The allocated memory must vary depending on the number of elements stored in the
     * vector. The byte_size routine calculates the total amount of space required, given
     * the correct number of elements.
     */
    struct data_type {

        // methods
        size_t byte_size() const {
            return size(nelem_);
        }

        static size_t byte_size(size_t nelem) {
            return sizeof(data_type) + (sizeof(T) * (nelem-1));
        }

        // members
        size_t nelem_;
        T data_[1];
    };


private: // types (atomic constructors)

    // Define some actions that can be used on these data types.
    class AppendConstructor : public AtomicConstructor<data_type> {

    public: //methods

        AppendConstructor(const data_type* original, const T& new_elem) :
            original_(original),
            newElem_(new_elem) {}

        // Construct a new data_type object to represent the data with the new element
        // appended.
        //  - If no previous data, create new data.
        //  - Otherwise, copy all elements accross.
        virtual void make(data_type* object) const {
            size_t nelem = original_ ? original_->nelem_ : 0;
            for (size_t i = 0; i < nelem; ++i) {
                object->data_[i] = original_->data_[i];
            }

            object->data_[nelem] = newElem_;
            object->nelem_ = nelem + 1;
        }

        virtual size_t size() const {
            return data_type::byte_size((original_ ? original_->nelem_ : 0) + 1);
        }

    private: // members

        const data_type* original_;
        const T& newElem_;
    };

public:

    // TODO: This should only be used during construction. TODO.
    void nullify();

    void push_back(const T& new_elem);

    size_t size() const;

    const T& operator[] (size_t i) const;

protected:

    PersistentPtr<data_type> items_;
};


// -------------------------------------------------------------------------------------------------


template <typename T>
void PersistentVector<T>::nullify() {
    items_.nullify();
}


template <typename T>
size_t PersistentVector<T>::size() const {

    return items_.null() ? 0 : items_->nelem_;
}


template <typename T>
const T& PersistentVector<T>::operator[] (size_t i) const {

    return items_->data_[i];
}

template <typename T>
void PersistentVector<T>::push_back(const T& new_elem) {

    // To extend the vector, we need to make a copy of an existing vector, add the item to
    // the copy, swap the target pointer and only _then_ delete the original.

    PersistentPtr<data_type> ptmp = items_;
    const data_type * original = items_.null() ? 0 : items_.get();

    AppendConstructor appendConstructor(original, new_elem);

    items_.allocate(appendConstructor);

    if (!ptmp.null()) {
        ptmp.free();
    }
}


// -------------------------------------------------------------------------------------------------


} // namespace pmem

#endif // tree_PersistentVector_H

