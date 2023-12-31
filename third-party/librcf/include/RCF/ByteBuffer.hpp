
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

#ifndef INCLUDE_RCF_BYTEBUFFER_HPP
#define INCLUDE_RCF_BYTEBUFFER_HPP

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <RCF/Export.hpp>

namespace RCF {

    class MemOstream;
    typedef std::shared_ptr<MemOstream> MemOstreamPtr;

    class ReallocBuffer;
    typedef std::shared_ptr<ReallocBuffer> ReallocBufferPtr;

    /// ByteBuffer is a internally reference counted buffer class, designed to hold a large chunk of data and allow
    /// it to be passed around in a program, without incurring any copying overhead. It is conceptually similar to a
    /// std::shared_ptr< std::vector<char> >.
    class RCF_EXPORT ByteBuffer
    {
    public:

        ByteBuffer();

        explicit
        ByteBuffer(std::size_t pvlen);

        explicit
        ByteBuffer(
            const std::vector<char> & vc);

        explicit
        ByteBuffer(
            const std::string & s);

        explicit
        ByteBuffer(
            std::shared_ptr<std::vector<char> > spvc,
            bool readOnly = false);

        explicit
        ByteBuffer(
            ReallocBufferPtr sprb,
            bool readOnly = false);

        explicit
        ByteBuffer(
            MemOstreamPtr spos,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            std::size_t leftMargin,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            std::shared_ptr<MemOstream> spos,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            std::size_t leftMargin,
            std::shared_ptr<MemOstream> spos,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            std::shared_ptr<std::vector<char> > spvc,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            std::size_t leftMargin,
            std::shared_ptr<std::vector<char> > spvc,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            ReallocBufferPtr sprb,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            std::size_t leftMargin,
            ReallocBufferPtr sprb,
            bool readOnly = false);

        ByteBuffer(
            const ByteBuffer & byteBuffer,
            std::size_t offset = 0,
            std::size_t len = std::size_t(-1));

        char *              getPtr()            const;
        std::size_t         getLength()         const;
        std::size_t         getLeftMargin()     const;
        bool                getReadOnly()       const;
        bool                isEmpty()           const;
        std::string         string()            const;

        void                setLeftMargin(std::size_t len);
        void                expandIntoLeftMargin(std::size_t len);
        ByteBuffer          release();
        void                swap(ByteBuffer & rhs);
        void                clear();

                            operator bool();
        bool                operator !();

        static const std::size_t npos;

    private:
        // sentries
        std::shared_ptr< std::vector<char> >      mSpvc;
        std::shared_ptr< MemOstream >             mSpos;
        std::shared_ptr< ReallocBuffer >          mSprb;

        char *                                      mPv;
        std::size_t                                 mPvlen;
        std::size_t                                 mLeftMargin;
        bool                                        mReadOnly;
    };

    RCF_EXPORT bool operator==(const ByteBuffer &lhs, const ByteBuffer &rhs);

    RCF_EXPORT std::size_t lengthByteBuffers(
        const std::vector<ByteBuffer> &byteBuffers);

    RCF_EXPORT void forEachByteBuffer(
        std::function<void(const ByteBuffer&)> functor,
        const std::vector<ByteBuffer> &byteBuffers,
        std::size_t offset,
        std::size_t length = -1);
    
    RCF_EXPORT ByteBuffer sliceByteBuffer(
        const std::vector<ByteBuffer> &slicedBuffers,
        std::size_t offset,
        std::size_t length = -1);

    RCF_EXPORT void sliceByteBuffers(
        std::vector<ByteBuffer> &slicedBuffers,
        const std::vector<ByteBuffer> &byteBuffers,
        std::size_t offset,
        std::size_t length = std::size_t(-1));

    RCF_EXPORT void copyByteBuffers(
        const std::vector<ByteBuffer> &byteBuffers,
        char *pch);

    RCF_EXPORT void copyByteBuffers(
        const std::vector<ByteBuffer> &byteBuffers,
        ByteBuffer &byteBuffer);

} // namespace RCF

namespace SF {

    class Archive;

    RCF_EXPORT void serialize(SF::Archive &ar, RCF::ByteBuffer &byteBuffer);

}

#endif // ! INCLUDE_RCF_BYTEBUFFER_HPP
