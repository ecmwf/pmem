/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   March 2016

#include "eckit/io/DataHandle.h"
#include "eckit/parser/JSONParser.h"

#include "tree/TreeSchema.h"


using namespace eckit;

namespace tree {

//----------------------------------------------------------------------------------------------------------------------

TreeSchema::TreeSchema(DataHandle& dh) {

    // Parse the supplied
    JSONParser parser(dh);
    Value parsed(parser.parse());

    Log::info() << "Schema: " << parsed << std::endl;

}

TreeSchema::~TreeSchema() {}

//----------------------------------------------------------------------------------------------------------------------

}
