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

#include "ecbuild/boost_test_framework.h"

#include "pmem/PersistentPtr.h"

using namespace std;
using namespace pmem;
using namespace eckit;


// TODO:
// - Test &, *, get dereferences
// - Test mismatch behaviour of type_id
// - Check valid()
// - Check free()
// - Check pool explicit, and pool implicit, allocations
// - Check that cross-pool allocations are a thing
// - Check that we can access things cross-pools when allocated cross pools.
// - Check that allocate fails if not on persistent memory
// - Check that replace does what it says on the tin

//----------------------------------------------------------------------------------------------------------------------

class CustomType {

public: // constructor

    class Constructor : public AtomicConstructor<CustomType> {
        virtual void make(CustomType &object) const {
            object.data1_ = 1111;
            object.data2_ = 2222;
        }
    };

public: // members

    uint32_t data1_;
    uint32_t data2_;
};


// How many possibilities do we want?
const size_t root_elems = 3;


class RootType {

public: // constructor

    class Constructor : public AtomicConstructor<RootType> {
        virtual void make(RootType &object) const {
            for (size_t i = 0; i < root_elems; i++) {
                object.data_[i].nullify();
            }
        }
    };

public: // members

    PersistentPtr<CustomType> data_[root_elems];
};

//----------------------------------------------------------------------------------------------------------------------
//
//// And structure the pool with types
//
//template<> int pmem::PersistentPtr<RootType>::type_id = POBJ_ROOT_TYPE_NUM;
//template<> int pmem::PersistentPtr<CustomType>::type_id = 1;
//
//// Create a global fixture, so that this pool is only created once, and destroyed once.
//
//
//
//class SuitePoolFixture {
//
//    SuitePoolFixture() {}
//    ~SuitePoolFixture() {}
//};
//
//BOOST_GLOBAL_FIXTURE( SuitePoolFixture )

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE( test_pmem_persistent_ptr )

BOOST_AUTO_TEST_CASE( test_pmem_persistent_ptr_size )
{
    PersistentPtr<CustomType> ptr1;
    PersistentPtrBase ptr_base;

    // Check that everything has the right size, and that the PersistentPtr is only a wrapper around the underlying
    // object (with no further padding).

    BOOST_CHECK_EQUAL(sizeof(PersistentPtr<CustomType>), sizeof(PersistentPtrBase));
    BOOST_CHECK_EQUAL(sizeof(ptr1), sizeof(ptr_base));
    BOOST_CHECK_EQUAL(sizeof(ptr1), sizeof(PMEMoid));
    BOOST_CHECK_EQUAL(sizeof(ptr1), 16);
}

BOOST_AUTO_TEST_CASE( test_pmem_persistent_ptr_nullification )
{
    PersistentPtr<CustomType> ptr;

    // Forcibly make the pointer not null (note it is a wrapper around PMEMoid, so just cast it).

    reinterpret_cast<PMEMoid*>(&ptr)->off = 0xBAADBEEF;
    reinterpret_cast<PMEMoid*>(&ptr)->pool_uuid_lo = 0xBAADBEEF;

    BOOST_CHECK(!ptr.null());

    // Now nullify the pointer

    ptr.nullify();
    BOOST_CHECK(ptr.null());

    BOOST_CHECK_EQUAL(reinterpret_cast<PMEMoid*>(&ptr)->off, 0);
    BOOST_CHECK_EQUAL(reinterpret_cast<PMEMoid*>(&ptr)->pool_uuid_lo, 0);
}

BOOST_AUTO_TEST_CASE( test_pmem_persistent_ptr_equality )
{
    PersistentPtr<CustomType> ptr1;
    PersistentPtr<CustomType> ptr2;
    PersistentPtr<CustomType> ptr3;
    PersistentPtr<CustomType> ptr4;
    PersistentPtr<CustomType> ptr5;

    // Forcibly modify the elements of the PersistentPtrs to test all possible equality/inequality combinations.

    reinterpret_cast<PMEMoid*>(&ptr1)->off = 0xBAADBEEF;
    reinterpret_cast<PMEMoid*>(&ptr1)->pool_uuid_lo = 0xBAADBEEF;

    reinterpret_cast<PMEMoid*>(&ptr2)->off = 0xBAADBEEF;
    reinterpret_cast<PMEMoid*>(&ptr2)->pool_uuid_lo = 0xBAADBEEF;

    reinterpret_cast<PMEMoid*>(&ptr3)->off = 0xABABABAB;
    reinterpret_cast<PMEMoid*>(&ptr3)->pool_uuid_lo = 0xBAADBEEF;

    reinterpret_cast<PMEMoid*>(&ptr4)->off = 0xBAADBEEF;
    reinterpret_cast<PMEMoid*>(&ptr4)->pool_uuid_lo = 0xABABABAB;

    reinterpret_cast<PMEMoid*>(&ptr5)->off = 0xABABABAB;
    reinterpret_cast<PMEMoid*>(&ptr5)->pool_uuid_lo = 0xABABABAB;

    // And test the operators

    BOOST_CHECK_EQUAL(ptr1, ptr2);
    BOOST_CHECK(ptr1 == ptr2);
    BOOST_CHECK(!(ptr1 != ptr2));

    BOOST_CHECK(!(ptr1 == ptr3));
    BOOST_CHECK(ptr1 != ptr3);

    BOOST_CHECK(!(ptr1 == ptr4));
    BOOST_CHECK(ptr1 != ptr4);

    BOOST_CHECK(!(ptr1 == ptr5));
    BOOST_CHECK(ptr1 != ptr5);
}


//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
