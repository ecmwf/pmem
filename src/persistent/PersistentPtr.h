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


#ifndef persistent_PersistentPtr_H
#define persistent_PersistentPtr_H

#include "libpmemobj.h"


namespace pmem {

// -------------------------------------------------------------------------------------------------


template <typename T>
class PersistentPtr {

public: // members

    /// This static is used to perform type id lookups.
    /// Needs to be defined GLOBALLY.

    static int type_id;

public: // methods

    /*
     * Constructors
     */
    PersistentPtr() :
        oid_(OID_NULL) {}

    /*
     * Access the stored object
     */

    T& operator*() const {
        *get();
    }

    T* operator->() const {
        return get();
    };

    T* get() const {
        return (T*)pmemobj_direct(oid_);
    }

    /*
     * Status testing?
     */
    bool null() const {
        return oid_.off == 0;
    }

    bool valid() const {
        return pmemobj_type_num(oid_) == type_id;
    }

    /*
     * Modification
     */
    void nullify() {
        oid_ = OID_NULL;
    }

//    /// Note that allocation and setting of the pointer are atomic. The work of setting up the
//    /// object is done inside the functor atomic_constructor<T>.
//    void allocate(PMEMobjpool * pop, atomic_constructor<T>& constructor) {
//        // n.b. We don't want to assert(null()). We may be updating, say, pointers in a chain of
//        //      objects, with atomic rearrangement. That is fine.
//        ::pmemobj_alloc(pop, &oid_, constructor.size(), type_id,
//                        &persistent_ptr<T>::constructor, &constructor);
//    }
//
//    void allocate_root(PMEMobjpool * pop, atomic_constructor<T>& constructor) {
//        ::pmemobj_alloc(pop, NULL, constructor.size(), type_id,
//                        &persistent_ptr<T>::constructor, &constructor);
//    }

    /// Deallocate the memory. Note that this atomically frees and sets oid_ == OID_NULL.
    void free() {
        ::pmemobj_free(&oid_);
    }

    /*
     * useful accessors
     * TODO: Consider if these should be pulled out of the class, into helper routines.
     */
    static PersistentPtr get_root_object(PMEMobjpool * pop) {
        return PersistentPtr(pmemobj_root(pop, sizeof(T)));
    }

private: // methods

    /// Don't support user-manipulation of the oid directly, but we need to have a way internally.
    PersistentPtr(PMEMoid oid) :
        oid_(oid) {}

//    /// This is a static routine that can be passed to the atomic allocation routines. All the logic
//    /// should be passed in as the functor atomic_constructor<T>.
//    static void constructor(PMEMobjpool * pop, void * ptr, void * arg) {
//        T * obj = reinterpret_cast<T*>(ptr);
//        const atomic_constructor<T> * constr_fn = reinterpret_cast<const atomic_constructor<T>*>(arg);
//
//        constr_fn->build(obj);
//        ::pmemobj_persist(pop, obj, constr_fn->size());
//    }

    friend bool operator== (const PersistentPtr& lhs, const PersistentPtr& rhs) {
        return ((lhs.oid_.off == rhs.oid_.off) && (lhs.oid_.pool_uuid_lo == rhs.oid_.pool_uuid_lo));
    };

private: // members

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


// -------------------------------------------------------------------------------------------------

}

#endif // persistent_PersistentPtr_H
