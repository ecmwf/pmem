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

#include "eckit/testing/Setup.h"

#include "pmem/PersistentPtr.h"

#include "test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;
using namespace eckit::testing;

BOOST_GLOBAL_FIXTURE(Setup)

//----------------------------------------------------------------------------------------------------------------------

/// A custom type to allocate objects in the tests

class CustomType : public PersistentType<CustomType> {

public: // constructor

    class Constructor : public AtomicConstructor<CustomType> {
        virtual void make(CustomType &object) const {
            object.data1_ = 1111;
            object.data2_ = 2222;
        }
    };

    CustomType(const uint32_t elem) :
        data1_(elem), data2_(elem) {}

public: // members

    uint32_t data1_;
    uint32_t data2_;
};

/// A different type to play the devils advocate

class OtherType : public PersistentType<OtherType> {
public: // members
    uint32_t other1_;
    uint32_t other2_;
};

/// Define a root type. Each test that does allocation should use a different element in the root object.

// How many possibilities do we want?
const size_t root_elems = 4;


class RootType : public PersistentType<RootType> {

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

template<> uint64_t pmem::PersistentType<RootType>::type_id = POBJ_ROOT_TYPE_NUM;
template<> uint64_t pmem::PersistentType<CustomType>::type_id = 1;
template<> uint64_t pmem::PersistentType<OtherType>::type_id = 2;

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

    BOOST_CHECK_THROW(ptr.allocate_ctr(ctr), SeriousBug);
}


BOOST_AUTO_TEST_CASE( test_pmem_persistent_ptr_direct_allocate_ctr )
{
    BOOST_CHECK(global_root->data_[3].null());

    CustomType::Constructor ctr;
    global_root->data_[3].allocate_ctr(ctr);

    BOOST_CHECK(!global_root->data_[3].null());
    BOOST_CHECK(global_root->data_[3].valid());

    // Check that persistent pointers in volatile vs persistent space are the same

    PersistentPtr<CustomType> p1 = global_root->data_[3];

    BOOST_CHECK(!global_root->data_[3].null());
    BOOST_CHECK(global_root->data_[3].valid());
    BOOST_CHECK(p1 == global_root->data_[3]);
    BOOST_CHECK(!(p1 != global_root->data_[3]));

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

    // Check that replace also works

    global_root->data_[3].replace_ctr(ctr);

    PersistentPtr<CustomType> p2 = global_root->data_[3];

    BOOST_CHECK(p1 != p2);
    BOOST_CHECK_EQUAL(p2->data1_, 1111);
    BOOST_CHECK_EQUAL(p2->data2_, 2222);
}


BOOST_AUTO_TEST_CASE( test_pmem_persistent_ptr_direct_allocate )
{
    BOOST_CHECK(global_root->data_[0].null());

    global_root->data_[0].allocate(999);

    BOOST_CHECK(!global_root->data_[0].null());
    BOOST_CHECK(global_root->data_[0].valid());

    // Check that persistent pointers in volatile vs persistent space are the same

    PersistentPtr<CustomType> p1 = global_root->data_[0];

    BOOST_CHECK(!global_root->data_[0].null());
    BOOST_CHECK(global_root->data_[0].valid());
    BOOST_CHECK(p1 == global_root->data_[0]);
    BOOST_CHECK(!(p1 != global_root->data_[0]));

    // Check that we can dereference this PersistentPtr to give access to the memory mapped region

    BOOST_CHECK_EQUAL(p1->data1_, 999);
    BOOST_CHECK_EQUAL(p1->data2_, 999);

    const CustomType* pelem = p1.get();

    BOOST_CHECK_EQUAL(pelem->data1_, 999);
    BOOST_CHECK_EQUAL(pelem->data2_, 999);

    const CustomType& relem(*p1);

    BOOST_CHECK_EQUAL(relem.data1_, 999);
    BOOST_CHECK_EQUAL(relem.data2_, 999);

    BOOST_CHECK_EQUAL(&relem, pelem);

    // Test that this pointer is in persistent memory, and is in the same pool as the global root

    PMEMobjpool* root_pool = ::pmemobj_pool_by_ptr(global_root.get());
    PMEMobjpool* elem_pool = ::pmemobj_pool_by_ptr(pelem);

    BOOST_CHECK(pelem != (void*)global_root.get());
    BOOST_CHECK(root_pool != 0);
    BOOST_CHECK(elem_pool != 0);
    BOOST_CHECK_EQUAL(root_pool, elem_pool);

    // Check that replace also works

    global_root->data_[0].replace(888);

    PersistentPtr<CustomType> p2 = global_root->data_[0];

    BOOST_CHECK(p1 != p2);
    BOOST_CHECK_EQUAL(p2->data1_, 888);
    BOOST_CHECK_EQUAL(p2->data2_, 888);
}


BOOST_AUTO_TEST_CASE( test_pemem_persistent_ptr_cross_pool )
{
    // Open a second pool

    UniquePool uniq;
    PersistentPool pool2(uniq.path_, auto_pool_size, "second-pool", RootType::Constructor());

    // Explicitly allocate an object in the second pool

    BOOST_CHECK(global_root->data_[1].null());

    CustomType::Constructor ctr;
    global_root->data_[1].allocate_ctr(pool2, ctr);

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

    // And check an explicit replace

    global_root->data_[1].replace_ctr(pool2a, ctr);

    PersistentPtr<CustomType> p2 = global_root->data_[1];

    BOOST_CHECK(p1 != p2);
    BOOST_CHECK_EQUAL(p2->data1_, 1111);
    BOOST_CHECK_EQUAL(p2->data2_, 2222);
    BOOST_CHECK_EQUAL(raw_pool2a, ::pmemobj_pool_by_ptr(p2.get()));

    // And clean everything up

    pool2a.remove();
}


BOOST_AUTO_TEST_CASE( test_pmem_persistent_ptr_typeid )
{
    BOOST_CHECK(global_root->data_[2].null());

    CustomType::Constructor ctr;
    global_root->data_[2].allocate_ctr(ctr);

    PersistentPtr<CustomType> p = global_root->data_[2];

    BOOST_CHECK(!p.null());
    BOOST_CHECK(p.valid());

    // Check the type_id explicitly

    BOOST_CHECK_EQUAL(PersistentType<CustomType>::type_id, ::pmemobj_type_num(p.raw()));

    // Cast the PersistentPtr to a different type. Now we can see things go wrong.

    /// @note This line will throw a strict aliasing warning when compiled with optimisations on. That is OK.
    ///       It is (correctly) reporting that we would incur problems if we tried to do this for real...

    PersistentPtr<OtherType> pother = *reinterpret_cast<PersistentPtr<OtherType>*>(&p);

    BOOST_CHECK(!pother.null());
    BOOST_CHECK(!pother.valid()); // @note No longer valid. Fails type_id check.

    // And explicitly check the type_id

    // We may have changed the wrapper pointer, but we haven't changed the type_id stored in the persistent layer.
    BOOST_CHECK_EQUAL(::pmemobj_type_num(p.raw()), ::pmemobj_type_num(pother.raw()));
    BOOST_CHECK(PersistentType<OtherType>::type_id != ::pmemobj_type_num(pother.raw()));
}


//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
