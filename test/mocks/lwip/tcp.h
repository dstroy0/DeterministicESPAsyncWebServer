#pragma once
#include <stdint.h>
#include <string.h>

typedef int8_t err_t;
typedef uint16_t u16_t;

#define ERR_OK ((err_t)0)
#define ERR_MEM ((err_t) - 1)
#define ERR_VAL ((err_t) - 6)
#define ERR_ABRT ((err_t) - 8)

#define IPADDR_TYPE_ANY 0
#define IP_ANY_TYPE nullptr
#define TCP_WRITE_FLAG_COPY 0x01

struct tcp_pcb
{
};

struct pbuf
{
    void *payload;
    uint16_t len;
    uint16_t tot_len;
    struct pbuf *next;
};

// Return a stable non-null address so init() path succeeds in native tests.
// We use a single static instance since the mock never actually owns memory.
static struct tcp_pcb _mock_pcb;
// Typed callback aliases matching the real lwIP API so transport.cpp compiles
// without modification.
typedef err_t (*tcp_accept_fn)(void *arg, struct tcp_pcb *newpcb, err_t err);
typedef err_t (*tcp_recv_fn)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
typedef err_t (*tcp_sent_fn)(void *arg, struct tcp_pcb *tpcb, u16_t len);
typedef void (*tcp_err_fn)(void *arg, err_t err);

inline struct tcp_pcb *tcp_new_ip_type(int)
{
    return &_mock_pcb;
}
inline err_t tcp_bind(struct tcp_pcb *, void *, uint16_t)
{
    return ERR_OK;
}
inline struct tcp_pcb *tcp_listen_with_backlog(struct tcp_pcb *p, uint8_t)
{
    return p;
}
inline void tcp_arg(struct tcp_pcb *, void *)
{
}
inline void tcp_accept(struct tcp_pcb *, tcp_accept_fn)
{
}
inline void tcp_recv(struct tcp_pcb *, tcp_recv_fn)
{
}
inline void tcp_sent(struct tcp_pcb *, tcp_sent_fn)
{
}
inline void tcp_err(struct tcp_pcb *, tcp_err_fn)
{
}
inline void tcp_abort(struct tcp_pcb *)
{
}
// ---------------------------------------------------------------------------
// Optional write capture - off by default; tests enable with tcp_capture_reset()
// ---------------------------------------------------------------------------

struct TcpCapture
{
    char buf[4096];
    size_t len;
};

inline bool &_tcp_capture_active()
{
    static bool v = false;
    return v;
}

inline TcpCapture &_tcp_capture()
{
    static TcpCapture c = {};
    return c;
}

inline void tcp_capture_reset()
{
    _tcp_capture().len = 0;
    _tcp_capture().buf[0] = '\0';
    _tcp_capture_active() = true;
}

inline void tcp_capture_disable()
{
    _tcp_capture_active() = false;
}

inline const char *tcp_captured()
{
    return _tcp_capture().buf;
}
inline size_t tcp_captured_len()
{
    return _tcp_capture().len;
}

inline err_t tcp_write(struct tcp_pcb *, const void *data, uint16_t len, uint8_t)
{
    if (_tcp_capture_active())
    {
        TcpCapture &c = _tcp_capture();
        size_t avail = sizeof(c.buf) - c.len - 1;
        size_t n = (len < avail) ? (size_t)len : avail;
        if (n > 0)
        {
            memcpy(c.buf + c.len, data, n);
            c.len += n;
            c.buf[c.len] = '\0';
        }
    }
    return ERR_OK;
}
inline void tcp_output(struct tcp_pcb *)
{
}
inline err_t tcp_close(struct tcp_pcb *)
{
    return ERR_OK;
}
inline void tcp_recved(struct tcp_pcb *, uint16_t)
{
}
inline void pbuf_free(struct pbuf *)
{
}
