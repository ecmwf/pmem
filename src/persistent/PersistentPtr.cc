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


} // namespace pmem
