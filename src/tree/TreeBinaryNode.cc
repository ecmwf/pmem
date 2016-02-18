/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016

#include "tree/TreeBinaryNode.h"
#include "tree/TreeLeafNode.h"

#include "eckit/exception/Exceptions.h"

namespace treetool {


// -------------------------------------------------------------------------------------------------


TreeLeafNode * TreeBinaryNode::leaf(size_t idx) const {
    NOTIMP;
}

// -------------------------------------------------------------------------------------------------

} // namespace treetool
