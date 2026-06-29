# Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
# SPDX-License-Identifier: AGPL-3.0-or-later
"""MQTT interop: run the mosquitto reference broker for the device client.

Role: the *device is the client*; this peer is the broker (mosquitto, the
reference implementation) plus a paho monitor that subscribes to `#` and prints
everything the device publishes, and can publish back so the device's subscribe
path is exercised too. Requires mosquitto on PATH and the paho-mqtt package.
"""

from __future__ import annotations

import shutil
import subprocess
import sys
import time

from ._common import Probe, require

NAME = "mqtt-broker"
HELP = "run the mosquitto broker (+ paho monitor) for the device MQTT client"


def add_args(p) -> None:
    p.add_argument("--bind", default="0.0.0.0", help="listen address (default 0.0.0.0)")
    p.add_argument("--port", type=int, default=1883, help="MQTT port (default 1883)")
    p.add_argument("--topic", default="#", help="topic filter to monitor (default # = all)")
    p.add_argument("--publish", nargs=2, metavar=("TOPIC", "PAYLOAD"), help="publish once after start, then keep monitoring")
    p.add_argument("--seconds", type=int, default=0, help="auto-stop after N seconds (0 = run until Ctrl-C)")


def run(args) -> bool:
    mosquitto = shutil.which("mosquitto")
    pr = Probe(f"mqtt-broker :{args.port}")
    if not mosquitto:
        pr.check("mosquitto on PATH", False, "install mosquitto (apt install mosquitto / choco install mosquitto)")
        return pr.summary()
    mqtt = require("paho.mqtt.client")

    # Bind mosquitto to the requested interface/port with anonymous access (a lab rig).
    conf = f"listener {args.port} {args.bind}\nallow_anonymous true\n"
    proc = subprocess.Popen(
        [mosquitto, "-v", "-c", "/dev/stdin"] if sys.platform != "win32" else [mosquitto, "-v", "-p", str(args.port)],
        stdin=subprocess.PIPE if sys.platform != "win32" else None,
        text=True,
    )
    if sys.platform != "win32" and proc.stdin:
        proc.stdin.write(conf)
        proc.stdin.close()
    time.sleep(0.6)  # let the broker bind before the monitor connects
    pr.check("broker started", proc.poll() is None, f"pid {proc.pid}")

    client = mqtt.Client(client_id="interop-monitor")

    def on_connect(c, *_):
        c.subscribe(args.topic)
        print(f"mqtt-broker: monitoring '{args.topic}' (Ctrl-C to stop)")

    def on_message(c, u, msg):
        print(f"  device -> {msg.topic}: {msg.payload!r}")

    client.on_connect = on_connect
    client.on_message = on_message
    try:
        client.connect("127.0.0.1", args.port, keepalive=30)
        pr.check("monitor connected to broker", True)
        if args.publish:
            client.publish(args.publish[0], args.publish[1])
            print(f"mqtt-broker: published to {args.publish[0]}")
        client.loop_start()
        deadline = time.time() + args.seconds if args.seconds else None
        while proc.poll() is None and (deadline is None or time.time() < deadline):
            time.sleep(0.2)
    except KeyboardInterrupt:
        print("\nmqtt-broker: stopping")
    finally:
        client.loop_stop()
        proc.terminate()
        try:
            proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            proc.kill()
    return pr.summary()
