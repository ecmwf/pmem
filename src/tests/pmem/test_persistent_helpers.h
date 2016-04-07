/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Simon Smart
/// @date   April 2016

#ifndef pmem_test_persistent_helpers_h
#define pmem_test_persistent_helpers_h

#include "eckit/filesystem/PathName.h"

#include "pmem/AtomicConstructor.h"
#include "pmem/PersistentPool.h"

//----------------------------------------------------------------------------------------------------------------------

/// A fixture to obtain a unique name for this pool.

struct UniquePool {

    UniquePool() :
        path_("") {

        const char* tmpdir = ::getenv("TMPDIR");
        if (!tmpdir)
            tmpdir = ::getenv("SCRATCHDIR");
        if (!tmpdir)
            tmpdir = "/tmp";

        eckit::PathName basepath(std::string(tmpdir) + "/pool");
        path_ = eckit::PathName::unique(basepath);
    }
    ~UniquePool() {}

    eckit::PathName path_;
};


/// Some standard constants to assist testing

const size_t auto_pool_size = 1024 * 1024 * 20;
const std::string auto_pool_name = "pool-name";

/// A structure to automatically create and clean up a pool (used except in the tests where this
/// functionality is being directly tested

struct AutoPool {

    AutoPool(const pmem::AtomicConstructorBase& constructor) :
        path_(UniquePool().path_),
        pool_(path_, auto_pool_size, auto_pool_name, constructor) {}
    ~AutoPool() { pool_.remove(); }

    eckit::PathName path_;
    pmem::PersistentPool pool_;
};

//----------------------------------------------------------------------------------------------------------------------

#endif // pmem_test_persistent_helpers_h
