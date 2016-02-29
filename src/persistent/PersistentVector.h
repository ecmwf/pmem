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

    // This is the data that gets stored
    //
    // - It needs to be in a persistent pointer...
    struct data_type {

        size_t nelem_;
        T data_[1];

        size_t byte_size() const {
            return size(nelem_);
        }

        static size_t byte_size(size_t nelem) {
            return sizeof(data_type) + (sizeof(T) * (nelem-1));
        }
    };

private: // types (atomic constructors)

    // Define some actions that can be used on these data types.
    class AppendConstructor : public AtomicConstructor<data_type> {

    public: //methods

        AppendConstructor(const data_type* original, const T& new_elem) :
            original_(original),
            newElem_(new_elem) {}

        virtual void make(data_type* object) const {
            eckit::Log::info() << "Append constructor" << std::endl << std::flush;
            if (original_) {
                object->nelem_ = original_->nelem_ + 1;
                for (size_t i = 0; i < object->nelem_-1; i++) {
                    object->data_[i] = original_->data_[i];
                }
            } else {
                object->nelem_ = 1;
            }
            eckit::Log::info() << "Old copied" << std::endl << std::flush;

            object->data_[object->nelem_-1] = newElem_;
            eckit::Log::info() << "Done" << std::endl << std::flush;
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
    void nullify() {
        items_.nullify();
    }


    void push_back(const T& new_elem) {

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


    size_t size() const {
        if (items_.null()) {
            return 0;
        }
        return items_->nelem_;
    }


    const T& operator[] (size_t i) {
        return items_->data_[i];
    }

protected:

    PersistentPtr<data_type> items_;
};


// -------------------------------------------------------------------------------------------------


} // namespace pmem

#endif // tree_PersistentVector_H

