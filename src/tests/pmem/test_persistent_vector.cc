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

#include "eckit/testing/Test.h"

#include "pmem/PersistentVector.h"

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
    public:
        Constructor(uint32_t value) : value_(value) {}
        virtual void make(CustomType &object) const {
            object.data1_ = value_;
            object.data2_ = value_;
        }
    private:
        uint32_t value_;
    };

    // Also provide an interface using a constructor, such that magic happers
    CustomType(uint32_t val) : data1_(val), data2_(val) {}
    CustomType(uint32_t val, uint32_t val2) : data1_(val), data2_(val2) {}

public: // members

    uint32_t data1_;
    uint32_t data2_;
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

    PersistentVector<CustomType> data_[root_elems];
};

//----------------------------------------------------------------------------------------------------------------------

// And structure the pool with types

template<> uint64_t pmem::PersistentType<RootType>::type_id = POBJ_ROOT_TYPE_NUM;
template<> uint64_t pmem::PersistentType<CustomType>::type_id = 1;
template<> uint64_t pmem::PersistentType<pmem::PersistentVectorData<CustomType> >::type_id = 2;

// Create a global fixture, so that this pool is only created once, and destroyed once.

AutoPool globalAutoPool((RootType::Constructor()));

struct GlobalRootFixture : public PersistentPtr<RootType> {
 GlobalRootFixture() : PersistentPtr<RootType>(globalAutoPool.pool_.getRoot<RootType>()) {}
    ~GlobalRootFixture() { nullify(); }
};

GlobalRootFixture global_root;

//----------------------------------------------------------------------------------------------------------------------

CASE( "test_pmem_persistent_vector_size" )
{
    PersistentVector<CustomType> pv;

    EXPECT(sizeof(PersistentVector<CustomType>) == sizeof(PersistentPtr<CustomType>));
    EXPECT(sizeof(pv) == sizeof(PersistentPtr<CustomType>));
    EXPECT(sizeof(pv) == sizeof(PMEMoid));
    EXPECT(sizeof(global_root->data_[0]) == sizeof(PMEMoid));
}

CASE( "test_pmem_persistent_vector_not_pmem" )
{
    // If we have a PersistentVector that is not in persistent memory, then doing push_back will cause
    // exceptions to be thrown in allocate()

    PersistentVector<CustomType> pv;

    EXPECT_THROWS_AS(pv.push_back_ctr(CustomType::Constructor(1234)), SeriousBug);
}

CASE( "test_pmem_persistent_vector_push_back" )
{
    PersistentVector<CustomType>& pv(global_root->data_[0]);

    // Check that allocation works on an (as-of-yet null) PersistentVector

    EXPECT(pv.null());
    EXPECT(pv.size() == size_t(0));
    EXPECT(pv.allocated_size() == size_t(0));

    pv.push_back_ctr(CustomType::Constructor(1111));

    EXPECT(!pv.null());
    EXPECT(pv.size() == size_t(1));
    EXPECT(pv.allocated_size() == size_t(1));
    EXPECT(pv->full()); // Internal to PersistentVectorData

    PersistentPtr<CustomType> pp0 = pv[0];
    PersistentPtr<PersistentVectorData<CustomType> > pd0 = pv;
    EXPECT(pv[0]->data1_ == uint32_t(1111));
    EXPECT(pv[0]->data2_ == uint32_t(1111));

    // Check that the next push_back works (will need to internally reallocate, as it goes in powers of two).

    pv.push_back_ctr(CustomType::Constructor(2222));

    EXPECT(!pv.null());
    EXPECT(pv.size() == size_t(2));
    EXPECT(pv.allocated_size() == size_t(2));
    EXPECT(pv->full()); // Internal to PersistentVectorData

    EXPECT(pv[0]->data1_ == uint32_t(1111));
    EXPECT(pv[0]->data2_ == uint32_t(1111));

    PersistentPtr<CustomType> pp1 = pv[0];
    EXPECT(pv[1]->data1_ == uint32_t(2222));
    EXPECT(pv[1]->data2_ == uint32_t(2222));

    // Data itself is not reallocated, but the data vector is.
    PersistentPtr<PersistentVectorData<CustomType> > pd1 = pv;
    EXPECT(pd1 != pd0);
    EXPECT(pp0 == pv[0]);

    // Check that the next 2 push_back works. Push back 4 doesn't need to reallocate.

    pv.push_back_ctr(CustomType::Constructor(3333));
    PersistentPtr<PersistentVectorData<CustomType> > pd2 = pv;

    EXPECT(pv.size() == size_t(3));
    EXPECT(pv.allocated_size() == size_t(4));
    EXPECT(!pv->full()); // Internal to PersistentVectorData

    pv.push_back_ctr(CustomType::Constructor(4444));
    PersistentPtr<PersistentVectorData<CustomType> > pd3 = pv;

    EXPECT(pv.size() == size_t(4));
    EXPECT(pv.allocated_size() == size_t(4));
    EXPECT(pv->full()); // Internal to PersistentVectorData

    EXPECT(pv[0]->data1_ == uint32_t(1111));
    EXPECT(pv[0]->data2_ == uint32_t(1111));

    EXPECT(pv[1]->data1_ == uint32_t(2222));
    EXPECT(pv[1]->data2_ == uint32_t(2222));

    EXPECT(pv[2]->data1_ == uint32_t(3333));
    EXPECT(pv[2]->data2_ == uint32_t(3333));

    EXPECT(pv[3]->data1_ == uint32_t(4444));
    EXPECT(pv[3]->data2_ == uint32_t(4444));

    // Data itself is not reallocated.
    EXPECT(pp0 == pv[0]);
    EXPECT(pp1 == pv[0]);

    EXPECT(pd2 != pd0);
    EXPECT(pd2 != pd1);
    EXPECT(pd2 == pd3); // The vectors data member is not extended when it isn't full.
}

