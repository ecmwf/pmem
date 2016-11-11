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

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "eckit/types/FixedString.h"
#include "eckit/types/Types.h"

#include "pmem/PersistentPtr.h"
#include "pmem/PersistentVector.h"
#include "pmem/PersistentBuffer.h"

namespace eckit {
    class DataBlob;
}


namespace tree {

//----------------------------------------------------------------------------------------------------------------------

class TreeNode : public pmem::PersistentType<TreeNode> {

public: // types

    typedef std::vector<std::pair<std::string, std::string> > KeyType;

    struct LeafExistsError : public eckit::Exception {
        LeafExistsError(const std::string&, const eckit::CodeLocation&);
    };

public: // Construction objects

    class Constructor : public pmem::AtomicConstructor<TreeNode> {

    public: // methods

        /// Construct a leaf (data) node. Leaves do not have a name.
        Constructor(const std::string& value, const eckit::DataBlob& blob);

        /// Construct a normal node, and its children.
        Constructor(const std::string& value, const KeyType& subkeys, const eckit::DataBlob& blob);

        virtual void make (TreeNode& object) const;

    private: // members

        std::string value_;

        const KeyType* subkeys_;

        const eckit::DataBlob& blob_;
    };

public: // methods

    /// Add a new node
    /// @param key - The value used to select this sub-node from the current node
    /// @param name - Select which key-value pair is examined to select sub-sub-nodes
//    void addNode(const std::string& key, const std::string& name, const eckit::DataBlob& blob);

    void addNode(const KeyType& key, const eckit::DataBlob& blob);

    /// How many subnodes are there to this node?
    size_t nodeCount() const;

    // n.b. this is non-const. May return a reference to the current object, that can be
    //      built on.
    std::vector<pmem::PersistentPtr<TreeNode> > lookup(const eckit::StringDict& key);

    void printTree(std::ostream& os, std::string pad="") const;

    /// The value by which this node is associated to its _parent's_ key.
    const eckit::FixedString<12>& value() const;

    /// The key by which child nodes are selected
    const eckit::FixedString<12>& key() const;

    /// Does this node contain data?
    bool leaf() const;

    const void * data() const;

    size_t dataSize() const;

protected: //

    /// A utility method to facilitate testing.
    const pmem::PersistentVector<TreeNode>& items() const;

private: // members

    pmem::PersistentVector<TreeNode> items_;

    pmem::PersistentPtr<pmem::PersistentBuffer> data_;

    eckit::FixedString<12> value_;

    eckit::FixedString<12> key_;

private:

    friend std::ostream& operator<< (std::ostream&, const TreeNode&);

    friend class TreeNode::Constructor;
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace tree

#endif // tree_TreeNode_H
