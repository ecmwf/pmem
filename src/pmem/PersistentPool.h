/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016

#include "libpmemobj.h"

#include <cstddef>
#include <string>

#ifndef pmem_PersistentPool_H
#define pmem_PersistentPool_H

#include "persistent/AtomicConstructor.h"


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
                  AtomicConstructorBase& constructor);
   ~PersistentPool();

   // TODO: Keep track of open objects, so they can be invalidated?

   // Query the status of the pool

   const bool newPool() const;

   /// Obtain the cached value for the pool's UUID.
   static uint64_t poolUUID(PMEMobjpool*);

   void setUUID(uint64_t uuid) const;

protected: // methods

   // Get hold of the root object.
   template <typename T>
   PersistentPtr<T> getRoot() const;

protected: // members

    PMEMobjpool * pop_;

    bool newPool_;
};

// -------------------------------------------------------------------------------------------------

template <typename T>
PersistentPtr<T> PersistentPool::getRoot()  const {
    PersistentPtr<T> tmp = PersistentPtr<T>(::pmemobj_root(pop_, sizeof(T)));
    setUUID(tmp.oid_.pool_uuid_lo);
    return tmp;
}

// -------------------------------------------------------------------------------------------------

} // namespace pmem

#endif // pmem_PersistentPool_H
