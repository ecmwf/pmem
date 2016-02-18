/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016


#ifndef persistent_TreePool_H
#define persistent_TreePool_H


#include "persistent/PersistentPool.h"


// -------------------------------------------------------------------------------------------------

// Forward declaration
namespace pmem {
    template <typename T> class PersistentPtr;
}

// -------------------------------------------------------------------------------------------------

namespace treetool {

class TreeRoot;

// -------------------------------------------------------------------------------------------------

/*
 * Here we define the persistent pool used in this project
 *
 * --> The TreePool needs to know:
 *
 *       i) The data type of the root
 *      ii) How to construct the root object if required
 *     iii) How to map type_ids to actual types
 */

class TreePool : public pmem::PersistentPool {

public: // methods

    TreePool(const eckit::PathName& path, const size_t size);
    ~TreePool();

    pmem::PersistentPtr<TreeRoot> root() const;

};

// -------------------------------------------------------------------------------------------------

}


#endif // persistent_TreePool_H
