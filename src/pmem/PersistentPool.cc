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
#include <sys/stat.h>

#include "eckit/filesystem/PathName.h"
#include "eckit/log/Bytes.h"
#include "eckit/log/Log.h"

#include "pmem/AtomicConstructor.h"
#include "pmem/Exceptions.h"
#include "pmem/PersistentPool.h"
#include "pmem/PersistentPtr.h"

using namespace eckit;

namespace pmem {

// -------------------------------------------------------------------------------------------------

/// Open an existing persistent pool.
///
/// \param path The pool file to use
/// \param name The name to associate with the pool.
PersistentPool::PersistentPool(const eckit::PathName& path, const std::string& name) :
    path_(path),
    newPool_(false) {

    // Get the pool size from the system
    struct stat stat_buf;
    int rc = stat(path.asString().c_str(), &stat_buf);
    ASSERT(rc == 0);

    Log::info() << "Opening persistent pool: " << path << std::endl;
    Log::info() << "Pool size: " << Bytes(stat_buf.st_size) << std::endl;

    pool_ = ::pmemobj_open(path.localPath(), name.c_str());

    if (!pool_)
        throw PersistentOpenError(path, errno, Here());

    Log::info() << "Pool opened: " << pool_ << std::endl;
}

/// Constructs or opens a persistent pool
///
/// \param path The pool file to use
/// \param size The size in bytes to use if the pool is being created.
/// \param name The name to associate with the pool.
/// \param constructor Constructor object for the root node.
PersistentPool::PersistentPool(const eckit::PathName& path, const size_t size, const std::string& name,
                               const AtomicConstructorBase& constructor) :
    path_(path),
    newPool_(false) {

    Log::info() << "Creating persistent pool: " << path << std::endl;

    Log::info() << "Size: " << Bytes(size) << std::endl;

    // TODO: Consider permissions. Currently this is world R/W-able.
    pool_ = ::pmemobj_create(path.localPath(), name.c_str(), size, 0666);
    newPool_ = true;

    if (!pool_)
        throw PersistentCreateError(path, errno, Here());
    Log::info() << "Pool created: " << pool_ << std::endl;

    Log::info() << "Initialising root element" << std::endl;

    // The const cast in this expression is due to the interface to ::pmemobj_root_construct. This is immediately
    // recast to const AtomicConstructorBase* inside ::pmem_constructor.
    ::pmemobj_root_construct(pool_, constructor.size(), pmem_constructor,
                             const_cast<AtomicConstructorBase*>(&constructor));
}


PersistentPool::~PersistentPool() {

    // Note that we don`t need to close the pool if it has been nullified
    if (pool_)
        close();
}


void PersistentPool::close() {

    Log::info() << "Closing persistent pool." << std::endl;
    ASSERT(pool_);

    ::pmemobj_close(pool_);
    pool_ = 0;
}

void PersistentPool::remove() {

    close();

    Log::info() << "Deleting persistent pool: " << path_ << std::endl;
    ::remove(path_.asString().c_str());
}

// -------------------------------------------------------------------------------------------------

} // namespace pmem
