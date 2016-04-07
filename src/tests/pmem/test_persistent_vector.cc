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

#include "pmem/PersistentVector.h"

#include "test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;

// TODO:
//
// - Test that it just wraps PersistentPtr (i.e. it is the same size!!!)
// - Test size(), full(), available() [max size]
// - Check explicit call to resize(). Should work from empty, and allready allocated.
// - Test push_back
//   - On empty list
//   - On list with a size that equals maxSize
//   - On list with available space
// - Test consistency check
// - test operator[]

//----------------------------------------------------------------------------------------------------------------------

/// A custom type to allocate objects in the tests

class CustomType {

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

public: // members

    uint32_t data1_;
    uint32_t data2_;
};


/// Define a root type. Each test that does allocation should use a different element in the root object.

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

    PersistentVector<CustomType> data_[root_elems];
};

//----------------------------------------------------------------------------------------------------------------------

// And structure the pool with types

template<> uint64_t pmem::PersistentPtr<RootType>::type_id = POBJ_ROOT_TYPE_NUM;
template<> uint64_t pmem::PersistentPtr<CustomType>::type_id = 1;
template<> uint64_t pmem::PersistentPtr<pmem::PersistentVectorData<CustomType> >::type_id = 2;

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

BOOST_AUTO_TEST_SUITE( test_pmem_persistent_vector )

BOOST_AUTO_TEST_CASE( test_pmem_persistent_vector_size )
{
    PersistentVector<CustomType> pv;

    BOOST_CHECK_EQUAL(sizeof(PersistentVector<CustomType>), sizeof(PersistentPtr<CustomType>));
    BOOST_CHECK_EQUAL(sizeof(pv), sizeof(PersistentPtr<CustomType>));
    BOOST_CHECK_EQUAL(sizeof(pv), sizeof(PMEMoid));
    BOOST_CHECK_EQUAL(sizeof(global_root->data_[0]), sizeof(PMEMoid));
}

BOOST_AUTO_TEST_CASE( test_pmem_persistent_vector_not_pmem )
{
    // If we have a PersistentVector that is not in persistent memory, then doing push_back will cause
    // exceptions to be thrown in allocate()

    PersistentVector<CustomType> pv;

    BOOST_CHECK_THROW(pv.push_back(CustomType::Constructor(1234)), SeriousBug);
}

BOOST_AUTO_TEST_CASE( test_pmem_persistent_vector_push_back )
{
    PersistentVector<CustomType>& pv(global_root->data_[0]);

    // Check that allocation works on an (as-of-yet null) PersistentVector

    BOOST_CHECK(pv.null());
    BOOST_CHECK_EQUAL(pv.size(), 0);
    BOOST_CHECK_EQUAL(pv.allocated_size(), 0);

    pv.push_back(CustomType::Constructor(1111));

    BOOST_CHECK(!pv.null());
    BOOST_CHECK_EQUAL(pv.size(), 1);
    BOOST_CHECK_EQUAL(pv.allocated_size(), 1);

    PersistentPtr<CustomType> pp0 = pv[0];
    PersistentPtr<PersistentVectorData<CustomType> > pd0 = pv;
    BOOST_CHECK_EQUAL(pv[0]->data1_, 1111);
    BOOST_CHECK_EQUAL(pv[0]->data2_, 1111);

    // Check that the next push_back works (will need to internally reallocate, as it goes in powers of two).

    pv.push_back(CustomType::Constructor(2222));

    BOOST_CHECK(!pv.null());
    BOOST_CHECK_EQUAL(pv.size(), 2);
    BOOST_CHECK_EQUAL(pv.allocated_size(), 2);

    BOOST_CHECK_EQUAL(pv[0]->data1_, 1111);
    BOOST_CHECK_EQUAL(pv[0]->data2_, 1111);

    PersistentPtr<CustomType> pp1 = pv[0];
    BOOST_CHECK_EQUAL(pv[1]->data1_, 2222);
    BOOST_CHECK_EQUAL(pv[1]->data2_, 2222);

    // Data itself is not reallocated, but the data vector is.
    PersistentPtr<PersistentVectorData<CustomType> > pd1 = pv;
    BOOST_CHECK(pd1 != pd0);
    BOOST_CHECK_EQUAL(pp0, pv[0]);

    // Check that the next 2 push_back works. Push back 4 doesn't need to reallocate.

    pv.push_back(CustomType::Constructor(3333));
    PersistentPtr<PersistentVectorData<CustomType> > pd2 = pv;

    BOOST_CHECK_EQUAL(pv.size(), 3);
    BOOST_CHECK_EQUAL(pv.allocated_size(), 4);

    pv.push_back(CustomType::Constructor(4444));
    PersistentPtr<PersistentVectorData<CustomType> > pd3 = pv;

    BOOST_CHECK_EQUAL(pv.size(), 4);
    BOOST_CHECK_EQUAL(pv.allocated_size(), 4);

    BOOST_CHECK_EQUAL(pv[0]->data1_, 1111);
    BOOST_CHECK_EQUAL(pv[0]->data2_, 1111);

    BOOST_CHECK_EQUAL(pv[1]->data1_, 2222);
    BOOST_CHECK_EQUAL(pv[1]->data2_, 2222);

    BOOST_CHECK_EQUAL(pv[2]->data1_, 3333);
    BOOST_CHECK_EQUAL(pv[2]->data2_, 3333);

    BOOST_CHECK_EQUAL(pv[3]->data1_, 4444);
    BOOST_CHECK_EQUAL(pv[3]->data2_, 4444);

    // Data itself is not reallocated.
    BOOST_CHECK_EQUAL(pp0, pv[0]);
    BOOST_CHECK_EQUAL(pp1, pv[0]);

    BOOST_CHECK(pd2 != pd0);
    BOOST_CHECK(pd2 != pd1);
    BOOST_CHECK(pd2 == pd3); // The vectors data member is not extended when it isn't full.
}

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
