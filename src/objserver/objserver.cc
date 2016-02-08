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
#include "eckit/runtime/Application.h"

#include <string>

#include <errno.h>
#include <fcntl.h>
//#include "libpmem.h"
#include <unistd.h>
#include "libpmemobj.h"
#include <stdio.h>
#include <cstdlib>

using namespace eckit;

namespace objserver {

// -------------------------------------------------------------------------------------------------


class ObjectServer : public Application {

public: // methods

    ObjectServer(int argc, char** argv);
    virtual ~ObjectServer();

    virtual void run();

    void run_tx();

    void run_atomic();

private: // members

    size_t pmemLength_;
    
    PathName pmemPath_;
};

// -------------------------------------------------------------------------------------------------


ObjectServer::ObjectServer(int argc, char** argv) :
    Application(argc, argv),
    pmemPath_(Resource<std::string>("-path", "")),
    pmemLength_(Resource<size_t>("-length", 4096 * 4096 * 20))
    {}


ObjectServer::~ObjectServer() {}


void ObjectServer::run() {
    eckit::Log::info() << "Inside the run routine" << std::endl;

//    run_tx();
    run_atomic();


}


// -------------------------------------------------------------------------------------------------
const size_t buflen_max = 100;
const size_t buflen_min = 10;


class root_obj_atomic;
class list_obj_atomic;
POBJ_LAYOUT_BEGIN(string_store_atomic);
POBJ_LAYOUT_ROOT(string_store_atomic, root_obj_atomic);
POBJ_LAYOUT_TOID(string_store_atomic, list_obj_atomic);
POBJ_LAYOUT_END(string_store_atomic);

    
struct root_obj_atomic  {
    TOID(list_obj_atomic) next;
};

struct list_obj_atomic {
    size_t len;
    TOID(list_obj_atomic) next;

