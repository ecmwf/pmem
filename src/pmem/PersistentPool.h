/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016


#ifndef pmem_PersistentPool_H
#define pmem_PersistentPool_H

#include <cstddef>
#include <string>

#include "libpmemobj.h"

#include "eckit/filesystem/PathName.h"
#include "eckit/memory/NonCopyable.h"

#include "pmem/AtomicConstructor.h"


//----------------------------------------------------------------------------------------------------------------------

namespace pmem {

// Forward declarations
template <typename T> class PersistentPtr;

//----------------------------------------------------------------------------------------------------------------------

class PersistentPool : private eckit::NonCopyable {

public: // methods

    /// Open existing persistent pool
    PersistentPool(const eckit::PathName& path, const std::string& name);

    /// Create new persistent pool
    PersistentPool(const eckit::PathName& path, const size_t size, const std::string& name,
                   const AtomicConstructorBase& constructor);

    ~PersistentPool();

    /// Close the pool
    void close();

    /// Delete the pool file, removing all traces to the pool
    /// @note This permanently invalidates any access, and any pointers/PersistentPtrs.
    void remove();

    /// What is the on-disk size of this pool?
    size_t size() const;

    /// Is this pool new?
    bool newPool() const;

    /// Get hold of the root object.
    template <typename T>
    PersistentPtr<T> getRoot() const;

    /// Get hold of the raw libpmemobj pool
    PMEMobjpool * raw_pool() const;

    /// Find the pool again!
    const eckit::PathName& path() const;

    /// Obtain the UUID of the pool for later identification
    uint64_t uuid() const;

protected: // members

    eckit::PathName path_;

    PMEMobjpool * pool_;

    bool newPool_;

    size_t size_;
};

//----------------------------------------------------------------------------------------------------------------------

/// Get the root object. In any derived classes, this should be explicitly overridden as a non-template method.
/// (So the user doesn't need to provide the template parameters).

template <typename T>
PersistentPtr<T> PersistentPool::getRoot()  const {
    return PersistentPtr<T>(::pmemobj_root(pool_, sizeof(T)));
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace pmem

#endif // pmem_PersistentPool_H
