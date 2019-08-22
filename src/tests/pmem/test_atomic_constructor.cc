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

#include <cstdint>

#include "eckit/testing/Test.h"

#include "pmem/AtomicConstructor.h"

using namespace std;
using namespace pmem;
using namespace eckit::testing;

namespace {

    struct LocalType {

        LocalType() : elem(11), elem2(99) {}
        LocalType(int x) : elem(x), elem2(99) {}
        LocalType(double x, std::string y) : elem(22), elem2(33) {}

        uint32_t elem;
        uint32_t elem2;
    };

    template<> uint64_t ::pmem::PersistentType<LocalType>::type_id = 99;
}


namespace pmem {
// Define a custom size and type_id function for the single-parameter constructor
template<>
size_t ::pmem::AtomicConstructor1Base<LocalType, int>::size() const {
    return x1_ * 3;
}

template<>
uint64_t AtomicConstructor1Base<LocalType, int>::type_id() const {
    return 4321;
}
}

//----------------------------------------------------------------------------------------------------------------------

CASE( "test_pmem_atomic_constructor_default_size" )
{
    struct Ctr : public AtomicConstructor<LocalType> {
        Ctr() {}
        ~Ctr() {}
        virtual void make(LocalType& obj) const {}
    };

    Ctr ctr;

    EXPECT(ctr.size() == 8);
}


CASE( "test_pmem_atomic_constructor_manual_size" )
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

    EXPECT(ctr.size() == 1234);
}


CASE( "test_pmem_atomic_constructor_build" )
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
    EXPECT(!spy.called());
    ctr.build(&obj);
    EXPECT(spy.called());

    EXPECT(obj.elem == 666);
    EXPECT(obj.elem2 == 666);
}

//----------------------------------------------------------------------------------------------------------------------

// Consider the magic automatic constructors

CASE( "test_pmem_atomic_constructor0" )
{
    AtomicConstructor0<LocalType> ctr;

    EXPECT(ctr.size() == 8);
    EXPECT(ctr.type_id() == 99);

    LocalType x;
    x.elem = 0;
    x.elem2 = 0;

    ctr.build(&x);
    EXPECT(x.elem == 11);

}


CASE( "test_pmem_atomic_constructor1" )
{
    // This test makes use of the customised size/type_id functions!

    int arg1 = 33;
    AtomicConstructor1<LocalType, int> ctr(arg1);

    EXPECT(ctr.size() == 99);
    EXPECT(ctr.type_id() == 4321);

    LocalType x;
    x.elem = 0;
    x.elem2 = 0;

    ctr.build(&x);
    EXPECT(x.elem == 33);

}


CASE( "test_pmem_atomic_constructor2" )
{
    // This test makes use of the customised size/type_id functions!

    int arg1 = 33;
    std::string arg2 = "An argument";
    AtomicConstructor2<LocalType, double, std::string> ctr(arg1, arg2);

    EXPECT(ctr.size() == 8);
    EXPECT(ctr.type_id() == 99);

    LocalType x;
    x.elem = 0;
    x.elem2 = 0;

    ctr.build(&x);
    EXPECT(x.elem == 22);
    EXPECT(x.elem2 == 33);
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
