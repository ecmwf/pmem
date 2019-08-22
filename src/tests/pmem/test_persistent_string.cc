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

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "pmem/PersistentString.h"
#include "pmem/PersistentPtr.h"

#include "test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;
using namespace eckit::testing;

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

CASE( "test_pmem_persistent_string_valid_persistent_ptr" )
{
    // If PersistentBuffer is not OK, this will trigger StaticAssert
    PersistentPtr<PersistentString> ptr;
}

CASE( "test_pmem_persistent_string_allocate" )
{
    std::string test_string = "I am a test string";

    AutoPool ap((RootType::Constructor()));
    PersistentPtr<RootType> root = ap.pool_.getRoot<RootType>();

    root->data_.allocate(test_string);

    std::string get_back(root->data_->c_str(), root->data_->length());
    EXPECT(get_back == test_string);
}

CASE( "test_pmem_persistent_string_size" )
{
    std::string str_in("");
    AtomicConstructor1<PersistentString, std::string> ctr(str_in);

    // n.b. we store the null character, so that data() and c_str() can be implemented O(1) according to the std.

    // Check that space is allocated to store the data, and to store the size of the data
    EXPECT(ctr.size() == sizeof(size_t) + sizeof(char));

    std::string str_in2("1234");
    AtomicConstructor1<PersistentString, std::string> ctr2(str_in2);

    // Check that space is allocated to store the data, and to store the size of the data
    EXPECT(ctr2.size() == sizeof(size_t) + 5 * sizeof(char));
}


CASE( "test_pmem_persistent_string_empty" )
{
    PersistentMock<PersistentString> stringMock(std::string(""));
    PersistentString& str(stringMock.object());

    EXPECT(str == std::string(""));
    EXPECT(str.size() == size_t(0));
    EXPECT(str.length() == size_t(0));
}


CASE( "test_pmem_persistent_string_real" )
{
    PersistentMock<PersistentString> stringMock(std::string("this is a string"));
    PersistentString& str(stringMock.object());

    EXPECT(str == std::string("this is a string"));
    EXPECT(str.size() == size_t(16));
    EXPECT(str.length() == size_t(16));

    EXPECT(str[0] == 't');
    EXPECT(str[15] == 'g');

    EXPECT(::strcmp("this is a string", str.c_str()) == 0);
}


CASE( "test_pmem_persistent_string_out_of_range" )
{
    struct Tester {
        void operator() () {
            PersistentMock<PersistentString> stringMock(std::string("this is a string"));
            PersistentString& str(stringMock.object());

            str[666];
        }
    };

    //BOOST_CHECK_THROW(Tester()(), OutOfRange);
}


CASE( "test_pmem_persistent_string_equality_operators" )
{
    PersistentMock<PersistentString> stringMock1(std::string("This is a string 1"));
    PersistentMock<PersistentString> stringMock1b(std::string("This is a string 1"));
    PersistentMock<PersistentString> stringMock2(std::string("This is a string 2"));

    PersistentString& str1(stringMock1.object());
    PersistentString& str1b(stringMock1b.object());
    PersistentString& str2(stringMock2.object());

    // Compare two Persistent Strings

    EXPECT(str1 == str1b);         EXPECT(!(str1 != str1b));
    EXPECT(str1 != str2);          EXPECT(!(str1 == str2));
    EXPECT(str2 != str1);          EXPECT(!(str2 == str1));

    // Compare persistent strings to C strings

    const char* cstrsame = "This is a string 1";
    const char* cstrdiff = "This is another str";
    const char* cstrshorter = "This";
    const char* cstrlonger = "This is a string that is somewhat longer";

    EXPECT(str1 == cstrsame);      EXPECT(!(str1 != cstrsame));
    EXPECT(str1 != cstrdiff);      EXPECT(!(str1 == cstrdiff));
    EXPECT(str1 != cstrshorter);   EXPECT(!(str1 == cstrshorter));
    EXPECT(str1 != cstrlonger);    EXPECT(!(str1 == cstrlonger));

    EXPECT(cstrsame == str1);      EXPECT(!(cstrsame != str1));
    EXPECT(cstrdiff != str1);      EXPECT(!(cstrdiff == str1));
    EXPECT(cstrshorter != str1);   EXPECT(!(cstrshorter == str1));
    EXPECT(cstrlonger != str1);    EXPECT(!(cstrlonger == str1));

    // Compare persistent strings to std::strings

    std::string strsame(cstrsame);
    std::string strdiff(cstrdiff);
    std::string strshorter(cstrshorter);
    std::string strlonger(cstrlonger);

    EXPECT(str1 == strsame);       EXPECT(!(str1 != strsame));
    EXPECT(str1 != strdiff);       EXPECT(!(str1 == strdiff));
    EXPECT(str1 != strshorter);    EXPECT(!(str1 == strshorter));
    EXPECT(str1 != strlonger);     EXPECT(!(str1 == strlonger));

    EXPECT(strsame == str1);       EXPECT(!(strsame != str1));
    EXPECT(strdiff != str1);       EXPECT(!(strdiff == str1));
    EXPECT(strshorter != str1);    EXPECT(!(strshorter == str1));
    EXPECT(strlonger != str1);     EXPECT(!(strlonger == str1));
}


//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
