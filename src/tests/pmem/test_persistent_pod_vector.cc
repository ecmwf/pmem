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

#include "pmem/PersistentPODVector.h"
#include "pmem/PersistentType.h"

#include "test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;
using namespace eckit::testing;

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

AutoPool globalAutoPool((RootType::Constructor()));

struct GlobalRootFixture : public PersistentPtr<RootType> {
 GlobalRootFixture() : PersistentPtr<RootType>(globalAutoPool.pool_.getRoot<RootType>()) {}
    ~GlobalRootFixture() { nullify(); }
};

GlobalRootFixture global_root;


//----------------------------------------------------------------------------------------------------------------------

CASE ("test_pmem_persistent_pod_vector_size")
{
    PersistentPODVector<uint64_t> pv;

    EXPECT(sizeof(PersistentPODVector<uint64_t>) == sizeof(PersistentPtr<PersistentType<uint64_t> >));
    EXPECT(sizeof(pv) == sizeof(PersistentPtr<PersistentType<uint64_t> >));
    EXPECT(sizeof(pv) == sizeof(PMEMoid));
    EXPECT(sizeof(global_root->data_[0]) == sizeof(PMEMoid));
}

CASE( "test_pmem_persistent_pod_vector_not_pmem" )
{
    // If we have a PersistentPODVector that is not in persistent memory, then doing push_back will cause
    // exceptions to be thrown in allocate()

    PersistentPODVector<uint64_t> pv;

    EXPECT_THROWS_AS(pv.push_back(1234), SeriousBug);
}

CASE( "test_pmem_persistent_pod_vector_push_back" )
{
    PersistentPODVector<size_t>& pv(global_root->data_[0]);

    // Check that allocation works on an (as-of-yet null) PersistentPODVector

    EXPECT(pv.null());
    EXPECT(pv.size() == size_t(0));
    EXPECT(pv.allocated_size() == size_t(0));

    pv.push_back(1111);

    EXPECT(!pv.null());
    EXPECT(pv.size() == size_t(1));
    EXPECT(pv.allocated_size() == size_t(1));
    EXPECT(pv->full()); // Internal to PersistentPODVectorData

    EXPECT(pv[0] == uint64_t(1111));
    const uint64_t * data_ptr_a = &pv[0];

    // Check that the next push_back works (will need to internally reallocate, as it goes in powers of two).

    pv.push_back(2222);

    EXPECT(!pv.null());
    EXPECT(pv.size() == size_t(2));
    EXPECT(pv.allocated_size() == size_t(2));
    EXPECT(pv->full()); // Internal to PersistentPODVectorData

    EXPECT(pv[0] == uint64_t(1111));
    EXPECT(pv[1] == uint64_t(2222));

    // The data has been reallocated in the previous push_back

    const uint64_t * data_ptr_b = &pv[0];
    EXPECT(data_ptr_b != data_ptr_a);

    // Check that the next 2 push_back works. Push back 4 doesn't need to reallocate.

    pv.push_back(3333);

    EXPECT(pv.size() == size_t(3));
    EXPECT(pv.allocated_size() == size_t(4));
    EXPECT(!pv->full()); // Internal to PersistentPODVectorData

    const uint64_t * data_ptr_c = &pv[0];
    EXPECT(data_ptr_b != data_ptr_c);

    pv.push_back(4444);

    EXPECT(pv.size() == size_t(4));
    EXPECT(pv.allocated_size() == size_t(4));
    EXPECT(pv->full()); // Internal to PersistentPODVectorData

    EXPECT(pv[0] == uint64_t(1111));
    EXPECT(pv[1] == uint64_t(2222));
    EXPECT(pv[2] == uint64_t(3333));
    EXPECT(pv[3] == uint64_t(4444));

    // Vector is reallocated again reallocated.

    const uint64_t * data_ptr_d = &pv[0];
    EXPECT(data_ptr_d != data_ptr_b);
    EXPECT(data_ptr_d == data_ptr_c);
}

CASE( "test_pmem_persistent_pod_vector_resize" )
{
    PersistentPODVector<uint64_t>& pv(global_root->data_[1]);

    // Run resize on an empty vector, should allocate initial space.

    EXPECT(pv.null());
    EXPECT(pv.size() == size_t(0));
    EXPECT(pv.allocated_size() == size_t(0));

    pv.resize(4);

    EXPECT(!pv.null());
    EXPECT(pv.size() == size_t(0));
    EXPECT(pv.allocated_size() == size_t(4));

    // Part-fill the available space

    pv.push_back(9999);
    pv.push_back(8888);
    pv.push_back(7777);

    EXPECT(pv.size() == size_t(3));
    EXPECT(pv.allocated_size() == size_t(4));

    // Resize the vector

    pv.resize(6);

    EXPECT(pv.size() == size_t(3));
    EXPECT(pv.allocated_size() == size_t(6));

    // Fill the vector completely

    pv.push_back(6666);
    pv.push_back(5555);
    pv.push_back(4444);

    // Resize the vector again now that it is full

    pv.resize(8);

    EXPECT(pv.size() == size_t(6));
    EXPECT(pv.allocated_size() == size_t(8));

    pv.push_back(3333);

    EXPECT(pv.size() == size_t(7));
    EXPECT(pv.allocated_size() == size_t(8));

    // Check that all the data has been preserved throughout

    EXPECT(pv[0] == uint64_t(9999));
    EXPECT(pv[1] == uint64_t(8888));
    EXPECT(pv[2] == uint64_t(7777));
    EXPECT(pv[3] == uint64_t(6666));
    EXPECT(pv[4] == uint64_t(5555));
    EXPECT(pv[5] == uint64_t(4444));
    EXPECT(pv[6] == uint64_t(3333));
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}

