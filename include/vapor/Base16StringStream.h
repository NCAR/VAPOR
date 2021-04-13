#pragma once

#include <streambuf>
#include <ostream>
#include <istream>
#include <memory>



//! \class Base16StreamBuf
//!
//! \brief A custom std stream buffer that saves data piped through it into a base16 encoded string.
//!
//! \author Stanislaw Jaroszynski

class Base16StreamBuf : public std::streambuf {
public:
    std::string      _string;
    int              _i = 0;
    virtual int_type overflow(int_type c) override;
};


//! \class Base16StringStream
//!
//! \brief An implementation similar to std::stringstream which results in a base16 encoded string.
//!
//! \author Stanislaw Jaroszynski

class Base16StringStream : public std::ostream {
    Base16StreamBuf _buf;

public:
    Base16StringStream() : std::ostream(&_buf) {}
    const std::string &ToString() { return _buf._string; }
};



//! \class Base16DecoderStream
//!
//! \brief Creates a raw std istream from a base16 encoded string.
//!
//! \author Stanislaw Jaroszynski

class Base16DecoderStream : public std::istream {
    struct MemBuf : std::streambuf {
        MemBuf(char *buf, size_t size) { this->setg(buf, buf, buf + size); }
    };

    static void Base16Decode(const std::string &in, char *out);

    std::unique_ptr<char> _data;
    MemBuf                _buf;

public:
    Base16DecoderStream(const std::string &s);
};
