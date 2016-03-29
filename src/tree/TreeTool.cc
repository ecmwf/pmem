/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#include <iterator>

#include "eckit/config/JSONConfiguration.h"
#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/io/DataBlob.h"
#include "eckit/io/FileHandle.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/Separator.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/parser/JSONDataBlob.h"
#include "eckit/parser/JSONParser.h"
#include "eckit/runtime/Tool.h"
#include "eckit/types/Types.h"

#include "pmem/PersistentPool.h"
#include "pmem/PersistentPtr.h"

#include "tree/PersistentBuffer.h"
#include "tree/TreeNode.h"
#include "tree/TreePool.h"
#include "tree/TreeRoot.h"
#include "tree/TreeSchema.h"

using namespace eckit;
using namespace eckit::option;
using namespace pmem;

namespace tree {

// -------------------------------------------------------------------------------------------------


class TreeTool : public Tool {

public: // methods

    TreeTool(int argc, char** argv);
    virtual ~TreeTool();

    virtual void run();

    static void usage(const std::string& tool);
};


//----------------------------------------------------------------------------------------------------------------------


TreeTool::TreeTool(int argc, char** argv) :
    Tool(argc, argv) {}


TreeTool::~TreeTool() {}


void TreeTool::usage(const std::string& tool) {

    Log::info() << std::endl;
    Log::info() << "Usage: " << tool << " [--key=value ...] <pool_file>" << std::endl;
    Log::info() << std::flush;
}


//----------------------------------------------------------------------------------------------------------------------


void TreeTool::run() {

    // Specify command line arguments

    std::vector<Option*> options;

    options.push_back(new Separator("Options for creating a new pool"));
    options.push_back(new SimpleOption<bool>("create", "Insert an element as specified by the pool and data keys"));
    options.push_back(new SimpleOption<size_t>("size", "The size of the pool file to create"));
    options.push_back(new SimpleOption<PathName>("schema", "The file containing the data schema"));

    options.push_back(new Separator("Options for adding new leaves to the tree"));
    options.push_back(new SimpleOption<bool>("insert", "Insert an element as specified by the pool and data keys"));
    options.push_back(new SimpleOption<std::string>("key", "The key to insert. This is a json object of key-value pairs"));
    options.push_back(new SimpleOption<PathName>("data", "The file containing the data to insert"));

    CmdArgs args(&usage, 1, options);

    size_t pool_size = args.getLong("size", 20 * 1024 * 1024);
    PathName path = args.args()[0];
    Log::info() << "Specified pool: " << path << std::endl;

    // ---------------------------------------------------------------------------------
    // Now start the tool.

    TreePool pool(path, pool_size, args.has("size"));

    PersistentPtr<TreeRoot> root = pool.root();

    Log::info() << "Valid: " << (root->valid() ? "true" : "false") << std::endl;

    if (args.getBool("create", false)) {

        Log::status() << "Attempting to create new pool";

        PathName schema_path(args.getString("schema"));
        TreeSchema schema(schema_path);

        StringDict inkey;
        inkey["type"] = "fc";
        inkey["param"] = "2t";
        inkey["year"] = "2016";
        inkey["month"] = "03";
        inkey["subday"] = "04";

        schema.processInsertKey(inkey);
    }

    // Deal with our special cases first
    if (args.getBool("insert", false)) {

        TreeObject tree(*root);

        // TODO: We should have a cached in-memory tree object that is separate from the TreeRoot in persistent memory (this way it can
        //       hold the decoded schema.

        std::istringstream iss(args.getString("key"));
        JSONParser parser(iss);

        StringDict key;
        JSONParser::toStrDict(parser.parse(), key);

        // Get the data to store into a blob.
        PathName data_path(args.getString("data"));
        ScopedPtr<DataHandle> data_file(data_path.fileHandle());
        Length len = data_file->openForRead();

        JSONDataBlob blob(*data_file, len);
        tree.addNode(key, blob);

    } else if (root->rootNode().null()) {

        // If the tree is empty, provide some default contents for demonstration purposes.

        std::vector<std::pair<std::string, std::string> > key;
        key.push_back(std::make_pair("type", "fc"));
        key.push_back(std::make_pair("param", "2t"));
        key.push_back(std::make_pair("year", "2016"));
        key.push_back(std::make_pair("month", "03"));
        key.push_back(std::make_pair("day", "04"));

        std::vector<std::pair<std::string, std::string> > key2;
        key2.push_back(std::make_pair("type", "fc"));
        key2.push_back(std::make_pair("param", "2t"));
        key2.push_back(std::make_pair("year", "2016"));
        key2.push_back(std::make_pair("month", "02"));
        key2.push_back(std::make_pair("day", "04"));

        std::vector<std::pair<std::string, std::string> > key3;
        key3.push_back(std::make_pair("type", "fc"));
        key3.push_back(std::make_pair("param", "2t"));
        key3.push_back(std::make_pair("year", "2016"));
        key3.push_back(std::make_pair("month", "03"));
        key3.push_back(std::make_pair("day", "05"));

        std::string data("{\"text\": \"Example\"}");
        JSONDataBlob blob(data.c_str(), data.length());

        std::string data2("{\"text\": \"This is a second example\"}");
        JSONDataBlob blob2(data2.c_str(), data2.length());

        std::string data3("{\"text\": \"We are finally getting somewhere!\"}");
        JSONDataBlob blob3(data3.c_str(), data3.length());

        // For now, we aren't assuming feature/print-vector
        Log::info() << "Key: " << key << std::endl;
        Log::info() << "Key2: " << key2 << std::endl;
        Log::info() << "Key3: " << key3 << std::endl;

        root->addNode(key, blob);
        root->addNode(key2, blob2);
        root->addNode(key3, blob3);

    }


    PersistentPtr<TreeNode> rootNode = root->rootNode();

    Log::info() << *rootNode << std::endl;

    Log::info() << "===================================" << std::endl;
    rootNode->printTree(Log::info());
    Log::info() << std::endl;
    Log::info() << "===================================" << std::endl;

    // Do a lookup
    // N.B. This is done with a _map_, whereas the insertion is done with a _vector_
    // TODO: Decide how we want to do the data schema, otherwise this could end up with weird duplication
    //       (e.g. keys with different orders looking the same on lookup).
    std::map<FixedString<12>, FixedString<12> > lookup;
    lookup["type"] = std::string("fc");
    lookup["param"] = std::string("2t");
    lookup["year"] = std::string("2016");
    lookup["month"] = std::string("03");

    Log::info() << lookup << std::endl;
    std::vector<PersistentPtr<TreeNode> > nodes = rootNode->lookup(lookup);

    Log::info() << "[";
    for (std::vector<PersistentPtr<TreeNode> >::const_iterator it = nodes.begin();
         it != nodes.end(); ++it) {
        Log::info() << (*it)->name();
        if ((*it)->leaf()) {
            std::string tmp(static_cast<const char*>((*it)->data()), (*it)->dataSize());
            Log::info() << " -- " << tmp << std::endl;
        }
        Log::info() << ", ";
    }
    Log::info() << "]" << std::endl;

}

// -------------------------------------------------------------------------------------------------

} // namespace tree


int main(int argc, char** argv) {

    tree::TreeTool app(argc, argv);

    app.start();

    return 0;
}

