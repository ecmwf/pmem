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

#include "ecbuild/boost_test_framework.h"

#include "eckit/testing/Setup.h"

#include "pmem/PersistentPODVector.h"
#include "pmem/PersistentType.h"

#include "test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;
using namespace eckit::testing;

BOOST_GLOBAL_FIXTURE(Setup);

//----------------------------------------------------------------------------------------------------------------------

/// Define a root type. Each test that does allocation should use a different element in the root object.

// How many possibilities do we want?
const size_t root_elems = 2;


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

    PersistentPODVector<uint64_t> data_[root_elems];
};

//----------------------------------------------------------------------------------------------------------------------

// And structure the pool with types

template<> uint64_t pmem::PersistentType<RootType>::type_id = POBJ_ROOT_TYPE_NUM;
template<> uint64_t pmem::PersistentType<pmem::PersistentPODVector<uint64_t>::data_type>::type_id = 1;

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

BOOST_GLOBAL_FIXTURE( SuitePoolFixture );

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE( test_pmem_persistent_pod_vector )

BOOST_AUTO_TEST_CASE( test_pmem_persistent_pod_vector_size )
{
    PersistentPODVector<uint64_t> pv;

    BOOST_CHECK_EQUAL(sizeof(PersistentPODVector<uint64_t>), sizeof(PersistentPtr<PersistentType<uint64_t> >));
    BOOST_CHECK_EQUAL(sizeof(pv), sizeof(PersistentPtr<PersistentType<uint64_t> >));
    BOOST_CHECK_EQUAL(sizeof(pv), sizeof(PMEMoid));
    BOOST_CHECK_EQUAL(sizeof(global_root->data_[0]), sizeof(PMEMoid));
}

BOOST_AUTO_TEST_CASE( test_pmem_persistent_pod_vector_not_pmem )
{
    // If we have a PersistentPODVector that is not in persistent memory, then doing push_back will cause
    // exceptions to be thrown in allocate()

    PersistentPODVector<uint64_t> pv;

    BOOST_CHECK_THROW(pv.push_back(1234), SeriousBug);
}

BOOST_AUTO_TEST_CASE( test_pmem_persistent_pod_vector_push_back )
{
    PersistentPODVector<size_t>& pv(global_root->data_[0]);

    // Check that allocation works on an (as-of-yet null) PersistentPODVector

    BOOST_CHECK(pv.null());
    BOOST_CHECK_EQUAL(pv.size(), size_t(0));
    BOOST_CHECK_EQUAL(pv.allocated_size(), size_t(0));

    pv.push_back(1111);

    BOOST_CHECK(!pv.null());
    BOOST_CHECK_EQUAL(pv.size(), size_t(1));
    BOOST_CHECK_EQUAL(pv.allocated_size(), size_t(1));
    BOOST_CHECK(pv->full()); // Internal to PersistentPODVectorData

    BOOST_CHECK_EQUAL(pv[0], uint64_t(1111));
    const uint64_t * data_ptr_a = &pv[0];

    // Check that the next push_back works (will need to internally reallocate, as it goes in powers of two).

    pv.push_back(2222);

    BOOST_CHECK(!pv.null());
    BOOST_CHECK_EQUAL(pv.size(), size_t(2));
    BOOST_CHECK_EQUAL(pv.allocated_size(), size_t(2));
    BOOST_CHECK(pv->full()); // Internal to PersistentPODVectorData

    BOOST_CHECK_EQUAL(pv[0], uint64_t(1111));
    BOOST_CHECK_EQUAL(pv[1], uint64_t(2222));

    // The data has been reallocated in the previous push_back

    const uint64_t * data_ptr_b = &pv[0];
    BOOST_CHECK(data_ptr_b != data_ptr_a);

    // Check that the next 2 push_back works. Push back 4 doesn't need to reallocate.

    pv.push_back(3333);

    BOOST_CHECK_EQUAL(pv.size(), size_t(3));
    BOOST_CHECK_EQUAL(pv.allocated_size(), size_t(4));
    BOOST_CHECK(!pv->full()); // Internal to PersistentPODVectorData

    const uint64_t * data_ptr_c = &pv[0];
    BOOST_CHECK(data_ptr_b != data_ptr_c);

    pv.push_back(4444);

    BOOST_CHECK_EQUAL(pv.size(), size_t(4));
    BOOST_CHECK_EQUAL(pv.allocated_size(), size_t(4));
    BOOST_CHECK(pv->full()); // Internal to PersistentPODVectorData

    BOOST_CHECK_EQUAL(pv[0], uint64_t(1111));
    BOOST_CHECK_EQUAL(pv[1], uint64_t(2222));
    BOOST_CHECK_EQUAL(pv[2], uint64_t(3333));
    BOOST_CHECK_EQUAL(pv[3], uint64_t(4444));

    // Vector is reallocated again reallocated.

    const uint64_t * data_ptr_d = &pv[0];
    BOOST_CHECK(data_ptr_d != data_ptr_b);
    BOOST_CHECK(data_ptr_d == data_ptr_c);
}

BOOST_AUTO_TEST_CASE( test_pmem_persistent_pod_vector_resize )
{
    PersistentPODVector<uint64_t>& pv(global_root->data_[1]);

    // Run resize on an empty vector, should allocate initial space.

    BOOST_CHECK(pv.null());
    BOOST_CHECK_EQUAL(pv.size(), size_t(0));
    BOOST_CHECK_EQUAL(pv.allocated_size(), size_t(0));

    pv.resize(4);

    BOOST_CHECK(!pv.null());
    BOOST_CHECK_EQUAL(pv.size(), size_t(0));
    BOOST_CHECK_EQUAL(pv.allocated_size(), size_t(4));

    // Part-fill the available space

    pv.push_back(9999);
    pv.push_back(8888);
    pv.push_back(7777);

    BOOST_CHECK_EQUAL(pv.size(), size_t(3));
    BOOST_CHECK_EQUAL(pv.allocated_size(), size_t(4));

    // Resize the vector

    pv.resize(6);

    BOOST_CHECK_EQUAL(pv.size(), size_t(3));
    BOOST_CHECK_EQUAL(pv.allocated_size(), size_t(6));

    // Fill the vector completely

    pv.push_back(6666);
    pv.push_back(5555);
    pv.push_back(4444);

    // Resize the vector again now that it is full

    pv.resize(8);

    BOOST_CHECK_EQUAL(pv.size(), size_t(6));
    BOOST_CHECK_EQUAL(pv.allocated_size(), size_t(8));

    pv.push_back(3333);

    BOOST_CHECK_EQUAL(pv.size(), size_t(7));
    BOOST_CHECK_EQUAL(pv.allocated_size(), size_t(8));

    // Check that all the data has been preserved throughout

    BOOST_CHECK_EQUAL(pv[0], uint64_t(9999));
    BOOST_CHECK_EQUAL(pv[1], uint64_t(8888));
    BOOST_CHECK_EQUAL(pv[2], uint64_t(7777));
    BOOST_CHECK_EQUAL(pv[3], uint64_t(6666));
    BOOST_CHECK_EQUAL(pv[4], uint64_t(5555));
    BOOST_CHECK_EQUAL(pv[5], uint64_t(4444));
    BOOST_CHECK_EQUAL(pv[6], uint64_t(3333));
}

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
