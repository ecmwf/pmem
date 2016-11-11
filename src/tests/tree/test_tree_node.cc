/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#define BOOST_TEST_MODULE test_tree

#include "ecbuild/boost_test_framework.h"

#include "eckit/parser/JSONDataBlob.h"

#include "tree/TreeNode.h"

#include "tests/pmem/test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;
using namespace tree;

//----------------------------------------------------------------------------------------------------------------------

/// Define a root type. Each test that does allocation should use a different element in the root object.

// How many possibilities do we want?
const size_t root_elems = 7;


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

    PersistentPtr<TreeNode> data_[root_elems];
};


/// Write a 'spy' derived object, that allows us to examine the pretected internal members, to check that
/// they are doing the right things.

class TreeNodeSpy : public TreeNode {
public:
    using TreeNode::items;
};


//----------------------------------------------------------------------------------------------------------------------

// And structure the pool with types

template<> uint64_t pmem::PersistentType<RootType>::type_id = POBJ_ROOT_TYPE_NUM;
template<> uint64_t pmem::PersistentType<TreeNode>::type_id = 1;
template<> uint64_t pmem::PersistentType<pmem::PersistentVectorData<TreeNode> >::type_id = 2;

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

BOOST_GLOBAL_FIXTURE( SuitePoolFixture )

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE( test_tree_node )

BOOST_AUTO_TEST_CASE( test_tree_node_raw_blob )
{
    PersistentPtr<TreeNode>& first(global_root->data_[0]);

    BOOST_CHECK(first.null());

    // Ensure that we can do a direct allocation of a leaf node

    std::string data("\"data 1234\"");
    eckit::JSONDataBlob blob(data.c_str(), data.length());

    first.allocate(TreeNode::Constructor("value1", blob));

    BOOST_CHECK(!first.null());

    // Check that we have created a leaf node, with no subnodes

    BOOST_CHECK(first->leaf());
    BOOST_CHECK_EQUAL(first->nodeCount(), 0);

    // Leaf nodes have a value, by which they are referred by their parents, but no key (as they have no
    // subnodes)

    BOOST_CHECK_EQUAL(first->value(), "value1");
    BOOST_CHECK_EQUAL(first->key(), "");

    // Check that the data has been copied into persistent memory

    BOOST_CHECK(first->data() != blob.buffer());

    PMEMobjpool * pool_data = ::pmemobj_pool_by_ptr(first->data());
    PMEMobjpool * pool_root = ::pmemobj_pool_by_ptr(global_root.get());

    BOOST_CHECK(pool_data != 0);
    BOOST_CHECK_EQUAL(pool_data, pool_root);

    // Check the data contents

    BOOST_CHECK_EQUAL(first->dataSize(), data.length());

    std::string str_out(static_cast<const char*>(first->data()), first->dataSize());

    BOOST_CHECK_EQUAL(data, str_out);
}


