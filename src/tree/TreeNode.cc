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


void TreeNode::addNode(const std::string& name) {

    PMEMobjpool* pop = ::pmemobj_pool_by_ptr(this);

    // TODO: We should be able to create a constructor for the pair, and pass that
    //       in to push_back!

    PersistentPtr<TreeNode> new_node;
    TreeNodeConstructor constructor(name);
    new_node.allocate(pop, constructor);

    // TODO: Separate the "name" of a node (i.e. which field it will be selecting in the key-value
    //       pair hashmap, and the value which applies to its parent map, which goes in here.
    items_.push_back(std::make_pair(eckit::FixedString<12>(name), new_node));
}


size_t TreeNode::nodeCount() const {
    return items_.size();
}



std::ostream& operator<< (std::ostream& os, const TreeNode& node) {
    os << "TreeNode()";
    return os;
}


// -------------------------------------------------------------------------------------------------

} // namespace treetool
