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

namespace {
    std::map<PMEMobjpool*, uint64_t> pool_uuid_map;
}

namespace pmem {

// -------------------------------------------------------------------------------------------------

PersistentPool::PersistentPool(const eckit::PathName& path, const size_t size, const std::string& name,
                               AtomicConstructorBase& constructor) :
    newPool_(false) {

    // If the persistent pool already exists, then open it. Otherwise create it.
    if (::access(path.localPath(), F_OK) == -1) {

        Log::info() << "Creating persistent pool: " << path << std::endl;

        Log::info() << "Size: " << float(size) / (1024 * 1024) << std::endl << std::flush;

        // TODO: Consider permissions. Currently this is world R/W-able.
        pop_ = ::pmemobj_create(path.localPath(), name.c_str(), size, 0666);
        newPool_ = true;

        if (!pop_)
            throw PersistentCreateError(path, errno, Here());
        Log::info() << "Pool created: " << pop_ << std::endl;

    } else {

        Log::info() << "Opening persistent pool: " << path << std::endl;

        pop_ = ::pmemobj_open(path.localPath(), name.c_str());

        if (!pop_)
            throw PersistentOpenError(path, errno, Here());

        Log::info() << "Pool opened: " << pop_ << std::endl;

    }

    if (::pmemobj_root_size(pop_) == 0) {
        Log::info() << "Initialising root element" << std::endl << std::flush;
        ::pmemobj_root_construct(pop_, constructor.size(), persistentConstructor, &constructor);
    }
}


PersistentPool::~PersistentPool() {}


uint64_t PersistentPool::poolUUID(PMEMobjpool * pop) {

    std::map<PMEMobjpool*, uint64_t>::const_iterator uuid = pool_uuid_map.find(pop);

    if (uuid == pool_uuid_map.end())
        throw SeriousBug("Attempting to obtain UUID of unknown pool");

    return uuid->second;
}


void PersistentPool::setUUID(uint64_t uuid) const {

    std::map<PMEMobjpool*, uint64_t>::const_iterator it_uuid = pool_uuid_map.find(pop_);

    if (it_uuid == pool_uuid_map.end()) {
        pool_uuid_map[pop_] = uuid;
    } else {
        if (uuid != it_uuid->second)
            throw SeriousBug("Two identical pools registered with differing UUIDs");
    }
}


// -------------------------------------------------------------------------------------------------

} // namespace pmem
