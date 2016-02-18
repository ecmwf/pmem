/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016


#include "persistent/PersistentPool.h"


#include "eckit/filesystem/PathName.h"

#include <unistd.h>

namespace pmem {

// -------------------------------------------------------------------------------------------------

PersistentPool::PersistentPool(const eckit::PathName& path, const size_t size, const std::string& name) {

    // If the persistent pool already exists, then open it. Otherwise create it.
    if (::access(path.localPath(), F_OK) == -1) {

        // TODO: Consider permissions. Currently this is world R/W-able.
        pop_ = ::pmemobj_create(path.localPath(), name.c_str(), size, 0666);
        newPool_ = true;

    } else {

        pop_ = ::pmemobj_open(path.localPath(), name.c_str());
        newPool_ = false;

    }

    if (!pop_) {

    }
}


PersistentPool::~PersistentPool() {}


// -------------------------------------------------------------------------------------------------

} // namespace pmem
