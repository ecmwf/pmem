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

#include "eckit/parser/JSONDataBlob.h"
#include "eckit/testing/Test.h"

#include "pmem/tree/TreeNode.h"

#include "tests/pmem/test_persistent_helpers.h"

using namespace std;
using namespace pmem;
using namespace eckit;
using namespace eckit::testing;
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

AutoPool globalAutoPool((RootType::Constructor()));
PersistentPool* global_pool = &globalAutoPool.pool_;

struct GlobalRootFixture : public PersistentPtr<RootType> {
 GlobalRootFixture() : PersistentPtr<RootType>(globalAutoPool.pool_.getRoot<RootType>()) {}
    ~GlobalRootFixture() { nullify(); }
};

GlobalRootFixture global_root;

//----------------------------------------------------------------------------------------------------------------------

CASE( "test_tree_node_placeholder" ) {
//    EXPECT(false);
}

CASE( "test_tree_node_raw_blob" )
{
    PersistentPtr<TreeNode>& first(global_root->data_[0]);

    EXPECT(first.null());

    // Ensure that we can do a direct allocation of a leaf node

    std::string data("\"data 1234\"");
    eckit::JSONDataBlob blob(data.c_str(), data.length());

    first.setPersist(TreeNode::allocateLeaf(*global_pool, "value1", blob));

    EXPECT(!first.null());

    // Check that we have created a leaf node, with no subnodes

    EXPECT(first->leaf());
    EXPECT(first->nodeCount() == size_t(0));

    // Leaf nodes have a value, by which they are referred by their parents, but no key (as they have no
    // subnodes)

    EXPECT(first->value() == "value1");
    EXPECT(first->key() == "");

    // Check that the data has been copied into persistent memory

    EXPECT(first->data() != blob.buffer());

    PMEMobjpool * pool_data = ::pmemobj_pool_by_ptr(first->data());
    PMEMobjpool * pool_root = ::pmemobj_pool_by_ptr(global_root.get());

    EXPECT(pool_data != 0);
    EXPECT(pool_data == pool_root);

    // Check the data contents

    EXPECT(first->dataSize() == data.length());

    std::string str_out(static_cast<const char*>(first->data()), first->dataSize());

    EXPECT(data == str_out);
}


CASE( "test_tree_node_construct_recursive" )
{
    PersistentPtr<TreeNode>& first(global_root->data_[1]);

    EXPECT(first.null());

    // Check that we can insert a leaf at a certain depth

    TreeNode::KeyType key;
    key.push_back(std::make_pair("key1", "value1"));
    key.push_back(std::make_pair("key2", "value2"));
    key.push_back(std::make_pair("key3", "value3"));

    std::string data("\"data 1234\"");
    eckit::JSONDataBlob blob(data.c_str(), data.length());

    first.setPersist(TreeNode::allocateNested(*global_pool, "SAMPLE", key, blob));

    // Check that we have created the correct structure (by manually walking the tree).

    EXPECT(!first.null());
    EXPECT(!first->leaf());
    EXPECT(first->nodeCount() == size_t(1));

    EXPECT(first->value() == "SAMPLE");
    EXPECT(first->key() == "key1");
    EXPECT(first->dataSize() == size_t(0));
    EXPECT(first->data() == 0);

    // ... next node

    EXPECT((*reinterpret_cast<TreeNodeSpy*>(first.get())).items().size() == size_t(1));
    const PersistentPtr<TreeNode> child1 = (*reinterpret_cast<TreeNodeSpy*>(first.get())).items()[0];

    EXPECT(!child1.null());
    EXPECT(!child1->leaf());
    EXPECT(child1->nodeCount() == size_t(1));

    EXPECT(child1->value() == "value1");
    EXPECT(child1->key() == "key2");
    EXPECT(child1->dataSize() == size_t(0));
    EXPECT(child1->data() == 0);

    // ... next node

    EXPECT((*reinterpret_cast<TreeNodeSpy*>(child1.get())).items().size() == size_t(1));
    const PersistentPtr<TreeNode> child2 = (*reinterpret_cast<TreeNodeSpy*>(child1.get())).items()[0];

    EXPECT(!child2.null());
    EXPECT(!child2->leaf());
    EXPECT(child2->nodeCount() == size_t(1));

    EXPECT(child2->value() == "value2");
    EXPECT(child2->key() == "key3");
    EXPECT(child2->dataSize() == size_t(0));
    EXPECT(child2->data() == 0);

    // ... leaf node

    EXPECT((*reinterpret_cast<TreeNodeSpy*>(child2.get())).items().size() == size_t(1));
    const PersistentPtr<TreeNode> child3 = (*reinterpret_cast<TreeNodeSpy*>(child2.get())).items()[0];

    EXPECT(!child3.null());
    EXPECT(child3->leaf());
    EXPECT(child3->nodeCount() == size_t(0));

    EXPECT(child3->value() == "value3");
    EXPECT(child3->key() == "");

    // Check that the data has been copied into persistent memory

    EXPECT(child3->data() != 0);
    EXPECT(child3->data() != blob.buffer());

    PMEMobjpool * pool_data = ::pmemobj_pool_by_ptr(child3->data());
    PMEMobjpool * pool_root = ::pmemobj_pool_by_ptr(global_root.get());

    EXPECT(pool_data != 0);
    EXPECT(pool_data == pool_root);

    // Check the data contents

    EXPECT(child3->dataSize() == data.length());

    std::string str_out(static_cast<const char*>(child3->data()), child3->dataSize());

    EXPECT(data == str_out);
}


