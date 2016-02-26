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



namespace treetool {

// -------------------------------------------------------------------------------------------------



std::ostream& operator<< (std::ostream& os, const TreeNode& node) {
    os << "TreeNode()";
    return os;
}


// -------------------------------------------------------------------------------------------------

} // namespace treetool
