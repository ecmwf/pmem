/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016

#include "eckit/io/DataBlob.h"
#include "eckit/log/Log.h"
#include "eckit/parser/JSONDataBlob.h"
#include "eckit/types/Types.h"

#include "tree/PersistentBuffer.h"
#include "tree/TreeNode.h"
#include "tree/TreeRoot.h"
#include "tree/TreeSchema.h"

using namespace eckit;
using namespace pmem;


namespace tree {

// -------------------------------------------------------------------------------------------------


TreeRoot::Constructor::Constructor(const TreeSchema& schema) :
    schema_(schema) {}


void TreeRoot::Constructor::make(TreeRoot& object) const {

    object.tag_ = TreeRootTag;
    object.node_.nullify();
    object.schema_.nullify();

    // Creata a data blob from the schema, so we can store it
    std::string json = schema_.json_str();

    PersistentBuffer::Constructor ctr(json.c_str(), json.length());

    object.schema_.allocate(ctr);
}

// -------------------------------------------------------------------------------------------------

/*
 * We can use whatever knowledge we have to test the validity of the structure.
 *
 * For now, we just check that the tag is set (i.e. it is initialised).
 */
bool TreeRoot::valid() const {

    return ( tag_ == TreeRootTag &&
             !schema_.null() );
}


PersistentPtr<TreeNode> TreeRoot::rootNode() const {

    return node_;
}


void TreeRoot::addNode(const KeyType& key, const eckit::DataBlob& blob) {

    ASSERT(key.size() != 0);

    Log::info() << "addNode: " << key << std::endl << std::flush;

    // If we don't yet have a root node, we need to create it.
    // n.b. This could in principle be done in the TreeRoot constructor, if we assumed we
    //      knew the data schema in advance.
    if (node_.null()) {
        TreeNode::Constructor ctr(key[0].first, key, blob);
        node_.allocate(ctr);
    } else {
        ASSERT(node_->name() == key[0].first);
        node_->addNode(key, blob);
    }
}

// -------------------------------------------------------------------------------------------------

TreeObject::TreeObject(TreeRoot &root) :
    root_(root) {

    std::string str_schema(reinterpret_cast<const char*>(root_.schema_->data()), root_.schema_->size());
    std::istringstream iss(str_schema);
    schema_ = TreeSchema(iss);

    Log::info() << "Created TreeObject wrapper." << std::endl;
    Log::info() << "Schema: " << schema_ << std::endl;
}

TreeObject::~TreeObject() {}

void TreeObject::print(std::ostream& os) const {
    os << "TreeObject [TreeRoot wrapper]";
}

void TreeObject::addNode(const StringDict& key, const DataBlob &blob) {

    root_.addNode(schema_.processInsertKey(key), blob);
}


void TreeObject::printTree(std::ostream& os) const {

    PersistentPtr<TreeNode> rootNode = root_.rootNode();
    if (!rootNode.null())
        rootNode->printTree(os);
}


std::vector<PersistentPtr<TreeNode> > TreeObject::lookup(const StringDict &key) {
    PersistentPtr<TreeNode> rootNode = root_.rootNode();
    if (!rootNode.null())
        return rootNode->lookup(key);
    else
        return std::vector<PersistentPtr<TreeNode> >();
}

// -------------------------------------------------------------------------------------------------

} // namespace tree
