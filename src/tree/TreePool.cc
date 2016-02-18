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

#include "persistent/PersistentPtr.h"
#include "persistent/AtomicConstructor.h"

#include "eckit/log/Log.h"

using namespace eckit;
using namespace pmem;


// Register this type with the type management
// TODO: Can we ensure that there are no type-conflicts?

template<> int pmem::PersistentPtr<treetool::TreeRoot>::type_id = POBJ_ROOT_TYPE_NUM;



namespace treetool {

// -------------------------------------------------------------------------------------------------

/*
 * Pass in information about the object to construct to the PersistentPool root object,
 * which doesn't need to care!!!
 *
 * --> In principle we don't need to use a TreePool class, we could use PersistentPool
 *     nakedly.
 * --> This is just a little cleaner.
 */

class RootConstructor : public AtomicConstructor<TreeRoot> {
public:

    virtual void make (TreeRoot * object) const {
        object->tag_ = TreeRootTag;
    }
};

RootConstructor rootConstructor;

// -------------------------------------------------------------------------------------------------


TreePool::TreePool(const eckit::PathName &path, const size_t size) :
    PersistentPool(path, size, "tree-pool", rootConstructor) {}


TreePool::~TreePool() {}


PersistentPtr<TreeRoot> TreePool::root() const {
    return getRoot<TreeRoot>();
}

// -------------------------------------------------------------------------------------------------

} // namespace treetool
