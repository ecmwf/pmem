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

/// Constructs or opens a persistent pool
///
/// \param path The pool file to use
/// \param size The size in bytes to use if the pool is being created.
/// \param name The name to associate with the pool.
/// \param constructor Constructor object for the root node.
/// \param sizeSpecified true if the size was manually specified (not default). [default = false]
PersistentPool::PersistentPool(const eckit::PathName& path, const size_t size, const std::string& name,
                               AtomicConstructorBase& constructor, bool sizeSpecified) :
    newPool_(false) {

    // If the persistent pool already exists, then open it. Otherwise create it.
    if (::access(path.localPath(), F_OK) == -1) {

        Log::info() << "Creating persistent pool: " << path << std::endl;

        Log::info() << "Size: " << Bytes(size) << std::endl;

        // TODO: Consider permissions. Currently this is world R/W-able.
        pool_ = ::pmemobj_create(path.localPath(), name.c_str(), size, 0666);
        newPool_ = true;

        if (!pool_)
            throw PersistentCreateError(path, errno, Here());
        Log::info() << "Pool created: " << pool_ << std::endl;

    } else {

        // Get the pool size from the system
        struct stat stat_buf;
        int rc = stat(path.asString().c_str(), &stat_buf);
        ASSERT(rc == 0);

        Log::info() << "Opening persistent pool: " << path << std::endl;
        Log::info() << "Pool size: " << Bytes(stat_buf.st_size) << std::endl;
        if (stat_buf.st_size != size_t(size) && sizeSpecified)
            Log::warning() << "WARNING: Pool size does not match the manually specified size" << std::endl;

        pool_ = ::pmemobj_open(path.localPath(), name.c_str());

        if (!pool_)
            throw PersistentOpenError(path, errno, Here());

        Log::info() << "Pool opened: " << pool_ << std::endl;

    }

    if (::pmemobj_root_size(pool_) == 0) {
        Log::info() << "Initialising root element" << std::endl;
        ::pmemobj_root_construct(pool_, constructor.size(), pmem_constructor, &constructor);
    }
}


PersistentPool::~PersistentPool() {}

// -------------------------------------------------------------------------------------------------

} // namespace pmem
