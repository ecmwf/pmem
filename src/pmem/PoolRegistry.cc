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
/// @date   Feb 2017


#include "pmem/PoolRegistry.h"
#include "pmem/PersistentPool.h"

#include "eckit/exception/Exceptions.h"

using namespace eckit;


namespace pmem {


// -------------------------------------------------------------------------------------------------

PoolRegistry::PoolRegistry() {}

PoolRegistry::~PoolRegistry() {}


PoolRegistry& PoolRegistry::instance() {

    static PoolRegistry registry_singleton;

    return registry_singleton;
}


void PoolRegistry::registerPool(PersistentPool& pool) {

    PMEMobjpool* handle = pool.raw_pool();

    // We are registering a pool. It needs to not already exist...
    ASSERT(pools_.find(handle) == pools_.end());

    pools_.insert(std::make_pair(handle, &pool));
}


PersistentPool& PoolRegistry::poolFromPointer(void* ptr) {

    PMEMobjpool* handle = ::pmemobj_pool_by_ptr(ptr);

    if (handle == 0)
        throw SeriousBug("Requested pointer not in mapped pmem space", Here());

    ASSERT(pools_.find(handle) != pools_.end());

    PersistentPool* pool = pools_[handle];
    return *pool;
}

// -------------------------------------------------------------------------------------------------


} // namespace pmem
