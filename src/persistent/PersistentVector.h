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


#ifndef tree_PersistentVector_H
#define tree_PersistentVector_H

/*
 * Modus-operandi:
 *
 * A persistent vector wraps the PersistentPtr functionality. Each change to the vector
 * is a re-allocation of the array contained inside the persistent vector.
 *
 * TODO:
 * N.B. An advantage of the interface, is that this can easily be extended. If we use
 *      fixed sizes of vector, we don't need to re-allocate until we reach a certain
 *      number of elements. We can then point to a "continuation array".
 */


namespace pmem {


// -------------------------------------------------------------------------------------------------


template <typename T>
class PersistentVector {

public:

    // TODO: This should only be used during construction. TODO.
    void nullify() {
        items_.nullify();
    }

protected:

    PersistentPtr<T[]> items_;
};


// -------------------------------------------------------------------------------------------------


} // namespace pmem

#endif // tree_PersistentVector_H

