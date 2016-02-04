/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#include "eckit/runtime/Application.h"

using namespace eckit;

namespace objserver {

// -------------------------------------------------------------------------------------------------


class ObjectServer : public Application {

public: // methods

    ObjectServer(int argc, char** argv);
    virtual ~ObjectServer();

    virtual void run();
};

// -------------------------------------------------------------------------------------------------


ObjectServer::ObjectServer(int argc, char** argv) :
    Application(argc, argv) {}


ObjectServer::~ObjectServer() {}


void ObjectServer::run() {
    eckit::Log::info() << "inside the run routine" << std::endl;
}

// -------------------------------------------------------------------------------------------------

} // namespace objserver

int main(int argc, char** argv) {

    objserver::ObjectServer app(argc, argv);

    app.start();

    return 0;
}
