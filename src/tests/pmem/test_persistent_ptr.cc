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

#include "pmem/PersistentPtr.h"

#include "test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;
using namespace eckit::testing;

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

AutoPool globalAutoPool((RootType::Constructor()));

struct GlobalRootFixture : public PersistentPtr<RootType> {
 GlobalRootFixture() : PersistentPtr<RootType>(globalAutoPool.pool_.getRoot<RootType>()) {}
    ~GlobalRootFixture() { nullify(); }
};

GlobalRootFixture global_root;

//----------------------------------------------------------------------------------------------------------------------

CASE( "test_pmem_persistent_ptr_size" )
{
    PersistentPtr<CustomType> ptr1;
    PersistentPtrBase ptr_base;

    // Check that everything has the right size, and that the PersistentPtr is only a wrapper around the underlying
    // object (with no further padding).

    EXPECT(sizeof(PersistentPtr<CustomType>) == sizeof(PersistentPtrBase));
    EXPECT(sizeof(ptr1) == sizeof(ptr_base));
    EXPECT(sizeof(ptr1) == sizeof(PMEMoid));
    EXPECT(sizeof(ptr1) == size_t(16));
}


CASE( "test_pmem_persistent_ptr_nullification" )
{
    PersistentPtr<CustomType> ptr;

    // Forcibly make the pointer not null (note it is a wrapper around PMEMoid, so just cast it).

    reinterpret_cast<PMEMoid*>(&ptr)->off = 0xBAADBEEF;
    reinterpret_cast<PMEMoid*>(&ptr)->pool_uuid_lo = 0xBAADBEEF;

    EXPECT(!ptr.null());

    // Now nullify the pointer

    ptr.nullify();
    EXPECT(ptr.null());

    EXPECT(reinterpret_cast<PMEMoid*>(&ptr)->off == uint64_t(0));
    EXPECT(reinterpret_cast<PMEMoid*>(&ptr)->pool_uuid_lo == uint64_t(0));
}


CASE( "test_pmem_persistent_ptr_equality" )
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

    EXPECT(ptr1 == ptr2);
    EXPECT(ptr1 == ptr2);
    EXPECT(!(ptr1 != ptr2));

    EXPECT(!(ptr1 == ptr3));
    EXPECT(ptr1 != ptr3);

    EXPECT(!(ptr1 == ptr4));
    EXPECT(ptr1 != ptr4);

    EXPECT(!(ptr1 == ptr5));
    EXPECT(ptr1 != ptr5);
}


CASE( "test_pmem_persistent_ptr_allocate_needs_pmem" )
{
    PersistentPtr<CustomType> ptr;

    CustomType::Constructor ctr;

    EXPECT_THROWS_AS(ptr.allocate_ctr(ctr), SeriousBug);
}


CASE( "test_pmem_persistent_ptr_direct_allocate_ctr" )
{
    EXPECT(global_root->data_[3].null());

    CustomType::Constructor ctr;
    global_root->data_[3].allocate_ctr(ctr);

    EXPECT(!global_root->data_[3].null());
    EXPECT(global_root->data_[3].valid());

    // Check that persistent pointers in volatile vs persistent space are the same

    PersistentPtr<CustomType> p1 = global_root->data_[3];

    EXPECT(!global_root->data_[3].null());
    EXPECT(global_root->data_[3].valid());
    EXPECT(p1 == global_root->data_[3]);
    EXPECT(!(p1 != global_root->data_[3]));

    // Check that we can dereference this PersistentPtr to give access to the memory mapped region

    EXPECT(p1->data1_ == uint32_t(1111));
    EXPECT(p1->data2_ == uint32_t(2222));

    const CustomType* pelem = p1.get();

    EXPECT(pelem->data1_ == uint32_t(1111));
    EXPECT(pelem->data2_ == uint32_t(2222));

    const CustomType& relem(*p1);

    EXPECT(relem.data1_ == uint32_t(1111));
    EXPECT(relem.data2_ == uint32_t(2222));

    EXPECT(&relem == pelem);

    // Test that this pointer is in persistent memory, and is in the same pool as the global root

    PMEMobjpool* root_pool = ::pmemobj_pool_by_ptr(global_root.get());
    PMEMobjpool* elem_pool = ::pmemobj_pool_by_ptr(pelem);

    EXPECT(pelem != (void*)global_root.get());
    EXPECT(root_pool != 0);
    EXPECT(elem_pool != 0);
    EXPECT(root_pool == elem_pool);

    // Check that replace also works

    global_root->data_[3].replace_ctr(ctr);

    PersistentPtr<CustomType> p2 = global_root->data_[3];

    EXPECT(p1 != p2);
    EXPECT(p2->data1_ == uint32_t(1111));
    EXPECT(p2->data2_ == uint32_t(2222));
}