    char buf[buflen_min];
};

struct init_info {
    char c;
    size_t len;
};


void construct_list_element(PMEMobjpool * pop, void * ptr, void * arg) {
    list_obj_atomic * obj = reinterpret_cast<list_obj_atomic*>(ptr);
    const init_info * info = reinterpret_cast<const init_info*>(arg);

    Log::info() << "Building!!!" << std::endl;
    // Initialise an the list object with the correct number of characters
    // of the specified type
    obj->next = OID_NULL;
    obj->len = info->len;
    memset(obj->buf, info->c, info->len);

    pmemobj_persist(pop, obj, info->len - buflen_min + sizeof(list_obj_atomic));
}


void construct_root_element(PMEMobjpool * pop, void * ptr, void * arg) {
    root_obj_atomic * root = reinterpret_cast<root_obj_atomic*>(ptr);

    root->next = OID_NULL;
    pmemobj_persist(pop, root, sizeof(root_obj_atomic));
}


void ObjectServer::run_atomic() {




    // TODO:
    // - Append a new list element for each run.
    // - Convert from using transactions to using atomic operations
    // - Wrap the atomic operations inside C++ code!
    // - Make the list reference arbitrary BLOCKs of memory, of randomized size.



    PMEMobjpool * pop = NULL;

    // Do different things if the file exists, or if it needs to be created
    if (::access(pmemPath_.localPath(), F_OK) != -1) {

        Log::info() << "Opening memory pool: " << POBJ_LAYOUT_NAME(string_store_atomic) << std::endl;
        pop = pmemobj_open(pmemPath_.localPath(), POBJ_LAYOUT_NAME(string_store_atomic));
        if (pop == NULL) {
            Log::error() << "Failed to open memory pool" << std::endl;
            return;
        }

        TOID(root_obj_atomic) root = POBJ_ROOT(pop, root_obj_atomic);

        // Normally we would use the POBJ_NEW macro, but by using the pmemobj_alloc routine
        // directly, we can allocate arbitrarily sized objects.
        // TODO: Wrap this all in a C++ class!

        PMEMoid * target;

        if (TOID_IS_NULL(D_RO(root)->next)) {
            Log::info() << "Only the root object..." << std::endl;
            target = &D_RW(root)->next.oid;
        } else {

            TOID(list_obj_atomic) obj = D_RW(root)->next;
            while (!TOID_IS_NULL(obj)) {

                std::string rstring(D_RO(obj)->buf, D_RO(obj)->len);
                Log::info() << "Read (" << D_RO(obj)->len << "): " << rstring << std::endl;

                target = &D_RW(obj)->next.oid;
                obj = D_RW(obj)->next;
            };
        }

        // Note that the allocated size does not equal the size of the object, as there is some
        // accounting information included as well.
        size_t genlen = buflen_min + (rand() % (buflen_max-buflen_min));
        size_t objlen = genlen - buflen_min + sizeof(list_obj_atomic);
        Log::info() << "Allocating new: " << genlen << " bytes" << std::endl;

        init_info info;
        info.len = genlen;
        info.c = 'a';
        pmemobj_alloc(pop, target, objlen, TOID_TYPE_NUM(list_obj_atomic), construct_list_element, &info);

    } else {

        Log::info() << "Creating memory pool: " << POBJ_LAYOUT_NAME(string_store_atomic) << std::endl;
        pop = pmemobj_create(pmemPath_.localPath(), POBJ_LAYOUT_NAME(string_store_atomic), 
                                           PMEMOBJ_MIN_POOL, 0666);
        if (pop == NULL) {
            Log::error() << "Failed to create memory pool" << std::endl;
            return;
        }

        POBJ_NEW(pop, NULL, root_obj_atomic, construct_root_element, NULL);
    }

    Log::info() << "Closing" << std::endl;
    pmemobj_close(pop);
    Log::info() << "Done" << std::endl;
}


// -------------------------------------------------------------------------------------------------


void ObjectServer::run_tx() {

    const size_t buflen_max = 100;
    const size_t buflen_min = 10;


    class root_obj;
    class list_obj;
    POBJ_LAYOUT_BEGIN(string_store);
    POBJ_LAYOUT_ROOT(string_store, root_obj);
    POBJ_LAYOUT_TOID(string_store, list_obj);
    POBJ_LAYOUT_END(string_store);

    class list_obj {
    public:
        size_t len;
        TOID(list_obj) next;

        char buf[buflen_min];
    };
    
    class root_obj {
    public:
        TOID(list_obj) next;
    };


    // TODO:
    // - Append a new list element for each run.
    // - Convert from using transactions to using atomic operations
    // - Wrap the atomic operations inside C++ code!
    // - Make the list reference arbitrary BLOCKs of memory, of randomized size.



    PMEMobjpool * pop = NULL;

    // Do different things if the file exists, or if it needs to be created
    if (::access(pmemPath_.localPath(), F_OK) != -1) {

        Log::info() << "Opening memory pool: " << POBJ_LAYOUT_NAME(string_store) << std::endl;
        pop = pmemobj_open(pmemPath_.localPath(), POBJ_LAYOUT_NAME(string_store));
        if (pop == NULL) {
            Log::error() << "Failed to open memory pool" << std::endl;
            return;
        }

        TOID(root_obj) root = POBJ_ROOT(pop, root_obj);

        /*
         * Note that EVERYTHING is inside a giant transaction, as we don't want to modify
         * only SOME elements of the chain, and not others...
         */

        TX_BEGIN(pop) {

            TOID(list_obj) * next = &D_RW(root)->next;
            TOID(list_obj) * obj;

            bool broot;
            if (TOID_IS_NULL(*next)) {
                broot = true;
                TX_ADD(root);
            } else {
                broot = false;

                while (!TOID_IS_NULL(*next)) {
                    obj = next;
                    next = &D_RW(*obj)->next;

                    std::string rstring(D_RO(*obj)->buf, D_RO(*obj)->len);
                    Log::info() << "Read (" << D_RO(*obj)->len << "): " << rstring << std::endl;

                    // SDS: We could be more picky here, and prepare a smaller region.
                    TX_ADD(*obj);
                    memset(D_RW(*obj)->buf, rstring[0]+1, D_RO(*obj)->len);
                };
            }

            // Note that the allocated size does not equal the size of the object, as there is some
            // accounting information included as well.
            size_t genlen = buflen_min + (rand() % (buflen_max-buflen_min));
            size_t objlen = genlen - buflen_min + sizeof(list_obj);
            Log::info() << "Allocating new: " << genlen << " bytes" << std::endl;
            TOID(list_obj) new_obj = (TOID(list_obj))(pmemobj_tx_alloc(objlen, TOID_TYPE_NUM(list_obj)));

            //TOID(list_obj) new_obj = TX_NEW(list_obj);
            D_RW(new_obj)->len = genlen;
            D_RW(new_obj)->next = OID_NULL;
            memset(D_RW(new_obj)->buf, 'a', genlen);

                //TX_MEMSET(D_RW(root)->buf, 'a', buflen);

            *next = new_obj;

        } TX_ONABORT {
            Log::error() << "Transaction failed" << std::endl;
        } TX_ONCOMMIT {
            Log::error() << "Transaction committed" << std::endl;
        } TX_END;

    } else {

        Log::info() << "Creating memory pool: " << POBJ_LAYOUT_NAME(string_store) << std::endl;
        pop = pmemobj_create(pmemPath_.localPath(), POBJ_LAYOUT_NAME(string_store), 
                                           PMEMOBJ_MIN_POOL, 0666);
        if (pop == NULL) {
            Log::error() << "Failed to create memory pool" << std::endl;
            return;
        }

        TOID(root_obj) root = POBJ_ROOT(pop, root_obj);

        TX_BEGIN(pop) {
            TX_SET(root, next, OID_NULL);
        } TX_END;
    }

    Log::info() << "Closing" << std::endl;
    pmemobj_close(pop);
    Log::info() << "Done" << std::endl;
}

// -------------------------------------------------------------------------------------------------

} // namespace objserver

