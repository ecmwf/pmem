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

#include <string>

#include "eckit/filesystem/PathName.h"
#include "eckit/io/Buffer.h"
#include "eckit/memory/NonCopyable.h"
#include "eckit/testing/Test.h"
#include "eckit/types/FixedString.h"

#include "pmem/PersistentBuffer.h"
#include "pmem/PersistentPtr.h"

#include "test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;
using namespace eckit::testing;

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


CASE( "test_pmem_persistent_buffer_valid_persistent_ptr" )
{
    // If PersistentBuffer is not OK, this will trigger StaticAssert
    PersistentPtr<PersistentBuffer> ptr;
}

CASE( "test_pmem_persistent_buffer_allocate" )
{
    std::string test_string = "I am a test string";

    AutoPool ap((RootType::Constructor()));
    PersistentPtr<RootType> root = ap.pool_.getRoot<RootType>();

    root->data_[0].allocate(test_string.data(), test_string.length());

    std::string get_back(static_cast<const char*>(root->data_[0]->data()), root->data_[0]->size());
    EXPECT(get_back == test_string);
}

CASE( "test_pmem_persistent_buffer_size" )
{
    const void* dat = 0;
    size_t len = 1234;
    AtomicConstructor2<PersistentBuffer, const void*, size_t> ctr(dat, len);

    // Check that space is allocated to store the data, and to store the size of the data
    EXPECT(ctr.size() == 1234 + sizeof(size_t));
}


CASE( "test_pmem_persistent_buffer_size_zero" )
{
    const void* dat = 0;
    size_t len(0);
    AtomicConstructor2<PersistentBuffer, const void*, size_t> ctr(dat, len);

    // If we specify a zero-sized buffer, then check that the constructor does the right thing...
    EXPECT(ctr.size() == sizeof(size_t));

    Buffer buf(ctr.size());
    PersistentBuffer& buf_ref(*reinterpret_cast<PersistentBuffer*>((void*)buf));
    ctr.make(buf_ref);

    EXPECT(buf_ref.size() == size_t(0));
}


CASE( "test_pmem_persistent_stores_data" )
{
    std::string dat("SOME DATA");
    const void* data_ptr = dat.data();
    size_t len = dat.length();

    AtomicConstructor2<PersistentBuffer, const void*, size_t> ctr(data_ptr, len);

    EXPECT(ctr.size() == dat.length() + sizeof(size_t));

    Buffer buf(ctr.size());
    PersistentBuffer& buf_ref(*reinterpret_cast<PersistentBuffer*>((void*)buf));
    ctr.make(buf_ref);

    EXPECT(buf_ref.size() == dat.length());
//    EXPECT(::memcmp(dat.c_str(), buf_ref.data(), buf_ref.size()), 0);
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
