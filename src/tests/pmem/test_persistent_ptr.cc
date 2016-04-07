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

#include <fcntl.h>

#include "ecbuild/boost_test_framework.h"

#include "pmem/PersistentPtr.h"

#include "test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;


// TODO:
// - Test mismatch behaviour of type_id
// - Check valid()
// - Check free()
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

// And structure the pool with types

template<> int pmem::PersistentPtr<RootType>::type_id = POBJ_ROOT_TYPE_NUM;
template<> int pmem::PersistentPtr<CustomType>::type_id = 1;

// Create a global fixture, so that this pool is only created once, and destroyed once.

PersistentPtr<RootType> global_root;

struct SuitePoolFixture {

    SuitePoolFixture() : autoPool_(RootType::Constructor()) {
        Log::info() << "Opening global pool" << std::endl;
        global_root = autoPool_.pool_.getRoot<RootType>();
    }
    ~SuitePoolFixture() {
        global_root.nullify();
    }

    AutoPool autoPool_;
};

BOOST_GLOBAL_FIXTURE( SuitePoolFixture )

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


BOOST_AUTO_TEST_CASE( test_pmem_persistent_ptr_allocate_needs_pmem )
{
    PersistentPtr<CustomType> ptr;

    CustomType::Constructor ctr;

    BOOST_CHECK_THROW(ptr.allocate(ctr), SeriousBug);
}


BOOST_AUTO_TEST_CASE( test_pmem_persistent_ptr_direct_allocate )
{
    BOOST_CHECK(global_root->data_[0].null());

    CustomType::Constructor ctr;
    global_root->data_[0].allocate(ctr);

    BOOST_CHECK(!global_root->data_[0].null());
    BOOST_CHECK(global_root->data_[0].valid());

    // Check that persistent pointers in volatile vs persistent space are the same

    PersistentPtr<CustomType> p1 = global_root->data_[0];

    BOOST_CHECK(!global_root->data_[0].null());
    BOOST_CHECK(global_root->data_[0].valid());
    BOOST_CHECK(p1 == global_root->data_[0]);
    BOOST_CHECK(!(p1 != global_root->data_[0]));

    // Check that we can dereference this PersistentPtr to give access to the memory mapped region

    BOOST_CHECK_EQUAL(p1->data1_, 1111);
    BOOST_CHECK_EQUAL(p1->data2_, 2222);

    const CustomType* pelem = p1.get();

    BOOST_CHECK_EQUAL(pelem->data1_, 1111);
    BOOST_CHECK_EQUAL(pelem->data2_, 2222);

    const CustomType& relem(*p1);

    BOOST_CHECK_EQUAL(relem.data1_, 1111);
    BOOST_CHECK_EQUAL(relem.data2_, 2222);

    BOOST_CHECK_EQUAL(&relem, pelem);

    // Test that this pointer is in persistent memory, and is in the same pool as the global root

    PMEMobjpool* root_pool = ::pmemobj_pool_by_ptr(global_root.get());
    PMEMobjpool* elem_pool = ::pmemobj_pool_by_ptr(pelem);

    BOOST_CHECK(pelem != (void*)global_root.get());
    BOOST_CHECK(root_pool != 0);
    BOOST_CHECK(elem_pool != 0);
    BOOST_CHECK_EQUAL(root_pool, elem_pool);
}


BOOST_AUTO_TEST_CASE( test_pemem_persistent_ptr_cross_pool )
{
    // Open a second pool

    UniquePool uniq;
    PersistentPool pool2(uniq.path_, auto_pool_size, "second-pool", RootType::Constructor());

    // Explicitly allocate an object in the second pool

    BOOST_CHECK(global_root->data_[1].null());

    CustomType::Constructor ctr;
    global_root->data_[1].allocate(pool2, ctr);

    PersistentPtr<CustomType> p1 = global_root->data_[1];
    BOOST_CHECK(!p1.null());
    BOOST_CHECK_EQUAL(p1->data1_, 1111);
    BOOST_CHECK_EQUAL(p1->data2_, 2222);

    CustomType * volatile_p1 = p1.get();

    // Check that this pointer is not in the same pool!

    PMEMobjpool * raw_pool1 = ::pmemobj_pool_by_ptr(global_root.get());
    PMEMobjpool * raw_pool2 = ::pmemobj_pool_by_ptr(pool2.getRoot<RootType>().get());
    PMEMobjpool * raw_pool_p1 = ::pmemobj_pool_by_ptr(volatile_p1);

    BOOST_CHECK(raw_pool1 != raw_pool2);
    BOOST_CHECK(raw_pool_p1 == raw_pool2);

    // Close the pool. OH NO!

    pool2.close();

    // Memory access to anything pointing to pool2 is now invalid.
    // p1->data1_  gives a memory access violation.

    // Open an unused pool. This should enforce that when we reopen pool2, it gets mapped to a different place
    // in memory

    AutoPool ap((RootType::Constructor()));

    // Reopen the pool. It will be mapped in a different place.

    PersistentPool pool2a(uniq.path_, "second-pool");

    PMEMobjpool * raw_pool2a = ::pmemobj_pool_by_ptr(pool2a.getRoot<RootType>().get());

    BOOST_CHECK(raw_pool2a != raw_pool2);
    BOOST_CHECK(raw_pool2a != raw_pool1);

    // Check that the pointer now maps to the new region!

    CustomType * volatile_p2 = p1.get();
    PMEMobjpool * raw_pool_p2 = ::pmemobj_pool_by_ptr(volatile_p2);

    BOOST_CHECK(volatile_p1 != volatile_p2);
    BOOST_CHECK(raw_pool_p2 == raw_pool2a);
    BOOST_CHECK(raw_pool_p2 != raw_pool2);

    BOOST_CHECK_EQUAL(p1->data1_, 1111);
    BOOST_CHECK_EQUAL(p1->data2_, 2222);
    BOOST_CHECK_EQUAL(volatile_p2->data1_, 1111);
    BOOST_CHECK_EQUAL(volatile_p2->data2_, 2222);

    // And clean everything up

    pool2a.remove();
}


//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
