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

#include "tree/TreeNode.h"

#include "persistent/PolymorphicPersistentPtr.h"
#include "persistent/PersistentVector.h"

#include "eckit/types/FixedString.h"


namespace treetool {

// -------------------------------------------------------------------------------------------------


// N.B. This is to be stored in PersistentPtr --> NO virtual behaviour.

class TreeRoot {
public: // methods

    bool valid() const;

public: // members

    eckit::FixedString<8> tag_;

//    pmem::PolymorphicPersistentPtr<TreeNode> root_;

    // TODO: This is just for testing.

    pmem::PersistentPtr<TreeNode> node_;

};


// A consistent definition of the tag for comparison purposes.
const eckit::FixedString<8> TreeRootTag = "999TREE9";


// -------------------------------------------------------------------------------------------------

}

#endif // tree_TreeRoot_H