BOOST_AUTO_TEST_CASE( test_tree_node_construct_recursive )
{
    PersistentPtr<TreeNode>& first(global_root->data_[1]);

    BOOST_CHECK(first.null());

    // Check that we can insert a leaf at a certain depth

    TreeNode::KeyType key;
    key.push_back(std::make_pair("key1", "value1"));
    key.push_back(std::make_pair("key2", "value2"));
    key.push_back(std::make_pair("key3", "value3"));

    std::string data("\"data 1234\"");
    eckit::JSONDataBlob blob(data.c_str(), data.length());

    first.allocate(TreeNode::Constructor("SAMPLE", key, blob));

    // Check that we have created the correct structure (by manually walking the tree).

    BOOST_CHECK(!first.null());
    BOOST_CHECK(!first->leaf());
    BOOST_CHECK_EQUAL(first->nodeCount(), 1);

    BOOST_CHECK_EQUAL(first->value(), "SAMPLE");
    BOOST_CHECK_EQUAL(first->key(), "key1");
    BOOST_CHECK_EQUAL(first->dataSize(), 0);
    BOOST_CHECK(first->data() == 0);

    // ... next node

    BOOST_CHECK_EQUAL((*reinterpret_cast<TreeNodeSpy*>(first.get())).items().size(), 1);
    const PersistentPtr<TreeNode> child1 = (*reinterpret_cast<TreeNodeSpy*>(first.get())).items()[0];

    BOOST_CHECK(!child1.null());
    BOOST_CHECK(!child1->leaf());
    BOOST_CHECK_EQUAL(child1->nodeCount(), 1);

    BOOST_CHECK_EQUAL(child1->value(), "value1");
    BOOST_CHECK_EQUAL(child1->key(), "key2");
    BOOST_CHECK_EQUAL(child1->dataSize(), 0);
    BOOST_CHECK(child1->data() == 0);

    // ... next node

    BOOST_CHECK_EQUAL((*reinterpret_cast<TreeNodeSpy*>(child1.get())).items().size(), 1);
    const PersistentPtr<TreeNode> child2 = (*reinterpret_cast<TreeNodeSpy*>(child1.get())).items()[0];

    BOOST_CHECK(!child2.null());
    BOOST_CHECK(!child2->leaf());
    BOOST_CHECK_EQUAL(child2->nodeCount(), 1);

    BOOST_CHECK_EQUAL(child2->value(), "value2");
    BOOST_CHECK_EQUAL(child2->key(), "key3");
    BOOST_CHECK_EQUAL(child2->dataSize(), 0);
    BOOST_CHECK(child2->data() == 0);

    // ... leaf node

    BOOST_CHECK_EQUAL((*reinterpret_cast<TreeNodeSpy*>(child2.get())).items().size(), 1);
    const PersistentPtr<TreeNode> child3 = (*reinterpret_cast<TreeNodeSpy*>(child2.get())).items()[0];

    BOOST_CHECK(!child3.null());
    BOOST_CHECK(child3->leaf());
    BOOST_CHECK_EQUAL(child3->nodeCount(), 0);

    BOOST_CHECK_EQUAL(child3->value(), "value3");
    BOOST_CHECK_EQUAL(child3->key(), "");

    // Check that the data has been copied into persistent memory

    BOOST_CHECK(child3->data() != 0);
    BOOST_CHECK(child3->data() != blob.buffer());

    PMEMobjpool * pool_data = ::pmemobj_pool_by_ptr(child3->data());
    PMEMobjpool * pool_root = ::pmemobj_pool_by_ptr(global_root.get());

    BOOST_CHECK(pool_data != 0);
    BOOST_CHECK_EQUAL(pool_data, pool_root);

    // Check the data contents

    BOOST_CHECK_EQUAL(child3->dataSize(), data.length());

    std::string str_out(static_cast<const char*>(child3->data()), child3->dataSize());

    BOOST_CHECK_EQUAL(data, str_out);
}


