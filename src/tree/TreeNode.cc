/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016


#include "tree/TreeNode.h"
#include "tree/PersistentBuffer.h"
#include "persistent/PersistentPtr.h"
#include "persistent/AtomicConstructor.h"

#include "eckit/log/Log.h"
#include "eckit/io/DataBlob.h"


using namespace eckit;
using namespace pmem;


namespace treetool {

// -------------------------------------------------------------------------------------------------


TreeNode::Constructor::Constructor(const std::string& name, const void* data, size_t length) :
    name_(name),
    data_(data),
    length_(length) {

    // Quick sanity check
    ASSERT(data_ == 0 || length_ != 0);
}


void TreeNode::Constructor::make(TreeNode * object) const {
    object->name_ = eckit::FixedString<12>(name_);
    object->items_.nullify();
    object->data_.nullify();

    // If there is data attached to this node, then store it.
    if (data_ != 0) {
        PersistentBuffer::Constructor ctr(data_, length_);
        object->data_.allocate(ctr);
    }
}

// -------------------------------------------------------------------------------------------------


void TreeNode::addNode(const std::vector<std::pair<std::string, std::string> > key,
                       const eckit::DataBlob& blob) {

    // Check that this is supposed to be a subkey of this element.
    // TODO: What happens if we repeat eter a key --> should fail here. TEST.
    ASSERT(key.size() > 0);
    ASSERT(name_ == key[0].first);

    FixedString<12> value = key[0].second;
    for (size_t i = 0; i < items_.size(); i++) {
        if (items_[i].first == value) {

            items_[i].second->addNode(
                std::vector<std::pair<std::string, std::string> >(key.begin()+1, key.end()),
                blob);
            return;
        }
    }

    // TODO: We should be passing this key into an overall constructor, and avoiding
    //       extracting the pop, and the explicit stagewise building.

    PMEMobjpool* pop = ::pmemobj_pool_by_ptr(this);

    // If we haven't found a key, then we need to insert it!
    // build the entire sub-tree before we insert it.
    PersistentPtr<TreeNode> new_node;
    if (key.size() == 1) {
        TreeNode::Constructor ctr("", blob.buffer(), blob.length());
        new_node.allocate(pop, ctr);
    } else {
        TreeNode::Constructor ctr(key[1].first, 0, 0);
        new_node.allocate(pop, ctr);

        new_node->addNode(
            std::vector<std::pair<std::string, std::string> >(key.begin()+1, key.end()),
            blob);
    }

    items_.push_back(std::make_pair(value, new_node));
}

void TreeNode::addNode(const std::string& key, const std::string& name, const DataBlob& blob) {

    PMEMobjpool* pop = ::pmemobj_pool_by_ptr(this);

    // TODO: We should be able to create a constructor for the pair, and pass that
    //       in to push_back!

    PersistentPtr<TreeNode> new_node;
    TreeNode::Constructor constructor(name, blob.buffer(), blob.length());
    new_node.allocate(pop, constructor);

    items_.push_back(std::make_pair(eckit::FixedString<12>(key), new_node));
}


size_t TreeNode::nodeCount() const {
    return items_.size();
}


std::string TreeNode::name() const {
    return name_;
}


std::vector<PersistentPtr<TreeNode> >
TreeNode::lookup(const std::map<FixedString<12>, FixedString<12> >& request) {

//    Log::info() << "Node lookup: " << request << std::endl;

    // 3 possibilities:
    //
    // - name of current node is a key in request --> do lookup on subnodes
    // - name of current node is not a key --> return all subnodes
    // - name of current node is not a key, and no subnodes --> return this node.

    std::vector<PersistentPtr<TreeNode> > result;

    if (request.find(name_) != request.end()) {

        // Test subnodes, and include those that match.
        FixedString<12> value = request.find(name_)->second;
        for (size_t i = 0; i < items_.size(); i++) {
            if (items_[i].first == value) {
                std::vector<PersistentPtr<TreeNode> > tmp_nodes = items_[i].second->lookup(request);
                result.insert(result.end(), tmp_nodes.begin(), tmp_nodes.end());
            }
        }

    } else if (items_.size() == 0) {

   //     // Add self to the list --> We need the persistent ptr!!!
        result.push_back(PersistentPtr<TreeNode>(this));

    } else {

        // Include all sub-nodes
        for (size_t i = 0; i < items_.size(); i++) {
            std::vector<PersistentPtr<TreeNode> > tmp_nodes = items_[i].second->lookup(request);
            result.insert(result.end(), tmp_nodes.begin(), tmp_nodes.end());
        }
    }

    return result;
}


void TreeNode::printTree(std::ostream& os, std::string pad) const {

    // n.b. the parent is responsible for indenting the opening brace.
    os << "{" << std::endl;
    std::string pad2(pad + "  ");
    os << pad2 << "key: " << name_ << "," << std::endl;
    os << pad2 << "data: " << (data_.null() ? "missing" : "present")
       << "," << std::endl;
    os << pad2 << "items: [";

    std::string pad4(pad2 + "  ");
    for (size_t i = 0; i < items_.size(); i++) {
        if (i > 0) os << ",";
        os << std::endl << pad4 << std::string(items_[i].first) << ": ";
        items_[i].second->printTree(os, pad4);
    }

    if (items_.size() > 0) os << std::endl << pad2;
    os << "]" << std::endl;

    // The parent is responsible for terminating the final line if desired.
    os << pad << "}";
}


std::ostream& operator<< (std::ostream& os, const TreeNode& node) {
    os << "TreeNode(key=" << node.name_
       << ", data=" << (node.data_.null() ? "missing" : "present")
       << ")";
    return os;
}


// -------------------------------------------------------------------------------------------------

} // namespace treetool
