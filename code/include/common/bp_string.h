#ifndef BP_DYNSTRING_H
#define BP_DYNSTRING_H

#include <cstring>
#include <cstdlib>

namespace bp
{
    class string
    {
    public:
        string() : _len(0), _buf('\0') {}
        string(const char* str)
        {
            _len = strlen(str);
            _buf = new char[_len + 1];
            memcpy(_buf, str, _len);
            _buf[_len] = '\0';
        }

        string(const string& other)
        {
            _len = other._len;
            _buf = new char[_len + 1];
            memcpy(_buf, other._buf, _len + 1);
        }

        string& operator=(const string& other)
        {
            if (this != &other)
            {
                delete[] _buf;
                _len = other._len;
                _buf = new char[_len + 1];
                memcpy(_buf, other._buf, _len + 1);
            }
            return *this;
        }

        ~string() { delete[] _buf; }

        const char* c_str() const { return (const char*)_buf; }

    public:
        template <typename... Args>
        static string format(const char* format, Args... args)
        {
            int len = snprintf(nullptr, 0, format, args...);
            if (len <= 0)
                return string();

            string result;
            result._len = static_cast<size_t>(len);
            result._buf = new char[result._len + 1];
            snprintf(result._buf, result._len + 1, format, args...);
            return result;
        }

    private:
        char* _buf;
        size_t _len;
    };
}

#endif