CASE( "test_tree_node_construct_addNode" )
{
    PersistentPtr<TreeNode>& first(global_root->data_[2]);

    EXPECT(first.null());

    // Check that we can insert a leaf at a certain depth

    TreeNode::KeyType key;
    key.push_back(std::make_pair("key1", "value1"));
    key.push_back(std::make_pair("key2", "value2"));
    key.push_back(std::make_pair("key3", "value3"));

    std::string data("\"data 1234\"");
    eckit::JSONDataBlob blob(data.c_str(), data.length());

    first.setPersist(TreeNode::allocateNested(*global_pool, "SAMPLE", key, blob));

    // Add a second element to the tree being built

    TreeNode::KeyType key2;
    key2.push_back(std::make_pair("key1", "value1"));
    key2.push_back(std::make_pair("key2", "value2a"));
    key2.push_back(std::make_pair("key99", "valueX"));

    std::string data2("\"Another bit of data that is a bit longer than the previous one.......\"");
    eckit::JSONDataBlob blob2(data2.c_str(), data2.length());

    first->addNode(key2, blob2);

    // Check that the structure is correct.

    EXPECT(!first.null());
    EXPECT(!first->leaf());
    EXPECT(first->nodeCount() == size_t(1));

    EXPECT(first->value() == "SAMPLE");
    EXPECT(first->key() == "key1");
    EXPECT(first->dataSize() == size_t(0));
    EXPECT(first->data() == 0);

    // ... next node

    EXPECT((*reinterpret_cast<TreeNodeSpy*>(first.get())).items().size() == size_t(1));
    const PersistentPtr<TreeNode> child1 = (*reinterpret_cast<TreeNodeSpy*>(first.get())).items()[0];

    EXPECT(!child1.null());
    EXPECT(!child1->leaf());
    EXPECT(child1->nodeCount() == size_t(2));

    EXPECT(child1->value() == "value1");
    EXPECT(child1->key() == "key2");
    EXPECT(child1->dataSize() == size_t(0));
    EXPECT(child1->data() == 0);

    // ... Follow only the second branch (have checked the original add in a previous test).

    EXPECT((*reinterpret_cast<TreeNodeSpy*>(child1.get())).items().size() == size_t(2));
    const PersistentPtr<TreeNode> child2 = (*reinterpret_cast<TreeNodeSpy*>(child1.get())).items()[1];

    EXPECT(!child2.null());
    EXPECT(!child2->leaf());
    EXPECT(child2->nodeCount() == size_t(1));

    EXPECT(child2->value() == "value2a");
    EXPECT(child2->key() == "key99");
    EXPECT(child2->dataSize() == size_t(0));
    EXPECT(child2->data() == 0);

    // ... leaf node

    EXPECT((*reinterpret_cast<TreeNodeSpy*>(child2.get())).items().size() == size_t(1));
    const PersistentPtr<TreeNode> child3 = (*reinterpret_cast<TreeNodeSpy*>(child2.get())).items()[0];

    EXPECT(!child3.null());
    EXPECT(child3->leaf());
    EXPECT(child3->nodeCount() == size_t(0));

    EXPECT(child3->value() == "valueX");
    EXPECT(child3->key() == "");

    // Check that the data has been copied into persistent memory

    EXPECT(child3->data() != 0);
    EXPECT(child3->data() != blob.buffer());

    PMEMobjpool * pool_data = ::pmemobj_pool_by_ptr(child3->data());
    PMEMobjpool * pool_root = ::pmemobj_pool_by_ptr(global_root.get());

    EXPECT(pool_data != 0);
    EXPECT(pool_data == pool_root);

    // Check the data contents

    EXPECT(child3->dataSize() == data2.length());

    std::string str_out(static_cast<const char*>(child3->data()), child3->dataSize());

    EXPECT(data2 == str_out);
}


