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

    PersistentPool pool(pmemPath_, pmemLength_, "tree-exp");

}

// -------------------------------------------------------------------------------------------------

} // namespace treetool


int main(int argc, char** argv) {

    treetool::TreeTool app(argc, argv);

    app.start();

    return 0;
}

