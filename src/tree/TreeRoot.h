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

#include "eckit/types/FixedString.h"

#include "pmem/PersistentVector.h"

#include "tree/TreeNode.h"

namespace eckit {
    class DataBlob;
}


namespace treetool {

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

}

#endif // tree_TreeRoot_H
