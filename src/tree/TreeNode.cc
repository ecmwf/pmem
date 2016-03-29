/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016

#include "eckit/log/Log.h"
#include "eckit/io/DataBlob.h"
#include "eckit/types/Types.h"

#include "pmem/PersistentPtr.h"
#include "pmem/AtomicConstructor.h"

#include "tree/TreeNode.h"
#include "tree/PersistentBuffer.h"

using namespace eckit;
using namespace pmem;


namespace tree {

//----------------------------------------------------------------------------------------------------------------------


/// We actually store pairs of items in the vector, so here we create a constructor object for the
/// _pairs_ of items that are going to be stored. This constructor internally constructs:
///
/// a) Any DataBuffers taht are required to hold the blobs
/// b) A recursed tree of subnodes as required.

class NodeItemConstructor : public AtomicConstructor<TreeNode::Item> {

public: // methods

    NodeItemConstructor(const eckit::FixedString<12>& value,
                        const std::vector<std::pair<std::string, std::string> >& subkeys,
                        const eckit::DataBlob& blob) :
        value_(value),
        subkeys_(subkeys),
        blob_(blob) { }

    virtual void make(TreeNode::Item &object) const {

        object.first = value_;
        object.second.nullify();

        if (subkeys_.size() == 0) {
            TreeNode::Constructor ctr(blob_);
            object.second.allocate(ctr);
        } else {
            TreeNode::Constructor ctr(subkeys_[0].first, subkeys_, blob_);
            object.second.allocate(ctr);
        }
    }

private: // members

    const eckit::FixedString<12>& value_;
    const std::vector<std::pair<std::string, std::string> >& subkeys_;
    const eckit::DataBlob& blob_;
};

//----------------------------------------------------------------------------------------------------------------------


TreeNode::Constructor::Constructor(const DataBlob& blob) :
    name_(""),
    subkeys_(0),
    blob_(blob) {

    ASSERT(blob.length() != 0);
}


TreeNode::Constructor::Constructor(const std::string& name,
                                   const std::vector<std::pair<std::string, std::string> >& subkeys,
                                   const DataBlob& blob) :
    name_(name),
    subkeys_(NULL),
    blob_(blob) {

    if (subkeys.size() > 0)
        subkeys_ = &subkeys;

    ASSERT(blob.length() != 0);
}


void TreeNode::Constructor::make(TreeNode& object) const {

    object.name_ = eckit::FixedString<12>(name_);
    object.items_.nullify();
    object.data_.nullify();

    // If this is the final node in the chain, then we need to store the data! Otherwise we need to continue
    // building the chain.
    if (subkeys_) {
        const std::vector<std::pair<std::string, std::string> >& sk(*subkeys_);

        ASSERT(sk.size() > 0);
        ASSERT(sk[0].first == name_);
        std::vector<std::pair<std::string, std::string> > new_subkeys(sk.begin()+1, sk.end());
        NodeItemConstructor ctr(sk[0].second, new_subkeys, blob_);
        object.items_.push_back(ctr);
    } else {
        PersistentBuffer::Constructor ctr(blob_.buffer(), blob_.length());
        object.data_.allocate(ctr);
    }
}


//----------------------------------------------------------------------------------------------------------------------


void TreeNode::addNode(const std::vector<std::pair<std::string, std::string> > key,
                       const eckit::DataBlob& blob) {

    // Check that this is supposed to be a subkey of this element.
    // TODO: What happens if we repeat eter a key --> should fail here. TEST.
    Log::info() << "NM: " << name_ << std::endl;
    Log::info() << "KY: " << key[0].first << std::endl;
    ASSERT(key.size() > 0);
    ASSERT(name_ == key[0].first);

    // May not add subnodes to a leaf node.
    ASSERT(data_.null());

    // Find the sub-node, and recurse down into that to do the additions.
    FixedString<12> value = key[0].second;
    for (size_t i = 0; i < items_.size(); i++) {
        if (items_[i].first == value) {

            std::vector<std::pair<std::string, std::string> > subkeys(key.begin()+1, key.end());
            items_[i].second->addNode(subkeys, blob);
            return;
        }
    }

    // TODO: What happens if we are adding data in again...

    // We have reached the bottom of the known tree. Create new nodes from here-on down.
    std::vector<std::pair<std::string, std::string> > subkeys(key.begin()+1, key.end());
    NodeItemConstructor ctr(value, subkeys, blob);
    items_.push_back(ctr);
}


size_t TreeNode::nodeCount() const {
    return items_.size();
}


std::string TreeNode::name() const {
    return name_;
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


std::vector<PersistentPtr<TreeNode> >
TreeNode::lookup(const std::map<FixedString<12>, FixedString<12> >& request) {

//    Log::info() << "Node lookup: " << request << std::endl;

    // 3 possibilities:
    //
    // - name of current node is a key in request --> do lookup on subnodes
    // - name of current node is not a key --> return all subnodes
    // - name of current node is not a key, and no subnodes --> return this node.

    ASSERT(!leaf());

    std::vector<PersistentPtr<TreeNode> > result;

    if (request.find(name_) != request.end()) {

        // Test subnodes, and include those that match.
        FixedString<12> value = request.find(name_)->second;
        for (size_t i = 0; i < items_.size(); i++) {
            if (items_[i].first == value) {
                if (items_[i].second->leaf()) {
                    result.push_back(items_[i].second);

                    // TODO: If we have unique names, this can exit the loop here.
                } else {
                    std::vector<PersistentPtr<TreeNode> > tmp_nodes = items_[i].second->lookup(request);
                    result.insert(result.end(), tmp_nodes.begin(), tmp_nodes.end());
                }
            }
        }

    } else {

        // Include all sub-nodes
        for (size_t i = 0; i < items_.size(); i++) {
            if (items_[i].second->leaf()) {
                result.push_back(items_[i].second);
            } else {
                std::vector<PersistentPtr<TreeNode> > tmp_nodes = items_[i].second->lookup(request);
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

} // namespace tree
