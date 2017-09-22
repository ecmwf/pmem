/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/*
 * This software was developed as part of the EC H2020 funded project NextGenIO
 * (Project ID: 671951) www.nextgenio.eu
 */

/// @author Simon Smart
/// @date   March 2016

#include "tree/TreeMultIO.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/io/MemoryHandle.h"


using namespace eckit;
using namespace multio;

namespace tree {

// ------------------------------------------------------------------------------------------------

namespace {

    DataSinkBuilder<TreeMultIO> TreeMultIOBuilder("tree");

}

// ------------------------------------------------------------------------------------------------

TreeMultIO::TreeMultIO(const Configuration& config) :
    DataSink(config),
    pool_(config.getString("path"), config.getLong("size")) {
}

TreeMultIO::~TreeMultIO() {
}

void TreeMultIO::write(DataBlobPtr blob, JournalRecordPtr record) {

    if (record && journalAlways_) {
        record->addWriteEntry(blob, id_);
    }

    try {

        Log::info() << "Writing an object." << std::endl;

    }
    catch(Exception& e) {

        // If something goes wrong, we need to journal (unless we have already journalled above)
        if (record) {
            if (!journalAlways_) record->addWriteEntry(blob, id_);
        } else {
            throw;
        }
    }
}

void TreeMultIO::print(std::ostream& os) const
{
    os << "TreeMultIO()";
}

// ------------------------------------------------------------------------------------------------

} // tree