CASE( "test_pmem_persistent_vector_push_back_constructors" )
{
    // Use the normal constructor interface

    PersistentVector<CustomType>& pv(global_root->data_[3]);

    // Check that allocation works on an (as-of-yet null) PersistentVector

    EXPECT(pv.null());
    EXPECT(pv.size() == size_t(0));
    EXPECT(pv.allocated_size() == size_t(0));

    pv.push_back(1111);

    EXPECT(!pv.null());
    EXPECT(pv.size() == size_t(1));
    EXPECT(pv.allocated_size() == size_t(1));
    EXPECT(pv->full()); // Internal to PersistentVectorData

    PersistentPtr<CustomType> pp0 = pv[0];
    PersistentPtr<PersistentVectorData<CustomType> > pd0 = pv;
    EXPECT(pv[0]->data1_ == uint32_t(1111));
    EXPECT(pv[0]->data2_ == uint32_t(1111));

    // Check that the next push_back works (will need to internally reallocate, as it goes in powers of two).

    pv.push_back(2222, 3333);

    EXPECT(!pv.null());
    EXPECT(pv.size() == size_t(2));
    EXPECT(pv.allocated_size() == size_t(2));
    EXPECT(pv->full()); // Internal to PersistentVectorData

    EXPECT(pv[0]->data1_ == uint32_t(1111));
    EXPECT(pv[0]->data2_ == uint32_t(1111));

    EXPECT(pv[1]->data1_ == uint32_t(2222));
    EXPECT(pv[1]->data2_ == uint32_t(3333));

    // Data itself is not reallocated, but the data vector is.
    PersistentPtr<PersistentVectorData<CustomType> > pd1 = pv;
    EXPECT(pd1 != pd0);
    EXPECT(pp0 == pv[0]);
}

CASE( "test_pmem_persistent_vector_resize" )
{
    PersistentVector<CustomType>& pv(global_root->data_[1]);

    // Run resize on an empty vector, should allocate initial space.

    EXPECT(pv.null());
    EXPECT(pv.size() == size_t(0));
    EXPECT(pv.allocated_size() == size_t(0));

    pv.resize(4);

    EXPECT(!pv.null());
    EXPECT(pv.size() == size_t(0));
    EXPECT(pv.allocated_size() == size_t(4));

    // Part-fill the available space

    pv.push_back_ctr(CustomType::Constructor(9999));      PersistentPtr<CustomType> p0 = pv[0];
    pv.push_back_ctr(CustomType::Constructor(8888));      PersistentPtr<CustomType> p1 = pv[1];
    pv.push_back_ctr(CustomType::Constructor(7777));      PersistentPtr<CustomType> p2 = pv[2];

    EXPECT(pv.size() == size_t(3));
    EXPECT(pv.allocated_size() == size_t(4));

    // Resize the vector

    pv.resize(6);

    EXPECT(pv.size() == size_t(3));
    EXPECT(pv.allocated_size() == size_t(6));

    // Fill the vector completely

    pv.push_back_ctr(CustomType::Constructor(6666));      PersistentPtr<CustomType> p3 = pv[3];
    pv.push_back_ctr(CustomType::Constructor(5555));      PersistentPtr<CustomType> p4 = pv[4];
    pv.push_back_ctr(CustomType::Constructor(4444));      PersistentPtr<CustomType> p5 = pv[5];

    // Resize the vector again now that it is full

    pv.resize(8);

    EXPECT(pv.size() == size_t(6));
    EXPECT(pv.allocated_size() == size_t(8));

    pv.push_back_ctr(CustomType::Constructor(3333));      PersistentPtr<CustomType> p6 = pv[6];

    EXPECT(pv.size() == size_t(7));
    EXPECT(pv.allocated_size() == size_t(8));

    // Check that all the data has been preserved throughout

    EXPECT(pv[0]->data1_ == uint32_t(9999));
    EXPECT(pv[1]->data1_ == uint32_t(8888));
    EXPECT(pv[2]->data1_ == uint32_t(7777));
    EXPECT(pv[3]->data1_ == uint32_t(6666));
    EXPECT(pv[4]->data1_ == uint32_t(5555));
    EXPECT(pv[5]->data1_ == uint32_t(4444));
    EXPECT(pv[6]->data1_ == uint32_t(3333));

    // Check that none of the _data_ has been moved during the reallocation of the vectors internal data
    // (i.e. check the persistent pointers returned by the vector are the same as they were immediately
    // after they were stored).

    EXPECT(p0 == pv[0]);
    EXPECT(p1 == pv[1]);
    EXPECT(p2 == pv[2]);
    EXPECT(p3 == pv[3]);
    EXPECT(p4 == pv[4]);
    EXPECT(p5 == pv[5]);
    EXPECT(p6 == pv[6]);
}