BOOST_AUTO_TEST_CASE( test_tree_node_construct_addNode )
{
    PersistentPtr<TreeNode>& first(global_root->data_[2]);

    BOOST_CHECK(first.null());

    // Check that we can insert a leaf at a certain depth

    TreeNode::KeyType key;
    key.push_back(std::make_pair("key1", "value1"));
    key.push_back(std::make_pair("key2", "value2"));
    key.push_back(std::make_pair("key3", "value3"));

    std::string data("\"data 1234\"");
    eckit::JSONDataBlob blob(data.c_str(), data.length());

    first.allocate(TreeNode::Constructor("SAMPLE", key, blob));

    // Add a second element to the tree being built

    TreeNode::KeyType key2;
    key2.push_back(std::make_pair("key1", "value1"));
    key2.push_back(std::make_pair("key2", "value2a"));
    key2.push_back(std::make_pair("key99", "valueX"));

    std::string data2("\"Another bit of data that is a bit longer than the previous one.......\"");
    eckit::JSONDataBlob blob2(data2.c_str(), data2.length());

    first->addNode(key2, blob2);

    // Check that the structure is correct.

    BOOST_CHECK(!first.null());
    BOOST_CHECK(!first->leaf());
    BOOST_CHECK_EQUAL(first->nodeCount(), 1);

    BOOST_CHECK_EQUAL(first->value(), "SAMPLE");
    BOOST_CHECK_EQUAL(first->key(), "key1");
    BOOST_CHECK_EQUAL(first->dataSize(), 0);
    BOOST_CHECK(first->data() == 0);

    // ... next node

    BOOST_CHECK_EQUAL((*reinterpret_cast<TreeNodeSpy*>(first.get())).items().size(), 1);
    const PersistentPtr<TreeNode> child1 = (*reinterpret_cast<TreeNodeSpy*>(first.get())).items()[0];

    BOOST_CHECK(!child1.null());
    BOOST_CHECK(!child1->leaf());
    BOOST_CHECK_EQUAL(child1->nodeCount(), 2);

    BOOST_CHECK_EQUAL(child1->value(), "value1");
    BOOST_CHECK_EQUAL(child1->key(), "key2");
    BOOST_CHECK_EQUAL(child1->dataSize(), 0);
    BOOST_CHECK(child1->data() == 0);

    // ... Follow only the second branch (have checked the original add in a previous test).

    BOOST_CHECK_EQUAL((*reinterpret_cast<TreeNodeSpy*>(child1.get())).items().size(), 2);
    const PersistentPtr<TreeNode> child2 = (*reinterpret_cast<TreeNodeSpy*>(child1.get())).items()[1];

    BOOST_CHECK(!child2.null());
    BOOST_CHECK(!child2->leaf());
    BOOST_CHECK_EQUAL(child2->nodeCount(), 1);

    BOOST_CHECK_EQUAL(child2->value(), "value2a");
    BOOST_CHECK_EQUAL(child2->key(), "key99");
    BOOST_CHECK_EQUAL(child2->dataSize(), 0);
    BOOST_CHECK(child2->data() == 0);

    // ... leaf node

    BOOST_CHECK_EQUAL((*reinterpret_cast<TreeNodeSpy*>(child2.get())).items().size(), 1);
    const PersistentPtr<TreeNode> child3 = (*reinterpret_cast<TreeNodeSpy*>(child2.get())).items()[0];

    BOOST_CHECK(!child3.null());
    BOOST_CHECK(child3->leaf());
    BOOST_CHECK_EQUAL(child3->nodeCount(), 0);

    BOOST_CHECK_EQUAL(child3->value(), "valueX");
    BOOST_CHECK_EQUAL(child3->key(), "");

    // Check that the data has been copied into persistent memory

    BOOST_CHECK(child3->data() != 0);
    BOOST_CHECK(child3->data() != blob.buffer());

    PMEMobjpool * pool_data = ::pmemobj_pool_by_ptr(child3->data());
    PMEMobjpool * pool_root = ::pmemobj_pool_by_ptr(global_root.get());

    BOOST_CHECK(pool_data != 0);
    BOOST_CHECK_EQUAL(pool_data, pool_root);

    // Check the data contents

    BOOST_CHECK_EQUAL(child3->dataSize(), data2.length());

    std::string str_out(static_cast<const char*>(child3->data()), child3->dataSize());

    BOOST_CHECK_EQUAL(data2, str_out);
}


BOOST_AUTO_TEST_CASE( test_tree_node_construct_branch_value )
{
    // When we are attempting to read a nodes children, the key is already set
    //
    // --> Branching must occur by specifying a different value, not a different key
    // --> (As shown in test _construct_addNode it is possible to have differing keys after the branch point).

    PersistentPtr<TreeNode>& first(global_root->data_[3]);

    BOOST_CHECK(first.null());

    // Insert a leaf at a certain depth

    TreeNode::KeyType key;
    key.push_back(std::make_pair("key1", "value1"));
    key.push_back(std::make_pair("key2", "value2"));
    key.push_back(std::make_pair("key3", "value3"));

    std::string data("\"data 1234\"");
    eckit::JSONDataBlob blob(data.c_str(), data.length());

    first.allocate(TreeNode::Constructor("SAMPLE", key, blob));

    // Attempt to branch by changing key2

    TreeNode::KeyType key2;
    key2.push_back(std::make_pair("key1", "value1"));
    key2.push_back(std::make_pair("key2a", "value2"));
    key2.push_back(std::make_pair("key3", "value3"));

    std::string data2("\"Some more data\"");
    eckit::JSONDataBlob blob2(data2.c_str(), data2.length());

    BOOST_CHECK_THROW(first->addNode(key2, blob2), AssertionFailed);
}


BOOST_AUTO_TEST_CASE( test_tree_node_construct_duplicate )
{
    // When we are attempting to read a nodes children, the key is already set
    //
    // --> Branching must occur by specifying a different value, not a different key
    // --> (As shown in test _construct_addNode it is possible to have differing keys after the branch point).

    PersistentPtr<TreeNode>& first(global_root->data_[4]);

    BOOST_CHECK(first.null());

    // Insert a leaf at a certain depth

    TreeNode::KeyType key;
    key.push_back(std::make_pair("key1", "value1"));
    key.push_back(std::make_pair("key2", "value2"));
    key.push_back(std::make_pair("key3", "value3"));

    std::string data("\"data 1234\"");
    eckit::JSONDataBlob blob(data.c_str(), data.length());

    first.allocate(TreeNode::Constructor("SAMPLE", key, blob));

    // What happens when we attempt to insert a leaf to the same key?

    std::string data2("\"Another bit of data\"");
    eckit::JSONDataBlob blob2(data2.c_str(), data2.length());

    BOOST_CHECK_THROW(first->addNode(key, blob2), TreeNode::LeafExistsError);
}


