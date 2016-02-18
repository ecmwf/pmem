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

#include <unistd.h>

#include "objserver/layout_dlist.h"
#include "objserver/persistent_ptr.h"


using namespace eckit;
using namespace pmem;

// -------------------------------------------------------------------------------------------------

class DListTool : public Tool {
public: // methods

    DListTool(int argc, char** argv);
    virtual ~DListTool();

    virtual void run();

protected: // methods

    void newpool();
    void existingpool();

private: // members

    size_t pmemLength_;
    
    PathName pmemPath_;
};

// -------------------------------------------------------------------------------------------------

// TODO:
// - Initialise list
// - Element push_front
// - Rebuild
// - Remove.

DListTool::DListTool(int argc, char** argv) :
    Tool(argc, argv),
    pmemPath_(Resource<std::string>("-path", "")),
    pmemLength_(Resource<size_t>("-length", 4096 * 4096 * 20)) {}


DListTool::~DListTool() {}


void DListTool::run() {
    Log::info() << "Running double-linked-list tool" << std::endl;
    Log::info() << "Specified pmem pool: " << pmemPath_ << std::endl;

    if (::access(pmemPath_.localPath(), F_OK) != -1) {
        existingpool();
    } else {
        newpool();
    }
}


void DListTool::newpool() {

    Log::info() << "Creating memory pool: " << POBJ_LAYOUT_NAME(dlist_layout) << std::endl;

    PMEMobjpool * pop = pmemobj_create(pmemPath_.localPath(), POBJ_LAYOUT_NAME(dlist_layout), 
                                       pmemLength_, 0666);
    if (pop == NULL) {
        Log::error() << "Failed to create memory pool" << std::endl;
        return;
    }

    // Create a persistent list, and allocate space for it in pmem
    persistent_list<buffer_type> dlist(pop, true);

    pmemobj_close(pop);
}


void DListTool::existingpool() {

    Log::info() << "Opening memory pool: " << POBJ_LAYOUT_NAME(dlist_layout) << std::endl;
    PMEMobjpool * pop = pmemobj_open(pmemPath_.localPath(), POBJ_LAYOUT_NAME(dlist_layout));
    if (pop == NULL) {
        Log::error() << "Failed to open memory pool" << std::endl;
        return;
    }

    persistent_list<buffer_type> dlist(pop, true);


    // TODO: pass the data in...
    class DataConstructor : public persistent_list<buffer_type>::data_constructor {
    public:

        DataConstructor(size_t size) : size_(size) {}

        virtual void make(buffer_type* obj) const {
            obj->len = size_;
        }

        virtual size_t size() const {
            return sizeof(buffer_type) - sizeof(size_t) + size_;
        }

    private:
        size_t size_;
    };

    
    // 1. Iterate through the existing members to see them
    // TODO: Proper iterators

    persistent_list<buffer_type>::const_iterator it = dlist.begin();
    for (; it != dlist.end(); ++it) {
        Log::info() << "Found: ..." << std::endl;
    }

    // 2. Extend the list

    dlist.push_back(DataConstructor(15));

    pmemobj_close(pop);
}


// -------------------------------------------------------------------------------------------------


int main(int argc, char** argv) {

    DListTool tool(argc, argv);
    return tool.start();

}
