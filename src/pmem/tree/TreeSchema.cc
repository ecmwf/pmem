/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/*
 * This software was developed as part of the EC H2020 funded project NextGenIO
 * (Project ID: 671951) www.nextgenio.eu
 */

/// @author Simon Smart
/// @date   March 2016

#include "eckit/filesystem/PathName.h"
#include "eckit/io/Buffer.h"
#include "eckit/io/DataHandle.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/parser/JSON.h"
#include "eckit/parser/JSONParser.h"
#include "eckit/value/Value.h"

#include "pmem/tree/TreeSchema.h"

using namespace eckit;

namespace tree {

//----------------------------------------------------------------------------------------------------------------------

TreeSchema::TreeSchema(PathName& path) {

    ScopedPtr<DataHandle> schema_file(path.fileHandle());

    Buffer buf(schema_file->openForRead());
    schema_file->read(buf, buf.size());

    // Get the JSON data into an istream, rather than an anonymous blob of bytes.
    std::string json_str(buf, buf.size());
    std::istringstream iss(json_str);

    init(iss);
}

TreeSchema::TreeSchema(std::istream& s) {
    init(s);
}

TreeSchema::TreeSchema() {}

TreeSchema::~TreeSchema() {}


void TreeSchema::init(std::istream &s) {

    // Parse the data as a value
    JSONParser parser(s);
    Value parsed(parser.parse());

    if (!parsed.isList())
        throw UserError("Supplied tree-schema must be a list", Here());

    ValueList schema_list(parsed.as<ValueList>());

    // TODO: Increase the complexity of the scheme (e.g. max/min values, data types, ...)
    keys_.clear();
    keys_.reserve(schema_list.size());
    for (ValueList::const_iterator it = schema_list.begin(); it != schema_list.end(); ++it) {
        keys_.push_back(std::string(*it));
    }

    Log::info() << "Initialised schema: " << keys_ << std::endl;
}


std::vector<std::pair<std::string, std::string> > TreeSchema::processInsertKey(const StringDict& inkey) const {

    std::vector<std::pair<std::string, std::string> > key;

    if (inkey.size() != keys_.size())
        throw UserError("Insertion key has wrong number of key:value pairs", Here());
    key.reserve(keys_.size());

    for (std::vector<std::string>::const_iterator key_it = keys_.begin(); key_it != keys_.end(); ++key_it) {

        StringDict::const_iterator value = inkey.find(*key_it);
        if (value == inkey.end())
            throw UserError(std::string("Required key \"") + *key_it + "\" missing in supplied insert key", Here());

        key.push_back(std::make_pair(*key_it, value->second));
    }

    Log::info() << "Processed key: " << key << std::endl;
    return key;
}


std::string TreeSchema::json_str() const {

    std::stringstream json_stream;
    JSON json(json_stream);

    json << makeVectorValue(keys_);
    return json_stream.str();
}


void TreeSchema::print(std::ostream& os) const {

    os << "TreeSchema(" << keys_ << ")";
}

//----------------------------------------------------------------------------------------------------------------------

}
