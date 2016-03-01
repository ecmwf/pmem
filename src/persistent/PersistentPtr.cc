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


#include "persistent/PersistentPtr.h"
#include "persistent/AtomicConstructor.h"

#include "eckit/log/Log.h"


using namespace eckit;


namespace pmem {


// -------------------------------------------------------------------------------------------------


/// This is a static routine that can be passed to the atomic allocation routines. All the logic
/// should be passed in as the functor AtomicConstructor<T>.
void persistentConstructor(PMEMobjpool * pop, void * obj, void * arg) {
    const AtomicConstructorBase * constr_fn = reinterpret_cast<const AtomicConstructorBase*>(arg);

    Log::info() << "Constructing persistent object of " << constr_fn->size()
                << " bytes at: " << obj << std::endl;

    constr_fn->build(obj);
    ::pmemobj_persist(pop, obj, constr_fn->size());
}


// -------------------------------------------------------------------------------------------------


PersistentPtrBase::PersistentPtrBase() :
    oid_(OID_NULL) {}


/// Return a persistent pointer from the actual pointer in pmem space
/// --> This is reverse engineered from pmem.io
/// --> TODO: Test this. Test this. Test this.
PersistentPtrBase::PersistentPtrBase(void* object) {

    PMEMobjpool * pop = ::pmemobj_pool_by_ptr(object);

    if (pop == 0) {
        throw eckit::SeriousBug("Attempting to obtain persistent pointer to volatile memory", Here());
    }

    // N.b. the pool uuid is available, internally, as pop->uuid. This requires
    //      libpmemobj/obj.h - but is intentionally NOT available in their C api
    //      HOWEVER, we need a persistent of the *this* pointer for C++, so a
    //      class __needs__ to be able access the UUID.
    //
    // --> Therefore we register the pool UUID when the root object is accessed.
    // --> This might not work if we only have inter-pool access. TBD.
    oid_.off = uintptr_t(object) - uintptr_t(pop);
    oid_.pool_uuid_lo = PersistentPool::poolUUID(pop);
}


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
