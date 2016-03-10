/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016

#include "tree/TreePool.h"
#include "tree/TreeRoot.h"
#include "tree/TreeNode.h"

#include "persistent/PersistentPtr.h"
#include "persistent/AtomicConstructor.h"

#include "eckit/log/Log.h"

using namespace eckit;
using namespace pmem;


// Register this type with the type management
// TODO: Can we ensure that there are no type-conflicts?

namespace treetool {
    class PersistentBuffer;
}


template<> int pmem::PersistentPtr<treetool::TreeRoot>::type_id = POBJ_ROOT_TYPE_NUM;
template<> int pmem::PersistentPtr<treetool::TreeNode>::type_id = 1;

template<> int pmem::PersistentPtr<pmem::PersistentVector<
    std::pair<eckit::FixedString<12>, pmem::PersistentPtr<treetool::TreeNode> > >::data_type>::type_id = 2;

template<> int pmem::PersistentPtr<treetool::PersistentBuffer>::type_id = 3;



namespace treetool {
// -------------------------------------------------------------------------------------------------

/*
 * Pass in information about the object to construct to the PersistentPool root object,
 * which doesn't need to care!!!
 *
 * --> In principle we don't need to use a TreePool class, we could use PersistentPool
 *     nakedly.
 * --> This is just a little cleaner.
 * --> For some reason, it complains if we just use TreeRoot::Constructor()
 */

namespace {
    TreeRoot::Constructor rootConstructor;
}

TreePool::TreePool(const eckit::PathName &path, const size_t size) :
    PersistentPool(path, size, "tree-pool", rootConstructor) {}


TreePool::~TreePool() {}


PersistentPtr<TreeRoot> TreePool::root() const {
    return getRoot<TreeRoot>();
}

// -------------------------------------------------------------------------------------------------

} // namespace treetool
