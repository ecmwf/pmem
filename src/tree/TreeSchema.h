/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   March 2016


#ifndef tree_TreeSchema_H
#define tree_TreeSchema_H

#include <vector>
#include <utility>
#include <string>

#include "eckit/types/Types.h" // Can't forward declare StringDict, as it is a typedef

namespace eckit {
    class PathName;
}

namespace tree {

//----------------------------------------------------------------------------------------------------------------------

class TreeSchema {

public: // methods

    // i) Read in from stream
    // ii) Obtain as JSON to write back
    // iii) Read in from json in pmem
    // iv) Convert map --> vector for insertion
    // v) Check completeness of supplied queries.

    TreeSchema(eckit::PathName& path);
    ~TreeSchema();

    std::vector<std::pair<std::string, std::string> > processInsertKey(const eckit::StringDict& key) const;

private: // members

    std::vector<std::string> keys_;
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace tree

#endif // tree_TreeSchema_H
