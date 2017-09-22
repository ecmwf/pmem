/*
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


#ifndef pmem_Exceptions_H
#define pmem_Exceptions_H

#include "eckit/exception/Exceptions.h"

namespace pmem {

// -------------------------------------------------------------------------------------------------

class PersistentError : public eckit::Exception {
public:
    PersistentError(const std::string&);
    PersistentError(const std::string&, const eckit::CodeLocation&);
};


class PersistentCreateError : public PersistentError {
public:
    PersistentCreateError(const std::string&, int error);
    PersistentCreateError(const std::string&, int error, const eckit::CodeLocation&);

    int errno_;
};


class PersistentOpenError : public PersistentError {
public:
    PersistentOpenError(const std::string&, int error);
    PersistentOpenError(const std::string&, int error, const eckit::CodeLocation&);

    int errno_;
};
// -------------------------------------------------------------------------------------------------

}

#endif // pmem_Exceptions_H
