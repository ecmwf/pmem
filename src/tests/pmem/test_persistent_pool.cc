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
#include "eckit/testing/Setup.h"
#include "eckit/types/FixedString.h"

#include "pmem/PersistentPtr.h"
#include "pmem/Exceptions.h"

#include "test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;
using namespace eckit::testing;

BOOST_GLOBAL_FIXTURE(Setup)

//----------------------------------------------------------------------------------------------------------------------

/// A root type for testing purposes

//class RootType : private eckit::NonCopyable {
class RootType : public PersistentType<RootType>
               , private eckit::NonCopyable {

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
template <> uint64_t pmem::PersistentType<RootType>::type_id = POBJ_ROOT_TYPE_NUM;

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE( test_pmem_persistent_pool )

BOOST_AUTO_TEST_CASE( test_pmem_persistent_pool_create )
{
    UniquePool p;
    PersistentPool pool(p.path_, 1024 * 1024 * 20, "pool-name", RootType::Constructor());

    // Check that the file has been created
    BOOST_CHECK_EQUAL(::access(p.path_.asString().c_str(), R_OK | W_OK), 0);

    BOOST_CHECK(pool.newPool());

    // Check that the file is correctly deleted when specified.
    pool.remove();
    BOOST_CHECK_EQUAL(::access(p.path_.asString().c_str(), F_OK), -1);
}


BOOST_AUTO_TEST_CASE( test_pmem_persistent_pool_init_root )
{
    AutoPool ap((RootType::Constructor()));

    PersistentPtr<RootType> root = ap.pool_.getRoot<RootType>();
    BOOST_CHECK_EQUAL(root->tag(), "ROOT1234");

    // Check that all the pool references add up

    BOOST_CHECK_EQUAL(ap.pool_.raw_pool(), ::pmemobj_pool_by_ptr(root.get()));
}


//BOOST_AUTO_TEST_CASE( test_pmem_persistent_pool_open )
//{
//    UniquePool p;
//
//    // Create and close the pool
//    PersistentPool pool(p.path_, 1024 * 1024 * 20, "pool-name", RootType::Constructor());
//    BOOST_CHECK(pool.newPool());
//    pool.close();
//
//    // Reopen the pool
//    PersistentPool pool2(p.path_, "pool-name");
//
//    // Check that this is not a new pool (i.e. it is opened)
//    BOOST_CHECK(!pool2.newPool());
//
//    // Check that we have got a pool of the correct size, with a correctly initialised root.
//    BOOST_CHECK_EQUAL(pool2.size(), 1024 * 1024 * 20);
//
//    PersistentPtr<RootType> root = pool2.getRoot<RootType>();
//    BOOST_CHECK_EQUAL(root->tag(), "ROOT1234");
//
//    // And cleanup after ourselves
//    pool2.remove();
//}
//
//
BOOST_AUTO_TEST_CASE( test_pmem_persistent_pool_check_pool_name )
{
    AutoPool ap((RootType::Constructor()));

    // Functor to make constructor amenable to BOOST_CHECK_THROW
    struct open_fail {
        void operator()(const PathName& path, const std::string& name) {
            PersistentPool opened_pool(path, name);
        }
    };

    // Attempt to open pool again...
    BOOST_CHECK_THROW(open_fail()(ap.path_, "WRONG-NAME"), PersistentOpenError);
}

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