int main(int argc, char** argv) {

    objserver::ObjectServer app(argc, argv);

    app.start();

    return 0;
}

/*

   This is a section for older bits


   Pure PMEM work:

    // Create a pmem file
    int fd = 0;
    Log::info() << "Opening pool file: " << pmemPath_ << std::endl;
    if ((fd = open(pmemPath_.localPath(), O_CREAT|O_RDWR, 0666)) < 0) {
        Log::error() << "Failed to open/create pool file" << std::endl;
        return;
    }

    // Allocate a defined sized region
    if ((errno = posix_fallocate(fd, 0, pmemLength_)) != 0) {
        Log::error() << "Failed to allocate pool memory" << std::endl;
        return;
    }

    // And memory map the pool
    void* pmemaddr;
    if ((pmemaddr = pmem_map(fd)) == NULL) {
        Log::error() << "Memory map failed" << std::endl;
    }

    close(fd);

    // Is this pmem?
    bool is_pmem = pmem_is_pmem(pmemaddr, pmemLength_);
   Log::info() << "Is PMEM PMEM? " << (is_pmem ? "true" : "false") << std::endl;


   // ---------------------------------------


   Adding to a linked list using transactions

   
    const size_t buflen = 10;


    class root_obj;
    class list_obj;
    POBJ_LAYOUT_BEGIN(string_store);
    POBJ_LAYOUT_ROOT(string_store, root_obj);
    POBJ_LAYOUT_TOID(string_store, list_obj);
    POBJ_LAYOUT_END(string_store);

    class list_obj {
    public:
        size_t len;
        char buf[buflen];
        TOID(list_obj) next;
    };
    
    class root_obj {
    public:
        TOID(list_obj) next;
    };


    // TODO:
    // - Append a new list element for each run.
    // - Convert from using transactions to using atomic operations
    // - Wrap the atomic operations inside C++ code!
    // - Make the list reference arbitrary BLOCKs of memory, of randomized size.



    PMEMobjpool * pop = NULL;

    // Do different things if the file exists, or if it needs to be created
    if (::access(pmemPath_.localPath(), F_OK) != -1) {

        Log::info() << "Opening memory pool: " << POBJ_LAYOUT_NAME(string_store) << std::endl;
        pop = pmemobj_open(pmemPath_.localPath(), POBJ_LAYOUT_NAME(string_store));
        if (pop == NULL) {
            Log::error() << "Failed to open memory pool" << std::endl;
            return;
        }

        TOID(root_obj) root = POBJ_ROOT(pop, root_obj);

        / *
         * Note that EVERYTHING is inside a giant transaction, as we don't want to modify
         * only SOME elements of the chain, and not others...
         * /

        TX_BEGIN(pop) {

            TOID(list_obj) * next = &D_RW(root)->next;
            TOID(list_obj) * obj;

            bool broot;
            if (TOID_IS_NULL(*next)) {
                broot = true;
                TX_ADD(root);
            } else {
                broot = false;

                while (!TOID_IS_NULL(*next)) {
                    obj = next;
                    next = &D_RW(*obj)->next;

                    std::string rstring(D_RO(*obj)->buf, D_RO(*obj)->len);
                    Log::info() << "Read: " << rstring << std::endl;

                    // SDS: We could be more picky here, and prepare a smaller region.
                    TX_ADD(*obj);
                    memset(D_RW(*obj)->buf, rstring[0]+1, buflen);
                };
            }

            TOID(list_obj) new_obj = TX_NEW(list_obj);
            D_RW(new_obj)->len = buflen;
            D_RW(new_obj)->next = OID_NULL;
            memset(D_RW(new_obj)->buf, 'a', buflen);

                //TX_MEMSET(D_RW(root)->buf, 'a', buflen);

            *next = new_obj;

        } TX_ONABORT {
            Log::error() << "Transaction failed" << std::endl;
        } TX_ONCOMMIT {
            Log::error() << "Transaction committed" << std::endl;
        } TX_END;

    } else {

        Log::info() << "Creating memory pool: " << POBJ_LAYOUT_NAME(string_store) << std::endl;
        pop = pmemobj_create(pmemPath_.localPath(), POBJ_LAYOUT_NAME(string_store), 
                                           PMEMOBJ_MIN_POOL, 0666);
        if (pop == NULL) {
            Log::error() << "Failed to create memory pool" << std::endl;
            return;
        }

        TOID(root_obj) root = POBJ_ROOT(pop, root_obj);

        TX_BEGIN(pop) {
            TX_SET(root, next, OID_NULL);
        } TX_END;
    }

    pmemobj_close(pop);


    // ----------------------------------------

   */
