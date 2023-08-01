#ifndef BP_VECTOR_H
#define BP_VECTOR_H

namespace bp
{
    template <typename _Type>
    class vec
    {
    public:
        constexpr vec() : _data(nullptr), _size(0), _capacity(0) {}

        vec(vec&& other) noexcept : _data(other._data), _size(other._size), _capacity(other._capacity) {
            other._data = nullptr;
            other._size = 0;
            other._capacity = 0;
        }

        vec& operator=(vec&& other) noexcept {
            if (this != &other) {
                delete[] _data;
                _data = other._data;
                _size = other._size;
                _capacity = other._capacity;
                other._data = nullptr;
                other._size = 0;
                other._capacity = 0;
            }
            return *this;
        }

        ~vec()
        {
            delete[] _data;
        }

        // Copy Constructor
        vec(const vec& other)
        {
            _size = other._size;
            _capacity = other._capacity;
            _data = new _Type[_capacity];
            for (size_t i = 0; i < _size; ++i) {
                _data[i] = other._data[i];
            }
        }

        // Assignment
        vec& operator=(const vec& other) {
            if (this != &other) {
                delete[] _data;
                _size = other._size;
                _capacity = other._capacity;
                _data = new _Type[_capacity];
                for (size_t i = 0; i < _size; ++i) {
                    _data[i] = other._data[i];
                }
            }
            return *this;
        }

        // General
        void push_back(const _Type& element) {
            if (_size >= _capacity) {
                if (_capacity == 0) {
                    _capacity = 1;
                } else {
                    _capacity *= 2;
                }
                _Type* new_data = new _Type[_capacity];
                if (_data) {
                    memcpy(new_data, _data, _size * sizeof(_Type));
                    delete[] _data;
                }
                _data = new_data;
            }
            memcpy(&_data[_size++], &element, sizeof(element));
        }

        void reserve(size_t capacity) {
            if (capacity > _capacity) {
                _Type* newData = new _Type[capacity];
                for (size_t i = 0; i < _size; ++i) {
                    newData[i] = _data[i];
                }
                delete[] _data;
                _data = newData;
                _capacity = capacity;
            }
        }

        void clear() {
            delete[] _data;
            _data = nullptr;
            _size = 0;
            _capacity = 0;
        }

        void pop_back() {
            if (_size > 0) {
                --_size;
            }
        }

        const size_t size() const { return _size; }

        _Type& back() {
            return _data[_size - 1];
        }

        const _Type& back() const {
            return _data[_size - 1];
        }

        _Type& operator[](size_t index) {
            return _data[index];
        }

        const _Type& operator[](size_t index) const {
            return _data[index];
        }

        // Iterator Functions
        _Type* begin() {
            return _data;
        }

        const _Type* begin() const {
            return _data;
        }

        _Type* end() {
            return _data + _size;
        }

        const _Type* end() const {
            return _data + _size;
        }

    private:
        _Type* _data;
        size_t _size;
        size_t _capacity;
    };
}

#endif