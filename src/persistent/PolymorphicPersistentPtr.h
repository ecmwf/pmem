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

// The types provided must:
// 
//  i) Define a data type, data_type
// ii) Provide a constructor initialised with persistent_ptr<T::data_type>

///// template <typename T>
///// class PolymorphicPersistentPtr : public PersistentPtr<T::data_type> {
/////
///// public: // methods
/////
/////     // We want an object type, and a data type.
/////     typedef T object_type;
/////     typedef T::data_type data_type;
/////
/////
/////     // Accessors
///// //    object_type * object() const;
/////
/////
///// private: // members
/////
/////     // n.b. There is NO data here. Internally it is just a wrapper around the data pointer
/////
///// };


// -------------------------------------------------------------------------------------------------


// no. Needs to call factory
//template <typename T>
//PolymorphicPersistentPtr<T>::data_type PolymorphicPersistentPtr::object() const {
//    return data_type(*this);
//}


// -------------------------------------------------------------------------------------------------

}

#endif // persistent_PersistentPtr_H
