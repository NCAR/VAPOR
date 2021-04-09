#include <vapor/Base16StringStream.h>

std::streambuf::int_type Base16StreamBuf::overflow(std::streambuf::int_type c)
{
    if (c != EOF) {
        _string += "0123456789ABCDEF"[(c & 0xF0) >> 4];
        _string += "0123456789ABCDEF"[c & 0x0F];
    }
    return c;
}

void Base16DecoderStream::Base16Decode(const std::string &in, char *out)
{
    const char map[] = {
         0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        -1,-1,-1,-1,-1,-1,-1,
        10,11,12,13,14,15
    };
    for (int i = 0; i < in.size(); i += 2) {
        out[i/2] = (map[in[i]-48] << 4) | (map[in[i+1]-48]);
    }
}

Base16DecoderStream::Base16DecoderStream(const std::string &s)
: std::istream(&_buf)
, _data(new char[s.size()/2])
, _buf(_data.get(), s.size()/2)
{
    Base16Decode(s, _data.get());
}
