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

#include "pmem/PersistentVector.h"

#include "tree/TreeNode.h"

namespace eckit {
    class DataBlob;
}


namespace tree {

// -------------------------------------------------------------------------------------------------

// N.B. This is to be stored in PersistentPtr --> NO virtual behaviour.

class TreeRoot {

public: // Construction objects

    class Constructor : public pmem::AtomicConstructor<TreeRoot> {
    public: // methods
        Constructor();
        virtual void make(TreeRoot& object) const;
    };


public: // methods

    bool valid() const;

    void addNode(const std::vector<std::pair<std::string, std::string> >& key,
                 const eckit::DataBlob& blob);

    pmem::PersistentPtr<TreeNode> rootNode() const;

private: // members

    eckit::FixedString<8> tag_;

    pmem::PersistentPtr<TreeNode> node_;

};


// A consistent definition of the tag for comparison purposes.
const eckit::FixedString<8> TreeRootTag = "999TREE9";


// -------------------------------------------------------------------------------------------------

/// A volatile tree object, which wraps TreeRoot and allows it to perform in memory caching, schema management,
/// etc. that is decoupled to some degree from the format of what is stored.

class TreeObject : private eckit::NonCopyable {

public: // methods

    TreeObject(TreeRoot& root);
    ~TreeObject();

    void addNode(const std::map<std::string, std::string>& key, const eckit::DataBlob& blob);

protected: // methods

    void print(std::ostream&) const;

private: // members

    TreeRoot& root_;

private: // friends

    friend std::ostream& operator<<(std::ostream& os, const TreeObject& p) {
        p.print(os);
        return os;
    }
};

// -------------------------------------------------------------------------------------------------

} // namespace tree

#endif // tree_TreeRoot_H
