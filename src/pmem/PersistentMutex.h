/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/*
 * This software was developed as part of the EC H2020 funded project NextGenIO
 * (Project ID: 671951) www.nextgenio.eu
 */

/// @author Simon Smart
/// @date   Sept 2016

#ifndef pmem_PersistentMutex_H
#define pmem_PersistentMutex_H

#include "libpmemobj.h"


namespace pmem {

//----------------------------------------------------------------------------------------------------------------------

/// This is a Mutex class that is designed to operate with the eckit::AutoLock
class PersistentMutex {

public: // methods

    /// @note There is no initialisation required for the persistent mutexes. This is dealt with for us inside the
    ///       libpmemobj library. Magic!

    void lock()    {   ::pmemobj_mutex_lock(::pmemobj_pool_by_ptr(&mutex_), &mutex_); }
    void unlock()  { ::pmemobj_mutex_unlock(::pmemobj_pool_by_ptr(&mutex_), &mutex_); }

private: // members

    PMEMmutex mutex_;
};


//----------------------------------------------------------------------------------------------------------------------

}

#endif // pmem_PersistentMutex_H
