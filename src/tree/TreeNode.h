/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016


#ifndef tree_TreeNode_H
#define tree_TreeNode_H


#include "persistent/PersistentVector.h"
#include "persistent/PersistentPtr.h"

#include "eckit/types/FixedString.h"

#include <utility>
#include <string>
#include <vector>
#include <map>

namespace treetool {

class TreeLeafNode;


// -------------------------------------------------------------------------------------------------


class TreeNode {

public: // types

    typedef std::pair<eckit::FixedString<12>, pmem::PersistentPtr<TreeNode> > Item;

public: // methods

    /// Add a new node
    /// @param key - The value used to select this sub-node from the current node
    /// @param name - Select which key-value pair is examined to select sub-sub-nodes
    void addNode(const std::string& key, const std::string& name);

    size_t nodeCount() const;

    // n.b. this is non-const. May return a reference to the current object, that can be
    //      built on.
    std::vector<pmem::PersistentPtr<TreeNode> > lookup(const std::map<eckit::FixedString<12>, eckit::FixedString<12> >&);

//protected: // members
public: // TODO: Find a sensible way to get the atomic constructors working...

    pmem::PersistentVector<Item> items_;

    eckit::FixedString<12> name_;

private:

    friend std::ostream& operator<< (std::ostream&, const TreeNode&);
};

// -------------------------------------------------------------------------------------------------

} // namespace treetool

#endif // tree_TreeNode_H
