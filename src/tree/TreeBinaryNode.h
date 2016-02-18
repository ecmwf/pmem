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


#ifndef tree_TreeBinaryNode_H
#define tree_TreeBinaryNode_H


namespace treetool {

class TreeLeafNode;

// -------------------------------------------------------------------------------------------------


class TreeBinaryNode : public TreeNode {

public: // methods

    virtual TreeLeafNode * leaf(size_t idx) const;

};

// -------------------------------------------------------------------------------------------------

} // namespace treetool

#endif // tree_TreeBinaryNode_H
