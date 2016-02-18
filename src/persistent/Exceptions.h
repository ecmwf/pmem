/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016


#ifndef persistent_Exceptions_H
#define persistent_Exceptions_H

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
    PersistentCreateError(const std::string&, int errno);
    PersistentCreateError(const std::string&, int errno, const eckit::CodeLocation&);
};


class PersistentOpenError : public PersistentError {
public:
    PersistentOpenError(const std::string&, int errno);
    PersistentOpenError(const std::string&, int errno, const eckit::CodeLocation&);
};
// -------------------------------------------------------------------------------------------------

}

#endif // persistent_Exceptions_H
