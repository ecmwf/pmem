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
/// @date   Feb 2016

#include "eckit/log/Log.h"

#include "pmem/PersistentPtr.h"
#include "pmem/AtomicConstructor.h"

using namespace eckit;


namespace pmem {


// -------------------------------------------------------------------------------------------------


/// This is a static routine that can be passed to the atomic allocation routines. All the logic
/// should be passed in as the functor AtomicConstructor<T>.
int pmem_constructor(PMEMobjpool * pool, void * obj, void * arg) {
    const AtomicConstructorBase * constr_fn = reinterpret_cast<const AtomicConstructorBase*>(arg);

    Log::info() << "Constructing persistent object of " << constr_fn->size()
                << " bytes at: " << obj << std::endl;

    // The constructor should return zero for success. If it has failed (e.g. if a subobjects
    // allocation has failed) this needs to be propagated upwards so that the block reservation
    // can be correctly unwound.
    int ret = constr_fn->build(obj);
    if (ret != 0)
        ::pmemobj_persist(pool, obj, constr_fn->size());
    return ret;
}

// -------------------------------------------------------------------------------------------------


PersistentPtrBase::PersistentPtrBase() :
    oid_(OID_NULL) {}


PersistentPtrBase::PersistentPtrBase(PMEMoid oid) :
    oid_(oid) {}


void PersistentPtrBase::free() {
    ::pmemobj_free(&oid_);
}


void PersistentPtrBase::nullify() {
    oid_ = OID_NULL;
}


bool PersistentPtrBase::null() const {
    return oid_.off == 0;
}


// -------------------------------------------------------------------------------------------------


} // namespace pmem
