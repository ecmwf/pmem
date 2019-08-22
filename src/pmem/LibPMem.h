/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/*
 * This software was developed as part of the EC H2020 funded project NextGenIO
 * (Project ID: 671951) www.nextgenio.eu
 */

/// @author Simon Smart
/// @date   February 2017

#ifndef pmem_LibPMem_H
#define pmem_LibPMem_H

#include "eckit/system/Library.h"

namespace pmem {

//----------------------------------------------------------------------------------------------------------------------

class LibPMem : public eckit::system::Library {
public:

    LibPMem();

    static const LibPMem& instance();

protected:

    const void* addr() const;

    virtual std::string version() const;

    virtual std::string gitsha1(unsigned int count) const;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace eckit

#endif // pmem_LibPMem_H