CASE( "test_pmem_persistent_vector_consistency_check" )
{
    // Necessarily the nelem_ count is incremented after the allocation has succeeded atomically
    //
    // --> On later accesses, it may be out of date
    // --> consistency_check updates it if necessary

    // We define a PersistentVectorData abuser, which allows us to manipulate nelem_ ...
    // (this is obviously normally private).
    //
    // It also gives direct access to read the nelem_ member, as the normal size() routine
    // automatically runs a consistency_check(), so we can't do direct testing...

    class Abuser : public PersistentVectorData<CustomType> {
    public:
        void tweak_nelem(size_t n) { update_nelem(n); }
        size_t raw_size() const { return nelem_; }
    };

    // -------------
    PersistentVector<CustomType>& pv(global_root->data_[2]);

    pv.push_back_ctr(CustomType::Constructor(1234));
    pv.push_back_ctr(CustomType::Constructor(1235));
    pv.push_back_ctr(CustomType::Constructor(1236));

    EXPECT(!pv.null());
    EXPECT(pv.size() == size_t(3));
    EXPECT(pv.allocated_size() == size_t(4));
    EXPECT(!pv->full());

    // Manipulate the internal count, and check that consistency_check fixes it

    EXPECT(static_cast<Abuser*>(pv.get())->raw_size() == size_t(3));
    static_cast<Abuser*>(pv.get())->tweak_nelem(2);
    EXPECT(static_cast<Abuser*>(pv.get())->raw_size() == size_t(2));

    pv->consistency_check();
    EXPECT(static_cast<Abuser*>(pv.get())->raw_size() == size_t(3));

    // Manipulate the internal count. Check that it is automatically corrected if we try and read it.

    static_cast<Abuser*>(pv.get())->tweak_nelem(2);
    EXPECT(static_cast<Abuser*>(pv.get())->raw_size() == size_t(2));

    EXPECT(pv.size() == size_t(3));
    EXPECT(pv.allocated_size() == size_t(4));

    EXPECT(static_cast<Abuser*>(pv.get())->raw_size() == size_t(3));

    // Manipulate the internal count. Check that it is automatically corrected if we try and push_back

    static_cast<Abuser*>(pv.get())->tweak_nelem(2);
    EXPECT(static_cast<Abuser*>(pv.get())->raw_size() == size_t(2));

    pv.push_back_ctr(CustomType::Constructor(1237));

    EXPECT(static_cast<Abuser*>(pv.get())->raw_size() == size_t(4));
    EXPECT(pv.size() == size_t(4));
    EXPECT(pv.allocated_size() == size_t(4));
    EXPECT(pv->full());

    // Check that the full() check (used to determine if we need to resize on push_back) does the correction

    static_cast<Abuser*>(pv.get())->tweak_nelem(2);
    EXPECT(static_cast<Abuser*>(pv.get())->raw_size() == size_t(2));

    EXPECT(pv->full());

    EXPECT(static_cast<Abuser*>(pv.get())->raw_size() == size_t(4));
    EXPECT(pv.size() == size_t(4));
    EXPECT(pv.allocated_size() == size_t(4));

    // If we manipulate nelem_ the wrong way, we end up with null() gaps in the vector.
    // This will be picked up by consistency check.

    pv.resize(8);
    EXPECT(pv.size() == size_t(4));

    static_cast<Abuser*>(pv.get())->tweak_nelem(6);
    EXPECT(static_cast<Abuser*>(pv.get())->raw_size() == size_t(6));

    EXPECT_THROWS_AS(pv->consistency_check(), AssertionFailed);
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
