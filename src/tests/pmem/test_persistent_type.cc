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

#include <fcntl.h>

#include "eckit/testing/Test.h"

#include "pmem/PersistentType.h"
#include "pmem/PersistentPtr.h"

#include "test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;
using namespace eckit::testing;

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

CASE( "test_pmem_persistent_type_size" )
{
    PersistentPtr<PersistentType<CustomType> > ptr1;
    PersistentPtrBase ptr_base;

    // Check that everything has the right size, and that the PersistentPtr is only a wrapper around the underlying
    // object (with no further padding).

    EXPECT(sizeof(PersistentPtr<PersistentType<CustomType> >) == sizeof(PersistentPtrBase));
    EXPECT(sizeof(ptr1) == sizeof(ptr_base));
    EXPECT(sizeof(ptr1) == sizeof(PMEMoid));
    EXPECT(sizeof(ptr1) == size_t(16));
}


//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
