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


#ifndef pmem_AtomicConstructor_H
#define pmem_AtomicConstructor_H

#include <cstddef>
#include <string>

#include "eckit/exception/Exceptions.h"

#include "pmem/PersistentType.h"

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
    // Something to throw in a constructor, if necessary, to unwind allocations.
    struct AllocationError : public eckit::Exception {
        AllocationError(const std::string& what) : Exception(what) {}
    };

    AtomicConstructorBase() {}
    virtual ~AtomicConstructorBase() {}

    virtual int build(void * obj) const = 0;
    virtual size_t size() const = 0;
    virtual uint64_t type_id() const = 0;
};


template <typename T>
class AtomicConstructor : public AtomicConstructorBase {
public:

    /// Overload this to construct the specified object
    /// @note Throw an AtomicConstructorBase::Allocation error to cause the allocation to fail and the persistent
    ///       memory to be freed. If further objects are allocated inside this make function, and the exception is
    ///       allowed to escape, this will cause the allocation chain to unwind
    virtual void make (T& object) const = 0;

    // TODO: Should we pass a reference through to make instead?
    virtual int build (void * obj) const {
        T * object = reinterpret_cast<T*>(obj);
        ASSERT(object);
        try {
            make(*object);
        } catch (AllocationError& e) {
            return 1;
        }
        return 0;
    }

    /// Return the size of the object to be allocated in bytes. This is called by the
    /// ::pmem_constructor routine.
    ///
    /// @note This is virtual. This allows the constructor to actually allocate more memory
    ///       than the size of the declared structure (e.g. if allocating a buffer).
    virtual size_t size() const { return sizeof(T); }

    /// Return the type_id to use during construction. Generally this just echoes whatever
    /// is contained in the PersistentType<> structure for the type, but it can be customised
    virtual uint64_t type_id() const { return PersistentType<T>::type_id; }
};


// TODO: Generate these templates from a script... DRY.

// We use an indirection with AtomicConstructorNBase<T> -- > AtomicConstructorN<T>. This
// allows partial specialisation of functions (as in PersistentPODVector), which is not
// directly available in the C++ standard (sigh).


template <typename T>
class AtomicConstructor0Base : public AtomicConstructor<T> {

public: // methods

    virtual void make (T& object) const {
        new (&object) T;
    }

    virtual size_t size() const { return AtomicConstructor<T>::size(); }
    virtual uint64_t type_id() const { return AtomicConstructor<T>::type_id(); }
};


template <typename T>
class AtomicConstructor0 : public AtomicConstructor0Base<T> { };

// --


template <typename T, typename X1>
class AtomicConstructor1Base : public AtomicConstructor<T> {

public: // methods

    AtomicConstructor1Base(const X1& x1) : x1_(x1) {}

    virtual void make (T& object) const {
        new (&object) T(x1_);
    }

    virtual size_t size() const { return AtomicConstructor<T>::size(); }
    virtual uint64_t type_id() const { return AtomicConstructor<T>::type_id(); }

protected: // members

    const X1& x1_;
};


template <typename T, typename X1>
class AtomicConstructor1 : public AtomicConstructor1Base<T, X1> {
public:
    AtomicConstructor1(const X1& x1) : AtomicConstructor1Base<T,X1>(x1) {}
};

// --


template <typename T, typename X1, typename X2>
class AtomicConstructor2Base : public AtomicConstructor<T> {

public: // methods

    AtomicConstructor2Base(const X1& x1, const X2& x2) : x1_(x1), x2_(x2) {}

    virtual void make (T& object) const {
        new (&object) T(x1_, x2_);
    }

    virtual size_t size() const { return AtomicConstructor<T>::size(); }
    virtual uint64_t type_id() const { return AtomicConstructor<T>::type_id(); }

protected: // members

    const X1& x1_;
    const X2& x2_;
};


template <typename T, typename X1, typename X2>
class AtomicConstructor2 : public AtomicConstructor2Base<T, X1, X2> {
public:
    AtomicConstructor2(const X1& x1, const X2& x2) : AtomicConstructor2Base<T,X1,X2>(x1, x2) {}
};

// -------------------------------------------------------------------------------------------------

} // namespace pmem

#endif // pmem_AtomicConstructor_H
