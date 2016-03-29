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

namespace eckit {
    class DataHandle;
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

    TreeSchema(eckit::DataHandle& s);
    ~TreeSchema();

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace tree

#endif // tree_TreeSchema_H
