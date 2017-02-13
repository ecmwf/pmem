/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Simon Smart
/// @date March 2016


#ifndef tree_TreeNode_H
#define tree_TreeNode_H

#include <iosfwd>

#include "multio/DataSink.h"

#include "eckit/filesystem/PathName.h"

#include "tree/TreePool.h"

namespace tree {

// ------------------------------------------------------------------------------------------------

class TreeMultIO : public multio::DataSink {

public: // methods

    TreeMultIO(const eckit::Configuration& config);

    virtual ~TreeMultIO();

    virtual void write(eckit::DataBlobPtr blob, multio::JournalRecordPtr record);

protected: // methods

    virtual void print(std::ostream&) const;

private: // friends

    friend std::ostream &operator<<(std::ostream &s, const TreeMultIO &p) {
        p.print(s);
        return s;
    }

private: // members

    eckit::PathName path_;
    TreePool pool_;

};

// ------------------------------------------------------------------------------------------------

} // namespace tree

#endif // tree_TreeNode_H

