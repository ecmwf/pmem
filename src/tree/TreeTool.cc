/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#include "tree/TreePool.h"
#include "tree/TreeRoot.h"
#include "tree/TreeNode.h"
#include "tree/PersistentBuffer.h"

#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/runtime/Tool.h"

#include "persistent/PersistentPool.h"
#include "persistent/PersistentPtr.h"

using namespace eckit;
using namespace pmem;

namespace treetool {

// -------------------------------------------------------------------------------------------------


class TreeTool : public Tool {

public: // methods

    TreeTool(int argc, char** argv);
    virtual ~TreeTool();

    virtual void run();

private: // members

    size_t pmemLength_;
    
    PathName pmemPath_;
};

// -------------------------------------------------------------------------------------------------


TreeTool::TreeTool(int argc, char** argv) :
    Tool(argc, argv),
    pmemPath_(Resource<std::string>("-path", "")),
    pmemLength_(Resource<size_t>("-length", 4096 * 4096 * 20))
    {}


TreeTool::~TreeTool() {}


void TreeTool::run() {
    eckit::Log::info() << "Inside the run routine" << std::endl;

    TreePool pool(pmemPath_, pmemLength_);

    PersistentPtr<TreeRoot> root = pool.root();

    Log::info() << "Valid: " << (root->valid() ? "true" : "false") << std::endl;

    PersistentPtr<TreeNode> rootNode = root->rootNode();
    Log::info() << "Node: " << (rootNode.null() ? "null" : "init") << std::endl;

    std::string node_nms[] = {
        "node1",
        "node2",
        "node3",
        "node4",
        "node5",
        "node6",
        "node7",
        "node8",
        "node9",
        "node10",
        "node11",
        "node12"
    };


    if (!rootNode.null()) {

        size_t cnt = rootNode->nodeCount();

//        class TreePrintVisitor : public TreeNode::Visitor {
//        public:
//            void operator() (TreeNode& ) {
//
//
//            }
//        };

        Log::info() << *rootNode << std::endl;


    //    Log::info() << "===================================" << std::endl;
    //    Log::info() << "Node name: " << rootNode->name_ << std::endl;
    //    Log::info() << "Subnodes: " << cnt << std::endl;

    //    for (size_t i = 0; i < cnt; i++) {
    //        Log::info() << "  " << rootNode->items_[i].first
    //                    << " -- " << rootNode->items_[i].second->name_ << std::endl;

    //        if (!rootNode->items_[i].second->data_.null()) {
    //            std::string tmp(*rootNode->items_[i].second->data_,
    //                            rootNode->items_[i].second->data_->size());
    //            Log::info() << "STR: " << tmp << std::endl;
    //        }
    //    }

    //    Log::info() << "===================================" << std::endl;


        std::string name = cnt < 12 ? node_nms[cnt] : "higher";
        rootNode->addNode(name, "12345");

        // Do a lookup
        std::map<FixedString<12>, FixedString<12> > lookup;
        lookup["123456789012"] = std::string("node3");

        Log::info() << lookup << std::endl;
        std::vector<PersistentPtr<TreeNode> > nodes = rootNode->lookup(lookup);

        Log::info() << "[";
        for (std::vector<PersistentPtr<TreeNode> >::const_iterator it = nodes.begin();
             it != nodes.end(); ++it) {
            Log::info() << (*it)->name() << ", ";
        }
        Log::info() << "]" << std::endl;
    }
}

// -------------------------------------------------------------------------------------------------

} // namespace treetool


int main(int argc, char** argv) {

    treetool::TreeTool app(argc, argv);

    app.start();

    return 0;
}

