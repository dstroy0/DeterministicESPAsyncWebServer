# DeterministicESPAsyncWebServer

A deterministic, OSI-layered asynchronous web server for the ESP32 - RFC-compliant HTTP/1.1, HTTP/2 and
HTTP/3, plus a large opt-in protocol suite (WebSocket, SSE, MQTT, CoAP, SNMP, SSH, OPC UA, Modbus, and
many more), all zero-heap and fixed-buffer for predictable behavior on a microcontroller.

This site brings the hand-written guides and the generated API reference together: the guides (below) are
rendered from the project's Markdown via MyST, and the **API reference** is rendered from the Doxygen XML
via Breathe. The styling is "squirty" - the Furo theme carrying the project's Retro TTY Green Screen brand
and [Squirty the Injection Squid](https://github.com/dstroy0/DeterministicESPAsyncWebServer) as the mascot.

```{toctree}
:maxdepth: 2
:caption: Guides

architecture
osi-model
tcp-ip
languages
```

```{toctree}
:maxdepth: 2
:caption: Reference

api
```

## About this site

- The guides stay authored in `docs/` and `docs/learn/`; the pages here `include` them, so there is one
  source of truth.
- The API reference is generated: `docs/Doxyfile` emits XML into `docs/sphinx/xml`, and Breathe renders it.
- To rebuild locally, see the header of `docs/sphinx/conf.py`.
