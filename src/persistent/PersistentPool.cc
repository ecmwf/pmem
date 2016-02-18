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
#include "persistent/Exceptions.h"
#include "persistent/AtomicConstructor.h"
#include "persistent/PersistentPtr.h"

#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"

#include <unistd.h>

using namespace eckit;

namespace pmem {

// -------------------------------------------------------------------------------------------------

PersistentPool::PersistentPool(const eckit::PathName& path, const size_t size, const std::string& name,
                               AtomicConstructorBase& constructor) :
    newPool_(false) {

    // If the persistent pool already exists, then open it. Otherwise create it.
    if (::access(path.localPath(), F_OK) == -1) {

        Log::info() << "Creating persistent pool: " << path << std::endl;

        // TODO: Consider permissions. Currently this is world R/W-able.
        pop_ = ::pmemobj_create(path.localPath(), name.c_str(), size, 0666);
        newPool_ = true;

        Log::info() << "Root size: " << ::pmemobj_root_size(pop_) << std::endl;


        if (!pop_)
            throw PersistentCreateError(path, errno, Here());

        // We must initialise the root object without explicitly knowing its type

    } else {

        Log::info() << "Opening persistent pool: " << path << std::endl;

        pop_ = ::pmemobj_open(path.localPath(), name.c_str());

        if (!pop_)
            throw PersistentOpenError(path, errno, Here());

    }

    if (::pmemobj_root_size(pop_) == 0) {
        Log::info() << "Initialising root element" << std::endl;
        ::pmemobj_root_construct(pop_, constructor.size(), persistentConstructor, &constructor);
    }
}


PersistentPool::~PersistentPool() {}


// -------------------------------------------------------------------------------------------------

} // namespace pmem