CASE( "test_pmem_persistent_ptr_direct_allocate" )
{
    EXPECT(global_root->data_[0].null());

    global_root->data_[0].allocate(999);

    EXPECT(!global_root->data_[0].null());
    EXPECT(global_root->data_[0].valid());

    // Check that persistent pointers in volatile vs persistent space are the same

    PersistentPtr<CustomType> p1 = global_root->data_[0];

    EXPECT(!global_root->data_[0].null());
    EXPECT(global_root->data_[0].valid());
    EXPECT(p1 == global_root->data_[0]);
    EXPECT(!(p1 != global_root->data_[0]));

    // Check that we can dereference this PersistentPtr to give access to the memory mapped region

    EXPECT(p1->data1_ == uint32_t(999));
    EXPECT(p1->data2_ == uint32_t(999));

    const CustomType* pelem = p1.get();

    EXPECT(pelem->data1_ == uint32_t(999));
    EXPECT(pelem->data2_ == uint32_t(999));

    const CustomType& relem(*p1);

    EXPECT(relem.data1_ == uint32_t(999));
    EXPECT(relem.data2_ == uint32_t(999));

    EXPECT(&relem == pelem);

    // Test that this pointer is in persistent memory, and is in the same pool as the global root

    PMEMobjpool* root_pool = ::pmemobj_pool_by_ptr(global_root.get());
    PMEMobjpool* elem_pool = ::pmemobj_pool_by_ptr(pelem);

    EXPECT(pelem != (void*)global_root.get());
    EXPECT(root_pool != 0);
    EXPECT(elem_pool != 0);
    EXPECT(root_pool == elem_pool);

    // Check that replace also works

    global_root->data_[0].replace(888);

    PersistentPtr<CustomType> p2 = global_root->data_[0];

    EXPECT(p1 != p2);
    EXPECT(p2->data1_ == uint32_t(888));
    EXPECT(p2->data2_ == uint32_t(888));
}


CASE( "test_pemem_persistent_ptr_cross_pool" )
{
    // Open a second pool

    UniquePool uniq;
    PersistentPool pool2(uniq.path_, auto_pool_size, "second-pool", RootType::Constructor());

    // Explicitly allocate an object in the second pool

    EXPECT(global_root->data_[1].null());

    CustomType::Constructor ctr;
    global_root->data_[1].allocate_ctr(pool2, ctr);

    PersistentPtr<CustomType> p1 = global_root->data_[1];
    EXPECT(!p1.null());
    EXPECT(p1->data1_ == uint32_t(1111));
    EXPECT(p1->data2_ == uint32_t(2222));

    CustomType * volatile_p1 = p1.get();

    // Check that this pointer is not in the same pool!

    PMEMobjpool * raw_pool1 = ::pmemobj_pool_by_ptr(global_root.get());
    PMEMobjpool * raw_pool2 = ::pmemobj_pool_by_ptr(pool2.getRoot<RootType>().get());
    PMEMobjpool * raw_pool_p1 = ::pmemobj_pool_by_ptr(volatile_p1);

    EXPECT(raw_pool1 != raw_pool2);
    EXPECT(raw_pool_p1 == raw_pool2);

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

    EXPECT(raw_pool2a != raw_pool2);
    EXPECT(raw_pool2a != raw_pool1);

    // Check that the pointer now maps to the new region!

    CustomType * volatile_p2 = p1.get();
    PMEMobjpool * raw_pool_p2 = ::pmemobj_pool_by_ptr(volatile_p2);

    EXPECT(volatile_p1 != volatile_p2);
    EXPECT(raw_pool_p2 == raw_pool2a);
    EXPECT(raw_pool_p2 != raw_pool2);

    EXPECT(p1->data1_ == uint32_t(1111));
    EXPECT(p1->data2_ == uint32_t(2222));
    EXPECT(volatile_p2->data1_ == uint32_t(1111));
    EXPECT(volatile_p2->data2_ == uint32_t(2222));

    // And check an explicit replace

    global_root->data_[1].replace_ctr(pool2a, ctr);

    PersistentPtr<CustomType> p2 = global_root->data_[1];

    EXPECT(p1 != p2);
    EXPECT(p2->data1_ == uint32_t(1111));
    EXPECT(p2->data2_ == uint32_t(2222));
    EXPECT(raw_pool2a == ::pmemobj_pool_by_ptr(p2.get()));

    // And clean everything up

    pool2a.remove();
}


CASE( "test_pmem_persistent_ptr_typeid" )
{
    EXPECT(global_root->data_[2].null());

    CustomType::Constructor ctr;
    global_root->data_[2].allocate_ctr(ctr);

    PersistentPtr<CustomType> p = global_root->data_[2];

    EXPECT(!p.null());
    EXPECT(p.valid());

    // Check the type_id explicitly

    EXPECT(PersistentType<CustomType>::type_id == ::pmemobj_type_num(p.raw()));

    // Cast the PersistentPtr to a different type. Now we can see things go wrong.

    /// @note This line will throw a strict aliasing warning when compiled with optimisations on. That is OK.
    ///       It is (correctly) reporting that we would incur problems if we tried to do this for real...

    PersistentPtr<OtherType> pother = *reinterpret_cast<PersistentPtr<OtherType>*>(&p);

    EXPECT(!pother.null());
    EXPECT(!pother.valid()); // @note No longer valid. Fails type_id check.

    // And explicitly check the type_id

    // We may have changed the wrapper pointer, but we haven't changed the type_id stored in the persistent layer.
    EXPECT(::pmemobj_type_num(p.raw()) == ::pmemobj_type_num(pother.raw()));
    EXPECT(PersistentType<OtherType>::type_id != ::pmemobj_type_num(pother.raw()));
}


//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
