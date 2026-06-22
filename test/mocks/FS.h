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
// Path-aware registry (for serve_static / gzip tests)
//
// Legacy single-file API (mock_fs_set / open path-agnostic) is preserved for
// existing tests. When at least one mock_fs_add(path,...) entry is registered,
// open()/exists() become path-aware and the legacy single file is ignored.
// ---------------------------------------------------------------------------

struct MockFsEntry
{
    const char *path;
    const uint8_t *data;
    size_t size;
};

inline MockFsEntry *_mock_entries()
{
    static MockFsEntry e[16];
    return e;
}
inline int &_mock_entry_count()
{
    static int n = 0;
    return n;
}

inline void mock_fs_add(const char *path, const uint8_t *data, size_t size)
{
    int &n = _mock_entry_count();
    if (n < 16)
    {
        _mock_entries()[n].path = path;
        _mock_entries()[n].data = data;
        _mock_entries()[n].size = size;
        n++;
    }
}
inline void mock_fs_add(const char *path, const char *text)
{
    mock_fs_add(path, reinterpret_cast<const uint8_t *>(text), strlen(text));
}
inline void mock_fs_reset()
{
    _mock_entry_count() = 0;
    mock_fs_clear();
}
inline const MockFsEntry *_mock_find(const char *path)
{
    for (int i = 0; i < _mock_entry_count(); i++)
        if (strcmp(_mock_entries()[i].path, path) == 0)
            return &_mock_entries()[i];
    return nullptr;
}

// ---------------------------------------------------------------------------
// FS
// ---------------------------------------------------------------------------

class FS
{
  public:
    File open(const char *path, const char * /*mode*/ = "r")
    {
        if (_mock_entry_count() > 0)
        {
            const MockFsEntry *e = _mock_find(path);
            return e ? File(e->data, e->size) : File();
        }
        // Legacy path-agnostic single-file behavior.
        if (_mock_valid() && _mock_data())
            return File(_mock_data(), _mock_size());
        return File(); // invalid - signals file-not-found
    }

    bool exists(const char *path)
    {
        if (_mock_entry_count() > 0)
            return _mock_find(path) != nullptr;
        return _mock_valid(); // legacy: any path "exists" if a file is set
    }
};

} // namespace fs
