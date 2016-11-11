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

#include <string>

#include "ecbuild/boost_test_framework.h"

#include "eckit/filesystem/PathName.h"
#include "eckit/io/Buffer.h"
#include "eckit/memory/NonCopyable.h"
#include "eckit/testing/Setup.h"
#include "eckit/types/FixedString.h"

#include "pmem/PersistentBuffer.h"
#include "pmem/PersistentPtr.h"

#include "test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;
using namespace eckit::testing;

BOOST_GLOBAL_FIXTURE(Setup)


//----------------------------------------------------------------------------------------------------------------------

/// Define a root type. Each test that does allocation should use a different element in the root object.

// How many possibilities do we want?
const size_t root_elems = 1;


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

    PersistentPtr<PersistentBuffer> data_[root_elems];
};

//----------------------------------------------------------------------------------------------------------------------

// And structure the pool with types

template<> uint64_t pmem::PersistentType<RootType>::type_id = POBJ_ROOT_TYPE_NUM;
template<> uint64_t pmem::PersistentType<PersistentBuffer>::type_id = 1;
//----------------------------------------------------------------------------------------------------------------------


BOOST_AUTO_TEST_SUITE( test_pmem_persistent_buffer )

BOOST_AUTO_TEST_CASE( test_pmem_persistent_buffer_valid_persistent_ptr )
{
    // If PersistentBuffer is not OK, this will trigger StaticAssert
    PersistentPtr<PersistentBuffer> ptr;
}

BOOST_AUTO_TEST_CASE( test_pmem_persistent_buffer_allocate )
{
    std::string test_string = "I am a test string";

    AutoPool ap((RootType::Constructor()));
    PersistentPtr<RootType> root = ap.pool_.getRoot<RootType>();

    root->data_[0].allocate(PersistentBuffer::Constructor(test_string.data(), test_string.length()));

    std::string get_back(static_cast<const char*>(root->data_[0]->data()), root->data_[0]->size());
    BOOST_CHECK_EQUAL(get_back, test_string);
}

BOOST_AUTO_TEST_CASE( test_pmem_persistent_buffer_size )
{
    PersistentBuffer::Constructor ctr(0, 1234);

    // Check that space is allocated to store the data, and to store the size of the data
    BOOST_CHECK_EQUAL(ctr.size(), 1234 + sizeof(size_t));
}


BOOST_AUTO_TEST_CASE( test_pmem_persistent_buffer_size_zero )
{
    PersistentBuffer::Constructor ctr(0, 0);

    // If we specify a zero-sized buffer, then check that the constructor does the right thing...
    BOOST_CHECK_EQUAL(ctr.size(), sizeof(size_t));

    Buffer buf(ctr.size());
    PersistentBuffer& buf_ref(*reinterpret_cast<PersistentBuffer*>((void*)buf));
    ctr.make(buf_ref);

    BOOST_CHECK_EQUAL(buf_ref.size(), 0);
}


BOOST_AUTO_TEST_CASE( test_pmem_persistent_stores_data )
{
    std::string dat("SOME DATA");

    PersistentBuffer::Constructor ctr(dat.c_str(), dat.length());

    BOOST_CHECK_EQUAL(ctr.size(), dat.length() + sizeof(size_t));

    Buffer buf(ctr.size());
    PersistentBuffer& buf_ref(*reinterpret_cast<PersistentBuffer*>((void*)buf));
    ctr.make(buf_ref);

    BOOST_CHECK_EQUAL(buf_ref.size(), dat.length());
    BOOST_CHECK_EQUAL(::memcmp(dat.c_str(), buf_ref.data(), buf_ref.size()), 0);
}

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
