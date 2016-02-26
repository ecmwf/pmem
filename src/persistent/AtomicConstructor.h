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


#ifndef persistent_AtomicConstructor_H
#define persistent_AtomicConstructor_H

#include <cstddef>

#include "libpmemobj.h"

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

class AtomicConstructorBase {
public:
    virtual void build(void * obj) const = 0;
    virtual size_t size() const = 0;
};


template <typename T>
class AtomicConstructor : public AtomicConstructorBase {
public:

    // In general the "pop" argument should not be needed, but if the user desires to create
    // a nested tree of objects, then they will need it.
    virtual void make (T * object) const = 0;

    virtual void build (void * obj) const {
        T * object = reinterpret_cast<T*>(obj);
        make(object);
    }

    /// N.b. This is virtual. This allows the constructor to actually allocate more memory
    ///      than the size of the declared structure (e.g. if allocating a buffer).
    virtual size_t size() const { return sizeof(T); }
};


// -------------------------------------------------------------------------------------------------

} // namespace pmem

#endif // persistent_AtomicConstructor_H
