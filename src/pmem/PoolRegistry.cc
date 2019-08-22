/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/*
 * This software was developed as part of the EC H2020 funded project NextGenIO
 * (Project ID: 671951) www.nextgenio.eu
 */

/// @author Simon Smart
/// @date   Feb 2017


#include "pmem/PoolRegistry.h"
#include "pmem/PersistentPool.h"
#include "pmem/LibPMem.h"

#include "eckit/exception/Exceptions.h"

#include <mutex>

using namespace eckit;


namespace pmem {

namespace {
std::mutex registryMutex;
}


// -------------------------------------------------------------------------------------------------

PoolRegistry::PoolRegistry() {}

PoolRegistry::~PoolRegistry() {}


PoolRegistry& PoolRegistry::instance() {

    static PoolRegistry registry_singleton;

    return registry_singleton;
}


void PoolRegistry::registerPool(PersistentPool& pool) {

    std::lock_guard<std::mutex> lock(registryMutex);

    PMEMobjpool* handle = pool.raw_pool();

    Log::debug<LibPMem>() << "Registering pool (" << pool.path() << "): " << handle << std::endl;

    // We are registering a pool. It needs to not already exist...
    ASSERT(pools_.find(handle) == pools_.end());

    pools_.insert(std::make_pair(handle, &pool));
}


void PoolRegistry::deregisterPool(PersistentPool& pool) {

    std::lock_guard<std::mutex> lock(registryMutex);

    PMEMobjpool* handle = pool.raw_pool();

    Log::debug<LibPMem>() << "Deregistering pool (" << pool.path() << "): " << handle << std::endl;

    // We are registering a pool. It needs to not already exist...
    ASSERT(pools_.find(handle) != pools_.end());

    pools_.erase(handle);
}


PersistentPool& PoolRegistry::poolFromPointer(void* ptr) {

    std::lock_guard<std::mutex> lock(registryMutex);

    PMEMobjpool* handle = ::pmemobj_pool_by_ptr(ptr);

    if (handle == 0)
        throw SeriousBug("Requested pointer not in mapped pmem space", Here());

    ASSERT(pools_.find(handle) != pools_.end());

    PersistentPool* pool = pools_[handle];
    return *pool;
}

// -------------------------------------------------------------------------------------------------


} // namespace pmem