BOOST_AUTO_TEST_CASE( test_tree_node_construct_leaf_branch )
{
    // We cannot add further nodes to a leaf node.

    PersistentPtr<TreeNode>& first(global_root->data_[5]);

    BOOST_CHECK(first.null());

    // Insert a leaf at a certain depth

    TreeNode::KeyType key;
    key.push_back(std::make_pair("key1", "value1"));
    key.push_back(std::make_pair("key2", "value2"));

    std::string data("\"data 1234\"");
    eckit::JSONDataBlob blob(data.c_str(), data.length());

    first.allocate(TreeNode::Constructor("SAMPLE", key, blob));

    // What happens when we attempt to insert a leaf to the same key?

    key.push_back(std::make_pair("key3", "value3"));

    BOOST_CHECK_THROW(first->addNode(key, blob), TreeNode::LeafExistsError);
}

BOOST_AUTO_TEST_CASE( test_tree_node_locate_leaf )
{
    PersistentPtr<TreeNode>& first(global_root->data_[6]);

    BOOST_CHECK(first.null());

    // Construct a

    TreeNode::KeyType key;
    key.push_back(std::make_pair("key1", "value1"));
    key.push_back(std::make_pair("key2", "value2"));
    key.push_back(std::make_pair("key3", "value3"));

    std::string data("\"data 1234\"");
    eckit::JSONDataBlob blob(data.c_str(), data.length());


    TreeNode::KeyType key2;
    key2.push_back(std::make_pair("key1", "value1"));
    key2.push_back(std::make_pair("key2", "value2a"));
    key2.push_back(std::make_pair("key3", "value3"));

    std::string data2("\"Some more data\"");
    eckit::JSONDataBlob blob2(data2.c_str(), data2.length());


    TreeNode::KeyType key3;
    key3.push_back(std::make_pair("key1", "value1"));
    key3.push_back(std::make_pair("key2", "value2a"));
    key3.push_back(std::make_pair("key3", "value3a"));

    std::string data3("\"Woooooooohoooooooooooooooooooooo\"");
    eckit::JSONDataBlob blob3(data3.c_str(), data3.length());


    first.allocate(TreeNode::Constructor("SAMPLE", key, blob));
    first->addNode(key2, blob2);
    first->addNode(key3, blob3);

    // Do a lookup for a non-existent key

    StringDict request;
    request["key1"] = "value1";
    request["key2"] = "value_bad";
    request["key3"] = "value3";

    BOOST_CHECK_EQUAL(first->lookup(request).size(), 0);

    // Find a single key, as _fully_ specified

    StringDict request2;
    for (TreeNode::KeyType::const_iterator it = key2.begin(); it != key2.end(); ++it) {
        request2[it->first] = it->second;
    }

    std::vector<PersistentPtr<TreeNode> > result2 = first->lookup(request2);

    BOOST_CHECK_EQUAL(result2.size(), 1);
    BOOST_CHECK_EQUAL(result2[0]->value(), "value3");
    BOOST_CHECK_EQUAL(std::string((const char*)result2[0]->data(), result2[0]->dataSize()), data2);

    //// Do a wildcard (underspecified) lookup

    StringDict request3;
    request3["key1"] = "value1";
    request3["key3"] = "value3";

    std::vector<PersistentPtr<TreeNode> > result3 = first->lookup(request3);

    BOOST_CHECK_EQUAL(result3.size(), 2);
    BOOST_CHECK_EQUAL(result3[0]->value(), "value3");
    BOOST_CHECK_EQUAL(result3[1]->value(), "value3");
    BOOST_CHECK_EQUAL(std::string((const char*)result3[0]->data(), result3[0]->dataSize()), data);
    BOOST_CHECK_EQUAL(std::string((const char*)result3[1]->data(), result3[1]->dataSize()), data2);
}


//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
