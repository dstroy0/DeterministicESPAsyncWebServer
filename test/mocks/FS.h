#pragma once
// Mock filesystem for native unit tests.
// Mimics the subset of the ESP32 Arduino fs::FS / fs::File API used by
// DetWebServer::serve_file().

#include <stddef.h>
#include <string.h>

namespace fs
{

// ---------------------------------------------------------------------------
// Shared mock state -- use mock_fs_set() / mock_fs_clear() in tests
// ---------------------------------------------------------------------------

inline const uint8_t *&_mock_data()
{
    static const uint8_t *p = nullptr;
    return p;
}
inline size_t &_mock_size()
{
    static size_t s = 0;
    return s;
}
inline bool &_mock_valid()
{
    static bool v = false;
    return v;
}

inline void mock_fs_set(const uint8_t *data, size_t size)
{
    _mock_data() = data;
    _mock_size() = size;
    _mock_valid() = true;
}

inline void mock_fs_set(const char *text)
{
    _mock_data() = reinterpret_cast<const uint8_t *>(text);
    _mock_size() = strlen(text);
    _mock_valid() = true;
}

inline void mock_fs_clear()
{
    _mock_data() = nullptr;
    _mock_size() = 0;
    _mock_valid() = false;
}

// ---------------------------------------------------------------------------
// File
// ---------------------------------------------------------------------------

class File
{
    const uint8_t *_data;
    size_t _size;
    size_t _pos;
    bool _open;

  public:
    File() : _data(nullptr), _size(0), _pos(0), _open(false)
    {
    }
    File(const uint8_t *data, size_t size) : _data(data), _size(size), _pos(0), _open(true)
    {
    }

    size_t read(uint8_t *dst, size_t sz)
    {
        if (!_open || _pos >= _size)
            return 0;
        size_t avail = _size - _pos;
        size_t n = (sz < avail) ? sz : avail;
        memcpy(dst, _data + _pos, n);
        _pos += n;
        return n;
    }

    size_t size() const
    {
        return _size;
    }
    void close()
    {
        _open = false;
    }
    explicit operator bool() const
    {
        return _open;
    }
};

// ---------------------------------------------------------------------------
// FS
// ---------------------------------------------------------------------------

class FS
{
  public:
    File open(const char * /*path*/, const char * /*mode*/ = "r")
    {
        if (_mock_valid() && _mock_data())
            return File(_mock_data(), _mock_size());
        return File(); // invalid - signals file-not-found
    }
};

} // namespace fs
