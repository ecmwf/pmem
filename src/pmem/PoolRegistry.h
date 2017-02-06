/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @author Simon Smart
/// @date   Feb 2017

#ifndef pmem_PoolRegistry_H
#define pmem_PoolRegistry_H

#include "libpmemobj.h"

#include <map>


namespace pmem {

class PersistentPool;

//----------------------------------------------------------------------------------------------------------------------

class PoolRegistry {

public: // methods

    static PoolRegistry& instance();

    void registerPool(PersistentPool& pool);

    PersistentPool& poolFromPointer(void* ptr);

private: // methods

    std::map<PMEMobjpool* const, PersistentPool* const> pools_;

private: // methods

    // The constructor is private, ensuring that only the instance in the instance() method
    // can be constructed.
    PoolRegistry();
    ~PoolRegistry();
};

//----------------------------------------------------------------------------------------------------------------------

}

#endif // pmem_PoolRegistry_H