CASE( "test_tree_node_construct_branch_value" )
{
    // When we are attempting to read a nodes children, the key is already set
    //
    // --> Branching must occur by specifying a different value, not a different key
    // --> (As shown in test _construct_addNode it is possible to have differing keys after the branch point).

    PersistentPtr<TreeNode>& first(global_root->data_[3]);

    EXPECT(first.null());

    // Insert a leaf at a certain depth

    TreeNode::KeyType key;
    key.push_back(std::make_pair("key1", "value1"));
    key.push_back(std::make_pair("key2", "value2"));
    key.push_back(std::make_pair("key3", "value3"));

    std::string data("\"data 1234\"");
    eckit::JSONDataBlob blob(data.c_str(), data.length());

    first.setPersist(TreeNode::allocateNested(*global_pool, "SAMPLE", key, blob));

    // Attempt to branch by changing key2

    TreeNode::KeyType key2;
    key2.push_back(std::make_pair("key1", "value1"));
    key2.push_back(std::make_pair("key2a", "value2"));
    key2.push_back(std::make_pair("key3", "value3"));

    std::string data2("\"Some more data\"");
    eckit::JSONDataBlob blob2(data2.c_str(), data2.length());

    EXPECT_THROWS_AS(first->addNode(key2, blob2), AssertionFailed);
}


CASE( "test_tree_node_construct_duplicate" )
{
    // When we are attempting to read a nodes children, the key is already set
    //
    // --> Branching must occur by specifying a different value, not a different key
    // --> (As shown in test _construct_addNode it is possible to have differing keys after the branch point).

    PersistentPtr<TreeNode>& first(global_root->data_[4]);

    EXPECT(first.null());

    // Insert a leaf at a certain depth

    TreeNode::KeyType key;
    key.push_back(std::make_pair("key1", "value1"));
    key.push_back(std::make_pair("key2", "value2"));
    key.push_back(std::make_pair("key3", "value3"));

    std::string data("\"data 1234\"");
    eckit::JSONDataBlob blob(data.c_str(), data.length());

    first.setPersist(TreeNode::allocateNested(*global_pool, "SAMPLE", key, blob));

    // What happens when we attempt to insert a leaf to the same key?

    std::string data2("\"Another bit of data\"");
    eckit::JSONDataBlob blob2(data2.c_str(), data2.length());

    EXPECT_THROWS_AS(first->addNode(key, blob2), TreeNode::LeafExistsError);
}


CASE( "test_tree_node_construct_leaf_branch" )
{
    // We cannot add further nodes to a leaf node.

    PersistentPtr<TreeNode>& first(global_root->data_[5]);

    EXPECT(first.null());

    // Insert a leaf at a certain depth

    TreeNode::KeyType key;
    key.push_back(std::make_pair("key1", "value1"));
    key.push_back(std::make_pair("key2", "value2"));

    std::string data("\"data 1234\"");
    eckit::JSONDataBlob blob(data.c_str(), data.length());

    first.setPersist(TreeNode::allocateNested(*global_pool, "SAMPLE", key, blob));

    // What happens when we attempt to insert a leaf to the same key?

    key.push_back(std::make_pair("key3", "value3"));

    EXPECT_THROWS_AS(first->addNode(key, blob), TreeNode::LeafExistsError);
}

CASE( "test_tree_node_locate_leaf" )
{
    PersistentPtr<TreeNode>& first(global_root->data_[6]);

    EXPECT(first.null());

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


    first.setPersist(TreeNode::allocateNested(*global_pool, "SAMPLE", key, blob));
    first->addNode(key2, blob2);
    first->addNode(key3, blob3);

    // Do a lookup for a non-existent key

    StringDict request;
    request["key1"] = "value1";
    request["key2"] = "value_bad";
    request["key3"] = "value3";

    EXPECT(first->lookup(request).size() == size_t(0));

    // Find a single key, as _fully_ specified

    StringDict request2;
    for (TreeNode::KeyType::const_iterator it = key2.begin(); it != key2.end(); ++it) {
        request2[it->first] = it->second;
    }

    std::vector<PersistentPtr<TreeNode> > result2 = first->lookup(request2);

    EXPECT(result2.size() == size_t(1));
    EXPECT(result2[0]->value() == "value3");
    EXPECT(std::string((const char*)result2[0]->data(), result2[0]->dataSize()) == data2);

    //// Do a wildcard (underspecified) lookup

    StringDict request3;
    request3["key1"] = "value1";
    request3["key3"] = "value3";

    std::vector<PersistentPtr<TreeNode> > result3 = first->lookup(request3);

    EXPECT(result3.size() == size_t(2));
    EXPECT(result3[0]->value() == "value3");
    EXPECT(result3[1]->value() == "value3");
    EXPECT(std::string((const char*)result3[0]->data(), result3[0]->dataSize()) == data);
    EXPECT(std::string((const char*)result3[1]->data(), result3[1]->dataSize()) == data2);
}


//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
