/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016


#ifndef tree_TreeRoot_H
#define tree_TreeRoot_H

#include "eckit/memory/NonCopyable.h"
#include "eckit/types/FixedString.h"
#include "eckit/types/Types.h"

#include "pmem/PersistentVector.h"

#include "tree/TreeNode.h"
#include "tree/TreeSchema.h"

namespace eckit {
    class DataBlob;
}

namespace pmem {
    class PersistentBuffer;
}


namespace tree {

class TreeSchema;

// -------------------------------------------------------------------------------------------------

// N.B. This is to be stored in PersistentPtr --> NO virtual behaviour.

class TreeRoot : public pmem::PersistentType<TreeRoot> {

public: // types

    typedef TreeNode::KeyType KeyType;

public: // Construction objects

    class Constructor : public pmem::AtomicConstructor<TreeRoot> {
    public: // methods
        Constructor(const TreeSchema& schema);
        virtual void make(TreeRoot& object) const;
    private:
        const TreeSchema& schema_;
    };


public: // methods

    bool valid() const;

    void addNode(const KeyType& key, const eckit::DataBlob& blob);

    pmem::PersistentPtr<TreeNode> rootNode() const;

private: // members

    eckit::FixedString<8> tag_;

    pmem::PersistentPtr<TreeNode> node_;

    pmem::PersistentPtr<pmem::PersistentBuffer> schema_;

private: // friends

    friend class TreeObject;
};


// A consistent definition of the tag for comparison purposes.
const eckit::FixedString<8> TreeRootTag = "999TREE9";


// -------------------------------------------------------------------------------------------------

/// A volatile tree object, which wraps TreeRoot and allows it to perform in memory caching, schema management,
/// etc. that is decoupled to some degree from the format of what is stored.

class TreeObject : private eckit::NonCopyable {

public: // types

    typedef TreeRoot::KeyType KeyType;

public: // methods

    TreeObject(TreeRoot& root);
    ~TreeObject();

    void addNode(const eckit::StringDict& key, const eckit::DataBlob& blob);

    void printTree(std::ostream& os) const;

    std::vector<pmem::PersistentPtr<TreeNode> > lookup(const eckit::StringDict& key);

protected: // methods

    void print(std::ostream&) const;

private: // members

    TreeRoot& root_;
    TreeSchema schema_;

private: // friends

    friend std::ostream& operator<<(std::ostream& os, const TreeObject& p) {
        p.print(os);
        return os;
    }
};

// -------------------------------------------------------------------------------------------------

} // namespace tree

#endif // tree_TreeRoot_H
