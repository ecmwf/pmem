/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   August 2016

#include <algorithm>
#include <string>

#include "pmem/LibPMem.h"

#include "pmem/pmem_version.h"

namespace pmem {

//----------------------------------------------------------------------------------------------------------------------

static LibPMem libpmem;


LibPMem::LibPMem() : Library("pmem") {}

const LibPMem& LibPMem::instance() {
    return libpmem;
}

const void* LibPMem::addr() const {
    return this;
}

std::string LibPMem::version() const {
    return pmem_version_str();
}

std::string LibPMem::gitsha1(unsigned int count) const {
    std::string sha1(pmem_git_sha1());
    if(sha1.empty()) {
        return "not available";
    }

    return sha1.substr(0, std::min(count, 40u));
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace eckit

