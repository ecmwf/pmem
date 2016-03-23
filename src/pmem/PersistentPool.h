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

#include "pmem/AtomicConstructor.h"


// -------------------------------------------------------------------------------------------------

// Forward declarations
namespace eckit {
    class PathName;
}

// -------------------------------------------------------------------------------------------------

namespace pmem {

// Forward declarations
template <typename T> class PersistentPtr;


// -------------------------------------------------------------------------------------------------

class PersistentPool {

public: // methods

   PersistentPool(const eckit::PathName& path, const size_t size, const std::string& name,
                  AtomicConstructorBase& constructor, bool sizeSpecified=false);
   ~PersistentPool();

   // TODO: Keep track of open objects, so they can be invalidated?

   // Query the status of the pool

   const bool newPool() const;

protected: // methods

   // Get hold of the root object.
   template <typename T>
   PersistentPtr<T> getRoot() const;

protected: // members

    PMEMobjpool * pool_;

    bool newPool_;
};

// -------------------------------------------------------------------------------------------------

template <typename T>
PersistentPtr<T> PersistentPool::getRoot()  const {
    return PersistentPtr<T>(::pmemobj_root(pool_, sizeof(T)));
}

// -------------------------------------------------------------------------------------------------

} // namespace pmem

#endif // pmem_PersistentPool_H
