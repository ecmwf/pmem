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

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Setup.h"

#include "pmem/PersistentString.h"
#include "pmem/PersistentPtr.h"

#include "test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;
using namespace eckit::testing;

BOOST_GLOBAL_FIXTURE(Setup)

//----------------------------------------------------------------------------------------------------------------------

/// Define a root type.

// How many possibilities do we want?
const size_t root_elems = 1;


class RootType : public PersistentType<RootType> {

public: // constructor

    class Constructor : public AtomicConstructor<RootType> {
        virtual void make(RootType &object) const {
            for (size_t i = 0; i < root_elems; i++) {
                object.data_.nullify();
            }
        }
    };

public: // members

    PersistentPtr<PersistentString> data_;
};

//----------------------------------------------------------------------------------------------------------------------

// And structure the pool with types

template<> uint64_t pmem::PersistentType<RootType>::type_id = POBJ_ROOT_TYPE_NUM;
template<> uint64_t pmem::PersistentType<PersistentString>::type_id = 1;

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE( test_pmem_persistent_string )

BOOST_AUTO_TEST_CASE( test_pmem_persistent_string_valid_persistent_ptr )
{
    // If PersistentBuffer is not OK, this will trigger StaticAssert
    PersistentPtr<PersistentString> ptr;
}

BOOST_AUTO_TEST_CASE( test_pmem_persistent_string_allocate )
{
    std::string test_string = "I am a test string";

    AutoPool ap((RootType::Constructor()));
    PersistentPtr<RootType> root = ap.pool_.getRoot<RootType>();

    root->data_.allocate(PersistentString::Constructor(test_string));

    std::string get_back(root->data_->c_str(), root->data_->length());
    BOOST_CHECK_EQUAL(get_back, test_string);
}

BOOST_AUTO_TEST_CASE( test_pmem_persistent_string_size )
{
    PersistentString::Constructor ctr("");

    // n.b. we store the null character, so that data() and c_str() can be implemented O(1) according to the std.

    // Check that space is allocated to store the data, and to store the size of the data
    BOOST_CHECK_EQUAL(ctr.size(), sizeof(size_t) + sizeof(char));

    PersistentString::Constructor ctr2("1234");

    // Check that space is allocated to store the data, and to store the size of the data
    BOOST_CHECK_EQUAL(ctr2.size(), sizeof(size_t) + 5 * sizeof(char));
}


BOOST_AUTO_TEST_CASE( test_pmem_persistent_string_empty )
{
    PersistentString::Constructor ctr("");

    PersistentMock<PersistentString> stringMock(ctr);
    PersistentString& str(stringMock.object());

    BOOST_CHECK_EQUAL(str, std::string(""));
    BOOST_CHECK_EQUAL(str.size(), 0);
    BOOST_CHECK_EQUAL(str.length(), 0);
}


BOOST_AUTO_TEST_CASE( test_pmem_persistent_string_real )
{
    PersistentString::Constructor ctr("this is a string");

    PersistentMock<PersistentString> stringMock(ctr);
    PersistentString& str(stringMock.object());

    BOOST_CHECK_EQUAL(str, std::string("this is a string"));
    BOOST_CHECK_EQUAL(str.size(), 16);
    BOOST_CHECK_EQUAL(str.length(), 16);

    BOOST_CHECK_EQUAL(str[0], 't');
    BOOST_CHECK_EQUAL(str[15], 'g');

    BOOST_CHECK_EQUAL(::strcmp("this is a string", str.c_str()), 0);
}


BOOST_AUTO_TEST_CASE( test_pmem_persistent_string_out_of_range )
{
    struct Tester {
        void operator() () {
            PersistentString::Constructor ctr("this is a string");

            PersistentMock<PersistentString> stringMock(ctr);
            PersistentString& str(stringMock.object());

            str[666];
        }
    };

    BOOST_CHECK_THROW(Tester()(), OutOfRange);
}


BOOST_AUTO_TEST_CASE( test_pmem_persistent_string_equality_operators )
{
    PersistentMock<PersistentString> stringMock1(PersistentString::Constructor("This is a string 1"));
    PersistentMock<PersistentString> stringMock1b(PersistentString::Constructor("This is a string 1"));
    PersistentMock<PersistentString> stringMock2(PersistentString::Constructor("This is a string 2"));

    PersistentString& str1(stringMock1.object());
    PersistentString& str1b(stringMock1b.object());
    PersistentString& str2(stringMock2.object());

    // Compare two Persistent Strings

    BOOST_CHECK(str1 == str1b);         BOOST_CHECK(!(str1 != str1b));
    BOOST_CHECK(str1 != str2);          BOOST_CHECK(!(str1 == str2));
    BOOST_CHECK(str2 != str1);          BOOST_CHECK(!(str2 == str1));

    // Compare persistent strings to C strings

    const char* cstrsame = "This is a string 1";
    const char* cstrdiff = "This is another str";
    const char* cstrshorter = "This";
    const char* cstrlonger = "This is a string that is somewhat longer";

    BOOST_CHECK(str1 == cstrsame);      BOOST_CHECK(!(str1 != cstrsame));
    BOOST_CHECK(str1 != cstrdiff);      BOOST_CHECK(!(str1 == cstrdiff));
    BOOST_CHECK(str1 != cstrshorter);   BOOST_CHECK(!(str1 == cstrshorter));
    BOOST_CHECK(str1 != cstrlonger);    BOOST_CHECK(!(str1 == cstrlonger));

    BOOST_CHECK(cstrsame == str1);      BOOST_CHECK(!(cstrsame != str1));
    BOOST_CHECK(cstrdiff != str1);      BOOST_CHECK(!(cstrdiff == str1));
    BOOST_CHECK(cstrshorter != str1);   BOOST_CHECK(!(cstrshorter == str1));
    BOOST_CHECK(cstrlonger != str1);    BOOST_CHECK(!(cstrlonger == str1));

    // Compare persistent strings to std::strings

    std::string strsame(cstrsame);
    std::string strdiff(cstrdiff);
    std::string strshorter(cstrshorter);
    std::string strlonger(cstrlonger);

    BOOST_CHECK(str1 == strsame);       BOOST_CHECK(!(str1 != strsame));
    BOOST_CHECK(str1 != strdiff);       BOOST_CHECK(!(str1 == strdiff));
    BOOST_CHECK(str1 != strshorter);    BOOST_CHECK(!(str1 == strshorter));
    BOOST_CHECK(str1 != strlonger);     BOOST_CHECK(!(str1 == strlonger));

    BOOST_CHECK(strsame == str1);       BOOST_CHECK(!(strsame != str1));
    BOOST_CHECK(strdiff != str1);       BOOST_CHECK(!(strdiff == str1));
    BOOST_CHECK(strshorter != str1);    BOOST_CHECK(!(strshorter == str1));
    BOOST_CHECK(strlonger != str1);     BOOST_CHECK(!(strlonger == str1));
}


//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
