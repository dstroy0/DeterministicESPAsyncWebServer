#pragma once
// Mock filesystem for native unit tests.
// Mimics the subset of the ESP32 Arduino fs::FS / fs::File API used by
// DetWebServer::serve_file().

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

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
// Write capture (for streaming-upload tests): File::write() appends here when a
// file is opened in "w"/"a" mode.
// ---------------------------------------------------------------------------

inline uint8_t *_mock_wbuf()
{
    static uint8_t b[8192];
    return b;
}
inline size_t &_mock_wlen()
{
    static size_t n = 0;
    return n;
}
inline void mock_fs_write_reset()
{
    _mock_wlen() = 0;
}
inline size_t mock_fs_written()
{
    return _mock_wlen();
}
inline const uint8_t *mock_fs_wdata()
{
    return _mock_wbuf();
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
    time_t _mtime;

  public:
    File() : _data(nullptr), _size(0), _pos(0), _open(false), _mtime(0)
    {
    }
    File(const uint8_t *data, size_t size, time_t mtime = 0)
        : _data(data), _size(size), _pos(0), _open(true), _mtime(mtime)
    {
    }
    // Writable sink: bytes go to the shared write-capture buffer.
    explicit File(bool open_writable) : _data(nullptr), _size(0), _pos(0), _open(open_writable), _mtime(0)
    {
    }

    size_t write(const uint8_t *src, size_t sz)
    {
        if (!_open)
            return 0;
        size_t cap = 8192 - _mock_wlen();
        size_t w = sz < cap ? sz : cap;
        memcpy(_mock_wbuf() + _mock_wlen(), src, w);
        _mock_wlen() += w;
        return w;
    }
    size_t write(uint8_t b)
    {
        return write(&b, 1);
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

    // Mirrors fs::File::seek(pos, SeekSet) for Range request tests.
    bool seek(uint32_t pos)
    {
        if (!_open || pos > _size)
            return false;
        _pos = pos;
        return true;
    }

    size_t size() const
    {
        return _size;
    }
    time_t getLastWrite() const
    {
        return _mtime;
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
    time_t mtime;
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

inline void mock_fs_add(const char *path, const uint8_t *data, size_t size, time_t mtime = 0)
{
    int &n = _mock_entry_count();
    if (n < 16)
    {
        _mock_entries()[n].path = path;
        _mock_entries()[n].data = data;
        _mock_entries()[n].size = size;
        _mock_entries()[n].mtime = mtime;
        n++;
    }
}
inline void mock_fs_add(const char *path, const char *text, time_t mtime = 0)
{
    mock_fs_add(path, reinterpret_cast<const uint8_t *>(text), strlen(text), mtime);
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
    File open(const char *path, const char *mode = "r")
    {
        // Write/append mode -> a writable sink (captured via mock_fs_written()).
        if (mode && (mode[0] == 'w' || mode[0] == 'a'))
            return File(true);
        if (_mock_entry_count() > 0)
        {
            const MockFsEntry *e = _mock_find(path);
            return e ? File(e->data, e->size, e->mtime) : File();
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
