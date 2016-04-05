/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#define BOOST_TEST_MODULE test_pmem

#include <string>

#include "ecbuild/boost_test_framework.h"

#include "eckit/filesystem/PathName.h"
#include "eckit/memory/NonCopyable.h"
#include "eckit/types/FixedString.h"

#include "pmem/AtomicConstructor.h"
#include "pmem/PersistentPtr.h"

using namespace std;
using namespace pmem;
using namespace eckit;


// TODO: Are we going to automatically clean up any file that are left accidentally?

//----------------------------------------------------------------------------------------------------------------------

/// A root type for testing purposes

class RootType : private eckit::NonCopyable {

public: // constructor

    /// Constructor functor for use with pmem atomic functions.
    class Constructor : public AtomicConstructor<RootType> {
        void make(RootType& object) const {
            object.tag_ = FixedString<12>("ROOT1234");
        }
    };

public: // accessor methods

    const FixedString<12>& tag() { return tag_; }

private: // members

    FixedString<12> tag_;
};


/// Declare a unique typeid associated with the type
template <> int pmem::PersistentPtr<RootType>::type_id = POBJ_ROOT_TYPE_NUM;

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

        PathName basepath(std::string(tmpdir) + "/pool");
        path_ = PathName::unique(basepath);
    }
    ~UniquePool() {}

    eckit::PathName path_;
};

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE( test_pmem_persistent_pool )

BOOST_AUTO_TEST_CASE( test_pmem_persistent_pool_create )
{
    UniquePool p;
    PersistentPool pool(p.path_, 1024 * 1024 * 20, "pool-name", RootType::Constructor());

    // Check that the file has been created
    BOOST_CHECK_EQUAL(::access(p.path_.asString().c_str(), R_OK | W_OK), 0);

    // Check that the file is correctly deleted when specified.
    pool.remove();
    BOOST_CHECK_EQUAL(::access(p.path_.asString().c_str(), F_OK), -1);
}



// Test can retrieve pool size
// Test can retrieve space available
// We can get hold of the root object, even if it is the wrong type?
// Determine new/old pool status

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
