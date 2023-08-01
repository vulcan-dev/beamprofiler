#ifndef BP_STRING_H
#define BP_STRING_H

#include <cstring>
#include <cstdlib>
#include <cassert>

namespace bp
{
    // Stack Allocated String (Very basic, I don't need much from it.)
    template <size_t _Size>
    class fstring
    {
    public:
        // Constructor & Destructor
        //------------------------------------------------------------------------
        fstring()
        {
            _len = 0;
            _buf[0] = '\0';
        }

        fstring(const char* str)
        {
            _len = strlen(str);
            memcpy(&_buf, str, _len);
        }

        // Operators
        //------------------------------------------------------------------------
        fstring& operator=(const fstring& other)
        {
            assert(_Size > other._len);

            _len = other.len();
            memcpy(&_buf, other.c_str(), _len);
            _buf[_len] = '\0';

            return *this;
        }

        // Accessors
        //------------------------------------------------------------------------
        constexpr size_t len() const { return _len; }
        const char* c_str() const { return (const char*)_buf; }

    private:
        char _buf[_Size];
        size_t _len;
    };
}

#endif