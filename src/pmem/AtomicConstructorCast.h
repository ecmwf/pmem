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
/// @date   Feb 2016


#ifndef pmem_AtomicConstructorCast_H
#define pmem_AtomicConstructorCast_H

#include "pmem/AtomicConstructor.h"

namespace pmem {

// -------------------------------------------------------------------------------------------------

/// Allow one AtomicConstructor to masquerade as another one. eeep!
///
/// @note This should only be used with great care, as part of a type hierarchy. I.e. to allocate
///       a derived type in a base-class PersistentPtr.


template <typename S, typename T>
class AtomicConstructorCast : public AtomicConstructor<T> {
public:

    AtomicConstructorCast(const AtomicConstructor<S>& ctr) : ctr_(ctr) {}

    virtual size_t size() const { return ctr_.size(); }

    virtual uint64_t type_id() const { return ctr_.type_id(); }

    virtual void make(T& object) const { ctr_.make(static_cast<S&>(object)); }

private: // members
    const AtomicConstructor<S>& ctr_;
};


// -------------------------------------------------------------------------------------------------

} // namespace pmem

#endif // pmem_AtomicConstructorCast_H
