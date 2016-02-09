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

/*
 * Here we provide an interface for accessing the ATOMIC functionality of
 * libpmemobj.h.
 *
 * Note that this does NOT provide any functionality for accessing the
 * transaction based api.
 *
 * USAGE:
 *
 *   i) Define types as structs in a layout.h that is used for an entire project.
 *      To define object relationships use persistent_ptr<> below.
 *
 *  ii) Use the following macros to define unique IDs for the memory regions that
 *      permit type-checking on resumption from power-off, etc.
 *
 *        POBJ_LAYOUT_BEGIN(layout_name);
 *        POBJ_CPP_LAYOUT_ROOT(layout_name, root_structure);
 *        POJB_CPP_LAYOUT_TOID(layout_name, structure);
 *        POBJ_LAYOUT_END(layout_name)
 *
 *      These macros work in the same way as discussed in the pmem.io documentation
 *      (http://pmem.io/2015/06/18/ntx-alloc.html), but additionally hook the C
 *      macro based type system into the persistent_ptr class defined below.
 *
 * iii) Define functors derived from atomic_constructor<structure> to define
 *      arbitrarily sophisticated initialisation of the defined data structures.
 *
 *  iv) Build a datastructure in memory using the atomic operations allocate() and
 *      free(). To build the first (root) element, the allocate_root() method may be
 *      used, otherwise an object can only be created associated with an existing
 *      (but NULL) pointer.
 */


#ifndef objserver_persistent_ptr_H
#define objserver_persistent_ptr_H

#include "libpmemobj.h"

/*
 * Here we define a couple of macros to extend the layout definitions associated with the
 * C-Macro system. This injects the TOID type information, which is only obtainable from
 * the C macros, into the templated C++ type system.
 *
 * POBJ_LAYOUT_ROOT --> POBJ_CPP_LAYOUT_ROOT
 * POBJ_LAYOUT_TOID --> POBJ_CPP_LAYOUt_TOID
 */

#define POBJ_CPP_LAYOUT_ROOT(layout, t) \
    POBJ_LAYOUT_ROOT(layout, t) \
    template<> int ::pmem::persistent_ptr<t>::type_id = TOID_TYPE_NUM(t)

#define POBJ_CPP_LAYOUT_TOID(layout, t) \
    POBJ_LAYOUT_TOID(layout, t) \
    template<> int ::pmem::persistent_ptr<t>::type_id = TOID_TYPE_NUM(t)


namespace pmem {

// -------------------------------------------------------------------------------------------------


/*
 * This is the base class for atomically constructing a type in persistent memory. The derived
 * make(T* object) method is called (indirectly) from inside the atomic allocator in libpmemobj.h.
 *
 * There may be an arbitrary number of atomic constructors for any given structure, to allow
 * arbitrary complexity of usage. A constructor instance is passed into the allocate() method
 * as (effectively, if not strictly) a functor.
 */

template <typename T>
class atomic_constructor {
public:

    virtual void make (T * object) const = 0;

    void build (void * obj) const {
        T * object = reinterpret_cast<T*>(obj);
        make(object);
    }

    virtual size_t size() const { return sizeof(T); }
};


// -------------------------------------------------------------------------------------------------


template <typename T>
class persistent_ptr {

public: // members

    /// This static is used to perform type id lookups, so that the type checking that
    /// works in the C-MACRO based version continues to work in this C++ wrapper.
    ///
    /// TODO: Define updated POBJ_LAYOUT_* macros to define this type_id statically.
    static int type_id;

public: // methods

    /*
     * Constructors
     */
    persistent_ptr() :
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

    /// Note that allocation and setting of the pointer are atomic. The work of setting up the
    /// object is done inside the functor atomic_constructor<T>.
    void allocate(PMEMobjpool * pop, atomic_constructor<T>& constructor) {
        ASSERT(null());
        ::pmemobj_alloc(pop, &oid_, constructor.size(), type_id,
                        &persistent_ptr<T>::constructor, &constructor);
    }

    void allocate_root(PMEMobjpool * pop, atomic_constructor<T>& constructor) {
        ::pmemobj_alloc(pop, NULL, constructor.size(), type_id,
                        &persistent_ptr<T>::constructor, &constructor);
    }

    /// Deallocate the memory. Note that this atomically frees and sets oid_ == OID_NULL.
    void free() {
        ::pmemobj_free(&oid_);
    }

    /*
     * useful accessors
     * TODO: Consider if these should be pulled out of the class, into helper routines.
     */
    static persistent_ptr get_root_object(PMEMobjpool * pop) {
        return persistent_ptr(pmemobj_root(pop, sizeof(T)));
    }

private: // methods

    /// Don't support user-manipulation of the oid directly, but we need to have a way internally.
    persistent_ptr(PMEMoid oid) :
        oid_(oid) {}

    /// This is a static routine that can be passed to the atomic allocation routines. All the logic
    /// should be passed in as the functor atomic_constructor<T>.
    static void constructor(PMEMobjpool * pop, void * ptr, void * arg) {
        T * obj = reinterpret_cast<T*>(ptr);
        const atomic_constructor<T> * constr_fn = reinterpret_cast<const atomic_constructor<T>*>(arg);

        constr_fn->build(obj);
        ::pmemobj_persist(pop, obj, constr_fn->size());
    }


private: // members

    /*
     * N.B. There is only ONEC data member here. This is a PMEMoid, and is the SAME data that
     *      is held in the TOID() macro-enhanced data type in the C-bindings. (In C this is
     *      actually a named union, with the only data being a PMEMoid). This means that PMEMoid,
     *      TOID and persistent_ptr can be used interchangeably in the data format.
     *
     * N.B. 2 - Because we want to ensure the data stays the same, persistent_ptr MUST NOT be
     *          a virtual class.
     */

    PMEMoid oid_;
};


// -------------------------------------------------------------------------------------------------

}

#endif // objserver_persistent_ptr_H
