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


namespace {
    class TreeNodeConstructor : public AtomicConstructor<TreeNode> {
    public:

        virtual void make (TreeNode * object) const {
            Log::info() << "In a tree node constructor!" << std::endl << std::flush;

            object->name_ = eckit::FixedString<12>("123456789012");
            object->items_.nullify();
            Log::info() << "Construction done" << std::endl << std::flush;
        }
    };
}


// Obviously this is temporary, to demonstrate functionality
size_t TreeNode::playtime_child() {

    size_t sz0 = items_.size();

    // TODO: We should be able to create a constructor for the pair, and pass that
    //       in to push_back!

    PMEMobjpool* pop = ::pmemobj_pool_by_ptr(this);
    Log::info() << "Pool: " << pop << std::endl << std::flush;

    PersistentPtr<TreeNode> new_node;
    TreeNodeConstructor constructor;
    new_node.allocate(pop, constructor);

    items_.push_back(std::make_pair(eckit::FixedString<12>("000011112222"), new_node));

    return sz0;

}



std::ostream& operator<< (std::ostream& os, const TreeNode& node) {
    os << "TreeNode()";
    return os;
}


// -------------------------------------------------------------------------------------------------

} // namespace treetool
