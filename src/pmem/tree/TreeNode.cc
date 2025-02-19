/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/*
 * This software was developed as part of the EC H2020 funded project NextGenIO
 * (Project ID: 671951) www.nextgenio.eu
 */

/// @author Simon Smart
/// @date   Feb 2016

#include "eckit/io/DataBlob.h"
#include "eckit/log/Bytes.h"
#include "eckit/log/Log.h"
#include "eckit/types/Types.h"

#include "pmem/PersistentBuffer.h"
#include "pmem/PersistentPtr.h"
#include "pmem/AtomicConstructor.h"
#include "pmem/PoolRegistry.h"

#include "pmem/tree/TreeNode.h"

using namespace eckit;
using namespace pmem;


namespace tree {


TreeNode::LeafExistsError::LeafExistsError(const std::string& msg, const CodeLocation& here) :
    Exception(msg, here) {}

//----------------------------------------------------------------------------------------------------------------------


TreeNode::TreeNode(const std::string& key, const std::string& value) :
    value_(value),
    key_(key) {

    items_.nullify();
    data_.nullify();
}


TreeNode::TreeNode(const std::string& value, const PersistentPtr<PersistentBuffer>& dataBlob) :
    data_(dataBlob),
    value_(value),
    key_("") {

    items_.nullify();
}


PersistentPtr<TreeNode> TreeNode::allocateLeaf(PersistentPool& pool, const std::string& value, const DataBlob& blob) {

    PersistentPtr<PersistentBuffer> pBlob;
    pBlob = pool.allocate<PersistentBuffer>(blob.buffer(), blob.length());

    PersistentPtr<TreeNode> pNode;
    pNode = pool.allocate<TreeNode>(value, pBlob);

    return pNode;
}


/// This is an in-place constructor. Needs to know its pool already.

PersistentPtr<TreeNode> TreeNode::allocateNested(PersistentPool& pool,
                                                 const std::string& value,
                                                 const KeyType& keyChain,
                                                 const DataBlob& blob) {

    const std::string& leafValue(keyChain.size() == 0 ? value : keyChain.back().second);

    PersistentPtr<TreeNode> pNode = allocateLeaf(pool, leafValue, blob);

    if (keyChain.size() != 0) {

        for (int i = keyChain.size() - 1; i >= 0; i--) {

            const std::string& k(keyChain[i].first);
            const std::string& v(i > 0 ? keyChain[i-1].second : value);

            // Allocate the relevant node

            PersistentPtr<TreeNode> pNewNode = pool.allocate<TreeNode>(k, v);
            pNewNode->items_.push_back_elem(pNode);
            pNode = pNewNode;
        }
    }

    return pNode;
}


//----------------------------------------------------------------------------------------------------------------------


void TreeNode::addNode(const KeyType& key, const eckit::DataBlob& blob) {

    // Check that this is supposed to be a subkey of this element.
    // TODO: What happens if we repeat eter a key --> should fail here. TEST.
    ASSERT(key.size() > 0);
    ASSERT(key_ == key[0].first);

    // May not add subnodes to a leaf node.
    ASSERT(data_.null());

    // Find the sub-node, and recurse down into that to do the additions.
    FixedString<12> value = key[0].second;
    for (size_t i = 0; i < items_.size(); i++) {
        if (items_[i]->value() == value) {

            KeyType subkeys(key.begin()+1, key.end());
            if (items_[i]->leaf())
                throw LeafExistsError(std::string("The leaf ") + std::string(value) + " already exists", Here());
            items_[i]->addNode(subkeys, blob);
            return;
        }
    }

    // TODO: What happens if we are adding data in again...

    // We have reached the bottom of the known tree. Create new nodes from here-on down.

    KeyType subkeys(key.begin()+1, key.end());

    PersistentPool& pool(pmem::PoolRegistry::instance().poolFromPointer(this));

    items_.push_back_elem(allocateNested(pool, value, subkeys, blob));
}


size_t TreeNode::nodeCount() const {
    return items_.size();
}


const eckit::FixedString<12>& TreeNode::key() const {
    return key_;
}


const eckit::FixedString<12>& TreeNode::value() const {
    return value_;
}


bool TreeNode::leaf() const {
    return !data_.null();
}


const void * TreeNode::data() const {
    return data_.null() ? 0 : data_->data();
}


size_t TreeNode::dataSize() const {
    return data_.null() ? 0 : data_->size();
}


const pmem::PersistentVector<TreeNode>& TreeNode::items() const {
    return items_;
}


std::vector<PersistentPtr<TreeNode> >
TreeNode::lookup(const StringDict& request) {

    // Look through subnodes. Return depends on conditions
    //
    // i) If the current key() is found in the request, then find matching values() in subnodes
    // ii) If the current key() is not found in the request, consider all subnodes
    //
    // - All relevant leaf() subnodes should be added to the result
    // - The lookup should be propagated down into relevant non-leaf subnodes.

    ASSERT(!leaf());

    std::vector<PersistentPtr<TreeNode> > result;

    if (request.find(key_) != request.end()) {

        // Test subnodes, and include those that match.
        FixedString<12> value = request.find(key_)->second;
        for (size_t i = 0; i < items_.size(); i++) {
            if (items_[i]->value() == value) {
                if (items_[i]->leaf()) {
                    result.push_back(items_[i]);

                    // TODO: If we have unique names, this can exit the loop here.
                } else {
                    std::vector<PersistentPtr<TreeNode> > tmp_nodes = items_[i]->lookup(request);
                    result.insert(result.end(), tmp_nodes.begin(), tmp_nodes.end());
                }
            }
        }

    } else {

        // Include all sub-nodes
        for (size_t i = 0; i < items_.size(); i++) {
            if (items_[i]->leaf()) {
                result.push_back(items_[i]);
            } else {
                std::vector<PersistentPtr<TreeNode> > tmp_nodes = items_[i]->lookup(request);
                result.insert(result.end(), tmp_nodes.begin(), tmp_nodes.end());
            }
        }
    }

    return result;
}


void TreeNode::printTree(std::ostream& os, std::string pad) const {

    // n.b. the parent is responsible for indenting the opening brace.
    os << "{" << std::endl;
    std::string pad2(pad + "  ");

    if (leaf()) {
        os << pad2 << "data: " << Bytes(dataSize()) << std::endl;
    } else {
        os << pad2 << "key: " << key_ << "," << std::endl;
        os << pad2 << "items: [";

        std::string pad4(pad2 + "  ");
        for (size_t i = 0; i < items_.size(); i++) {
            if (i > 0) os << ",";
            os << std::endl << pad4 << std::string(items_[i]->value()) << ": ";
            items_[i]->printTree(os, pad4);
        }

        if (items_.size() > 0) os << std::endl << pad2;
        os << "]" << std::endl;
    }

    // The parent is responsible for terminating the final line if desired.
    os << pad << "}";
}


std::ostream& operator<< (std::ostream& os, const TreeNode& node) {
    os << "TreeNode(key=" << node.key()
       << ", data=" << (node.data_.null() ? "missing" : "present")
       << ")";
    return os;
}


// -------------------------------------------------------------------------------------------------

} // namespace tree
