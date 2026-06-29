# Hardware Hookup and Settings Guide

Many of this library's codecs do not talk to a web browser. They talk to **real
machines**: programmable logic controllers (PLCs), solar inverters, power-grid
sensors, building controllers, and other ESP32 boards. To use them you have to
connect the ESP32 to that machine with the right wires, the right adapter chip,
and the right communication settings.

This guide is the wiring-and-settings reference for every codec that needs
external hardware. It assumes **no prior electronics knowledge**: it explains
what each connection is, what part you need to buy, how to wire it, and what
numbers to type into your sketch.

> **One rule before anything else.** The ESP32's pins run at **3.3 volts and are
> not 5-volt tolerant.** Connecting a 5 V signal, a 12/24 V industrial signal, or
> an RS-232/RS-485 line **directly** to a GPIO pin can destroy the chip. Every
> serial bus below needs an adapter chip (a "transceiver" or "level shifter")
> between the machine and the ESP32. Never skip it.

## Contents

- [How these codecs work](#how-these-codecs-work)
- [The three ways a codec reaches its device](#the-three-ways-a-codec-reaches-its-device)
- [Quick-reference table](#quick-reference-table)
- [Serial field-bus codecs (you wire a transceiver)](#serial-field-bus-codecs-you-wire-a-transceiver)
    - [RS-485 wiring (the common case)](#rs-485-wiring-the-common-case)
    - [RS-232 wiring](#rs-232-wiring)
    - [Choosing and using the UART](#choosing-and-using-the-uart)
    - [Modbus RTU](#modbus-rtu)
    - [DF1 (Allen-Bradley)](#df1-allen-bradley)
    - [Host Link (Omron)](#host-link-omron)
    - [DNP3 and C37.118 over serial](#dnp3-and-c37118-over-serial)
- [CAN field-bus codecs (you wire a transceiver)](#can-field-bus-codecs-you-wire-a-transceiver)
    - [CAN wiring](#can-wiring)
    - [CANopen](#canopen)
    - [J1939](#j1939)
- [Networked industrial codecs (over Wi-Fi or Ethernet)](#networked-industrial-codecs-over-wi-fi-or-ethernet)
    - [Getting on the network](#getting-on-the-network)
    - [Modbus TCP and Modbus master](#modbus-tcp-and-modbus-master)
    - [SunSpec](#sunspec)
    - [S7comm (Siemens)](#s7comm-siemens)
    - [MELSEC (Mitsubishi)](#melsec-mitsubishi)
    - [FINS (Omron)](#fins-omron)
    - [BACnet/IP](#bacnetip)
    - [EtherNet/IP and CIP](#ethernetip-and-cip)
    - [DNP3 and C37.118 over IP](#dnp3-and-c37118-over-ip)
    - [OPC UA (server and client)](#opc-ua-server-and-client)
    - [SNMP (agent and traps)](#snmp-agent-and-traps)
- [Radio: ESP-NOW](#radio-esp-now)
- [Payload codecs that ride an existing link](#payload-codecs-that-ride-an-existing-link)
- [Electrical safety and reliability](#electrical-safety-and-reliability)

## How these codecs work

Every protocol codec in this library is **pure**: it turns a message into bytes
and bytes back into a message. It does **not** open the serial port or the network
socket for you. **Your sketch owns the wire.** That keeps the library deterministic
and lets the same codec run over a UART, a radio, Wi-Fi, or a host test.

In practice that means three jobs are yours:

1. **The hardware**: wire the adapter chip or join the network (this guide).
2. **The transport**: open the UART or socket and move the bytes (Arduino
   `HardwareSerial`, or this library's UDP/TCP transport where noted).
3. **The settings**: baud rate, parity, port number, device addresses.

The codec does the hard part in the middle: framing, checksums/CRCs, and the
wire format, all verified against the published spec. Each section below links to
the matching header in `src/services/` and its description in
[FEATURES.md](FEATURES.md).

## The three ways a codec reaches its device

| Family               | What carries the bytes                           | Extra hardware you supply                                   |
| -------------------- | ------------------------------------------------ | ----------------------------------------------------------- |
| **Serial field bus** | A UART pin pair, through an adapter chip         | An RS-485 or RS-232 transceiver, wiring, termination        |
| **CAN field bus**    | The ESP32 TWAI controller (or MCP2515 over SPI)  | A CAN transceiver (SN65HVD230), wiring, 120 ohm terminators |
| **Networked**        | TCP or UDP over the ESP32's Wi-Fi (or wired PHY) | Usually none beyond Wi-Fi; the device and a shared network  |
| **Radio (ESP-NOW)**  | The ESP32's built-in 2.4 GHz radio               | None; just a second ESP-class board as the peer             |

## Quick-reference table

"Flag" is the `DETWS_ENABLE_*` macro you set to compile the codec in (all default
**off**). "Talks to" is the machine on the other end.

| Codec         | Flag                         | Transport               | External hardware            | Typical settings                | Talks to                           |
| ------------- | ---------------------------- | ----------------------- | ---------------------------- | ------------------------------- | ---------------------------------- |
| Modbus RTU    | `DETWS_ENABLE_MODBUS_RTU`    | Serial (RS-485/232)     | RS-485 transceiver           | 19200 8E1, unit 1-247           | Modbus sensors, drives, meters     |
| Modbus TCP    | `DETWS_ENABLE_MODBUS`        | TCP                     | Wi-Fi/Ethernet               | port 502                        | Modbus PLCs and gateways           |
| Modbus master | `DETWS_ENABLE_MODBUS_MASTER` | TCP (client)            | Wi-Fi/Ethernet               | port 502                        | Modbus slave devices               |
| SunSpec       | `DETWS_ENABLE_SUNSPEC`       | Modbus (RTU or TCP)     | per Modbus row               | model base reg 40000            | Solar inverters, meters, batteries |
| DF1           | `DETWS_ENABLE_DF1`           | Serial (RS-232/485)     | RS-232 or RS-485 transceiver | 19200, full-duplex              | Allen-Bradley (Rockwell) PLCs      |
| Host Link     | `DETWS_ENABLE_HOSTLINK`      | Serial (RS-232/422/485) | transceiver                  | 9600 7E2, unit 00-31            | Omron PLCs (C-mode)                |
| FINS          | `DETWS_ENABLE_FINS`          | UDP                     | Wi-Fi/Ethernet               | UDP port 9600                   | Omron PLCs (FINS)                  |
| MELSEC        | `DETWS_ENABLE_MELSEC`        | TCP or UDP              | Wi-Fi/Ethernet               | port set on the PLC             | Mitsubishi PLCs (MC 3E)            |
| S7comm        | `DETWS_ENABLE_S7COMM`        | TCP (ISO-on-TCP)        | Wi-Fi/Ethernet               | port 102, rack/slot             | Siemens S7-300/400/1200/1500       |
| BACnet/IP     | `DETWS_ENABLE_BACNET`        | UDP                     | Wi-Fi/Ethernet               | UDP port 47808                  | Building automation controllers    |
| EtherNet/IP   | `DETWS_ENABLE_ENIP`          | TCP + UDP               | Wi-Fi/Ethernet               | TCP 44818, UDP 2222             | Allen-Bradley / ODVA devices       |
| CIP           | `DETWS_ENABLE_CIP`           | over EtherNet/IP        | Wi-Fi/Ethernet               | (rides ENIP)                    | CIP objects on ODVA devices        |
| DNP3          | `DETWS_ENABLE_DNP3`          | Serial or TCP           | transceiver or Wi-Fi         | TCP 20000, 16-bit addresses     | SCADA / utility outstations        |
| C37.118       | `DETWS_ENABLE_C37118`        | Serial, TCP or UDP      | transceiver or Wi-Fi         | no fixed port (often 4712/4713) | Power-grid PMUs / PDCs             |
| CANopen       | `DETWS_ENABLE_CANOPEN`       | CAN (TWAI or SPI)       | CAN transceiver              | 125k-1M bit/s, node 1-127       | Motion drives, I/O, CANopen nodes  |
| J1939         | `DETWS_ENABLE_J1939`         | CAN (TWAI or SPI)       | CAN transceiver              | 250k bit/s, 29-bit ids          | Trucks, tractors, gensets, marine  |
| OPC UA        | `DETWS_ENABLE_OPCUA`         | TCP                     | Wi-Fi/Ethernet               | port 4840, SecurityPolicy None  | OPC UA clients / SCADA             |
| OPC UA client | `DETWS_ENABLE_OPCUA_CLIENT`  | TCP (client)            | Wi-Fi/Ethernet               | port 4840                       | OPC UA servers                     |
| SNMP          | `DETWS_ENABLE_SNMP`          | UDP                     | Wi-Fi/Ethernet               | UDP 161 (agent), 162 (trap)     | Network monitoring systems         |
| ESP-NOW       | `DETWS_ENABLE_ESPNOW`        | 2.4 GHz radio           | none (a peer ESP board)      | shared channel, peer MAC        | Other ESP32 / ESP8266 boards       |

## Serial field-bus codecs (you wire a transceiver)

A "serial field bus" sends data one bit at a time down a wire, the way the
classic serial port did. Industrial buses use a tougher electrical standard than
a plain UART so the signal survives long noisy cable runs. The ESP32 cannot drive
those wires directly; a small **transceiver** chip sits between the ESP32's UART
and the bus.

### RS-485 wiring (the common case)

RS-485 is the workhorse of factory wiring. It sends each bit as the **difference**
between two wires (called **A**/**D-** and **B**/**D+**), which rejects electrical
noise and reaches over a kilometer. Many devices share one pair of wires (a
"multidrop" bus), so only one device may transmit at a time.

**What you need:** an RS-485 transceiver. Prefer a **3.3 V** part so its logic
pins match the ESP32 directly: `MAX3485`, `SP3485`, `ADM3485`, or `THVD1450`.
The classic `MAX485` is a 5 V part; if you use one, add a logic-level shifter on
its data pins. Cheap "RS-485 to TTL" breakout modules bundle the chip and the
screw terminals.

**The connections:**

| Transceiver pin             | Connect to                                     |
| --------------------------- | ---------------------------------------------- |
| `RO` (receiver out)         | ESP32 UART **RX**                              |
| `DI` (driver in)            | ESP32 UART **TX**                              |
| `DE` + `RE` (tied together) | One ESP32 **GPIO** (the direction-control pin) |
| `A` / `B`                   | The bus pair, to the same A/B on every device  |
| `VCC` / `GND`               | 3.3 V and ground, shared with the ESP32        |

Because two wires carry traffic both directions, you must tell the transceiver
when to talk and when to listen. That is the **direction-control pin**: drive the
GPIO **high** to transmit, **low** to receive. (Some modules auto-switch and need
no GPIO; they are simpler but slightly less reliable at high baud.)

**Two more parts that matter on a real bus:**

- **Termination**: a **120 ohm** resistor across A and B at **each of the two far
  ends** of the cable (not in the middle). It stops signal reflections.
- **Fail-safe bias**: one set of pull resistors (roughly 560-680 ohm: a pull-up
  on B to VCC and a pull-down on A to GND) somewhere on the bus, so an idle bus
  reads as a clean logic level. Many modules include these.

A 2-wire ("half-duplex") bus is by far the most common. A 4-wire ("full-duplex")
RS-485 uses two pairs (one to transmit, one to receive) and does not need the
direction pin.

### RS-232 wiring

RS-232 is the old PC serial port: one wire per direction, swinging between about
+/-3 and +/-15 volts, point-to-point (one device per port), short cables. You need
a **level shifter** such as the **3.3 V `MAX3232`** between the device's RS-232
connector and the ESP32 UART:

| MAX3232 side          | Connect to         |
| --------------------- | ------------------ |
| TTL `T1IN`            | ESP32 UART **TX**  |
| TTL `R1OUT`           | ESP32 UART **RX**  |
| RS-232 `T1OUT`/`R1IN` | the device's RX/TX |
| `VCC`/`GND`           | 3.3 V and ground   |

Cross TX to RX and RX to TX (a "null-modem" crossover) if both ends are devices
rather than a PC-and-modem pair.

### Choosing and using the UART

The ESP32 has three hardware UARTs. **UART0 is the USB programming/console port;
leave it alone.** Use **UART1** or **UART2** for the field bus. On the classic
ESP32, `Serial2` defaults to **GPIO16 (RX)** and **GPIO17 (TX)**, but you can map
a UART to almost any free GPIO. Avoid the boot **strapping pins** (GPIO0, 2, 5,
12, 15) and the **flash pins** (GPIO6-11), and on ESP32-S3/C3 pick any free GPIO
through the pin matrix.

Open the port with the baud rate and framing your device expects, then pump bytes
through the codec. A half-duplex RS-485 send looks like this:

```cpp
#include "services/modbus/modbus.h"

HardwareSerial &bus = Serial2;
const int DE_RE = 4; // RS-485 direction-control GPIO

void setup() {
  pinMode(DE_RE, OUTPUT);
  digitalWrite(DE_RE, LOW);                // listen by default
  bus.begin(19200, SERIAL_8E1, 16, 17);    // Modbus default: 19200, 8 data, even, 1 stop
}

// after the codec builds a reply of length n into resp[]:
//   digitalWrite(DE_RE, HIGH);            // take the bus
//   bus.write(resp, n);
//   bus.flush();                          // wait for the last bit out
//   digitalWrite(DE_RE, LOW);             // release, back to listening
```

Framing (deciding where one message ends and the next begins) is the transport's
job. For Modbus RTU that is the **3.5-character idle gap** between frames; collect
a frame, then hand it to the codec.

### Modbus RTU

- **Flag:** `DETWS_ENABLE_MODBUS_RTU` (turns on `DETWS_ENABLE_MODBUS` too).
- **Hardware:** RS-485 transceiver (occasionally RS-232 point-to-point).
- **Settings:** the Modbus spec default is **19200 baud, 8 data bits, even
  parity, 1 stop bit (8E1)**; **9600 8N1** is also very common. Every device on a
  bus must use the **same** baud and framing. Each slave has a **unit address**
  from **1 to 247** (0 is a broadcast with no reply).
- **Codec:** `modbus_rtu_process_adu()` validates the CRC-16 and the unit address
  and dispatches to the host-tested PDU layer; a bad CRC or a non-matching
  address is dropped silently, exactly as the spec requires. See
  `src/services/modbus/modbus.h`.

### DF1 (Allen-Bradley)

- **Flag:** `DETWS_ENABLE_DF1`.
- **Hardware:** RS-232 to a PLC serial port (a DB9), or RS-485 for multidrop;
  use the matching transceiver above.
- **Settings:** commonly **19200** (also 9600/4800), full-duplex DF1. The codec
  supports both check types: **BCC** (a simple checksum) and **CRC-16/ARC**;
  match what the PLC channel is configured for. Node/station addressing lives in
  the PCCC application header carried inside the frame.
- **Codec:** `df1_build_frame` / `df1_parse_frame` handle the `DLE STX ... DLE
ETX` framing and byte-stuffing. See `src/services/df1/df1.h`.

### Host Link (Omron)

- **Flag:** `DETWS_ENABLE_HOSTLINK`.
- **Hardware:** RS-232, RS-422, or RS-485 to an Omron PLC host-link port.
- **Settings:** a typical Omron host-link port is **9600 baud, 7 data bits, even
  parity, 2 stop bits (7E2)**; confirm the PLC's setting. Each PLC has a **unit
  number 00-31** that begins every command frame.
- **Codec:** `hostlink_build` / `hostlink_parse` build the `@` + unit + code +
  text + FCS + `*`CR ASCII frame and validate the XOR checksum (FCS). See
  `src/services/hostlink/hostlink.h`.

### DNP3 and C37.118 over serial

Both can run over serial as well as IP. Wire them like Modbus RTU (RS-485 or
RS-232 transceiver) and see their entries under
[networked codecs](#dnp3-and-c37118-over-ip) for addressing and the codec calls;
the framing is transport-independent.

## CAN field-bus codecs (you wire a transceiver)

CAN (Controller Area Network) is a two-wire differential bus (CAN_H / CAN_L)
used across factory automation and vehicles. It is cheap and robust, and the
ESP32 already has a CAN controller built in (called **TWAI**), so you only add a
**transceiver** chip to drive the differential pair.

### CAN wiring

You need a CAN transceiver between the ESP32 and the bus. Two cheap options:

- **SN65HVD230 breakout (~$1)** — 3.3 V, pairs directly with the ESP32's TWAI
  controller. Connect ESP32 `TX` GPIO -> transceiver `D` (TXD), transceiver `R`
  (RXD) -> ESP32 `RX` GPIO, plus 3V3 and GND. CAN_H / CAN_L go to the bus.
- **MCP2515 + TJA1050 module (~$2)** — a standalone CAN controller you talk to
  over **SPI** (use this if you would rather not use the internal TWAI, or need a
  second CAN channel). Wire SPI (SCK/MOSI/MISO/CS) + an interrupt GPIO.

Bus rules that matter: terminate **both ends** of the bus with a **120 ohm**
resistor (many breakouts have a jumper for it), keep stubs short, and set every
node to the **same bit rate** (125 kbit/s, 250 k, 500 k, and 1 Mbit/s are
common). All nodes on a CAN bus must agree on the bit rate.

### CANopen

`DETWS_ENABLE_CANOPEN`. CANopen (CiA 301) is the dominant higher-layer protocol
for CAN in factory automation: motion drives, I/O blocks, sensors. Each node has
an id 1-127. The codec builds and parses the messages; your sketch moves the
frames with the TWAI driver (or the MCP2515):

- Bring a node up: `canopen_build_nmt(&frame, CANOPEN_NMT_START, node)`.
- Read an object (expedited SDO): `canopen_build_sdo_read(&frame, node, index,
sub)`, send it, then `canopen_parse_sdo_response()` on the reply.
- Write an object: `canopen_build_sdo_write(...)`.
- Watch liveness: `canopen_parse_heartbeat()` on each `0x700+node` frame;
  `canopen_parse_emcy()` on emergencies.
- Receive process data: `canopen_parse()` classifies each frame, and a TPDO's
  `data[]` is the raw mapped payload.

This is the classic **wireless bridge**: poll CANopen drives over the wire and
publish their state over MQTT / HTTP / a WebSocket. SDO transfers are expedited
(<= 4 octets); segmented / block transfer is a future addition. See
`src/services/canopen/canopen.h`.

### J1939

`DETWS_ENABLE_J1939`. The CAN higher-layer protocol for heavy-duty vehicles,
agriculture, marine, and gensets - same wiring as above (transceiver +
terminators), but it uses **29-bit extended** ids, almost always at **250
kbit/s**. The codec packs and unpacks the id (priority / PGN / source /
destination) and handles multi-packet messages:

- Decode a received frame: `j1939_decode_id(frame.id, &id)` gives you the PGN,
  source, and destination; the 8 data octets are the parameter group's signals
  (SPNs), which you scale per the PGN definition.
- Ask a device for a PGN: `j1939_build_request(&frame, my_addr, dest, pgn)`.
- Announce your address: `j1939_build_address_claim(&frame, my_addr,
j1939_build_name(...))`.
- Long messages (> 8 octets, e.g. diagnostics): feed every received frame to
  `j1939_tp_feed(&rx, &frame)`; when it returns `J1939_TP_COMPLETE`, `rx.buf`
  holds the reassembled message for `rx.pgn`. To send one, `j1939_build_bam_cm()`
  then a `j1939_build_tp_dt()` per 7-octet chunk.

A classic **wireless gateway**: decode engine / transmission / genset PGNs off
the bus and publish them over MQTT or a web dashboard. See
`src/services/j1939/j1939.h`.

## Networked industrial codecs (over Wi-Fi or Ethernet)

These codecs reach a device over a network instead of a dedicated wire. The
"external hardware" is usually just the **target device plus a shared network**;
the ESP32's built-in Wi-Fi supplies the link.

### Getting on the network

- **Wi-Fi (the verified path):** join the ESP32 (station mode) to the **same
  network** as the industrial device, or to a network that can route to it. The
  device needs a reachable IP address; many PLCs ship with a fixed default IP you
  set with the vendor's tool.
- **Wired Ethernet (advanced):** for a hard-wired link, add an external Ethernet
  PHY (an RMII part such as the LAN8720, or an SPI part such as the W5500). Wired
  Ethernet is a roadmap item and is **not** hardware-verified in this library yet;
  Wi-Fi is the supported transport today.
- **Ports and firewalls:** open the protocol's TCP/UDP port (table above) between
  the ESP32 and the device. On a flat factory LAN this is automatic; across
  routers you may need a firewall rule.

### Modbus TCP and Modbus master

- **Flags:** `DETWS_ENABLE_MODBUS` (the ESP32 is the **slave/server**, using this
  library's TCP server with the `PROTO_MODBUS` listener) or
  `DETWS_ENABLE_MODBUS_MASTER` (the ESP32 is the **master/client**, your sketch
  owns the TCP connection).
- **Settings:** **TCP port 502.** The request carries a **unit identifier**
  (often 1, or 255/0xFF for a native TCP device). No serial parity or baud here;
  Modbus TCP wraps the same PDU in an MBAP header.
- **Codec:** the same `modbus_*` data model and PDU dispatch as RTU, minus the
  CRC (TCP already guarantees integrity). See `src/services/modbus/modbus.h`.

### SunSpec

- **Flag:** `DETWS_ENABLE_SUNSPEC`.
- **Hardware:** whatever your Modbus link uses (RS-485 for RTU, Wi-Fi/Ethernet
  for TCP). SunSpec is a **map on top of Modbus holding registers**, not a new
  wire.
- **Settings:** the device exposes a chain of "models" beginning at a base
  holding register (commonly **40000**, sometimes 50000) marked by the ASCII
  **`SunS`** signature. Read that register block over Modbus, then walk it.
- **Codec:** `sunspec_check_marker` / `sunspec_begin` / `sunspec_next_model` plus
  typed point readers. Makes a solar inverter, meter, or battery interoperable.
  See `src/services/sunspec/sunspec.h`.

### S7comm (Siemens)

- **Flag:** `DETWS_ENABLE_S7COMM` (needs `DETWS_ENABLE_COTP`).
- **Hardware:** Wi-Fi/Ethernet to a Siemens S7 PLC.
- **Settings:** **TCP port 102** (ISO-on-TCP). The connection is addressed by a
  **rack and slot** number that identify the CPU (for example rack 0 / slot 1 or
  slot 2 on an S7-300, rack 0 / slot 1 on an S7-1200/1500); the PLC must also
  permit "PUT/GET" access for external reads.
- **Codec:** `s7_build_setup` / `s7_build_read_request` / `s7_parse_header` /
  `s7_read_next_item`, wrapped with `cotp_build_dt` + `tpkt_build`. See
  `src/services/s7comm/s7comm.h`.

### MELSEC (Mitsubishi)

- **Flag:** `DETWS_ENABLE_MELSEC`.
- **Hardware:** Wi-Fi/Ethernet to a Mitsubishi PLC with an MC-protocol port open.
- **Settings:** **TCP or UDP**, on the **port you configure on the PLC** (MC
  protocol has no fixed IANA port). This codec speaks the **binary 3E** frame.
  Pick word or bit devices (D, M, X, Y, R, ...) by their device code.
- **Codec:** `melsec_build_read` / `melsec_parse_response`. See
  `src/services/melsec/melsec.h`.

### FINS (Omron)

- **Flag:** `DETWS_ENABLE_FINS`.
- **Hardware:** Wi-Fi/Ethernet to an Omron PLC. (FINS also has a serial sibling;
  for serial use [Host Link](#host-link-omron).)
- **Settings:** **FINS/UDP, port 9600** by default. Addressing is a
  **network / node / unit** triple for both source and destination; the node
  number usually matches the last octet of the PLC's IP by convention.
- **Codec:** `fins_build_command` / `fins_build_memory_area_read` /
  `fins_parse_response`, carried over this library's UDP transport
  (`det_udp_sendto`). See `src/services/fins/fins.h`.

### BACnet/IP

- **Flag:** `DETWS_ENABLE_BACNET`.
- **Hardware:** Wi-Fi/Ethernet to a building-automation network.
- **Settings:** **UDP port 47808** (hex 0xBAC0). Devices are identified by a
  **device instance** number; multi-subnet sites use a BBMD/`network number`. The
  codec builds the BVLC + NPDU envelope; the APDU object/service layer rides on
  top.
- **Codec:** see `src/services/bacnet/bacnet.h`.

### EtherNet/IP and CIP

- **Flags:** `DETWS_ENABLE_ENIP` (the encapsulation) and `DETWS_ENABLE_CIP` (the
  object messages, which turn ENIP on automatically).
- **Hardware:** Wi-Fi/Ethernet to an ODVA / Allen-Bradley device.
- **Settings:** **TCP port 44818** for explicit messaging (register a session,
  then send requests); **UDP port 2222** carries implicit (cyclic I/O) data. CIP
  addresses objects by a **class / instance / attribute** path (an EPATH).
- **Codec:** `enip_*` builds the 24-byte encapsulation header and CPF items;
  `cip_build_epath` + `cip_build_get_attribute_single` build the CIP request. See
  `src/services/enip/enip.h` and `src/services/cip/cip.h`.

### DNP3 and C37.118 over IP

- **DNP3** (`DETWS_ENABLE_DNP3`): the IEEE 1815 SCADA outstation link. Over IP it
  uses **TCP (or UDP) port 20000**; data-link **source and destination addresses**
  are 16-bit. The codec frames and CRC-checks the `0x0564` data-link layer; the
  transport-function and application (objects/function codes) layer on top. See
  `src/services/dnp3/dnp3.h`.
- **C37.118** (`DETWS_ENABLE_C37118`): the IEEE C37.118.2 synchrophasor stream
  from a power-grid PMU/PDC. There is **no IANA-assigned port** (TCP/UDP 4712 and
  4713 are common conventions). Each PMU/PDC is identified by an **IDCODE**; a
  Command frame starts/stops the data stream. See `src/services/c37118/c37118.h`.

### OPC UA (server and client)

- **Flags:** `DETWS_ENABLE_OPCUA` (the ESP32 is the **server**, using this
  library's TCP listener) and/or `DETWS_ENABLE_OPCUA_CLIENT` (the ESP32 connects
  out to another server; your sketch owns the socket).
- **Hardware:** Wi-Fi/Ethernet.
- **Settings:** **TCP port 4840** (the default OPC UA Binary endpoint, URL
  `opc.tcp://<ip>:4840`). This library implements **SecurityPolicy `None`** (no
  message encryption), so configure the peer to allow an unencrypted endpoint, or
  put the link on a trusted/segmented network.
- **Codec:** see `src/services/opcua/` and `src/services/opcua_client/`.

### SNMP (agent and traps)

- **Flags:** `DETWS_ENABLE_SNMP` (agent), `DETWS_ENABLE_SNMP_V3` (USM
  authentication/privacy), `DETWS_ENABLE_SNMP_TRAP` (outbound traps/informs).
- **Hardware:** Wi-Fi/Ethernet to your monitoring system.
- **Settings:** the agent listens on **UDP port 161**; traps and informs go to
  the manager on **UDP port 162**. v1/v2c use a **community string** (a shared
  password, sent in clear), so prefer **v3** (USM user with authentication and
  privacy keys) on untrusted networks.
- **Codec:** see `src/services/snmp/`.

## Radio: ESP-NOW

- **Flag:** `DETWS_ENABLE_ESPNOW`.
- **Hardware:** **none beyond a second board.** ESP-NOW uses the ESP32's built-in
  2.4 GHz radio to talk directly to other ESP32/ESP8266 boards with no router or
  access point in between.
- **Settings:** peers must be on the **same Wi-Fi channel**; you address a peer by
  its **6-byte MAC address** and register it before sending. Optional payload
  encryption uses a 16-byte primary key (PMK) and per-peer local key (LMK).
  Messages are short (up to ~250 bytes), so larger data must be chunked.
- **Codec:** a typed envelope plus a peer registry; the send side binds to the
  ESP-IDF `esp_now` API. See `src/services/espnow/`.

## Payload codecs that ride an existing link

These format data but do **not** add a new physical connection: they ride
whatever transport already carries the data (MQTT, CoAP, HTTP), so their hardware
needs are simply that transport's.

- **SenML** (`DETWS_ENABLE_SENML`, implies CBOR): a standard JSON/CBOR shape for
  sensor measurements. The "hardware" is whatever sensor produces the readings;
  send the encoded payload over MQTT/CoAP/HTTP. See `src/services/senml/`.
- **Sparkplug B** (`DETWS_ENABLE_SPARKPLUG`, implies Protobuf): an MQTT payload
  and topic convention for SCADA. Needs an **MQTT broker** and a Sparkplug host
  (see the MQTT client setup); no new wiring. See `src/services/sparkplug/`.
- **LwM2M TLV** (`DETWS_ENABLE_LWM2M`): the device-management value encoding
  carried over **CoAP** (this library's CoAP/UDP service); no new wiring. See
  `src/services/lwm2m/`.
- **Protobuf, CBOR, MessagePack, gRPC-Web, NATS, STOMP, Redis, WAMP, AMQP,
  CloudEvents, flow export, Proxy protocol**: pure data/format codecs over Wi-Fi
  sockets or as building blocks; no special hardware beyond the network.

## Electrical safety and reliability

Field wiring lives in a harsher world than a breadboard. A few habits protect the
ESP32 and keep the bus reliable:

- **Match the voltage.** Use a 3.3 V transceiver, or add a level shifter. Never
  wire an RS-232/RS-485 line or a 5/12/24 V signal straight to a GPIO.
- **Share a ground.** Serial links need a common ground reference between the
  ESP32 and the device, in addition to the data wires.
- **Isolate when grounds differ.** If the ESP32 and the machine sit on different
  power systems (very common in industry), use an **isolated** RS-485 transceiver
  (for example the ADM2483/ADM2587) or a digital isolator. This breaks ground
  loops and protects against surges.
- **Terminate and bias the bus** (the 120 ohm and bias resistors above). Most
  "it works on the bench, fails on the long cable" problems are missing
  termination.
- **Protect the inputs.** On long outdoor or motor-heavy runs, add TVS diodes
  across the data lines for surge protection, and keep data cable away from power
  cable.
- **Power the transceiver properly.** A bus driver can draw more than a sensor;
  do not starve it off a weak 3.3 V rail.
- **One talker at a time.** On a 2-wire RS-485 bus, make sure the direction pin
  returns to receive promptly after every transmit, or two devices will collide.

For the security posture of each protocol (many industrial protocols have **no
authentication or encryption** and must run on a trusted network), see
[SECURITY.md](SECURITY.md). For the full description and spec reference of each
codec, see [FEATURES.md](FEATURES.md) and [STANDARDS.md](STANDARDS.md).
