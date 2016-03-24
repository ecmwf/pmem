/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/io/DataBlob.h"
#include "eckit/runtime/Tool.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "pmem/PersistentPool.h"
#include "pmem/PersistentPtr.h"

#include "tree/TreePool.h"
#include "tree/TreeRoot.h"
#include "tree/TreeNode.h"
#include "tree/PersistentBuffer.h"

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
    options.push_back(new SimpleOption<size_t>("size", "The size of the pool file to create"));

    CmdArgs args(&usage, 1, options);

    size_t pool_size = args.getLong("size", 20 * 1024 * 1024);
    PathName path = args.args()[0];
    Log::info() << "Specified pool: " << path << std::endl;

    // ---------------------------------------------------------------------------------
    // Now start the tool.

    TreePool pool(path, pool_size, args.has("size"));

    PersistentPtr<TreeRoot> root = pool.root();

    Log::info() << "Valid: " << (root->valid() ? "true" : "false") << std::endl;

    PersistentPtr<TreeNode> rootNode = root->rootNode();
    Log::info() << "Node: " << (rootNode.null() ? "null" : "init") << std::endl;

    if (rootNode.null()) {

        // We have an empty tree. Initialise it!
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
        DataBlobPtr blob(DataBlobFactory::build("json", data.c_str(), data.length()));

        std::string data2("{\"text\": \"This is a second example\"}");
        DataBlobPtr blob2(DataBlobFactory::build("json", data2.c_str(), data2.length()));

        std::string data3("{\"text\": \"We are finally getting somewhere!\"}");
        DataBlobPtr blob3(DataBlobFactory::build("json", data3.c_str(), data3.length()));

        // For now, we aren't assuming feature/print-vector
//        Log::info() << "Key: " << key << std::endl;
//        Log::info() << "Key2: " << key2 << std::endl;
//        Log::info() << "Key3: " << key3 << std::endl;

        root->addNode(key, *blob);
        root->addNode(key2, *blob2);
        root->addNode(key3, *blob3);

    } else {

//        size_t cnt = rootNode->nodeCount();

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
}

// -------------------------------------------------------------------------------------------------

} // namespace tree


int main(int argc, char** argv) {

    tree::TreeTool app(argc, argv);

    app.start();

    return 0;
}

