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

    options.push_back(new SimpleOption<std::string>("request", "Specify a (potentially complex) request in a file"));

    options.push_back(new Separator("Options for creating a new pool"));
    options.push_back(new SimpleOption<bool>("create", "Insert an element as specified by the pool and data keys"));
    options.push_back(new SimpleOption<size_t>("size", "The size of the pool file to create"));
    options.push_back(new SimpleOption<PathName>("schema", "The file containing the data schema"));

    options.push_back(new Separator("Options for adding new leaves to the tree"));
    options.push_back(new SimpleOption<bool>("insert", "Insert an element as specified by the pool and data keys"));
    options.push_back(new SimpleOption<std::string>("key", "The key to insert. This is a json object of key-value pairs"));
    options.push_back(new SimpleOption<PathName>("data", "The file containing the data to insert"));

    options.push_back(new Separator("Options for inspecting the tree"));
    options.push_back(new SimpleOption<bool>("print", "Prints the tree in its entirety to stdout"));
    options.push_back(new SimpleOption<std::string>("lookup", "Specify a (partial) key (as JSON) to perform a lookup"));

    CmdArgs args(&usage, 1, options);

    size_t pool_size = args.getLong("size", 20 * 1024 * 1024);
    PathName path = args.args()[0];
    Log::info() << "Specified pool: " << path << std::endl;

    // ---------------------------------------------------------------------------------
    // Now start the tool.

    ScopedPtr<TreePool> pool;

    if (args.getBool("create", false)) {

        Log::status() << "Attempting to create new pool";

        PathName schema_path(args.getString("schema"));
        TreeSchema schema(schema_path);

        pool.reset(new TreePool(path, pool_size, schema));

    } else {

        // If we aren't creating a pool, then open it instead.
        pool.reset(new TreePool(path));
    }

    PersistentPtr<TreeRoot> root = pool->root();

    Log::info() << "Valid: " << (root->valid() ? "true" : "false") << std::endl;

    TreeObject tree(*root);

    // Do an insertion request

    if (args.getBool("insert", false)) {

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

    }

    // Doing lookup requests

    if (args.getBool("print", false)) {
        Log::info() << "===================================" << std::endl;
        tree.printTree(Log::info());
        Log::info() << std::endl;
        Log::info() << "===================================" << std::endl;
    }

    // Doing lookup requests

    std::string lookup = args.getString("lookup", "");
    if (lookup != "") {

        std::istringstream iss(lookup);
        JSONParser parser(iss);

        StringDict key;
        JSONParser::toStrDict(parser.parse(), key);

        std::vector<PersistentPtr<TreeNode> > nodes = tree.lookup(key);

        Log::info() << "Matching data" << std::endl;
        Log::info() << "=============" << std::endl;
        // TODO: We should probably output the matching keys as well as the data.
        for (std::vector<PersistentPtr<TreeNode> >::const_iterator it = nodes.begin();
             it != nodes.end(); ++it) {
            if ((*it)->leaf()) {
                std::string tmp(static_cast<const char*>((*it)->data()), (*it)->dataSize());
                Log::info() << tmp << std::endl;
            }
        }
    }

    // How about a multi-request in a JSON file

    std::string request_file = args.getString("request", "");
    if (request_file != "") {

        Log::info() << "Using request file: " << request_file << std::endl;

        PathName request_path(request_file);
        ScopedPtr<DataHandle> file(request_path.fileHandle());
        Length len = file->openForRead();

        JSONDataBlob blob(*file, len);

        std::string blob_str(blob.buffer(), blob.length());
        std::istringstream iss(blob_str);
        JSONParser parser(iss);

        ValueList requests = parser.parse().as<ValueList>();
        for (ValueList::const_iterator rq = requests.begin(); rq != requests.end(); ++rq) {

            ValueMap request = *rq;

            std::string op = request["operation"].as<std::string>();
            if (op == "insert") {

                StringDict key;
                JSONParser::toStrDict(request["key"], key);

                std::string data(request["data"]);
                JSONDataBlob blob(data.c_str(), data.length());

                tree.addNode(key, blob);
            }
        }
    }
}

// -------------------------------------------------------------------------------------------------

} // namespace tree


int main(int argc, char** argv) {

    tree::TreeTool app(argc, argv);

    app.start();

    return 0;
}

