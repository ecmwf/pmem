/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/*
 * This software was developed as part of the EC H2020 funded project NextGenIO
 * (Project ID: 671951) www.nextgenio.eu
 */

#define BOOST_TEST_MODULE test_pmem

#include <fcntl.h>

#include "ecbuild/boost_test_framework.h"

#include "eckit/testing/Setup.h"

#include "pmem/PersistentType.h"
#include "pmem/PersistentPtr.h"

#include "test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;
using namespace eckit::testing;

BOOST_GLOBAL_FIXTURE(Setup);

//----------------------------------------------------------------------------------------------------------------------

/// A custom type to allocate objects in the tests

class CustomType {
public: // members

    uint32_t data1_;
    uint32_t data2_;
};


class OtherType {
public: // members
    uint64_t other1_;
    uint64_t other2_;
};

//----------------------------------------------------------------------------------------------------------------------

// And structure the pool with types

template<> uint64_t pmem::PersistentType<CustomType>::type_id = 1;
template<> uint64_t pmem::PersistentType<OtherType>::type_id = 2;


//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE( test_pmem_persistent_type )

BOOST_AUTO_TEST_CASE( test_pmem_persistent_type_size )
{
    PersistentPtr<PersistentType<CustomType> > ptr1;
    PersistentPtrBase ptr_base;

    // Check that everything has the right size, and that the PersistentPtr is only a wrapper around the underlying
    // object (with no further padding).

    BOOST_CHECK_EQUAL(sizeof(PersistentPtr<PersistentType<CustomType> >), sizeof(PersistentPtrBase));
    BOOST_CHECK_EQUAL(sizeof(ptr1), sizeof(ptr_base));
    BOOST_CHECK_EQUAL(sizeof(ptr1), sizeof(PMEMoid));
    BOOST_CHECK_EQUAL(sizeof(ptr1), size_t(16));
}


//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
