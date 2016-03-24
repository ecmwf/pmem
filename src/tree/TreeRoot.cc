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

#include "tree/TreeRoot.h"
#include "tree/TreeNode.h"

using namespace eckit;
using namespace pmem;


namespace tree {

// -------------------------------------------------------------------------------------------------


TreeRoot::Constructor::Constructor() {}


void TreeRoot::Constructor::make(TreeRoot& object) const {

    object.tag_ = TreeRootTag;
    object.node_.nullify();
}

// -------------------------------------------------------------------------------------------------

/*
 * We can use whatever knowledge we have to test the validity of the structure.
 *
 * For now, we just check that the tag is set (i.e. it is initialised).
 */
bool TreeRoot::valid() const {

    return tag_ == TreeRootTag;
}


PersistentPtr<TreeNode> TreeRoot::rootNode() const {

    return node_;
}


void TreeRoot::addNode(const std::vector<std::pair<std::string, std::string> >& key,
                       const eckit::DataBlob& blob) {

    ASSERT(key.size() != 0);

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

} // namespace tree
