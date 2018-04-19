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

#include "pmem/tree/TreeSchema.h"
#include "pmem/tree/TreeNode.h"

using namespace std;
using namespace eckit;
using namespace eckit::testing;
using namespace tree;

//----------------------------------------------------------------------------------------------------------------------

CASE( "test_schema_incorrect_key_count" )
{
    std::string schema_str = "[\"key1\", \"key2\"]";
    std::istringstream iss(schema_str);
    TreeSchema schema(iss);

    // Test wrong number of keys
    StringDict key;
    key["key1"] = "value1";
    EXPECT_THROWS_AS(schema.processInsertKey(key), UserError);

    // With the correct number of arguments, this should pass
    key["key2"] = "value2";
    TreeNode::KeyType k = schema.processInsertKey(key);
    EXPECT(k.size() == size_t(2));

    // And again it should
    key["key3"] = "value3";
    EXPECT_THROWS_AS(schema.processInsertKey(key), UserError);
}

// Test that the correct keys are supplied

CASE( "test_schema_incorrect_key" )
{
    std::string schema_str = "[\"key1\", \"key2\"]";
    std::istringstream iss(schema_str);
    TreeSchema schema(iss);

    // Test wrong number of keys
    StringDict key;
    key["key1"] = "value1";
    key["key3"] = "value3";
    EXPECT_THROWS_AS(schema.processInsertKey(key), UserError);
}

// Test that keys are correctly ordered by the processor

CASE( "test_schema_key_ordering" )
{
    std::string schema_str = "[\"key2\", \"key1\"]";
    std::istringstream iss(schema_str);
    TreeSchema schema(iss);

    // With the correct number of arguments, this should pass
    StringDict key;
    key["key1"] = "value1";
    key["key2"] = "value2";

    TreeNode::KeyType k = schema.processInsertKey(key);
    EXPECT(k.size() == size_t(2));

    // Check that the keys are returned in the order specified in the schema

    EXPECT(k[0].first == "key2");
    EXPECT(k[0].second == "value2");
    EXPECT(k[1].first == "key1");
    EXPECT(k[1].second == "value1");
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
