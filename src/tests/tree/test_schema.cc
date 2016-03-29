/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#define BOOST_TEST_MODULE test_tree_schema

#include <string>

#include "ecbuild/boost_test_framework.h"

#include "eckit/exception/Exceptions.h"

#include "tree/TreeSchema.h"

using namespace std;
using namespace eckit;
using namespace tree;

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE( test_tree_schema )


BOOST_AUTO_TEST_CASE( test_schema_incorrect_key_count )
{
    std::string schema_str = "[\"key1\", \"key2\"]";
    std::istringstream iss(schema_str);
    TreeSchema schema(iss);

    // Test wrong number of keys
    StringDict key;
    key["key1"] = "value1";
    BOOST_CHECK_THROW(schema.processInsertKey(key), UserError);

    // With the correct number of arguments, this should pass
    key["key2"] = "value2";
    std::vector<std::pair<std::string, std::string> > k = schema.processInsertKey(key);
    BOOST_CHECK_EQUAL(k.size(), 2);

    // And again it should
    key["key3"] = "value3";
    BOOST_CHECK_THROW(schema.processInsertKey(key), UserError);
}

// Test that the correct keys are supplied

BOOST_AUTO_TEST_CASE( test_schema_incorrect_key )
{
    std::string schema_str = "[\"key1\", \"key2\"]";
    std::istringstream iss(schema_str);
    TreeSchema schema(iss);

    // Test wrong number of keys
    StringDict key;
    key["key1"] = "value1";
    key["key3"] = "value3";
    BOOST_CHECK_THROW(schema.processInsertKey(key), UserError);
}

// Test that keys are correctly ordered by the processor

BOOST_AUTO_TEST_CASE( test_schema_key_ordering )
{
    std::string schema_str = "[\"key2\", \"key1\"]";
    std::istringstream iss(schema_str);
    TreeSchema schema(iss);

    // With the correct number of arguments, this should pass
    StringDict key;
    key["key1"] = "value1";
    key["key2"] = "value2";

    std::vector<std::pair<std::string, std::string> > k = schema.processInsertKey(key);
    BOOST_CHECK_EQUAL(k.size(), 2);

    // Check that the keys are returned in the order specified in the schema

    BOOST_CHECK_EQUAL(k[0].first, "key2");
    BOOST_CHECK_EQUAL(k[0].second, "value2");
    BOOST_CHECK_EQUAL(k[1].first, "key1");
    BOOST_CHECK_EQUAL(k[1].second, "value1");
}

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
