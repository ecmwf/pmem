/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016

#include "tree/TreeRoot.h"
#include "tree/TreeNode.h"

#include "eckit/log/Log.h"

using namespace eckit;
using namespace pmem;


namespace treetool {

// -------------------------------------------------------------------------------------------------


TreeRoot::Constructor::Constructor() {}


void TreeRoot::Constructor::make(TreeRoot* object) const {

    object->tag_ = TreeRootTag;
    object->node_.nullify();
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

// -------------------------------------------------------------------------------------------------

} // namespace treetool
