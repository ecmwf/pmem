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
#include <stdint.h>

#include "ecbuild/boost_test_framework.h"

#include "pmem/AtomicConstructor.h"

using namespace std;
using namespace pmem;

namespace {

    struct LocalType {
        uint32_t elem;
        uint32_t elem2;
    };
}

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE( test_pmem_atomic_constructor )


BOOST_AUTO_TEST_CASE( test_pmem_atomic_constructor_default_size )
{
    struct Ctr : public AtomicConstructor<LocalType> {
        Ctr() {}
        ~Ctr() {}
        virtual void make(LocalType& obj) const {}
    };

    Ctr ctr;

    BOOST_CHECK_EQUAL(ctr.size(), 8);
}


BOOST_AUTO_TEST_CASE( test_pmem_atomic_constructor_manual_size )
{
    struct Ctr : public AtomicConstructor<LocalType> {
        Ctr() {}
        ~Ctr() {}
        virtual void make(LocalType& obj) const {}
        virtual size_t size() const {
            return 1234;
        }
    };

    Ctr ctr;

    BOOST_CHECK_EQUAL(ctr.size(), 1234);
}


BOOST_AUTO_TEST_CASE( test_pmem_atomic_constructor_build )
{
    // Define a "spy" type, that can be used to test that make() was called, despite make()
    // being const in the class
    //
    // (We want to explicitly test that the make() virtual overload is called, rather than the
    //  behaviour coming from somewhere else...).
    class Spy {

    public: // methods

        Spy() : called_(false) {}
        ~Spy() {}

        void call() const {
            called_ = true;
        }

        bool called() const {
            return called_;
        }

    private: // members

        mutable bool called_;
    };


    struct Ctr : public AtomicConstructor<LocalType> {

        Ctr(uint32_t value, const Spy& spy) : spy_(spy), value_(value) {}
        ~Ctr() {}

        virtual void make(LocalType& obj) const {
            obj.elem = value_;
            obj.elem2 = value_;
            spy_.call();
        }

        const Spy& spy_;
        uint32_t value_;
    };


    // Pass in a well defined value
    Spy spy;
    Ctr ctr(666, spy);

    // Manuallly initialise a localtype object
    LocalType obj;
    obj.elem = 0;
    obj.elem2 = 0;

    // Pass this object in as raw memory, as is done by ::pmem_constructor
    BOOST_CHECK(!spy.called());
    ctr.build(&obj);
    BOOST_CHECK(spy.called());

    BOOST_CHECK_EQUAL(obj.elem, 666);
    BOOST_CHECK_EQUAL(obj.elem2, 666);
}

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
