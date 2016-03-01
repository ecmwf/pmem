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

    Log::info() << "Root tag: " << root->tag_ << std::endl;

    Log::info() << "Valid: " << (root->valid() ? "true" : "false") << std::endl;

    Log::info() << "Node: " << (root->node_.null() ? "null" : "init") << std::endl;

    std::string nodes[] = {
        "node1",
        "node2",
        "node3",
        "node4",
        "node5",
        "node6"
    };


    if (!root->node_.null()) {

        size_t cnt = root->node_->nodeCount();

        Log::info() << "===================================" << std::endl;
        Log::info() << "Node name: " << root->node_->name_ << std::endl;
        Log::info() << "Subnodes: " << cnt << std::endl;

        for (size_t i = 0; i < cnt; i++) {
            Log::info() << "  " << root->node_->items_[i].first
                        << " -- " << root->node_->items_[i].second->name_ << std::endl;
        }

        Log::info() << "===================================" << std::endl;


        std::string name = cnt < 6 ? nodes[cnt] : "higher";
        root->node_->addNode(name, "12345");

        // Do a lookup
        std::map<FixedString<12>, FixedString<12> > lookup;
        lookup["123456789012"] = std::string("node3");

        Log::info() << lookup << std::endl;
        std::vector<PersistentPtr<TreeNode> > nodes = root->node_->lookup(lookup);

        Log::info() << "[";
//        for (std::vector<PersistentPtr<TreeNode> >::const_iterator it = nodes.begin();
//             it != nodes.end(); ++it) {
//            Log::info() << (*it)->name_ << ", ";
//        }
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

