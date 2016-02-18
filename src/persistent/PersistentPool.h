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

#ifndef persistent_PersistentPtr_H
#define persistent_PersistentPtr_H


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
public:

   PersistentPool(const eckit::PathName& path, const size_t size, const std::string& name);
   ~PersistentPool();

   // Get hold of the root object.
   template <typename T>
   PersistentPtr<T> getRoot();

   // TODO: Keep track of open objects, so they can be invalidated?

   // Query the status of the pool

   const bool newPool() const;

private:

    PMEMobjpool * pop_;

    bool newPool_;
};

// -------------------------------------------------------------------------------------------------

} // namespace pmem

#endif // persistent_PersistentPtr_H
