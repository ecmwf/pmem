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


#ifndef persistent_PolymorphicPersistentPtr_H
#define persistent_PolymorphicPersistentPtr_H

#include "persistent/PersistentPtr.h"


namespace pmem {


// -------------------------------------------------------------------------------------------------

// --> Fundamentally we need to overload get() to return a new OBJECT, which itself
//     must refer to the relevant PersistentPtr.
//
// --> We need to have _FACTORIES_ to return the correct PersistentPtr types, and the
//     the correct object types given the typeid
//
// --> We can then check that the deriving of the type is valid.

template <typename T>
class PolymorphicPersistentPtr : public PersistentPtr<T> {
};


// -------------------------------------------------------------------------------------------------

}

#endif // persistent_PersistentPtr_H
