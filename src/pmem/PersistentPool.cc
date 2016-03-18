/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016

#include <unistd.h>

#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"

#include "pmem/AtomicConstructor.h"
#include "pmem/Exceptions.h"
#include "pmem/PersistentPool.h"
#include "pmem/PersistentPtr.h"

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
        pool_ = ::pmemobj_create(path.localPath(), name.c_str(), size, 0666);
        newPool_ = true;

        if (!pool_)
            throw PersistentCreateError(path, errno, Here());
        Log::info() << "Pool created: " << pool_ << std::endl;

    } else {

        Log::info() << "Opening persistent pool: " << path << std::endl;

        pool_ = ::pmemobj_open(path.localPath(), name.c_str());

        if (!pool_)
            throw PersistentOpenError(path, errno, Here());

        Log::info() << "Pool opened: " << pool_ << std::endl;

    }

    if (::pmemobj_root_size(pool_) == 0) {
        Log::info() << "Initialising root element" << std::endl << std::flush;
        ::pmemobj_root_construct(pool_, constructor.size(), pmem_constructor, &constructor);
    }
}


PersistentPool::~PersistentPool() {}


uint64_t PersistentPool::poolUUID(PMEMobjpool * pool) {

    std::map<PMEMobjpool*, uint64_t>::const_iterator uuid = pool_uuid_map.find(pool);

    if (uuid == pool_uuid_map.end())
        throw SeriousBug("Attempting to obtain UUID of unknown pool");

    return uuid->second;
}


void PersistentPool::setUUID(uint64_t uuid) const {

    std::map<PMEMobjpool*, uint64_t>::const_iterator it_uuid = pool_uuid_map.find(pool_);

    if (it_uuid == pool_uuid_map.end()) {
        pool_uuid_map[pool_] = uuid;
    } else {
        if (uuid != it_uuid->second)
            throw SeriousBug("Two identical pools registered with differing UUIDs");
    }
}


// -------------------------------------------------------------------------------------------------

} // namespace pmem
