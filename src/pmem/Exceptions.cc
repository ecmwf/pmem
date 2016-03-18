/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016

#include <string>

#include "pmem/Exceptions.h"

using namespace eckit;

namespace pmem {

// -------------------------------------------------------------------------------------------------

PersistentError::PersistentError(const std::string& msg, const CodeLocation& loc) :
    Exception(msg, loc) {}


PersistentError::PersistentError(const std::string& msg) :
    Exception(msg) {}


PersistentCreateError::PersistentCreateError(const std::string& file, int error, const CodeLocation& loc) :
    PersistentError(
        std::string("Error creating persistent memory pool: ") + file + " (" + strerror(error) + ")",
        loc) {}


PersistentCreateError::PersistentCreateError(const std::string& file, int error) :
    PersistentError(
        std::string("Error creating persistent memory pool: ") + file + " (" + strerror(error) + ")") {}


PersistentOpenError::PersistentOpenError(const std::string& file, int error, const CodeLocation& loc) :
    PersistentError(
        std::string("Error opening persistent memory pool: ") + file + " (" + strerror(error) + ")",
        loc) {}


PersistentOpenError::PersistentOpenError(const std::string& file, int error) :
    PersistentError(
        std::string("Error opening persistent memory pool: ") + file + " (" + strerror(error) + ")") {}

// -------------------------------------------------------------------------------------------------

} // namespace pmem
