
//******************************************************************************
// RCF - Remote Call Framework
//
// Copyright (c) 2005 - 2018, Delta V Software. All rights reserved.
// http://www.deltavsoft.com
//
// RCF is distributed under dual licenses - closed source or GPL.
// Consult your particular license for conditions of use.
//
// If you have not purchased a commercial license, you are using RCF 
// under GPL terms.
//
// Version: 3.0
// Contact: support <at> deltavsoft.com 
//
//******************************************************************************

#ifndef INCLUDE_RCF_HTTPENDPOINT_HPP
#define INCLUDE_RCF_HTTPENDPOINT_HPP

#include <RCF/Export.hpp>
#include <RCF/TcpEndpoint.hpp>

namespace RCF {

    /// Represents a HTTP endpoint. 
    
    /// RCF implements HTTP endpoints with an HTTP envelope around the native RCF protocol.
    /// The primary use case for HttpEndpoint is client/server communication that may need to pass through forward or reverse HTTP proxies.
    class RCF_EXPORT HttpEndpoint : public TcpEndpoint
    {
    public:
        // *** SWIG BEGIN ***

        /// Constructs an HTTP endpoint on the given port number.
        HttpEndpoint(int port);

        /// Constructs an HTTP endpoint on the given IP address and port number.
        HttpEndpoint(const std::string & ip, int port);

        // *** SWIG END ***

        /// Returns a string representation of the HTTP endpoint.
        std::string asString() const;

        ServerTransportUniquePtr createServerTransport() const;
        ClientTransportUniquePtr createClientTransport() const;
        EndpointPtr clone() const;
    };

} // namespace RCF

#endif // INCLUDE_RCF_HTTPENDPOINT_HPP
