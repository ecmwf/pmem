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
#include "persistent/PersistentPtr.h"
#include "persistent/AtomicConstructor.h"

#include "eckit/log/Log.h"


using namespace eckit;
using namespace pmem;


namespace treetool {

// -------------------------------------------------------------------------------------------------


// Put constructor classes into an anonymous namespace, so they cannot pollute anything
// outside the file.
namespace {

class TreeNodeConstructor : public AtomicConstructor<TreeNode> {
public:

    TreeNodeConstructor(const std::string& name) :
        name_(name) {}

    virtual void make (TreeNode * object) const {
        Log::info() << "In a tree node constructor!" << std::endl << std::flush;

        object->name_ = eckit::FixedString<12>(name_);
        object->items_.nullify();
        Log::info() << "Construction done" << std::endl << std::flush;
    }

private:
    std::string name_;
};

}

// -------------------------------------------------------------------------------------------------


void TreeNode::addNode(const std::string& key, const std::string& name) {

    PMEMobjpool* pop = ::pmemobj_pool_by_ptr(this);

    // TODO: We should be able to create a constructor for the pair, and pass that
    //       in to push_back!

    PersistentPtr<TreeNode> new_node;
    TreeNodeConstructor constructor(name);
    new_node.allocate(pop, constructor);

    items_.push_back(std::make_pair(eckit::FixedString<12>(key), new_node));
}


size_t TreeNode::nodeCount() const {
    return items_.size();
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
                Log::info() << "Iterating into: " << items_[i].second.oid_.off << ", " << items_[i].second.oid_.pool_uuid_lo << std::endl;
                std::vector<PersistentPtr<TreeNode> > tmp_nodes = items_[i].second->lookup(request);
                result.insert(result.end(), tmp_nodes.begin(), tmp_nodes.end());
            }
        }

    } else if (items_.size() == 0) {

   //     // Add self to the list --> We need the persistent ptr!!!
        Log::info() << "Pushing this one: " << std::endl;
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


std::ostream& operator<< (std::ostream& os, const TreeNode& node) {
    os << "TreeNode(" << node.name_ << ")";
    return os;
}


// -------------------------------------------------------------------------------------------------

} // namespace treetool
