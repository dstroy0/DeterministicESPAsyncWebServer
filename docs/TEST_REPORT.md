# Test Report

**Generated:** 2026-07-03 17:38:32
**Command:** `pio test` over 118 auto-discovered native envs (excludes native_pentest, native_codeql)
**Result:** ❌ 1222 passed, 35 failed - 213s

---

## Summary

| Suite                    | Environment             | Tests | Status |     Duration |
| :----------------------- | :---------------------- | ----: | :----: | -----------: |
| `test_canopen`           | `native_canopen`        |    17 |   ✅   | 00:00:14.494 |
| `test_det_primitives`    | `native_det_primitives` |     5 |   ✅   | 00:00:00.656 |
| `test_det_ip`            | `native_det_ip`         |     9 |   ✅   | 00:00:00.674 |
| `test_promisc`           | `native_promisc`        |     8 |   ✅   | 00:00:00.664 |
| `test_bus_capture`       | `native_bus_capture`    |     5 |   ✅   | 00:00:00.669 |
| `test_j1939`             | `native_j1939`          |     9 |   ✅   | 00:00:00.672 |
| `test_devicenet`         | `native_devicenet`      |     8 |   ✅   | 00:00:00.675 |
| `test_nmea2000`          | `native_nmea2000`       |     6 |   ✅   | 00:00:00.694 |
| `test_mbus`              | `native_mbus`           |    11 |   ✅   | 00:00:00.659 |
| `test_iec60870`          | `native_iec60870`       |     8 |   ✅   | 00:00:00.696 |
| `test_sdi12`             | `native_sdi12`          |     6 |   ✅   | 00:00:00.668 |
| `test_dmx`               | `native_dmx`            |     5 |   ✅   | 00:00:00.694 |
| `test_nmea0183`          | `native_nmea0183`       |     7 |   ✅   | 00:00:00.666 |
| `test_iolink`            | `native_iolink`         |     5 |   ✅   | 00:00:00.668 |
| `test_http_ota`          | `native_ota`            |     3 |   ✅   | 00:00:00.693 |
| `test_provisioning`      | `native_prov`           |     5 |   ✅   | 00:00:00.705 |
| `test_ssh_crypto`        | `native_ssh`            |    39 |   ✅   | 00:00:03.785 |
| `test_ssh_auth`          | `native_ssh`            |    18 |   ✅   | 00:00:00.627 |
| `test_ssh_server`        | `native_ssh`            |    16 |   ✅   | 00:00:00.890 |
| `test_ssh_transport`     | `native_ssh`            |    27 |   ✅   | 00:00:01.124 |
| `test_ssh_channel`       | `native_ssh`            |    20 |   ✅   | 00:00:00.592 |
| `test_ssh_hardening`     | `native_ssh_hardened`   |     2 |   ✅   | 00:00:00.918 |
| `test_snmp_ber`          | `native_snmp`           |    16 |   ✅   | 00:00:00.741 |
| `test_snmp_agent`        | `native_snmp`           |    19 |   ✅   | 00:00:00.561 |
| `test_snmp_v3`           | `native_snmp_v3`        |    15 |   ✅   | 00:00:02.117 |
| `test_coap`              | `native_coap`           |    41 |   ✅   | 00:00:00.991 |
| `test_coap`              | `native_coap_observe`   |    41 |   ✅   | 00:00:00.794 |
| `test_webdav`            | `native_webdav`         |    19 |   ✅   | 00:00:00.701 |
| `test_modbus`            | `native_modbus`         |    22 |   ✅   | 00:00:00.692 |
| `test_cloudevents`       | `native_cloudevents`    |     7 |   ✅   | 00:00:00.744 |
| `test_redis_resp`        | `native_redis`          |     8 |   ✅   | 00:00:00.716 |
| `test_stomp`             | `native_stomp`          |    14 |   ✅   | 00:00:00.698 |
| `test_mqtt_sn`           | `native_mqtt_sn`        |    13 |   ✅   | 00:00:00.720 |
| `test_flow_export`       | `native_flow_export`    |     6 |   ✅   | 00:00:00.694 |
| `test_protobuf`          | `native_protobuf`       |    13 |   ✅   | 00:00:00.696 |
| `test_preempt_queue`     | `native_preempt_queue`  |    11 |   ✅   | 00:00:00.737 |
| `test_dma`               | `native_dma`            |    11 |   ✅   | 00:00:00.796 |
| `test_forward`           | `native_forward`        |    16 |   ✅   | 00:00:00.871 |
| `test_gateway`           | `native_gateway`        |    11 |   ✅   | 00:00:00.791 |
| `test_lora`              | `native_lora`           |    13 |   ✅   | 00:00:00.681 |
| `test_nrf24`             | `native_nrf24`          |    10 |   ✅   | 00:00:00.697 |
| `test_enocean`           | `native_enocean`        |     9 |   ✅   | 00:00:00.700 |
| `test_pn532`             | `native_pn532`          |    10 |   ✅   | 00:00:00.701 |
| `test_sigfox`            | `native_sigfox`         |     7 |   ✅   | 00:00:00.685 |
| `test_zwave`             | `native_zwave`          |     9 |   ✅   | 00:00:00.701 |
| `test_zigbee`            | `native_zigbee`         |     9 |   ✅   | 00:00:00.687 |
| `test_thread`            | `native_thread`         |    13 |   ✅   | 00:00:00.682 |
| `test_wamp`              | `native_wamp`           |    12 |   ✅   | 00:00:00.698 |
| `test_sunspec`           | `native_sunspec`        |     5 |   ✅   | 00:00:00.661 |
| `test_c37118`            | `native_c37118`         |     6 |   ✅   | 00:00:00.696 |
| `test_dnp3`              | `native_dnp3`           |     8 |   ✅   | 00:00:00.684 |
| `test_grpcweb`           | `native_grpcweb`        |     7 |   ✅   | 00:00:00.670 |
| `test_lwm2m_tlv`         | `native_lwm2m_tlv`      |    11 |   ✅   | 00:00:00.679 |
| `test_fins`              | `native_fins`           |     6 |   ✅   | 00:00:00.663 |
| `test_hostlink`          | `native_hostlink`       |     7 |   ✅   | 00:00:00.697 |
| `test_senml`             | `native_senml`          |     9 |   ✅   | 00:00:00.759 |
| `test_df1`               | `native_df1`            |     9 |   ✅   | 00:00:00.732 |
| `test_cotp`              | `native_cotp`           |     6 |   ✅   | 00:00:00.698 |
| `test_s7comm`            | `native_s7comm`         |     8 |   ✅   | 00:00:00.708 |
| `test_melsec`            | `native_melsec`         |     6 |   ✅   | 00:00:00.701 |
| `test_bacnet`            | `native_bacnet`         |     9 |   ✅   | 00:00:00.726 |
| `test_enip`              | `native_enip`           |     6 |   ✅   | 00:00:00.708 |
| `test_amqp`              | `native_amqp`           |     7 |   ✅   | 00:00:00.699 |
| `test_cip`               | `native_cip`            |     9 |   ✅   | 00:00:00.714 |
| `test_nats`              | `native_nats`           |    14 |   ✅   | 00:00:00.742 |
| `test_proxy_protocol`    | `native_proxy_protocol` |     8 |   ✅   | 00:00:00.712 |
| `test_sparkplug`         | `native_sparkplug`      |     7 |   ✅   | 00:00:00.716 |
| `test_modbus_master`     | `native_modbus_master`  |     5 |   ✅   | 00:00:00.720 |
| `test_ota_rollback`      | `native_ota_rollback`   |     5 |   ✅   | 00:00:00.706 |
| `test_totp`              | `native_totp`           |     4 |   ✅   | 00:00:00.746 |
| `test_webhook`           | `native_webhook`        |     5 |   ✅   | 00:00:00.713 |
| `test_radio_power`       | `native_radio_power`    |     2 |   ✅   | 00:00:00.678 |
| `test_dns_resolver`      | `native_dns_resolver`   |     4 |   ✅   | 00:00:00.685 |
| `test_audit_log`         | `native_audit_log`      |    16 |   ✅   | 00:00:00.713 |
| `test_oidc`              | `native_oidc`           |    17 |   ✅   | 00:00:00.944 |
| `test_vfs`               | `native_vfs`            |    11 |   ✅   | 00:00:00.729 |
| `test_graphql`           | `native_graphql`        |    32 |   ✅   | 00:00:00.689 |
| `test_espnow`            | `native_espnow`         |     7 |   ✅   | 00:00:00.705 |
| `test_oauth2`            | `native_oauth2`         |     8 |   ✅   | 00:00:00.711 |
| `test_opcua`             | `native_opcua`          |    38 |   ✅   | 00:00:00.713 |
| `test_opcua_client`      | `native_opcua_client`   |    14 |   ✅   | 00:00:00.727 |
| `test_syslog`            | `native_syslog`         |    10 |   ✅   | 00:00:00.716 |
| `test_jwt`               | `native_jwt`            |    16 |   ✅   | 00:00:00.730 |
| `test_http_client`       | `native_http_client`    |    15 |   ✅   | 00:00:00.690 |
| `test_compliance`        | `native_compliance`     |    15 |   ✅   | 00:00:00.698 |
| `test_mqtt`              | `native_mqtt`           |    22 |   ✅   | 00:00:00.676 |
| `test_ws_client`         | `native_ws_client`      |    16 |   ✅   | 00:00:00.731 |
| `test_scratch`           | `native_scratch`        |    14 |   ✅   | 00:00:00.713 |
| `test_snmp_trap`         | `native_snmp_trap`      |     7 |   ✅   | 00:00:00.720 |
| `test_inflate`           | `native_inflate`        |    12 |   ✅   | 00:00:00.681 |
| `test_deflate`           | `native_deflate`        |    10 |   ✅   | 00:00:00.731 |
| `test_time_source`       | `native_time_source`    |     9 |   ✅   | 00:00:00.676 |
| `test_config_store`      | `native_config_store`   |    14 |   ✅   | 00:00:00.690 |
| `test_device_id`         | `native_device_id`      |     4 |   ✅   | 00:00:00.685 |
| `test_auth_lockout`      | `native_auth_lockout`   |    10 |   ✅   | 00:00:00.681 |
| `test_csrf`              | `native_csrf`           |     9 |   ✅   | 00:00:00.741 |
| `test_telemetry`         | `native_telemetry`      |     8 |   ✅   | 00:00:00.687 |
| `test_dashboard`         | `native_dashboard`      |    15 |   ✅   | 00:00:00.697 |
| `test_net_egress`        | `native_net_egress`     |     6 |   ✅   | 00:00:00.683 |
| `test_partition_monitor` | `native_partition`      |     5 |   ✅   | 00:00:00.695 |
| `test_cbor`              | `native_cbor`           |    18 |   ✅   | 00:00:00.683 |
| `test_msgpack`           | `native_msgpack`        |    17 |   ✅   | 00:00:00.691 |
| `test_gpio_map`          | `native_gpio_map`       |     8 |   ✅   | 00:00:00.689 |
| `test_udp_telemetry`     | `native_udp_telemetry`  |     7 |   ✅   | 00:00:00.702 |
| `test_guardrails`        | `native_guardrails`     |     6 |   ✅   | 00:00:00.676 |
| `test_logbuf`            | `native_logbuf`         |     4 |   ✅   | 00:00:00.697 |
| `test_config_io`         | `native_config_io`      |     4 |   ✅   | 00:00:00.709 |
| `test_clock`             | `native_clock`          |     7 |   ✅   | 00:00:00.662 |
| `test_concurrency`       | `native_concurrency`    |     2 |   ✅   | 00:00:00.848 |
| `test_concurrency`       | `native_tsan`           |     2 |   ✅   | 00:00:01.219 |

---

## test_canopen - ✅ 17 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CANopen (CiA 301) message codec (services/canopen): NMT, SYNC,_

|   # | Test                                 | Status | Description                   |
| --: | :----------------------------------- | :----: | :---------------------------- |
|   1 | `test_nmt_start_node`                |   ✅   | Nmt start node                |
|   2 | `test_sync`                          |   ✅   | Sync                          |
|   3 | `test_heartbeat_roundtrip`           |   ✅   | Heartbeat roundtrip           |
|   4 | `test_emcy_roundtrip`                |   ✅   | Emcy roundtrip                |
|   5 | `test_pdo_roundtrip`                 |   ✅   | Pdo roundtrip                 |
|   6 | `test_sdo_read_request`              |   ✅   | Sdo read request              |
|   7 | `test_sdo_write_expedited`           |   ✅   | Sdo write expedited           |
|   8 | `test_sdo_upload_response_expedited` |   ✅   | Sdo upload response expedited |
|   9 | `test_sdo_abort_roundtrip`           |   ✅   | Sdo abort roundtrip           |
|  10 | `test_sdo_download_ack`              |   ✅   | Sdo download ack              |
|  11 | `test_parse_classifies`              |   ✅   | Parse classifies              |
|  12 | `test_build_arg_validation`          |   ✅   | Build arg validation          |
|  13 | `test_emcy_build_null_msef`          |   ✅   | Emcy build null msef          |
|  14 | `test_parse_all_function_codes`      |   ✅   | Parse all function codes      |
|  15 | `test_parse_emcy_rejections`         |   ✅   | Parse emcy rejections         |
|  16 | `test_parse_heartbeat_rejections`    |   ✅   | Parse heartbeat rejections    |
|  17 | `test_parse_sdo_response_variants`   |   ✅   | Parse sdo response variants   |

</details>

---

## test_det_primitives - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the shared no-stdlib primitives: the base-10 number parsers_

|   # | Test                | Status | Description  |
| --: | :------------------ | :----: | :----------- |
|   1 | `test_strtol`       |   ✅   | Strtol       |
|   2 | `test_strtoul`      |   ✅   | Strtoul      |
|   3 | `test_strtof`       |   ✅   | Strtof       |
|   4 | `test_utf8_valid`   |   ✅   | Utf8 valid   |
|   5 | `test_utf8_invalid` |   ✅   | Utf8 invalid |

</details>

---

## test_det_ip - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the DetIp address core (network_drivers/network/det_ip): RFC 4291 text_

|   # | Test                     | Status | Description                                                        |
| --: | :----------------------- | :----: | :----------------------------------------------------------------- |
|   1 | `test_v4_round_trip`     |   ✅   | V4 round trip                                                      |
|   2 | `test_from_v6_bytes`     |   ✅   | 2001:db8::1 as raw network-order bytes -> DetIp -> canonical text. |
|   3 | `test_ip_key`            |   ✅   | Ip key                                                             |
|   4 | `test_v6_canonical_5952` |   ✅   | RFC 5952: lower-case, no leading zeros, longest zero run -> "::".  |
|   5 | `test_v4_mapped`         |   ✅   | V4 mapped                                                          |
|   6 | `test_classify_v4`       |   ✅   | Classify v4                                                        |
|   7 | `test_classify_v6`       |   ✅   | Classify v6                                                        |
|   8 | `test_reject_malformed`  |   ✅   | Reject malformed                                                   |
|   9 | `test_equal_and_from_v4` |   ✅   | Equal and from v4                                                  |

</details>

---

## test_promisc - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Wi-Fi promiscuous capture helpers (services/promisc): the pure 802.11 MAC_

|   # | Test                       | Status | Description                                                                    |
| --: | :------------------------- | :----: | :----------------------------------------------------------------------------- |
|   1 | `test_beacon_mgmt`         |   ✅   | Mgmt (type 0), Beacon (subtype 8): fc0 = (8<<4)                                | (0<<2) = 0x80; no DS bits.                      |
|   2 | `test_data_from_ds`        |   ✅   | Data (type 2), from the AP: fc0 = (0<<4)                                       | (2<<2) = 0x08; from_ds = 0x02.                  |
|   3 | `test_data_to_ds`          |   ✅   | Data to the AP: to_ds = 0x01. a1 = BSSID, a2 = SRC, a3 = DST.                  |
|   4 | `test_qos_data_header_len` |   ✅   | QoS Data subtype 8: fc0 = (8<<4)                                               | (2<<2) = 0x88. Adds a 2-byte QoS Control field. |
|   5 | `test_wds_four_address`    |   ✅   | WDS: to_ds & from_ds set (fc1 = 0x03). Addr4 at offset 24; DST = a3, SRC = a4. |
|   6 | `test_control_frame`       |   ✅   | ACK (type 1, subtype 13): fc0 = (13<<4)                                        | (1<<2) = 0xD4. Only Addr1 (RA), 10-byte header. |
|   7 | `test_reject_short`        |   ✅   | Reject short                                                                   |
|   8 | `test_pcap_headers`        |   ✅   | Pcap headers                                                                   |

</details>

---

## test_bus_capture - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CAN listen-only capture framing (services/bus_capture): can_to_socketcan()_

|   # | Test                        | Status | Description          |
| --: | :-------------------------- | :----: | :------------------- |
|   1 | `test_standard_data_frame`  |   ✅   | Standard data frame  |
|   2 | `test_extended_id_sets_eff` |   ✅   | Extended id sets eff |
|   3 | `test_rtr_flag_and_no_data` |   ✅   | Rtr flag and no data |
|   4 | `test_masks_and_bounds`     |   ✅   | Masks and bounds     |
|   5 | `test_pcap_can_linktype`    |   ✅   | Pcap can linktype    |

</details>

---

## test_j1939 - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SAE J1939 codec (services/j1939): 29-bit id encode/decode (PDU1 + PDU2),_

|   # | Test                             | Status | Description               |
| --: | :------------------------------- | :----: | :------------------------ |
|   1 | `test_id_pdu2_roundtrip`         |   ✅   | Id pdu2 roundtrip         |
|   2 | `test_id_pdu1_roundtrip`         |   ✅   | Id pdu1 roundtrip         |
|   3 | `test_encode_rejects_bad_args`   |   ✅   | Encode rejects bad args   |
|   4 | `test_build_single_frame`        |   ✅   | Build single frame        |
|   5 | `test_request_pgn`               |   ✅   | Request pgn               |
|   6 | `test_address_claim_name`        |   ✅   | Address claim name        |
|   7 | `test_tp_num_packets`            |   ✅   | Tp num packets            |
|   8 | `test_tp_bam_roundtrip`          |   ✅   | Tp bam roundtrip          |
|   9 | `test_tp_out_of_sequence_errors` |   ✅   | Tp out of sequence errors |

</details>

---

## test_devicenet - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the DeviceNet link-adaptation codec (services/devicenet): the 4-group 11-bit_

|   # | Test                               | Status | Description                                                          |
| --: | :--------------------------------- | :----: | :------------------------------------------------------------------- |
|   1 | `test_id_group1`                   |   ✅   | Id group1                                                            |
|   2 | `test_id_group2`                   |   ✅   | Group 2: 10 MAC(6) MsgID(3). mac 0x21, unconnected explicit request. |
|   3 | `test_id_group3_and_4`             |   ✅   | Id group3 and 4                                                      |
|   4 | `test_header_and_frag_octets`      |   ✅   | Header and frag octets                                               |
|   5 | `test_build_explicit_single_frame` |   ✅   | Build explicit single frame                                          |
|   6 | `test_frag_non_fragmented`         |   ✅   | header octet with FRAG clear -> the body is complete in one frame.   |
|   7 | `test_frag_reassembly_roundtrip`   |   ✅   | Frag reassembly roundtrip                                            |
|   8 | `test_frag_out_of_order_errors`    |   ✅   | Frag out of order errors                                             |

</details>

---

## test_nmea2000 - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the NMEA 2000 codec (services/nmea2000): single-frame messages (J1939-based)_

|   # | Test                                           | Status | Description                             |
| --: | :--------------------------------------------- | :----: | :-------------------------------------- |
|   1 | `test_num_frames`                              |   ✅   | Num frames                              |
|   2 | `test_single_frame`                            |   ✅   | Single frame                            |
|   3 | `test_fastpacket_roundtrip`                    |   ✅   | Fastpacket roundtrip                    |
|   4 | `test_fastpacket_single_frame_completes`       |   ✅   | Fastpacket single frame completes       |
|   5 | `test_fastpacket_interleaved_sequence_ignored` |   ✅   | Fastpacket interleaved sequence ignored |
|   6 | `test_fastpacket_out_of_order_errors`          |   ✅   | Fastpacket out of order errors          |

</details>

---

## test_mbus - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the wired M-Bus codec (services/mbus): the ACK / short / long frame builders_

|   # | Test                            | Status | Description              |
| --: | :------------------------------ | :----: | :----------------------- |
|   1 | `test_ack`                      |   ✅   | Ack                      |
|   2 | `test_short_frame_roundtrip`    |   ✅   | Short frame roundtrip    |
|   3 | `test_req_ud2_fcb`              |   ✅   | Req ud2 fcb              |
|   4 | `test_long_frame_roundtrip`     |   ✅   | Long frame roundtrip     |
|   5 | `test_parse_rejects_corruption` |   ✅   | Parse rejects corruption |
|   6 | `test_dif_data_len`             |   ✅   | Dif data len             |
|   7 | `test_record_walk`              |   ✅   | Record walk              |
|   8 | `test_record_truncated_fails`   |   ✅   | Record truncated fails   |
|   9 | `test_build_and_parse_guards`   |   ✅   | Builder guards.          |
|  10 | `test_dif_data_len_remaining`   |   ✅   | Dif data len remaining   |
|  11 | `test_record_edges`             |   ✅   | Record edges             |

</details>

---

## test_iec60870 - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the IEC 60870-5-101/-104 codec (services/iec60870): the -104 APCI (I/S/U_

|   # | Test                                | Status | Description                  |
| --: | :---------------------------------- | :----: | :--------------------------- |
|   1 | `test_104_i_format_roundtrip`       |   ✅   | 104 i format roundtrip       |
|   2 | `test_104_s_format`                 |   ✅   | 104 s format                 |
|   3 | `test_104_u_format`                 |   ✅   | 104 u format                 |
|   4 | `test_104_sequence_numbers_15bit`   |   ✅   | 104 sequence numbers 15bit   |
|   5 | `test_asdu_header_roundtrip`        |   ✅   | Asdu header roundtrip        |
|   6 | `test_ioa_roundtrip`                |   ✅   | Ioa roundtrip                |
|   7 | `test_101_fixed_frame`              |   ✅   | 101 fixed frame              |
|   8 | `test_101_variable_frame_roundtrip` |   ✅   | 101 variable frame roundtrip |

</details>

---

## test_sdi12 - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SDI-12 codec (services/sdi12): the command builders, the measurement_

|   # | Test                                            | Status | Description                                                                          |
| --: | :---------------------------------------------- | :----: | :----------------------------------------------------------------------------------- |
|   1 | `test_command_builders`                         |   ✅   | Command builders                                                                     |
|   2 | `test_parse_measure_m`                          |   ✅   | aM! response "0" + "012" (12 s) + "2" (2 values).                                    |
|   3 | `test_parse_measure_concurrent_two_digit_count` |   ✅   | aC! response "0" + "013" (13 s) + "10" (10 values).                                  |
|   4 | `test_parse_values`                             |   ✅   | Parse values                                                                         |
|   5 | `test_crc_roundtrip`                            |   ✅   | Build a response, append the SDI-12 CRC, then verify it (and that corruption fails). |
|   6 | `test_crc_encode_printable`                     |   ✅   | Crc encode printable                                                                 |

</details>

---

## test_dmx - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the DMX512 + RDM codec (services/dmx): the DMX512 slot packet, and the RDM_

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_dmx_build_and_get`     |   ✅   | Dmx build and get     |
|   2 | `test_rdm_uid`               |   ✅   | Rdm uid               |
|   3 | `test_rdm_get_roundtrip`     |   ✅   | Rdm get roundtrip     |
|   4 | `test_rdm_set_with_data`     |   ✅   | Rdm set with data     |
|   5 | `test_rdm_parse_rejects_bad` |   ✅   | Rdm parse rejects bad |

</details>

---

## test_nmea0183 - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the NMEA 0183 codec (services/nmea0183): the XOR checksum, sentence build,_

|   # | Test                              | Status | Description               |
| --: | :-------------------------------- | :----: | :------------------------ |
|   1 | `test_checksum_known_vector`      |   ✅   | Checksum known vector     |
|   2 | `test_build`                      |   ✅   | Build                     |
|   3 | `test_parse_gga`                  |   ✅   | Parse gga                 |
|   4 | `test_field_helpers`              |   ✅   | Field helpers             |
|   5 | `test_parse_rejects_bad_checksum` |   ✅   | Flip the checksum digits. |
|   6 | `test_parse_rejects_no_dollar`    |   ✅   | Parse rejects no dollar   |
|   7 | `test_build_then_parse`           |   ✅   | Build then parse          |

</details>

---

## test_iolink - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the IO-Link (SDCI) data-link message codec (services/iolink): the MC / CKT /_

|   # | Test                                                  | Status | Description                                    |
| --: | :---------------------------------------------------- | :----: | :--------------------------------------------- |
|   1 | `test_mc_octet`                                       |   ✅   | read, Page channel, address 0x10 -> 0x80       | (1<<5) | 0x10 = 0xB0. |
|   2 | `test_ckt_cks_octets`                                 |   ✅   | Ckt cks octets                                 |
|   3 | `test_checksum_known_vector`                          |   ✅   | Checksum known vector                          |
|   4 | `test_finalize_preserves_type_and_detects_corruption` |   ✅   | Finalize preserves type and detects corruption |
|   5 | `test_device_reply_cks_roundtrip`                     |   ✅   | Device reply cks roundtrip                     |

</details>

---

## test_http_ota - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Tests the parser's streaming-body hook (DETWS_ENABLE_OTA): a body larger than_

|   # | Test                                    | Status | Description                      |
| --: | :-------------------------------------- | :----: | :------------------------------- |
|   1 | `test_large_body_streams_to_completion` |   ✅   | Large body streams to completion |
|   2 | `test_no_hooks_large_body_is_413`       |   ✅   | No hooks large body is 413       |
|   3 | `test_nonmatching_path_not_streamed`    |   ✅   | Nonmatching path not streamed    |

</details>

---

## test_provisioning - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for detws_prov_form_field(): the x-www-form-urlencoded field_

|   # | Test                      | Status | Description        |
| --: | :------------------------ | :----: | :----------------- |
|   1 | `test_plain_fields`       |   ✅   | Plain fields       |
|   2 | `test_url_decoding`       |   ✅   | Url decoding       |
|   3 | `test_missing_field`      |   ✅   | Missing field      |
|   4 | `test_no_substring_match` |   ✅   | No substring match |
|   5 | `test_capacity_bound`     |   ✅   | Capacity bound     |

</details>

---

## test_ssh_crypto - ✅ 39 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH crypto layer test suite._

|   # | Test                                         | Status | Description                                                        |
| --: | :------------------------------------------- | :----: | :----------------------------------------------------------------- |
|   1 | `test_sha256_empty`                          |   ✅   | SHA256("") = e3b0c44298fc1c149afb...                               |
|   2 | `test_sha256_abc`                            |   ✅   | SHA256("abc") = ba7816bf8f01cfea414140de5dae2ec73b00361bbef0469... |
|   3 | `test_sha256_448bit`                         |   ✅   | SHA256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") |
|   4 | `test_sha256_streaming`                      |   ✅   | Same as test_sha256_abc but using the streaming API.               |
|   5 | `test_hmac_sha256_tc1`                       |   ✅   | RFC 4231 Test Case 1                                               |
|   6 | `test_hmac_sha256_tc2`                       |   ✅   | RFC 4231 Test Case 2                                               |
|   7 | `test_hmac_sha256_tc3`                       |   ✅   | RFC 4231 Test Case 3                                               |
|   8 | `test_hmac_sha256_streaming`                 |   ✅   | Same as tc1 but via streaming API.                                 |
|   9 | `test_aes256ctr_encrypt`                     |   ✅   | NIST SP 800-38A, Section F.5.5                                     |
|  10 | `test_aes256ctr_decrypt`                     |   ✅   | AES-256-CTR decrypt is identical to encrypt.                       |
|  11 | `test_aes256ctr_multi_block`                 |   ✅   | NIST F.5.5 blocks 1-4 (64 bytes).                                  |
|  12 | `test_aes256ctr_wipe`                        |   ✅   | After wipe, the context should be all zeros.                       |
|  13 | `test_bn_roundtrip`                          |   ✅   | Round-trip: bytes → SshBigNum → bytes.                             |
|  14 | `test_bn_cmp_equal`                          |   ✅   | Bn cmp equal                                                       |
|  15 | `test_bn_cmp_less`                           |   ✅   | Bn cmp less                                                        |
|  16 | `test_bn_cmp_greater`                        |   ✅   | Bn cmp greater                                                     |
|  17 | `test_bn_is_zero`                            |   ✅   | Bn is zero                                                         |
|  18 | `test_bn_dh_validate_rejects_zero`           |   ✅   | Bn dh validate rejects zero                                        |
|  19 | `test_bn_dh_validate_rejects_one`            |   ✅   | Bn dh validate rejects one                                         |
|  20 | `test_bn_dh_validate_accepts_two`            |   ✅   | Bn dh validate accepts two                                         |
|  21 | `test_expmod_exp1`                           |   ✅   | Expmod exp1                                                        |
|  22 | `test_expmod_exp2`                           |   ✅   | Expmod exp2                                                        |
|  23 | `test_expmod_exp3`                           |   ✅   | Expmod exp3                                                        |
|  24 | `test_expmod_commutative`                    |   ✅   | Expmod commutative                                                 |
|  25 | `test_rsa_pkcs1_pad_structure`               |   ✅   | With d=1, sign(msg) = m^1 mod n = m (the padded message itself).   |
|  26 | `test_rsa_sign_verify_roundtrip`             |   ✅   | Install the real keypair into the native sign fixture.             |
|  27 | `test_rsa_encode_pubkey`                     |   ✅   | Rsa encode pubkey                                                  |
|  28 | `test_rsa_verify_valid_signature`            |   ✅   | Rsa verify valid signature                                         |
|  29 | `test_rsa_verify_rejects_tampered_signature` |   ✅   | Rsa verify rejects tampered signature                              |
|  30 | `test_rsa_verify_rejects_wrong_message`      |   ✅   | Rsa verify rejects wrong message                                   |
|  31 | `test_pkt_send_recv_unencrypted`             |   ✅   | Pkt send recv unencrypted                                          |
|  32 | `test_pkt_padding_alignment`                 |   ✅   | Packet length + padding must be multiple of 16.                    |
|  33 | `test_pkt_seq_increments`                    |   ✅   | Pkt seq increments                                                 |
|  34 | `test_pkt_disconnect_zeroes_state`           |   ✅   | Pkt disconnect zeroes state                                        |
|  35 | `test_pkt_encrypted_roundtrip`               |   ✅   | Pkt encrypted roundtrip                                            |
|  36 | `test_pkt_encrypted_fragmented`              |   ✅   | Pkt encrypted fragmented                                           |
|  37 | `test_pkt_encrypted_two_packets`             |   ✅   | Pkt encrypted two packets                                          |
|  38 | `test_ssh_kdf_canonical_mpint_k`             |   ✅   | Ssh kdf canonical mpint k                                          |
|  39 | `test_ssh_kdf_extension_chain`               |   ✅   | Ssh kdf extension chain                                            |

</details>

---

## test_ssh_auth - ✅ 18 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH user-authentication tests (RFC 4252): service request/accept, request_

|   # | Test                                          | Status | Description                                       |
| --: | :-------------------------------------------- | :----: | :------------------------------------------------ |
|   1 | `test_service_request_errors`                 |   ✅   | Service request errors                            |
|   2 | `test_build_response_guards`                  |   ✅   | Build response guards                             |
|   3 | `test_parse_request_truncations`              |   ✅   | Parse request truncations                         |
|   4 | `test_pubkey_blob_parse_failures`             |   ✅   | Pubkey blob parse failures                        |
|   5 | `test_pubkey_oversized_signed_prefix`         |   ✅   | Pubkey oversized signed prefix                    |
|   6 | `test_handle_request_index_and_parse_guards`  |   ✅   | Handle request index and parse guards             |
|   7 | `test_service_request_accept`                 |   ✅   | Service request accept                            |
|   8 | `test_service_request_rejects_unknown`        |   ✅   | Service request rejects unknown                   |
|   9 | `test_parse_password_request`                 |   ✅   | Parse password request                            |
|  10 | `test_parse_none_request`                     |   ✅   | Parse none request                                |
|  11 | `test_handle_request_success`                 |   ✅   | Handle request success                            |
|  12 | `test_handle_request_wrong_password_fails`    |   ✅   | Handle request wrong password fails               |
|  13 | `test_handle_none_request_fails_without_auth` |   ✅   | Handle none request fails without auth            |
|  14 | `test_handle_request_no_callback_fails`       |   ✅   | No callback installed → all credentials rejected. |
|  15 | `test_pubkey_probe_returns_pk_ok`             |   ✅   | Pubkey probe returns pk ok                        |
|  16 | `test_pubkey_valid_signature_succeeds`        |   ✅   | Pubkey valid signature succeeds                   |
|  17 | `test_pubkey_tampered_signature_fails`        |   ✅   | Pubkey tampered signature fails                   |
|  18 | `test_pubkey_unauthorized_key_fails`          |   ✅   | Pubkey unauthorized key fails                     |

</details>

---

## test_ssh_server - ✅ 16 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_End-to-end SSH server dispatcher test: drives a full handshake_

|   # | Test                                                 | Status | Description                                                           |
| --: | :--------------------------------------------------- | :----: | :-------------------------------------------------------------------- |
|   1 | `test_ssh_pkt_index_and_cap_guards`                  |   ✅   | Ssh pkt index and cap guards                                          |
|   2 | `test_ssh_pkt_recv_unencrypted_errors`               |   ✅   | Ssh pkt recv unencrypted errors                                       |
|   3 | `test_ssh_pkt_seq_overflow_guards`                   |   ✅   | Ssh pkt seq overflow guards                                           |
|   4 | `test_ssh_pkt_encrypted_roundtrip_and_mac_fail`      |   ✅   | Ssh pkt encrypted roundtrip and mac fail                              |
|   5 | `test_full_handshake_to_channel_data`                |   ✅   | Banner exchange already done out-of-band; seed V_C and enter KEXINIT. |
|   6 | `test_extinfo_build_advertises_server_sig_algs`      |   ✅   | Extinfo build advertises server sig algs                              |
|   7 | `test_extinfo_not_sent_without_ext_info_c`           |   ✅   | Extinfo not sent without ext info c                                   |
|   8 | `test_inbound_ext_info_ignored`                      |   ✅   | Inbound ext info ignored                                              |
|   9 | `test_large_client_kexinit_accepted`                 |   ✅   | Large client kexinit accepted                                         |
|  10 | `test_channel_open_before_auth_rejected`             |   ✅   | Channel open before auth rejected                                     |
|  11 | `test_disconnect_closes`                             |   ✅   | Disconnect closes                                                     |
|  12 | `test_ignore_is_noop`                                |   ✅   | Ignore is noop                                                        |
|  13 | `test_auth_bruteforce_disconnect`                    |   ✅   | The first SSH_MAX_AUTH_ATTEMPTS-1 failures keep the connection open.  |
|  14 | `test_auth_success_after_failures`                   |   ✅   | Auth success after failures                                           |
|  15 | `test_unimplemented_reply_for_unknown_message`       |   ✅   | Unimplemented reply for unknown message                               |
|  16 | `test_inbound_close_emits_eof_then_close_separately` |   ✅   | Open a channel so the close path has something to close (peer id 21). |

</details>

---

## test_ssh_transport - ✅ 27 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH transport handshake tests (RFC 4253): identification-string exchange and_

|   # | Test                                                       | Status | Description                                                      |
| --: | :--------------------------------------------------------- | :----: | :--------------------------------------------------------------- |
|   1 | `test_transport_index_guards`                              |   ✅   | Transport index guards                                           |
|   2 | `test_banner_and_build_caps`                               |   ✅   | Banner and build caps                                            |
|   3 | `test_kexinit_parse_field_and_trunc`                       |   ✅   | Kexinit parse field and trunc                                    |
|   4 | `test_kexdh_parse_and_handle_errors`                       |   ✅   | Kexdh parse and handle errors                                    |
|   5 | `test_server_banner_format`                                |   ✅   | Server banner format                                             |
|   6 | `test_recv_banner_complete`                                |   ✅   | Recv banner complete                                             |
|   7 | `test_recv_banner_bare_lf`                                 |   ✅   | Recv banner bare lf                                              |
|   8 | `test_recv_banner_split_across_reads`                      |   ✅   | Recv banner split across reads                                   |
|   9 | `test_recv_banner_skips_preamble_lines`                    |   ✅   | RFC 4253 §4.2 allows lines before the SSH identification string. |
|  10 | `test_kexinit_build_starts_with_msg_and_stores_is`         |   ✅   | Kexinit build starts with msg and stores is                      |
|  11 | `test_kexinit_parse_accepts_supported`                     |   ✅   | Kexinit parse accepts supported                                  |
|  12 | `test_kexinit_parse_accepts_when_ours_listed_among_others` |   ✅   | Kexinit parse accepts when ours listed among others              |
|  13 | `test_kexinit_parse_rejects_missing_kex`                   |   ✅   | Kexinit parse rejects missing kex                                |
|  14 | `test_kexinit_parse_rejects_missing_cipher`                |   ✅   | Kexinit parse rejects missing cipher                             |
|  15 | `test_kexinit_parse_rejects_truncated`                     |   ✅   | Kexinit parse rejects truncated                                  |
|  16 | `test_exchange_hash_matches_independent_assembly`          |   ✅   | Populate the session fields the hash reads.                      |
|  17 | `test_exchange_hash_changes_with_input`                    |   ✅   | Exchange hash changes with input                                 |
|  18 | `test_kexdh_parse_init_extracts_e_with_padding`            |   ✅   | Kexdh parse init extracts e with padding                         |
|  19 | `test_kexdh_parse_init_extracts_small_e`                   |   ✅   | Kexdh parse init extracts small e                                |
|  20 | `test_kexdh_parse_init_rejects_wrong_type`                 |   ✅   | Kexdh parse init rejects wrong type                              |
|  21 | `test_kexdh_parse_init_rejects_oversized_e`                |   ✅   | mpint with 300 magnitude bytes → exceeds 2048 bits.              |
|  22 | `test_kexdh_build_reply_structure`                         |   ✅   | Kexdh build reply structure                                      |
|  23 | `test_kexdh_handle_produces_reply_and_installs_keys`       |   ✅   | Kexdh handle produces reply and installs keys                    |
|  24 | `test_kexdh_handle_rejects_invalid_e`                      |   ✅   | Kexdh handle rejects invalid e                                   |
|  25 | `test_derive_keys_session_id_affects_output`               |   ✅   | Derive keys session id affects output                            |
|  26 | `test_rekey_needed_threshold`                              |   ✅   | Rekey needed threshold                                           |
|  27 | `test_begin_rekey_preserves_session_and_auth`              |   ✅   | Begin rekey preserves session and auth                           |

</details>

---

## test_ssh_channel - ✅ 20 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH connection-protocol (channel) tests - RFC 4254, including multiplexing_

|   # | Test                                                | Status | Description                                                          |
| --: | :-------------------------------------------------- | :----: | :------------------------------------------------------------------- |
|   1 | `test_open_session_confirms`                        |   ✅   | Open session confirms                                                |
|   2 | `test_open_unknown_type_fails`                      |   ✅   | Open unknown type fails                                              |
|   3 | `test_direct_tcpip_no_cb_prohibited`                |   ✅   | Forwarding is opt-in: with no open callback installed it is refused. |
|   4 | `test_direct_tcpip_accept_confirms`                 |   ✅   | Direct tcpip accept confirms                                         |
|   5 | `test_direct_tcpip_refused_connect_failed`          |   ✅   | Direct tcpip refused connect failed                                  |
|   6 | `test_forward_data_routes_to_forward_cb`            |   ✅   | Forward data routes to forward cb                                    |
|   7 | `test_shell_request_success_with_reply`             |   ✅   | Shell request success with reply                                     |
|   8 | `test_unknown_request_failure`                      |   ✅   | Unknown request failure                                              |
|   9 | `test_request_no_reply_produces_nothing`            |   ✅   | Request no reply produces nothing                                    |
|  10 | `test_inbound_data_invokes_callback`                |   ✅   | Inbound data invokes callback                                        |
|  11 | `test_inbound_data_window_replenish`                |   ✅   | Inbound data window replenish                                        |
|  12 | `test_inbound_data_exceeding_window_rejected`       |   ✅   | Inbound data exceeding window rejected                               |
|  13 | `test_outbound_data_frames_and_decrements_window`   |   ✅   | Outbound data frames and decrements window                           |
|  14 | `test_outbound_data_exceeding_peer_window_rejected` |   ✅   | Outbound data exceeding peer window rejected                         |
|  15 | `test_window_adjust_grows_peer_window`              |   ✅   | Window adjust grows peer window                                      |
|  16 | `test_build_close_emits_eof_and_close`              |   ✅   | Build close emits eof and close                                      |
|  17 | `test_inbound_close_routes_to_channel`              |   ✅   | Inbound close routes to channel                                      |
|  18 | `test_multiplex_two_channels_route_independently`   |   ✅   | Multiplex two channels route independently                           |
|  19 | `test_pool_full_open_fails`                         |   ✅   | Pool full open fails                                                 |
|  20 | `test_data_to_unknown_channel_rejected`             |   ✅   | Data to unknown channel rejected                                     |

</details>

---

## test_ssh_hardening - ✅ 2 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Built with DETWS_SSH_ALLOW_PASSWORD=0: verifies password authentication is_

|   # | Test                                               | Status | Description                                                            |
| --: | :------------------------------------------------- | :----: | :--------------------------------------------------------------------- |
|   1 | `test_password_refused_even_with_correct_callback` |   ✅   | Even a callback that accepts everything must not authenticate, because |
|   2 | `test_failure_advertises_publickey_only`           |   ✅   | Failure advertises publickey only                                      |

</details>

---

## test_snmp_ber - ✅ 16 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SNMP ASN.1 BER codec. Encodings are checked against_

|   # | Test                                                     | Status | Description                                                                 |
| --: | :------------------------------------------------------- | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_integer_vectors`                                   |   ✅   | Integer vectors                                                             |
|   2 | `test_oid_vector`                                        |   ✅   | 1.3.6.1 -> 06 03 2B 06 01                                                   |
|   3 | `test_octet_string_and_null`                             |   ✅   | Octet string and null                                                       |
|   4 | `test_counter32_keeps_unsigned`                          |   ✅   | 0x80000000 has the top bit set -> a leading 0x00 must be added.             |
|   5 | `test_sequence_roundtrip`                                |   ✅   | Sequence roundtrip                                                          |
|   6 | `test_oid_roundtrip`                                     |   ✅   | Oid roundtrip                                                               |
|   7 | `test_large_arc_roundtrip`                               |   ✅   | An arc > 127 exercises multi-byte base-128 encoding (e.g. enterprise 8072). |
|   8 | `test_oid_large_first_subidentifier_roundtrip`           |   ✅   | Oid large first subidentifier roundtrip                                     |
|   9 | `test_encoder_overflow_sets_not_ok`                      |   ✅   | Encoder overflow sets not ok                                                |
|  10 | `test_decoder_truncated_length_fails`                    |   ✅   | Claims 10 bytes of content but only 2 are present.                          |
|  11 | `test_decoder_longform_length_count_past_buffer_fails`   |   ✅   | Decoder longform length count past buffer fails                             |
|  12 | `test_decoder_longform_length_too_wide_fails`            |   ✅   | Decoder longform length too wide fails                                      |
|  13 | `test_decoder_longform_length_content_past_buffer_fails` |   ✅   | 0x82 0x01 0x00 = long form, length 256; only a few content bytes follow.    |
|  14 | `test_decoder_longform_length_max_uint32_fails`          |   ✅   | Decoder longform length max uint32 fails                                    |
|  15 | `test_decoder_indefinite_length_fails`                   |   ✅   | Decoder indefinite length fails                                             |
|  16 | `test_decoder_oversized_integer_fails`                   |   ✅   | Decoder oversized integer fails                                             |

</details>

---

## test_snmp_agent - ✅ 19 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SNMP v1/v2c agent core (snmp_agent_process). Each test_

|   # | Test                                        | Status | Description                                                                     |
| --: | :------------------------------------------ | :----: | :------------------------------------------------------------------------------ |
|   1 | `test_registration_and_rw_edges`            |   ✅   | Registration and rw edges                                                       |
|   2 | `test_ipaddress_value_encodes`              |   ✅   | Ipaddress value encodes                                                         |
|   3 | `test_set_wrong_type_and_unknown`           |   ✅   | Set wrong type and unknown                                                      |
|   4 | `test_getbulk_variants`                     |   ✅   | non-repeaters = 1, max-repetitions = 2, one varbind at the system prefix.       |
|   5 | `test_dispatch_value_types_and_malformed`   |   ✅   | uint-typed and OID-typed varbind values decode without error.                   |
|   6 | `test_get_string_v2c`                       |   ✅   | Get string v2c                                                                  |
|   7 | `test_get_unknown_v2c_exception`            |   ✅   | Get unknown v2c exception                                                       |
|   8 | `test_get_bad_instance_v2c_nosuchinstance`  |   ✅   | Get bad instance v2c nosuchinstance                                             |
|   9 | `test_get_unknown_v1_error`                 |   ✅   | Get unknown v1 error                                                            |
|  10 | `test_getnext_walks_to_first`               |   ✅   | Getnext walks to first                                                          |
|  11 | `test_getnext_past_end_endofmibview`        |   ✅   | Getnext past end endofmibview                                                   |
|  12 | `test_set_without_rw_community_denied`      |   ✅   | Set without rw community denied                                                 |
|  13 | `test_set_with_rw_community_invokes_setter` |   ✅   | Set with rw community invokes setter                                            |
|  14 | `test_set_readonly_not_writable`            |   ✅   | Set readonly not writable                                                       |
|  15 | `test_getbulk_returns_multiple`             |   ✅   | non-repeaters=0, max-repetitions=3, one repeater starting at the system prefix. |
|  16 | `test_dynamic_counter_value`                |   ✅   | Dynamic counter value                                                           |
|  17 | `test_uptime_is_timeticks`                  |   ✅   | Uptime is timeticks                                                             |
|  18 | `test_unknown_community_no_response`        |   ✅   | Unknown community no response                                                   |
|  19 | `test_v3_message_dropped`                   |   ✅   | V3 message dropped                                                              |

</details>

---

## test_snmp_v3 - ✅ 15 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SNMPv3 USM layer. The test acts as a full SNMP manager:_

|   # | Test                                            | Status | Description                                                                     |
| --: | :---------------------------------------------- | :----: | :------------------------------------------------------------------------------ |
|   1 | `test_v3_message_structure_rejections`          |   ✅   | V3 message structure rejections                                                 |
|   2 | `test_v3_init_and_boots_accessors`              |   ✅   | V3 init and boots accessors                                                     |
|   3 | `test_v3_discovery_variants`                    |   ✅   | V3 discovery variants                                                           |
|   4 | `test_v3_priv_not_configured`                   |   ✅   | V3 priv not configured                                                          |
|   5 | `test_v3_notify_paths`                          |   ✅   | V3 notify paths                                                                 |
|   6 | `test_localize_key_sha256_vector`               |   ✅   | password "maplesyrup", engineID 80 00 C0 DE 05 01 02 03 04 (the agent default). |
|   7 | `test_aes128_fips197_vector`                    |   ✅   | FIPS-197 C.1. CFB with IV = plaintext and zero input yields E_key(plaintext).   |
|   8 | `test_aes_cfb_roundtrip_partial_block`          |   ✅   | Aes cfb roundtrip partial block                                                 |
|   9 | `test_discovery_reports_engine_id`              |   ✅   | Discovery reports engine id                                                     |
|  10 | `test_authnopriv_get`                           |   ✅   | Authnopriv get                                                                  |
|  11 | `test_authpriv_get`                             |   ✅   | Authpriv get                                                                    |
|  12 | `test_wrong_auth_password_reports_wrong_digest` |   ✅   | Wrong auth password reports wrong digest                                        |
|  13 | `test_unknown_user_reports`                     |   ✅   | Unknown user reports                                                            |
|  14 | `test_not_in_time_window_reports`               |   ✅   | Not in time window reports                                                      |
|  15 | `test_inform_v3_builds_informrequest`           |   ✅   | Inform v3 builds informrequest                                                  |

</details>

---

## test_coap - ✅ 41 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CoAP server core (coap_server_process). Each test encodes a_

|   # | Test                                       | Status | Description                                                                 |
| --: | :----------------------------------------- | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_add_resource_limits`                 |   ✅   | Add resource limits                                                         |
|   2 | `test_short_and_truncated_token`           |   ✅   | Short and truncated token                                                   |
|   3 | `test_malformed_options_bad_request`       |   ✅   | Malformed options bad request                                               |
|   4 | `test_extended_delta_and_length_ignored`   |   ✅   | Extended delta and length ignored                                           |
|   5 | `test_oversized_path_and_query`            |   ✅   | Oversized path and query                                                    |
|   6 | `test_block_option_too_wide`               |   ✅   | Block option too wide                                                       |
|   7 | `test_block1_reserved_szx`                 |   ✅   | Block1 reserved szx                                                         |
|   8 | `test_block1_continue_no_space`            |   ✅   | Block1 continue no space                                                    |
|   9 | `test_response_payload_clamped`            |   ✅   | Response payload clamped                                                    |
|  10 | `test_response_buffer_too_small`           |   ✅   | Response buffer too small                                                   |
|  11 | `test_well_known_core_truncates`           |   ✅   | Well known core truncates                                                   |
|  12 | `test_observe_large_seq_encoding`          |   ✅   | Observe large seq encoding                                                  |
|  13 | `test_block2_explicit_paging`              |   ✅   | Block2 explicit paging                                                      |
|  14 | `test_block2_auto_when_large`              |   ✅   | Block2 auto when large                                                      |
|  15 | `test_block2_szx_clamped`                  |   ✅   | Block2 szx clamped                                                          |
|  16 | `test_block2_absent_for_small`             |   ✅   | Block2 absent for small                                                     |
|  17 | `test_block2_out_of_range`                 |   ✅   | Block2 out of range                                                         |
|  18 | `test_block2_reserved_szx`                 |   ✅   | Block2 reserved szx                                                         |
|  19 | `test_block1_upload_two_blocks`            |   ✅   | Block1 upload two blocks                                                    |
|  20 | `test_block1_out_of_order`                 |   ✅   | Block1 out of order                                                         |
|  21 | `test_block1_too_large`                    |   ✅   | Block1 too large                                                            |
|  22 | `test_observe_option_in_response`          |   ✅   | Observe option in response                                                  |
|  23 | `test_no_observe_option_when_seq_negative` |   ✅   | No observe option when seq negative                                         |
|  24 | `test_get_content`                         |   ✅   | Get content                                                                 |
|  25 | `test_not_found`                           |   ✅   | Not found                                                                   |
|  26 | `test_method_not_allowed`                  |   ✅   | Method not allowed                                                          |
|  27 | `test_non_request_type`                    |   ✅   | Non request type                                                            |
|  28 | `test_put_with_payload`                    |   ✅   | Put with payload                                                            |
|  29 | `test_multi_segment_path`                  |   ✅   | Multi segment path                                                          |
|  30 | `test_uri_query`                           |   ✅   | Uri query                                                                   |
|  31 | `test_empty_con_ping_rst`                  |   ✅   | Empty con ping rst                                                          |
|  32 | `test_bad_version_rst`                     |   ✅   | Bad version rst                                                             |
|  33 | `test_delete`                              |   ✅   | Delete                                                                      |
|  34 | `test_token_8_bytes`                       |   ✅   | Token 8 bytes                                                               |
|  35 | `test_extended_option_length`              |   ✅   | Extended option length                                                      |
|  36 | `test_ack_ignored`                         |   ✅   | Ack ignored                                                                 |
|  37 | `test_root_path`                           |   ✅   | Root path                                                                   |
|  38 | `test_unknown_method_not_allowed`          |   ✅   | Code 0.05 (FETCH) is a valid class-0 code we don't implement. RFC 7252 5.8: |
|  39 | `test_unknown_critical_option_bad_option`  |   ✅   | Hand-build: ver1/CON/TKL0, GET, MID, Uri-Path "temp", then Accept(17) - a   |
|  40 | `test_well_known_core_discovery`           |   ✅   | Well known core discovery                                                   |
|  41 | `test_well_known_core_rejects_post`        |   ✅   | Well known core rejects post                                                |

</details>

---

## test_coap - ✅ 41 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CoAP server core (coap_server_process). Each test encodes a_

|   # | Test                                       | Status | Description                                                                 |
| --: | :----------------------------------------- | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_add_resource_limits`                 |   ✅   | Add resource limits                                                         |
|   2 | `test_short_and_truncated_token`           |   ✅   | Short and truncated token                                                   |
|   3 | `test_malformed_options_bad_request`       |   ✅   | Malformed options bad request                                               |
|   4 | `test_extended_delta_and_length_ignored`   |   ✅   | Extended delta and length ignored                                           |
|   5 | `test_oversized_path_and_query`            |   ✅   | Oversized path and query                                                    |
|   6 | `test_block_option_too_wide`               |   ✅   | Block option too wide                                                       |
|   7 | `test_block1_reserved_szx`                 |   ✅   | Block1 reserved szx                                                         |
|   8 | `test_block1_continue_no_space`            |   ✅   | Block1 continue no space                                                    |
|   9 | `test_response_payload_clamped`            |   ✅   | Response payload clamped                                                    |
|  10 | `test_response_buffer_too_small`           |   ✅   | Response buffer too small                                                   |
|  11 | `test_well_known_core_truncates`           |   ✅   | Well known core truncates                                                   |
|  12 | `test_observe_large_seq_encoding`          |   ✅   | Observe large seq encoding                                                  |
|  13 | `test_block2_explicit_paging`              |   ✅   | Block2 explicit paging                                                      |
|  14 | `test_block2_auto_when_large`              |   ✅   | Block2 auto when large                                                      |
|  15 | `test_block2_szx_clamped`                  |   ✅   | Block2 szx clamped                                                          |
|  16 | `test_block2_absent_for_small`             |   ✅   | Block2 absent for small                                                     |
|  17 | `test_block2_out_of_range`                 |   ✅   | Block2 out of range                                                         |
|  18 | `test_block2_reserved_szx`                 |   ✅   | Block2 reserved szx                                                         |
|  19 | `test_block1_upload_two_blocks`            |   ✅   | Block1 upload two blocks                                                    |
|  20 | `test_block1_out_of_order`                 |   ✅   | Block1 out of order                                                         |
|  21 | `test_block1_too_large`                    |   ✅   | Block1 too large                                                            |
|  22 | `test_observe_option_in_response`          |   ✅   | Observe option in response                                                  |
|  23 | `test_no_observe_option_when_seq_negative` |   ✅   | No observe option when seq negative                                         |
|  24 | `test_get_content`                         |   ✅   | Get content                                                                 |
|  25 | `test_not_found`                           |   ✅   | Not found                                                                   |
|  26 | `test_method_not_allowed`                  |   ✅   | Method not allowed                                                          |
|  27 | `test_non_request_type`                    |   ✅   | Non request type                                                            |
|  28 | `test_put_with_payload`                    |   ✅   | Put with payload                                                            |
|  29 | `test_multi_segment_path`                  |   ✅   | Multi segment path                                                          |
|  30 | `test_uri_query`                           |   ✅   | Uri query                                                                   |
|  31 | `test_empty_con_ping_rst`                  |   ✅   | Empty con ping rst                                                          |
|  32 | `test_bad_version_rst`                     |   ✅   | Bad version rst                                                             |
|  33 | `test_delete`                              |   ✅   | Delete                                                                      |
|  34 | `test_token_8_bytes`                       |   ✅   | Token 8 bytes                                                               |
|  35 | `test_extended_option_length`              |   ✅   | Extended option length                                                      |
|  36 | `test_ack_ignored`                         |   ✅   | Ack ignored                                                                 |
|  37 | `test_root_path`                           |   ✅   | Root path                                                                   |
|  38 | `test_unknown_method_not_allowed`          |   ✅   | Code 0.05 (FETCH) is a valid class-0 code we don't implement. RFC 7252 5.8: |
|  39 | `test_unknown_critical_option_bad_option`  |   ✅   | Hand-build: ver1/CON/TKL0, GET, MID, Uri-Path "temp", then Accept(17) - a   |
|  40 | `test_well_known_core_discovery`           |   ✅   | Well known core discovery                                                   |
|  41 | `test_well_known_core_rejects_post`        |   ✅   | Well known core rejects post                                                |

</details>

---

## test_webdav - ✅ 19 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the WebDAV server core (services/webdav): method classification,_

|   # | Test                                      | Status | Description                                                                |
| --: | :---------------------------------------- | :----: | :------------------------------------------------------------------------- |
|   1 | `test_method_classification`              |   ✅   | Method classification                                                      |
|   2 | `test_depth_parsing`                      |   ✅   | Depth parsing                                                              |
|   3 | `test_xml_escape`                         |   ✅   | Xml escape                                                                 |
|   4 | `test_xml_escape_truncates_safely`        |   ✅   | Xml escape truncates safely                                                |
|   5 | `test_dest_absolute_uri`                  |   ✅   | Dest absolute uri                                                          |
|   6 | `test_dest_percent_decoded`               |   ✅   | Dest percent decoded                                                       |
|   7 | `test_dest_abs_path`                      |   ✅   | Dest abs path                                                              |
|   8 | `test_dest_rejects_malformed`             |   ✅   | Dest rejects malformed                                                     |
|   9 | `test_multistatus_file_and_collection`    |   ✅   | Multistatus file and collection                                            |
|  10 | `test_multistatus_escapes_href`           |   ✅   | Multistatus escapes href                                                   |
|  11 | `test_multistatus_entry_stops_when_full`  |   ✅   | Multistatus entry stops when full                                          |
|  12 | `test_proppatch_windows_timestamp`        |   ✅   | The PROPPATCH macOS Finder / Windows Explorer send after a PUT.            |
|  13 | `test_proppatch_multiple_and_self_closed` |   ✅   | Proppatch multiple and self closed                                         |
|  14 | `test_proppatch_remove_block`             |   ✅   | Proppatch remove block                                                     |
|  15 | `test_proppatch_escapes_href`             |   ✅   | Proppatch escapes href                                                     |
|  16 | `test_proppatch_empty_body_is_valid`      |   ✅   | Proppatch empty body is valid                                              |
|  17 | `test_proppatch_rejects_injection`        |   ✅   | A property tag carrying a stray '<' must not be echoed (no XML injection). |
|  18 | `test_proppatch_fuzz_bounded`             |   ✅   | Throw random and partial-XML bytes at the scanner: it must always stay in  |
|  19 | `test_proppatch_stops_when_full`          |   ✅   | Proppatch stops when full                                                  |

</details>

---

## test_modbus - ✅ 22 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Modbus TCP slave core (services/modbus): the data model and_

|   # | Test                                        | Status | Description                                          |
| --: | :------------------------------------------ | :----: | :--------------------------------------------------- |
|   1 | `test_read_holding_registers`               |   ✅   | Read holding registers                               |
|   2 | `test_read_input_registers`                 |   ✅   | Read input registers                                 |
|   3 | `test_read_coils_packs_bits`                |   ✅   | Read coils packs bits                                |
|   4 | `test_write_single_coil`                    |   ✅   | Write single coil                                    |
|   5 | `test_write_single_register`                |   ✅   | Write single register                                |
|   6 | `test_write_multiple_registers`             |   ✅   | Write multiple registers                             |
|   7 | `test_write_multiple_coils`                 |   ✅   | qty 5, 1 byte of data: bits 0..4 = 1,0,1,1,0 -> 0x0D |
|   8 | `test_exception_illegal_function`           |   ✅   | Exception illegal function                           |
|   9 | `test_exception_illegal_address`            |   ✅   | Read holding regs beyond the 64-register table.      |
|  10 | `test_exception_illegal_value`              |   ✅   | Exception illegal value                              |
|  11 | `test_write_single_coil_bad_value`          |   ✅   | Write single coil bad value                          |
|  12 | `test_non_modbus_protocol_id_ignored`       |   ✅   | Non modbus protocol id ignored                       |
|  13 | `test_truncated_frame_ignored`              |   ✅   | Truncated frame ignored                              |
|  14 | `test_discrete_and_input_accessors`         |   ✅   | Discrete and input accessors                         |
|  15 | `test_exceptions_per_function`              |   ✅   | FC1/FC2 read coils/discrete.                         |
|  16 | `test_small_response_buffer`                |   ✅   | Small response buffer                                |
|  17 | `test_rtu_crc16_known_vector`               |   ✅   | Rtu crc16 known vector                               |
|  18 | `test_rtu_read_holding_roundtrip`           |   ✅   | Rtu read holding roundtrip                           |
|  19 | `test_rtu_bad_crc_dropped`                  |   ✅   | Rtu bad crc dropped                                  |
|  20 | `test_rtu_wrong_address_dropped`            |   ✅   | Rtu wrong address dropped                            |
|  21 | `test_rtu_broadcast_executes_without_reply` |   ✅   | Rtu broadcast executes without reply                 |
|  22 | `test_rtu_edge_cases`                       |   ✅   | Rtu edge cases                                       |

</details>

---

## test_cloudevents - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CloudEvents v1.0 envelope (services/cloudevents): the_

|   # | Test                                 | Status | Description                   |
| --: | :----------------------------------- | :----: | :---------------------------- |
|   1 | `test_build_minimal`                 |   ✅   | Build minimal                 |
|   2 | `test_build_requires_id_source_type` |   ✅   | Build requires id source type |
|   3 | `test_build_with_json_data`          |   ✅   | Build with json data          |
|   4 | `test_build_with_string_data`        |   ✅   | Build with string data        |
|   5 | `test_build_overflow_fails_closed`   |   ✅   | Build overflow fails closed   |
|   6 | `test_from_headers_binary_mode`      |   ✅   | From headers binary mode      |
|   7 | `test_from_headers_missing_required` |   ✅   | From headers missing required |

</details>

---

## test_redis_resp - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Redis RESP2 codec (services/redis_resp): the command encoder_

|   # | Test                                  | Status | Description                    |
| --: | :------------------------------------ | :----: | :----------------------------- |
|   1 | `test_encode_command`                 |   ✅   | Encode command                 |
|   2 | `test_encode_binary_safe`             |   ✅   | Encode binary safe             |
|   3 | `test_encode_overflow_fails_closed`   |   ✅   | Encode overflow fails closed   |
|   4 | `test_parse_simple_and_error`         |   ✅   | Parse simple and error         |
|   5 | `test_parse_integer`                  |   ✅   | Parse integer                  |
|   6 | `test_parse_bulk_and_nil`             |   ✅   | Parse bulk and nil             |
|   7 | `test_parse_array_cursor`             |   ✅   | Parse array cursor             |
|   8 | `test_parse_incomplete_and_malformed` |   ✅   | Parse incomplete and malformed |

</details>

---

## test_stomp - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the STOMP 1.2 frame codec (services/stomp): the frame builder, the_

|   # | Test                                      | Status | Description                        |
| --: | :---------------------------------------- | :----: | :--------------------------------- |
|   1 | `test_build_send`                         |   ✅   | Build send                         |
|   2 | `test_build_cr_escape_and_guards`         |   ✅   | Build cr escape and guards         |
|   3 | `test_parse_more_edges`                   |   ✅   | Parse more edges                   |
|   4 | `test_header_and_unescape_null`           |   ✅   | Header and unescape null           |
|   5 | `test_build_no_headers_no_body`           |   ✅   | Build no headers no body           |
|   6 | `test_build_escapes_header`               |   ✅   | Build escapes header               |
|   7 | `test_build_overflow_fails_closed`        |   ✅   | Build overflow fails closed        |
|   8 | `test_round_trip`                         |   ✅   | Round trip                         |
|   9 | `test_parse_message_crlf`                 |   ✅   | Parse message crlf                 |
|  10 | `test_parse_content_length_body_with_nul` |   ✅   | Parse content length body with nul |
|  11 | `test_parse_skips_leading_heartbeats`     |   ✅   | Parse skips leading heartbeats     |
|  12 | `test_parse_incomplete_and_malformed`     |   ✅   | Parse incomplete and malformed     |
|  13 | `test_unescape`                           |   ✅   | Unescape                           |
|  14 | `test_unescape_rejects_bad`               |   ✅   | Unescape rejects bad               |

</details>

---

## test_mqtt_sn - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the MQTT-SN v1.2 wire codec (services/mqtt/mqtt_sn): the message_

|   # | Test                                       | Status | Description                                                                       |
| --: | :----------------------------------------- | :----: | :-------------------------------------------------------------------------------- |
|   1 | `test_make_flags`                          |   ✅   | DUP, QoS 2, retain, will, clean, short topic name.                                |
|   2 | `test_build_connect_bytes`                 |   ✅   | total = 1(len) + 1(type) + 1(flags) + 1(protoid) + 2(duration) + 4(clientid) = 10 |
|   3 | `test_build_publish_bytes`                 |   ✅   | total = 1+1+1(flags)+2(topic)+2(msgid)+2(data) = 9                                |
|   4 | `test_register_round_trip`                 |   ✅   | Register round trip                                                               |
|   5 | `test_parse_connack_regack_suback_publish` |   ✅   | Parse connack regack suback publish                                               |
|   6 | `test_three_octet_length`                  |   ✅   | Three octet length                                                                |
|   7 | `test_optional_fields`                     |   ✅   | PINGREQ with no client id is a 2-byte keep-alive.                                 |
|   8 | `test_overflow_and_malformed`              |   ✅   | Overflow and malformed                                                            |
|   9 | `test_build_regack_puback`                 |   ✅   | Build regack puback                                                               |
|  10 | `test_build_subscribe_variants`            |   ✅   | Build subscribe variants                                                          |
|  11 | `test_pingreq_with_client_id`              |   ✅   | Pingreq with client id                                                            |
|  12 | `test_build_guards`                        |   ✅   | Build guards                                                                      |
|  13 | `test_parse_typed_rejections`              |   ✅   | Parse typed rejections                                                            |

</details>

---

## test_flow_export - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the flow-export codec (services/flow_export): NetFlow v5 fixed records,_

|   # | Test                                | Status | Description                  |
| --: | :---------------------------------- | :----: | :--------------------------- |
|   1 | `test_v5_header_bytes`              |   ✅   | V5 header bytes              |
|   2 | `test_v5_record_bytes`              |   ✅   | V5 record bytes              |
|   3 | `test_v5_overflow_fails_closed`     |   ✅   | V5 overflow fails closed     |
|   4 | `test_ipfix_message_bytes`          |   ✅   | Ipfix message bytes          |
|   5 | `test_v9_count_and_padding`         |   ✅   | V9 count and padding         |
|   6 | `test_finish_overflow_fails_closed` |   ✅   | Finish overflow fails closed |

</details>

---

## test_protobuf - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Protocol Buffers wire codec (services/protobuf): the streaming_

|   # | Test                         | Status | Description                                                            |
| --: | :--------------------------- | :----: | :--------------------------------------------------------------------- |
|   1 | `test_writer_error_paths`    |   ✅   | A 5-byte varint does not fit a 4-byte buffer.                          |
|   2 | `test_reader_error_paths`    |   ✅   | Reader error paths                                                     |
|   3 | `test_float_bits_helper`     |   ✅   | Float bits helper                                                      |
|   4 | `test_vector_field1_150`     |   ✅   | Vector field1 150                                                      |
|   5 | `test_vector_string_testing` |   ✅   | Vector string testing                                                  |
|   6 | `test_zigzag_mapping`        |   ✅   | Decode: encoded 1 -> -1, 2 -> 1, 3 -> -2.                              |
|   7 | `test_fixed_and_float_bytes` |   ✅   | Fixed and float bytes                                                  |
|   8 | `test_round_trip_reader`     |   ✅   | Round trip reader                                                      |
|   9 | `test_int64_negative`        |   ✅   | Int64 negative                                                         |
|  10 | `test_varint_and_overflow`   |   ✅   | A multi-byte varint round-trips.                                       |
|  11 | `test_malformed_reads`       |   ✅   | Malformed reads                                                        |
|  12 | `test_varint_width_boundary` |   ✅   | The maximum 64-bit varint: nine 0xFF groups then 0x01 -> all bits set. |
|  13 | `test_empty_length_field`    |   ✅   | Empty length field                                                     |

</details>

---

## test_preempt_queue - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the preempting work queue (services/preempt_queue) host core: the_

|   # | Test                                       | Status | Description                                                                 |
| --: | :----------------------------------------- | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_start_validates_and_runs`            |   ✅   | Start validates and runs                                                    |
|   2 | `test_fifo_order`                          |   ✅   | Fifo order                                                                  |
|   3 | `test_urgent_goes_to_front`                |   ✅   | Urgent goes to front                                                        |
|   4 | `test_fail_closed_when_full`               |   ✅   | The test env sizes DETWS_PQ_DEPTH = 4.                                      |
|   5 | `test_high_water_tracks_peak`              |   ✅   | High water tracks peak                                                      |
|   6 | `test_from_isr_enqueues`                   |   ✅   | From isr enqueues                                                           |
|   7 | `test_drain_empties_and_reuses`            |   ✅   | Drain empties and reuses                                                    |
|   8 | `test_internal_lanes_outrank_user`         |   ✅   | DMA highest, then forward, then device, all above the user lane.            |
|   9 | `test_lanes_are_isolated`                  |   ✅   | The USER lane is already started by setUp; start the internal DMA lane too. |
|  10 | `test_lane_start_stop_running_independent` |   ✅   | Lane start stop running independent                                         |
|  11 | `test_lane_high_water_is_per_lane`         |   ✅   | Lane high water is per lane                                                 |

</details>

---

## test_dma - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the DMA ingest / egress simulator (services/dma) host core: an ingress_

|   # | Test                                   | Status | Description                     |
| --: | :------------------------------------- | :----: | :------------------------------ |
|   1 | `test_open_validates`                  |   ✅   | Open validates                  |
|   2 | `test_ingress_emits_rx_event`          |   ✅   | Ingress emits rx event          |
|   3 | `test_buffer_fills_then_partial_flush` |   ✅   | Buffer fills then partial flush |
|   4 | `test_ping_pong_flips_buffer`          |   ✅   | Ping pong flips buffer          |
|   5 | `test_egress_captures_tx`              |   ✅   | Egress captures tx              |
|   6 | `test_tx_one_in_flight_fail_closed`    |   ✅   | Tx one in flight fail closed    |
|   7 | `test_tx_rejects_bad_len`              |   ✅   | Tx rejects bad len              |
|   8 | `test_loopback_round_trip`             |   ✅   | Loopback round trip             |
|   9 | `test_feed_fail_closed_when_full`      |   ✅   | Feed fail closed when full      |
|  10 | `test_closed_channel_is_inert`         |   ✅   | Closed channel is inert         |
|  11 | `test_two_channels_independent`        |   ✅   | Two channels independent        |

</details>

---

## test_forward - ✅ 16 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the interface forwarding plane (services/forward): default-deny, an_

|   # | Test                                     | Status | Description                       |
| --: | :--------------------------------------- | :----: | :-------------------------------- |
|   1 | `test_default_deny`                      |   ✅   | Default deny                      |
|   2 | `test_allow_forwards`                    |   ✅   | Allow forwards                    |
|   3 | `test_no_self_forward`                   |   ✅   | No self forward                   |
|   4 | `test_deny_wins_over_allow`              |   ✅   | Deny wins over allow              |
|   5 | `test_multi_destination_fanout`          |   ✅   | Multi destination fanout          |
|   6 | `test_rate_cap_drops_then_reopens`       |   ✅   | Rate cap drops then reopens       |
|   7 | `test_send_failure_counted`              |   ✅   | Send failure counted              |
|   8 | `test_add_if_validation_and_table_full`  |   ✅   | Add if validation and table full  |
|   9 | `test_add_rule_table_full`               |   ✅   | Add rule table full               |
|  10 | `test_unregistered_destination_is_inert` |   ✅   | Unregistered destination is inert |
|  11 | `test_acl_deny_by_byte_pattern`          |   ✅   | Acl deny by byte pattern          |
|  12 | `test_acl_allowlist_default_deny`        |   ✅   | Acl allowlist default deny        |
|  13 | `test_acl_first_match_wins`              |   ✅   | Acl first match wins              |
|  14 | `test_acl_src_any_content_wildcard`      |   ✅   | Acl src any content wildcard      |
|  15 | `test_acl_short_frame_skips_entry`       |   ✅   | Acl short frame skips entry       |
|  16 | `test_acl_add_validation_and_table_full` |   ✅   | Acl add validation and table full |

</details>

---

## test_gateway - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the radio / wireless gateway bridge (services/gateway): an uplink_

|   # | Test                                        | Status | Description                          |
| --: | :------------------------------------------ | :----: | :----------------------------------- |
|   1 | `test_uplink_envelopes_and_publishes`       |   ✅   | Uplink envelopes and publishes       |
|   2 | `test_uplink_no_sink_drops`                 |   ✅   | Uplink no sink drops                 |
|   3 | `test_uplink_unknown_port_drops`            |   ✅   | Uplink unknown port drops            |
|   4 | `test_uplink_rate_cap`                      |   ✅   | Uplink rate cap                      |
|   5 | `test_uplink_sink_refusal_counted`          |   ✅   | Uplink sink refusal counted          |
|   6 | `test_downlink_transmits`                   |   ✅   | Downlink transmits                   |
|   7 | `test_downlink_no_tx_or_unknown_port_drops` |   ✅   | Downlink no tx or unknown port drops |
|   8 | `test_downlink_tx_refusal_counted`          |   ✅   | Downlink tx refusal counted          |
|   9 | `test_topic_format`                         |   ✅   | Topic format                         |
|  10 | `test_add_port_validation_and_table_full`   |   ✅   | Add port validation and table full   |
|  11 | `test_seq_increments_per_uplink`            |   ✅   | Seq increments per uplink            |

</details>

---

## test_lora - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the LoRa codec + SX127x driver (services/lora). The codec (RadioHead_

|   # | Test                                           | Status | Description                             |
| --: | :--------------------------------------------- | :----: | :-------------------------------------- |
|   1 | `test_frame_build_then_parse`                  |   ✅   | Frame build then parse                  |
|   2 | `test_frame_parse_rejects_short`               |   ✅   | Frame parse rejects short               |
|   3 | `test_frame_build_bounds`                      |   ✅   | Frame build bounds                      |
|   4 | `test_init_verifies_chip_and_lands_in_standby` |   ✅   | Init verifies chip and lands in standby |
|   5 | `test_init_fails_on_wrong_version`             |   ✅   | Init fails on wrong version             |
|   6 | `test_init_programs_frequency`                 |   ✅   | Init programs frequency                 |
|   7 | `test_send_loads_fifo_and_starts_tx`           |   ✅   | Send loads fifo and starts tx           |
|   8 | `test_tx_done_flag`                            |   ✅   | Tx done flag                            |
|   9 | `test_set_rx_enters_continuous`                |   ✅   | Set rx enters continuous                |
|  10 | `test_recv_reads_frame_and_rssi`               |   ✅   | Recv reads frame and rssi               |
|  11 | `test_recv_no_packet`                          |   ✅   | Recv no packet                          |
|  12 | `test_recv_crc_error_dropped`                  |   ✅   | Recv crc error dropped                  |
|  13 | `test_recv_truncates_to_cap`                   |   ✅   | Recv truncates to cap                   |

</details>

---

## test_nrf24 - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the nRF24L01+ driver (services/nrf24) against a mock chip that emulates_

|   # | Test                                  | Status | Description                    |
| --: | :------------------------------------ | :----: | :----------------------------- |
|   1 | `test_init_configures_and_powers_up`  |   ✅   | Init configures and powers up  |
|   2 | `test_init_fails_when_absent`         |   ✅   | Init fails when absent         |
|   3 | `test_send_pads_to_width_and_keys_tx` |   ✅   | Send pads to width and keys tx |
|   4 | `test_send_rejects_oversize`          |   ✅   | Send rejects oversize          |
|   5 | `test_tx_done_flag`                   |   ✅   | Tx done flag                   |
|   6 | `test_set_rx_enters_prx`              |   ✅   | Set rx enters prx              |
|   7 | `test_recv_reads_payload_and_pipe`    |   ✅   | Recv reads payload and pipe    |
|   8 | `test_recv_no_packet`                 |   ✅   | Recv no packet                 |
|   9 | `test_recv_fifo_empty_pipe`           |   ✅   | Recv fifo empty pipe           |
|  10 | `test_recv_truncates_to_cap`          |   ✅   | Recv truncates to cap          |

</details>

---

## test_enocean - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the EnOcean ESP3 codec (services/enocean): the CRC-8 (poly 0x07) against_

|   # | Test                                   | Status | Description                                                                       |
| --: | :------------------------------------- | :----: | :-------------------------------------------------------------------------------- |
|   1 | `test_crc8_known_answers`              |   ✅   | Crc8 known answers                                                                |
|   2 | `test_build_then_parse_round_trip`     |   ✅   | Build then parse round trip                                                       |
|   3 | `test_parse_rejects_bad_sync`          |   ✅   | Parse rejects bad sync                                                            |
|   4 | `test_parse_rejects_bad_header_crc`    |   ✅   | Parse rejects bad header crc                                                      |
|   5 | `test_parse_rejects_bad_data_crc`      |   ✅   | Parse rejects bad data crc                                                        |
|   6 | `test_parse_needs_more_bytes`          |   ✅   | Parse needs more bytes                                                            |
|   7 | `test_parse_rejects_over_length`       |   ✅   | A header claiming data_len 100 (> DETWS_ENOCEAN_MAX_DATA = 16) is rejected early. |
|   8 | `test_parse_resynchronises_after_junk` |   ✅   | Parse resynchronises after junk                                                   |
|   9 | `test_build_bounds`                    |   ✅   | Build bounds                                                                      |

</details>

---

## test_pn532 - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the PN532 NFC frame codec (services/pn532): the normal-information-frame_

|   # | Test                                         | Status | Description                                                              |
| --: | :------------------------------------------- | :----: | :----------------------------------------------------------------------- |
|   1 | `test_build_getfirmwareversion_kat`          |   ✅   | Host -> PN532 GetFirmwareVersion (command 0x02): the documented frame is |
|   2 | `test_parse_getfirmwareversion_response_kat` |   ✅   | PN532 -> host response: 00 00 FF 06 FA D5 03 32 01 06 07 E8 00.          |
|   3 | `test_build_then_parse_round_trip`           |   ✅   | Build then parse round trip                                              |
|   4 | `test_parse_rejects_bad_preamble_and_start`  |   ✅   | Parse rejects bad preamble and start                                     |
|   5 | `test_parse_rejects_bad_lcs`                 |   ✅   | Parse rejects bad lcs                                                    |
|   6 | `test_parse_rejects_bad_dcs`                 |   ✅   | Parse rejects bad dcs                                                    |
|   7 | `test_parse_needs_more_bytes`                |   ✅   | Parse needs more bytes                                                   |
|   8 | `test_parse_rejects_over_length`             |   ✅   | frame_len 20 (> DETWS_PN532_MAX_DATA + 1 = 9) is rejected early.         |
|   9 | `test_ack_frame`                             |   ✅   | Ack frame                                                                |
|  10 | `test_build_bounds`                          |   ✅   | Build bounds                                                             |

</details>

---

## test_sigfox - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Sigfox AT-command codec (services/sigfox): the AT$SF uplink command_

|   # | Test                             | Status | Description                                                                          |
| --: | :------------------------------- | :----: | :----------------------------------------------------------------------------------- |
|   1 | `test_build_uplink_hex_encode`   |   ✅   | Build uplink hex encode                                                              |
|   2 | `test_build_uplink_single_byte`  |   ✅   | Build uplink single byte                                                             |
|   3 | `test_build_uplink_bounds`       |   ✅   | Build uplink bounds                                                                  |
|   4 | `test_parse_response_ok`         |   ✅   | Parse response ok                                                                    |
|   5 | `test_parse_response_error`      |   ✅   | Parse response error                                                                 |
|   6 | `test_parse_response_pending`    |   ✅   | Parse response pending                                                               |
|   7 | `test_parse_response_error_wins` |   ✅   | If a buffer holds both (e.g. an echoed "OK" token then an ERROR), ERROR is reported. |

</details>

---

## test_zwave - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Z-Wave Serial API frame codec (services/zwave): the data-frame_

|   # | Test                               | Status | Description                                                                          |
| --: | :--------------------------------- | :----: | :----------------------------------------------------------------------------------- |
|   1 | `test_build_getversion_kat`        |   ✅   | Host -> controller FUNC_ID_ZW_GET_VERSION (0x15), a REQ with no data: the documented |
|   2 | `test_build_then_parse_round_trip` |   ✅   | Build then parse round trip                                                          |
|   3 | `test_parse_getversion_kat`        |   ✅   | Parse getversion kat                                                                 |
|   4 | `test_parse_rejects_bad_sof`       |   ✅   | Parse rejects bad sof                                                                |
|   5 | `test_parse_rejects_bad_checksum`  |   ✅   | Parse rejects bad checksum                                                           |
|   6 | `test_parse_needs_more_bytes`      |   ✅   | Parse needs more bytes                                                               |
|   7 | `test_parse_rejects_over_length`   |   ✅   | frame_len 80 (> DETWS_ZWAVE_MAX_DATA + 3 = 19) is rejected early.                    |
|   8 | `test_control_bytes`               |   ✅   | Control bytes                                                                        |
|   9 | `test_build_bounds`                |   ✅   | Build bounds                                                                         |

</details>

---

## test_zigbee - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Zigbee EZSP / ASH framing codec (services/zigbee): the CRC-16/CCITT_

|   # | Test                                       | Status | Description                                                                          |
| --: | :----------------------------------------- | :----: | :----------------------------------------------------------------------------------- |
|   1 | `test_crc16_rst_kat`                       |   ✅   | CRC-16/CCITT (poly 0x1021, init 0xFFFF) of {0xC0} is 0x38BC (the ASH RST frame CRC). |
|   2 | `test_encode_rst_frame_kat`                |   ✅   | The documented ASH RST frame is C0 38 BC 7E (control, CRC hi/lo, flag).              |
|   3 | `test_encode_decode_round_trip`            |   ✅   | Encode decode round trip                                                             |
|   4 | `test_byte_stuffing_round_trip`            |   ✅   | A payload full of reserved bytes must survive: none may appear raw in the body.      |
|   5 | `test_decode_needs_more_without_flag`      |   ✅   | Decode needs more without flag                                                       |
|   6 | `test_decode_rejects_bad_crc`              |   ✅   | Decode rejects bad crc                                                               |
|   7 | `test_decode_rejects_dangling_escape`      |   ✅   | Decode rejects dangling escape                                                       |
|   8 | `test_decode_rejects_small_payload_buffer` |   ✅   | Decode rejects small payload buffer                                                  |
|   9 | `test_encode_bounds`                       |   ✅   | Encode bounds                                                                        |

</details>

---

## test_thread - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Thread spinel / HDLC-lite framing codec (services/thread): the FCS_

|   # | Test                                         | Status | Description                                                                               |
| --: | :------------------------------------------- | :----: | :---------------------------------------------------------------------------------------- |
|   1 | `test_fcs_x25_check_value`                   |   ✅   | CRC-16/X-25 (poly 0x8408, init 0xFFFF, reflected, xorout 0xFFFF) of "123456789" = 0x906E. |
|   2 | `test_encode_decode_round_trip`              |   ✅   | A tiny spinel frame: header (flag                                                         | iid | tid) + command (PROP_VALUE_GET) + property. |
|   3 | `test_byte_stuffing_round_trip`              |   ✅   | Byte stuffing round trip                                                                  |
|   4 | `test_decode_needs_more_without_flag`        |   ✅   | Decode needs more without flag                                                            |
|   5 | `test_decode_rejects_bad_fcs`                |   ✅   | Decode rejects bad fcs                                                                    |
|   6 | `test_decode_rejects_dangling_escape`        |   ✅   | Decode rejects dangling escape                                                            |
|   7 | `test_decode_rejects_small_payload_buffer`   |   ✅   | Decode rejects small payload buffer                                                       |
|   8 | `test_encode_bounds`                         |   ✅   | Encode bounds                                                                             |
|   9 | `test_spinel_pack_uint_kats`                 |   ✅   | Spinel pack uint kats                                                                     |
|  10 | `test_spinel_pack_unpack_round_trip`         |   ✅   | Spinel pack unpack round trip                                                             |
|  11 | `test_spinel_unpack_needs_more_and_overflow` |   ✅   | Spinel unpack needs more and overflow                                                     |
|  12 | `test_spinel_command_build_parse_round_trip` |   ✅   | header 0x81, CMD_PROP_VALUE_SET, a large property id (multi-byte packed), a value.        |
|  13 | `test_spinel_command_through_hdlc`           |   ✅   | The command payload rides inside an HDLC frame: build the command, frame it, decode       |

</details>

---

## test_wamp - ✅ 12 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the WAMP codec (services/wamp): the message builders (JSON arrays over_

|   # | Test                                     | Status | Description                       |
| --: | :--------------------------------------- | :----: | :-------------------------------- |
|   1 | `test_build_hello`                       |   ✅   | Build hello                       |
|   2 | `test_build_subscribe_default_options`   |   ✅   | Build subscribe default options   |
|   3 | `test_build_publish_with_args`           |   ✅   | Build publish with args           |
|   4 | `test_build_publish_kwargs_only`         |   ✅   | Build publish kwargs only         |
|   5 | `test_build_call_and_register_and_yield` |   ✅   | Build call and register and yield |
|   6 | `test_build_unsubscribe_and_goodbye`     |   ✅   | Build unsubscribe and goodbye     |
|   7 | `test_build_overflow_fails_closed`       |   ✅   | Build overflow fails closed       |
|   8 | `test_parse_type_and_id`                 |   ✅   | Parse type and id                 |
|   9 | `test_parse_event_positions`             |   ✅   | Parse event positions             |
|  10 | `test_parse_get_uri_and_nesting`         |   ✅   | Parse get uri and nesting         |
|  11 | `test_parse_malformed`                   |   ✅   | Parse malformed                   |
|  12 | `test_get_uri_dest_bounds`               |   ✅   | Get uri dest bounds               |

</details>

---

## test_sunspec - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SunSpec Modbus codec (services/sunspec): the map writer, the_

|   # | Test                                | Status | Description                  |
| --: | :---------------------------------- | :----: | :--------------------------- |
|   1 | `test_build_and_walk`               |   ✅   | Build and walk               |
|   2 | `test_two_models`                   |   ✅   | Two models                   |
|   3 | `test_string_point`                 |   ✅   | String point                 |
|   4 | `test_marker_and_truncation`        |   ✅   | Marker and truncation        |
|   5 | `test_writer_overflow_fails_closed` |   ✅   | Writer overflow fails closed |

</details>

---

## test_c37118 - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the IEEE C37.118.2 synchrophasor frame codec (services/c37118): the_

|   # | Test                               | Status | Description                                    |
| --: | :--------------------------------- | :----: | :--------------------------------------------- |
|   1 | `test_crc_check_value`             |   ✅   | Crc check value                                |
|   2 | `test_build_command_bytes`         |   ✅   | Build command bytes                            |
|   3 | `test_command_round_trip`          |   ✅   | Command round trip                             |
|   4 | `test_data_frame_payload`          |   ✅   | Data frame payload                             |
|   5 | `test_parse_rejects_bad`           |   ✅   | A flipped payload bit must fail the CRC check. |
|   6 | `test_build_overflow_fails_closed` |   ✅   | Build overflow fails closed                    |

</details>

---

## test_dnp3 - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the DNP3 (IEEE 1815) data-link frame codec (services/dnp3): CRC-16/DNP,_

|   # | Test                               | Status | Description                                 |
| --: | :--------------------------------- | :----: | :------------------------------------------ |
|   1 | `test_dnp3_parse_guards`           |   ✅   | Dnp3 parse guards                           |
|   2 | `test_crc_check_value`             |   ✅   | Crc check value                             |
|   3 | `test_build_header_bytes`          |   ✅   | 10 header + 3 data + 2 block CRC = 15       |
|   4 | `test_round_trip_single_block`     |   ✅   | Round trip single block                     |
|   5 | `test_round_trip_multi_block`      |   ✅   | Round trip multi block                      |
|   6 | `test_header_only_frame`           |   ✅   | Header only frame                           |
|   7 | `test_parse_rejects_bad`           |   ✅   | A corrupted data octet fails the block CRC. |
|   8 | `test_build_overflow_fails_closed` |   ✅   | Build overflow fails closed                 |

</details>

---

## test_grpcweb - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the gRPC-Web message framing codec (services/grpcweb): the message and_

|   # | Test                               | Status | Description                 |
| --: | :--------------------------------- | :----: | :-------------------------- |
|   1 | `test_frame_message_bytes`         |   ✅   | Frame message bytes         |
|   2 | `test_compressed_flag`             |   ✅   | Compressed flag             |
|   3 | `test_trailer_frame`               |   ✅   | Trailer frame               |
|   4 | `test_trailer_status_only`         |   ✅   | Trailer status only         |
|   5 | `test_parse_stream`                |   ✅   | frame 1: the message        |
|   6 | `test_parse_incomplete`            |   ✅   | Parse incomplete            |
|   7 | `test_frame_overflow_fails_closed` |   ✅   | Frame overflow fails closed |

</details>

---

## test_lwm2m_tlv - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the OMA LwM2M TLV codec (services/lwm2m): the writer (raw + typed value_

|   # | Test                            | Status | Description              |
| --: | :------------------------------ | :----: | :----------------------- |
|   1 | `test_write_int_1byte`          |   ✅   | Write int 1byte          |
|   2 | `test_write_int_2byte`          |   ✅   | Write int 2byte          |
|   3 | `test_write_string_8bit_length` |   ✅   | Write string 8bit length |
|   4 | `test_write_16bit_id`           |   ✅   | Write 16bit id           |
|   5 | `test_round_trip_and_value_int` |   ✅   | Round trip and value int |
|   6 | `test_object_instance_nested`   |   ✅   | Object instance nested   |
|   7 | `test_write_16bit_length`       |   ✅   | Write 16bit length       |
|   8 | `test_read_24bit_length`        |   ✅   | Read 24bit length        |
|   9 | `test_value_int_4_and_8_byte`   |   ✅   | Value int 4 and 8 byte   |
|  10 | `test_zero_length_value`        |   ✅   | Zero length value        |
|  11 | `test_overflow_and_malformed`   |   ✅   | Overflow and malformed   |

</details>

---

## test_fins - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Omron FINS frame codec (services/fins): the command builder, the_

|   # | Test                           | Status | Description                                              |
| --: | :----------------------------- | :----: | :------------------------------------------------------- |
|   1 | `test_build_command_bytes`     |   ✅   | Build command bytes                                      |
|   2 | `test_memory_area_read`        |   ✅   | area 0xB0 (DM), word 100 = 0x0064, bit 0, read 10 words. |
|   3 | `test_parse_command`           |   ✅   | Parse command                                            |
|   4 | `test_parse_response_ok`       |   ✅   | Parse response ok                                        |
|   5 | `test_parse_response_error`    |   ✅   | Parse response error                                     |
|   6 | `test_overflow_and_truncation` |   ✅   | Overflow and truncation                                  |

</details>

---

## test_hostlink - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Omron Host Link (C-mode) frame codec (services/hostlink): the FCS,_

|   # | Test                               | Status | Description                                                                       |
| --: | :--------------------------------- | :----: | :-------------------------------------------------------------------------------- |
|   1 | `test_fcs_vector`                  |   ✅   | Fcs vector                                                                        |
|   2 | `test_build_dm_read`               |   ✅   | Build dm read                                                                     |
|   3 | `test_build_node_digits`           |   ✅   | Build node digits                                                                 |
|   4 | `test_round_trip`                  |   ✅   | Round trip                                                                        |
|   5 | `test_parse_response_end_code`     |   ✅   | Build a "response-shaped" frame: header RD, text = end code "00" + 4 data digits. |
|   6 | `test_parse_rejects_bad`           |   ✅   | Corrupt a text char -> FCS no longer matches.                                     |
|   7 | `test_build_overflow_fails_closed` |   ✅   | Build overflow fails closed                                                       |

</details>

---

## test_senml - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SenML (RFC 8428) pack builders (services/senml): SenML-JSON (exact_

|   # | Test                           | Status | Description             |
| --: | :----------------------------- | :----: | :---------------------- |
|   1 | `test_json_canonical`          |   ✅   | Json canonical          |
|   2 | `test_json_base_time_and_none` |   ✅   | Json base time and none |
|   3 | `test_cbor_all_kinds`          |   ✅   | Cbor all kinds          |
|   4 | `test_senml_null_args`         |   ✅   | Senml null args         |
|   5 | `test_json_multi_record`       |   ✅   | Json multi record       |
|   6 | `test_json_string_bool_time`   |   ✅   | Json string bool time   |
|   7 | `test_cbor_round_trip`         |   ✅   | Cbor round trip         |
|   8 | `test_cbor_base_name_key`      |   ✅   | Cbor base name key      |
|   9 | `test_overflow_fails_closed`   |   ✅   | Overflow fails closed   |

</details>

---

## test_df1 - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Allen-Bradley DF1 full-duplex frame codec (services/df1): the BCC and_

|   # | Test                               | Status | Description                          |
| --: | :--------------------------------- | :----: | :----------------------------------- |
|   1 | `test_bcc_vector`                  |   ✅   | Bcc vector                           |
|   2 | `test_crc_vector`                  |   ✅   | Crc vector                           |
|   3 | `test_build_bcc_frame`             |   ✅   | Build bcc frame                      |
|   4 | `test_build_dle_stuffing`          |   ✅   | Build dle stuffing                   |
|   5 | `test_round_trip_bcc`              |   ✅   | Round trip bcc                       |
|   6 | `test_round_trip_crc`              |   ✅   | Round trip crc                       |
|   7 | `test_empty_data_frame`            |   ✅   | Empty data frame                     |
|   8 | `test_parse_rejects_bad`           |   ✅   | Corrupt a data byte -> BCC mismatch. |
|   9 | `test_build_overflow_fails_closed` |   ✅   | Build overflow fails closed          |

</details>

---

## test_cotp - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the TPKT + COTP (X.224 class 0) frame codec (services/cotp): the TPKT_

|   # | Test                      | Status | Description                                  |
| --: | :------------------------ | :----: | :------------------------------------------- |
|   1 | `test_tpkt_bytes`         |   ✅   | Tpkt bytes                                   |
|   2 | `test_cotp_dt_bytes`      |   ✅   | Cotp dt bytes                                |
|   3 | `test_cotp_cr_bytes`      |   ✅   | Cotp cr bytes                                |
|   4 | `test_cotp_cr_with_tsaps` |   ✅   | Cotp cr with tsaps                           |
|   5 | `test_full_stack`         |   ✅   | total = 4 (tpkt) + 3 (cotp dt) + 4 (s7) = 11 |
|   6 | `test_parse_rejects_bad`  |   ✅   | Parse rejects bad                            |

</details>

---

## test_s7comm - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Siemens S7comm PDU codec (services/s7comm): the Setup Communication_

|   # | Test                               | Status | Description                 |
| --: | :--------------------------------- | :----: | :-------------------------- |
|   1 | `test_build_setup`                 |   ✅   | Build setup                 |
|   2 | `test_build_read_request`          |   ✅   | Build read request          |
|   3 | `test_read_request_bit_address`    |   ✅   | Read request bit address    |
|   4 | `test_parse_response_single`       |   ✅   | Parse response single       |
|   5 | `test_parse_response_padding`      |   ✅   | Parse response padding      |
|   6 | `test_parse_octet_and_error`       |   ✅   | Parse octet and error       |
|   7 | `test_parse_rejects_bad`           |   ✅   | Parse rejects bad           |
|   8 | `test_build_overflow_fails_closed` |   ✅   | Build overflow fails closed |

</details>

---

## test_melsec - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Mitsubishi MELSEC MC binary 3E codec (services/melsec): the batch-read_

|   # | Test                               | Status | Description                 |
| --: | :--------------------------------- | :----: | :-------------------------- |
|   1 | `test_build_read_bytes`            |   ✅   | Build read bytes            |
|   2 | `test_head_device_24bit`           |   ✅   | Head device 24bit           |
|   3 | `test_parse_response_ok`           |   ✅   | Parse response ok           |
|   4 | `test_parse_response_error`        |   ✅   | Parse response error        |
|   5 | `test_parse_rejects_bad`           |   ✅   | Parse rejects bad           |
|   6 | `test_build_overflow_fails_closed` |   ✅   | Build overflow fails closed |

</details>

---

## test_bacnet - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the BACnet/IP BVLC + NPDU codec (services/bacnet): the BVLC envelope and_

|   # | Test                                 | Status | Description                   |
| --: | :----------------------------------- | :----: | :---------------------------- |
|   1 | `test_bacnet_guards_and_truncations` |   ✅   | Bacnet guards and truncations |
|   2 | `test_bvlc_bytes`                    |   ✅   | Bvlc bytes                    |
|   3 | `test_npdu_local`                    |   ✅   | Npdu local                    |
|   4 | `test_npdu_dest`                     |   ✅   | Npdu dest                     |
|   5 | `test_npdu_broadcast`                |   ✅   | Npdu broadcast                |
|   6 | `test_npdu_parse_with_source`        |   ✅   | Npdu parse with source        |
|   7 | `test_full_stack`                    |   ✅   | Full stack                    |
|   8 | `test_parse_rejects_bad`             |   ✅   | Parse rejects bad             |
|   9 | `test_overflow_fails_closed`         |   ✅   | Overflow fails closed         |

</details>

---

## test_enip - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the EtherNet/IP encapsulation codec (services/enip): the header, the_

|   # | Test                               | Status | Description                 |
| --: | :--------------------------------- | :----: | :-------------------------- |
|   1 | `test_header_round_trip`           |   ✅   | Header round trip           |
|   2 | `test_register_session`            |   ✅   | Register session            |
|   3 | `test_send_rr_data_bytes`          |   ✅   | Send rr data bytes          |
|   4 | `test_send_rr_data_round_trip`     |   ✅   | Send rr data round trip     |
|   5 | `test_parse_rejects_bad`           |   ✅   | Parse rejects bad           |
|   6 | `test_build_overflow_fails_closed` |   ✅   | Build overflow fails closed |

</details>

---

## test_amqp - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the AMQP 0-9-1 frame codec (services/amqp): the protocol header, the frame_

|   # | Test                               | Status | Description                          |
| --: | :--------------------------------- | :----: | :----------------------------------- |
|   1 | `test_protocol_header`             |   ✅   | Protocol header                      |
|   2 | `test_build_method_bytes`          |   ✅   | Build method bytes                   |
|   3 | `test_method_round_trip`           |   ✅   | Method round trip                    |
|   4 | `test_heartbeat`                   |   ✅   | Heartbeat                            |
|   5 | `test_parse_stream`                |   ✅   | Parse stream                         |
|   6 | `test_parse_rejects_bad`           |   ✅   | A frame whose end octet is not 0xCE. |
|   7 | `test_build_overflow_fails_closed` |   ✅   | Build overflow fails closed          |

</details>

---

## test_cip - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CIP message codec (services/cip): the EPATH builder, the request_

|   # | Test                                    | Status | Description                      |
| --: | :-------------------------------------- | :----: | :------------------------------- |
|   1 | `test_cip_build_guards`                 |   ✅   | Cip build guards                 |
|   2 | `test_epath_8bit`                       |   ✅   | Epath 8bit                       |
|   3 | `test_epath_16bit`                      |   ✅   | Epath 16bit                      |
|   4 | `test_get_attr_single`                  |   ✅   | Get attr single                  |
|   5 | `test_build_request_with_data`          |   ✅   | Build request with data          |
|   6 | `test_parse_response_ok`                |   ✅   | Parse response ok                |
|   7 | `test_parse_response_additional_status` |   ✅   | Parse response additional status |
|   8 | `test_parse_response_error`             |   ✅   | Parse response error             |
|   9 | `test_rejects_bad`                      |   ✅   | Rejects bad                      |

</details>

---

## test_nats - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the NATS client protocol codec (services/nats): the CONNECT/PUB/SUB/UNSUB/_

|   # | Test                               | Status | Description                                                                    |
| --: | :--------------------------------- | :----: | :----------------------------------------------------------------------------- |
|   1 | `test_build_connect`               |   ✅   | Build connect                                                                  |
|   2 | `test_build_ping_pong`             |   ✅   | Build ping pong                                                                |
|   3 | `test_build_null_args`             |   ✅   | Build null args                                                                |
|   4 | `test_build_overflow_put_ch`       |   ✅   | cap 6: "PUB " fits, "foo" overflows in put_str -> ok=false, then put_ch bails. |
|   5 | `test_parse_edges`                 |   ✅   | Parse edges                                                                    |
|   6 | `test_build_pub`                   |   ✅   | Build pub                                                                      |
|   7 | `test_build_pub_with_reply`        |   ✅   | Build pub with reply                                                           |
|   8 | `test_build_pub_empty_payload`     |   ✅   | Build pub empty payload                                                        |
|   9 | `test_build_sub_and_unsub`         |   ✅   | Build sub and unsub                                                            |
|  10 | `test_parse_msg`                   |   ✅   | Parse msg                                                                      |
|  11 | `test_parse_msg_with_reply`        |   ✅   | Parse msg with reply                                                           |
|  12 | `test_parse_control_lines`         |   ✅   | Parse control lines                                                            |
|  13 | `test_parse_incomplete`            |   ✅   | Parse incomplete                                                               |
|  14 | `test_build_overflow_fails_closed` |   ✅   | Build overflow fails closed                                                    |

</details>

---

## test_proxy_protocol - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the HAProxy PROXY protocol codec (services/proxy_protocol): the v1 (text)_

|   # | Test                               | Status | Description                 |
| --: | :--------------------------------- | :----: | :-------------------------- |
|   1 | `test_v1_build`                    |   ✅   | V1 build                    |
|   2 | `test_v1_round_trip`               |   ✅   | V1 round trip               |
|   3 | `test_v2_build_bytes`              |   ✅   | V2 build bytes              |
|   4 | `test_v2_round_trip`               |   ✅   | V2 round trip               |
|   5 | `test_v1_unknown`                  |   ✅   | V1 unknown                  |
|   6 | `test_not_a_proxy_header`          |   ✅   | Not a proxy header          |
|   7 | `test_incomplete`                  |   ✅   | v1 prefix but no CRLF yet.  |
|   8 | `test_build_overflow_fails_closed` |   ✅   | Build overflow fails closed |

</details>

---

## test_sparkplug - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Sparkplug B codec (services/sparkplug): the topic builder, the Metric_

|   # | Test                            | Status | Description                                          |
| --: | :------------------------------ | :----: | :--------------------------------------------------- |
|   1 | `test_spb_error_and_kind_paths` |   ✅   | Spb error and kind paths                             |
|   2 | `test_topic`                    |   ✅   | Topic                                                |
|   3 | `test_metric_bytes`             |   ✅   | Metric bytes                                         |
|   4 | `test_payload_round_trip`       |   ✅   | Payload round trip                                   |
|   5 | `test_metric_int_and_string`    |   ✅   | skip name + datatype, read the int value (field 10). |
|   6 | `test_metric_alias`             |   ✅   | Metric alias                                         |
|   7 | `test_overflow_fails_closed`    |   ✅   | Overflow fails closed                                |

</details>

---

## test_modbus_master - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Modbus master codec (services/modbus/modbus_master): request_

|   # | Test                           | Status | Description                                                             |
| --: | :----------------------------- | :----: | :---------------------------------------------------------------------- |
|   1 | `test_build_read_bytes`        |   ✅   | Build read bytes                                                        |
|   2 | `test_build_rejects_bad_args`  |   ✅   | Build rejects bad args                                                  |
|   3 | `test_round_trip_holding_regs` |   ✅   | Round trip holding regs                                                 |
|   4 | `test_round_trip_exception`    |   ✅   | Read a wildly out-of-range address: the slave returns an exception ADU. |
|   5 | `test_parse_short_frame_fails` |   ✅   | Parse short frame fails                                                 |

</details>

---

## test_ota_rollback - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the OTA rollback decision (services/ota_rollback). The esp_ota_

|   # | Test                                     | Status | Description                                                 |
| --: | :--------------------------------------- | :----: | :---------------------------------------------------------- |
|   1 | `test_not_pending_waits`                 |   ✅   | A normally-booted (valid/undefined) image never rolls back. |
|   2 | `test_pending_self_test_ok_commits`      |   ✅   | Pending self test ok commits                                |
|   3 | `test_pending_within_window_waits`       |   ✅   | Pending within window waits                                 |
|   4 | `test_pending_window_elapsed_rolls_back` |   ✅   | Pending window elapsed rolls back                           |
|   5 | `test_self_test_ok_beats_window`         |   ✅   | A passing self-test commits even past the window.           |

</details>

---

## test_totp - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for TOTP (services/totp): the RFC 6238 Appendix B test vectors_

|   # | Test                          | Status | Description                                           |
| --: | :---------------------------- | :----: | :---------------------------------------------------- |
|   1 | `test_rfc6238_vectors`        |   ✅   | RFC 6238 Appendix B (SHA-1, T0=0, step=30, digits=8). |
|   2 | `test_verify_window`          |   ✅   | Verify window                                         |
|   3 | `test_base32_decode`          |   ✅   | Base32 decode                                         |
|   4 | `test_base32_rejects_invalid` |   ✅   | Base32 rejects invalid                                |

</details>

---

## test_webhook - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the webhook builders (services/webhook): IFTTT URL + payload_

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_ifttt_url`             |   ✅   | Ifttt url             |
|   2 | `test_payload_three_values`  |   ✅   | Payload three values  |
|   3 | `test_payload_omits_nulls`   |   ✅   | Payload omits nulls   |
|   4 | `test_payload_escapes_json`  |   ✅   | Payload escapes json  |
|   5 | `test_overflow_fails_closed` |   ✅   | Overflow fails closed |

</details>

---

## test_radio_power - ✅ 2 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the radio-power mode names (services/radio_power). Applying the_

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_ps_names`              |   ✅   | Ps names              |
|   2 | `test_apply_is_noop_on_host` |   ✅   | Apply is noop on host |

</details>

---

## test_dns_resolver - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the DNS answer classifier / verifier (services/dns_resolver)._

|   # | Test                             | Status | Description               |
| --: | :------------------------------- | :----: | :------------------------ |
|   1 | `test_classify`                  |   ✅   | Classify                  |
|   2 | `test_verify_rejects_suspicious` |   ✅   | Verify rejects suspicious |
|   3 | `test_verify_accepts_plausible`  |   ✅   | Verify accepts plausible  |
|   4 | `test_resolve_is_noop_on_host`   |   ✅   | Resolve is noop on host   |

</details>

---

## test_audit_log - ✅ 16 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the hash-chained audit log (services/audit_log). Verify the_

|   # | Test                                         | Status | Description                           |
| --: | :------------------------------------------- | :----: | :------------------------------------ |
|   1 | `test_append_assigns_monotonic_seq`          |   ✅   | Append assigns monotonic seq          |
|   2 | `test_chain_verifies_when_untouched`         |   ✅   | Chain verifies when untouched         |
|   3 | `test_tampered_message_breaks_chain`         |   ✅   | Tampered message breaks chain         |
|   4 | `test_tampered_hash_breaks_chain`            |   ✅   | Tampered hash breaks chain            |
|   5 | `test_tampered_category_breaks_chain`        |   ✅   | Tampered category breaks chain        |
|   6 | `test_ring_evicts_oldest_and_still_verifies` |   ✅   | Ring evicts oldest and still verifies |
|   7 | `test_tamper_after_wrap_detected_at_oldest`  |   ✅   | Tamper after wrap detected at oldest  |
|   8 | `test_reset_clears_everything`               |   ✅   | Reset clears everything               |
|   9 | `test_sink_receives_each_record`             |   ✅   | Sink receives each record             |
|  10 | `test_format_and_dump_json`                  |   ✅   | Format and dump json                  |
|  11 | `test_dump_json_reports_broken_chain`        |   ✅   | Dump json reports broken chain        |
|  12 | `test_format_fails_closed_on_small_buffer`   |   ✅   | Format fails closed on small buffer   |
|  13 | `test_null_msg_and_categories`               |   ✅   | Null msg and categories               |
|  14 | `test_json_escape_all_chars`                 |   ✅   | Json escape all chars                 |
|  15 | `test_format_fails_closed_all_stages`        |   ✅   | Format fails closed all stages        |
|  16 | `test_dump_fails_closed_all_stages`          |   ✅   | Dump fails closed all stages          |

</details>

---

## test_oidc - ✅ 17 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the OIDC RS256 ID-token verifier (services/oidc). Vectors are_

|   # | Test                                 | Status | Description                                                               |
| --: | :----------------------------------- | :----: | :------------------------------------------------------------------------ |
|   1 | `test_jwks_malformed_keys`           |   ✅   | Jwks malformed keys                                                       |
|   2 | `test_token_kid_guards`              |   ✅   | Token kid guards                                                          |
|   3 | `test_jwks_find_guards`              |   ✅   | Jwks find guards                                                          |
|   4 | `test_verify_guards_and_malformed`   |   ✅   | Verify guards and malformed                                               |
|   5 | `test_token_kid`                     |   ✅   | Token kid                                                                 |
|   6 | `test_jwks_find`                     |   ✅   | Jwks find                                                                 |
|   7 | `test_jwks_find_missing_kid_fails`   |   ✅   | Jwks find missing kid fails                                               |
|   8 | `test_verify_valid_token_and_claims` |   ✅   | Verify valid token and claims                                             |
|   9 | `test_verify_aud_array`              |   ✅   | Verify aud array                                                          |
|  10 | `test_reject_expired`                |   ✅   | Reject expired                                                            |
|  11 | `test_reject_wrong_issuer`           |   ✅   | Reject wrong issuer                                                       |
|  12 | `test_reject_wrong_audience`         |   ✅   | Reject wrong audience                                                     |
|  13 | `test_reject_non_rs256_header`       |   ✅   | Reject non rs256 header                                                   |
|  14 | `test_reject_tampered_payload`       |   ✅   | Reject tampered payload                                                   |
|  15 | `test_reject_tampered_signature`     |   ✅   | Reject tampered signature                                                 |
|  16 | `test_reject_unknown_key`            |   ✅   | JWKS whose only key has a different kid than the token's.                 |
|  17 | `test_reject_malformed`              |   ✅   | No kid extractable -> the sole JWKS key is selected, then the token shape |

</details>

---

## test_vfs - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the unified VFS (services/vfs) exercised through its built-in_

|   # | Test                                      | Status | Description                        |
| --: | :---------------------------------------- | :----: | :--------------------------------- |
|   1 | `test_write_then_read_file`               |   ✅   | Write then read file               |
|   2 | `test_streamed_write_and_read`            |   ✅   | Streamed write and read            |
|   3 | `test_write_mode_truncates`               |   ✅   | Write mode truncates               |
|   4 | `test_append_extends`                     |   ✅   | Append extends                     |
|   5 | `test_remove_and_rename`                  |   ✅   | Remove and rename                  |
|   6 | `test_missing_file_fails_closed`          |   ✅   | Missing file fails closed          |
|   7 | `test_read_buffer_too_small_fails_closed` |   ✅   | Read buffer too small fails closed |
|   8 | `test_file_full_is_bounded`               |   ✅   | File full is bounded               |
|   9 | `test_file_pool_exhaustion`               |   ✅   | File pool exhaustion               |
|  10 | `test_handle_pool_exhaustion`             |   ✅   | Handle pool exhaustion             |
|  11 | `test_unmounted_fails_closed`             |   ✅   | Unmounted fails closed             |

</details>

---

## test_graphql - ✅ 32 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the GraphQL query subset (services/graphql): selection shaping,_

|   # | Test                                   | Status | Description                                                                   |
| --: | :------------------------------------- | :----: | :---------------------------------------------------------------------------- |
|   1 | `test_malformed_tokens_fail`           |   ✅   | Malformed tokens fail                                                         |
|   2 | `test_query_keyword_forms_fail`        |   ✅   | Query keyword forms fail                                                      |
|   3 | `test_pool_limits`                     |   ✅   | Pool limits                                                                   |
|   4 | `test_string_pool_exhaustion`          |   ✅   | String pool exhaustion                                                        |
|   5 | `test_resolver_null_typed_value`       |   ✅   | Resolver null typed value                                                     |
|   6 | `test_resolver_path_overflow`          |   ✅   | 31,31,31,31: the 4th separator check trips (plen reaches 95, then '.' -> 96). |
|   7 | `test_arg_accessors_edges`             |   ✅   | Arg accessors edges                                                           |
|   8 | `test_flat_selection`                  |   ✅   | Flat selection                                                                |
|   9 | `test_string_escapes_decoded`          |   ✅   | \n \t \r \\ \/ and an unknown escape (\z) are all decoded by the arg lexer.   |
|  10 | `test_number_arg_variants_parse`       |   ✅   | float, exponent, signed-exponent and negative-int argument values all parse   |
|  11 | `test_bool_args`                       |   ✅   | Bool args                                                                     |
|  12 | `test_null_arg_value`                  |   ✅   | `null` parses; greet's name arg is then not a string, so it stays "?".        |
|  13 | `test_control_char_is_unicode_escaped` |   ✅   | Control char is unicode escaped                                               |
|  14 | `test_unterminated_string_arg_fails`   |   ✅   | Unterminated string arg fails                                                 |
|  15 | `test_arg_missing_colon_fails`         |   ✅   | Arg missing colon fails                                                       |
|  16 | `test_bad_arg_value_fails`             |   ✅   | Bad arg value fails                                                           |
|  17 | `test_trailing_junk_fails`             |   ✅   | Trailing junk fails                                                           |
|  18 | `test_long_field_name_hits_limit`      |   ✅   | Long field name hits limit                                                    |
|  19 | `test_null_inputs_fail_closed`         |   ✅   | Null inputs fail closed                                                       |
|  20 | `test_unknown_operation_keyword_fails` |   ✅   | Unknown operation keyword fails                                               |
|  21 | `test_selection_is_honored`            |   ✅   | Only the requested field appears.                                             |
|  22 | `test_nested_object`                   |   ✅   | Nested object                                                                 |
|  23 | `test_args_collected_along_path`       |   ✅   | `id` is on the object `sensor`; the leaf resolver `sensor.value` reads it.    |
|  24 | `test_scalar_types`                    |   ✅   | Scalar types                                                                  |
|  25 | `test_string_arg_and_escaping`         |   ✅   | String arg is decoded, and the resolver's output string is JSON-escaped.      |
|  26 | `test_unresolved_field_is_null`        |   ✅   | Unresolved field is null                                                      |
|  27 | `test_query_keyword_and_name`          |   ✅   | Query keyword and name                                                        |
|  28 | `test_comments_and_commas`             |   ✅   | Comments and commas                                                           |
|  29 | `test_parse_error_reports_errors`      |   ✅   | Parse error reports errors                                                    |
|  30 | `test_mutation_rejected`               |   ✅   | Mutation rejected                                                             |
|  31 | `test_depth_limit`                     |   ✅   | Depth limit                                                                   |
|  32 | `test_overflow_fails_closed`           |   ✅   | Overflow fails closed                                                         |

</details>

---

## test_espnow - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the ESP-NOW host-testable core (services/espnow): the typed_

|   # | Test                                            | Status | Description                              |
| --: | :---------------------------------------------- | :----: | :--------------------------------------- |
|   1 | `test_encode_decode_roundtrip`                  |   ✅   | Encode decode roundtrip                  |
|   2 | `test_encode_zero_length`                       |   ✅   | Encode zero length                       |
|   3 | `test_encode_rejects_oversize_and_small_buffer` |   ✅   | Encode rejects oversize and small buffer |
|   4 | `test_decode_rejects_corrupt`                   |   ✅   | bad magic                                |
|   5 | `test_peer_registry`                            |   ✅   | Peer registry                            |
|   6 | `test_peer_table_full_fails_closed`             |   ✅   | Peer table full fails closed             |
|   7 | `test_broadcast_address`                        |   ✅   | Broadcast address                        |

</details>

---

## test_oauth2 - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the OAuth2 token-endpoint client core (services/oauth2): building_

|   # | Test                                                   | Status | Description                                                           |
| --: | :----------------------------------------------------- | :----: | :-------------------------------------------------------------------- |
|   1 | `test_build_code_request_minimal`                      |   ✅   | Build code request minimal                                            |
|   2 | `test_build_code_request_with_secret_encodes_specials` |   ✅   | Build code request with secret encodes specials                       |
|   3 | `test_build_code_request_pkce`                         |   ✅   | Build code request pkce                                               |
|   4 | `test_build_refresh_request`                           |   ✅   | Build refresh request                                                 |
|   5 | `test_build_overflows_fail_closed`                     |   ✅   | Build overflows fail closed                                           |
|   6 | `test_parse_token_response`                            |   ✅   | Parse token response                                                  |
|   7 | `test_parse_minimal_response`                          |   ✅   | Only access_token present: still valid; optional fields stay empty/0. |
|   8 | `test_parse_error_response_fails`                      |   ✅   | Parse error response fails                                            |

</details>

---

## test_opcua - ✅ 38 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for OPC UA (services/opcua): the Binary built-in type codec (incl._

|   # | Test                                   | Status | Description                                                                 |
| --: | :------------------------------------- | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_parse_read_optional_fields`      |   ✅   | Parse read optional fields                                                  |
|   2 | `test_parse_rejections`                |   ✅   | Parse rejections                                                            |
|   3 | `test_build_guards_and_overflow`       |   ✅   | Build guards and overflow                                                   |
|   4 | `test_setters_and_endpoint_url`        |   ✅   | Setters and endpoint url                                                    |
|   5 | `test_variant_scalar_types`            |   ✅   | Variant scalar types                                                        |
|   6 | `test_variant_errors`                  |   ✅   | Variant errors                                                              |
|   7 | `test_datavalue_all_masks`             |   ✅   | Datavalue all masks                                                         |
|   8 | `test_nodeid_encodings`                |   ✅   | Nodeid encodings                                                            |
|   9 | `test_reader_underruns`                |   ✅   | Reader underruns                                                            |
|  10 | `test_codec_roundtrip`                 |   ✅   | Codec roundtrip                                                             |
|  11 | `test_string_null_roundtrip`           |   ✅   | String null roundtrip                                                       |
|  12 | `test_reader_underrun_latches`         |   ✅   | Reader underrun latches                                                     |
|  13 | `test_writer_overflow_fails_closed`    |   ✅   | Writer overflow fails closed                                                |
|  14 | `test_parse_header`                    |   ✅   | Parse header                                                                |
|  15 | `test_parse_hello`                     |   ✅   | Parse hello                                                                 |
|  16 | `test_parse_hello_rejects_short`       |   ✅   | Parse hello rejects short                                                   |
|  17 | `test_build_ack_negotiates`            |   ✅   | Build ack negotiates                                                        |
|  18 | `test_nodeid_roundtrip`                |   ✅   | Nodeid roundtrip                                                            |
|  19 | `test_filetime_from_unix`              |   ✅   | Filetime from unix                                                          |
|  20 | `test_parse_open`                      |   ✅   | Parse open                                                                  |
|  21 | `test_parse_open_rejects_wrong_type`   |   ✅   | Corrupt the message type so it is no longer "OPN".                          |
|  22 | `test_build_open_response`             |   ✅   | Build open response                                                         |
|  23 | `test_parse_msg`                       |   ✅   | Parse msg                                                                   |
|  24 | `test_parse_msg_rejects_non_msg`       |   ✅   | Parse msg rejects non msg                                                   |
|  25 | `test_build_create_session_response`   |   ✅   | Build create session response                                               |
|  26 | `test_build_activate_session_response` |   ✅   | Build activate session response                                             |
|  27 | `test_datavalue_good_int32`            |   ✅   | Datavalue good int32                                                        |
|  28 | `test_datavalue_bad_status`            |   ✅   | Datavalue bad status                                                        |
|  29 | `test_parse_read`                      |   ✅   | Parse read                                                                  |
|  30 | `test_build_read_response`             |   ✅   | Build read response                                                         |
|  31 | `test_parse_browse`                    |   ✅   | Parse browse                                                                |
|  32 | `test_build_browse_response`           |   ✅   | Build browse response                                                       |
|  33 | `test_build_browse_response_unknown`   |   ✅   | Build browse response unknown                                               |
|  34 | `test_build_close_session_response`    |   ✅   | Build close session response                                                |
|  35 | `test_build_get_endpoints`             |   ✅   | Build get endpoints                                                         |
|  36 | `test_build_service_fault`             |   ✅   | Build service fault                                                         |
|  37 | `test_datavalue_roundtrip`             |   ✅   | Datavalue roundtrip                                                         |
|  38 | `test_parse_and_build_write`           |   ✅   | Build a WriteRequest writing one Int32 to ns=1;i=10 (value-only DataValue). |

</details>

---

## test_opcua_client - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Round-trip tests for the OPC UA client (services/opcua_client): the client builds_

|   # | Test                                     | Status | Description                                                                            |
| --: | :--------------------------------------- | :----: | :------------------------------------------------------------------------------------- |
|   1 | `test_on_read_all_variant_types`         |   ✅   | On read all variant types                                                              |
|   2 | `test_client_parsers_reject_fault`       |   ✅   | Client parsers reject fault                                                            |
|   3 | `test_client_parsers_reject_malformed`   |   ✅   | Client parsers reject malformed                                                        |
|   4 | `test_hello_ack_roundtrip`               |   ✅   | Hello ack roundtrip                                                                    |
|   5 | `test_open_roundtrip`                    |   ✅   | Open roundtrip                                                                         |
|   6 | `test_session_roundtrip`                 |   ✅   | Session roundtrip                                                                      |
|   7 | `test_get_endpoints_roundtrip`           |   ✅   | Get endpoints roundtrip                                                                |
|   8 | `test_service_fault_rejected_by_parsers` |   ✅   | An unknown service draws a ServiceFault; a typed parser must reject it (wrong TypeId). |
|   9 | `test_read_roundtrip`                    |   ✅   | Read roundtrip                                                                         |
|  10 | `test_browse_roundtrip`                  |   ✅   | Browse roundtrip                                                                       |
|  11 | `test_write_roundtrip`                   |   ✅   | Write roundtrip                                                                        |
|  12 | `test_close_session_roundtrip`           |   ✅   | Close session roundtrip                                                                |
|  13 | `test_close_channel_is_clo`              |   ✅   | Close channel is clo                                                                   |
|  14 | `test_seq_and_request_id_increment`      |   ✅   | Seq and request id increment                                                           |

</details>

---

## test_syslog - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the RFC 5424 syslog client (syslog_format formatter + syslog_init /_

|   # | Test                                | Status | Description                  |
| --: | :---------------------------------- | :----: | :--------------------------- |
|   1 | `test_pri_local0_info`              |   ✅   | Pri local0 info              |
|   2 | `test_pri_computation_varies`       |   ✅   | daemon(3)*8 + err(3) = 27    |
|   3 | `test_nilvalue_for_empty_fields`    |   ✅   | Nilvalue for empty fields    |
|   4 | `test_empty_message_ok`             |   ✅   | Empty message ok             |
|   5 | `test_overflow_returns_zero`        |   ✅   | Overflow returns zero        |
|   6 | `test_length_matches_strlen`        |   ✅   | Length matches strlen        |
|   7 | `test_init_and_log_captured`        |   ✅   | Init and log captured        |
|   8 | `test_log_not_ready_when_no_server` |   ✅   | Log not ready when no server |
|   9 | `test_format_null_and_pri_clamp`    |   ✅   | Format null and pri clamp    |
|  10 | `test_init_truncates_long_fields`   |   ✅   | Init truncates long fields   |

</details>

---

## test_jwt - ✅ 16 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the JWT HS256 verifier. The reference token below was produced_

|   # | Test                              | Status | Description                           |
| --: | :-------------------------------- | :----: | :------------------------------------ |
|   1 | `test_base64url_strict_alphabet`  |   ✅   | URL-safe characters decode.           |
|   2 | `test_verify_malformed_headers`   |   ✅   | A third dot is not a valid JWT shape. |
|   3 | `test_bearer_extra_spaces`        |   ✅   | Bearer extra spaces                   |
|   4 | `test_claim_int_edges`            |   ✅   | Claim int edges                       |
|   5 | `test_claim_str_edges`            |   ✅   | Claim str edges                       |
|   6 | `test_valid_token_accepts`        |   ✅   | Valid token accepts                   |
|   7 | `test_wrong_secret_rejects`       |   ✅   | Wrong secret rejects                  |
|   8 | `test_tampered_payload_rejects`   |   ✅   | Tampered payload rejects              |
|   9 | `test_tampered_signature_rejects` |   ✅   | Tampered signature rejects            |
|  10 | `test_malformed_rejected`         |   ✅   | Malformed rejected                    |
|  11 | `test_alg_not_hs256_rejected`     |   ✅   | Alg not hs256 rejected                |
|  12 | `test_bearer_header`              |   ✅   | Bearer header                         |
|  13 | `test_claim_int`                  |   ✅   | Claim int                             |
|  14 | `test_claim_missing`              |   ✅   | Claim missing                         |
|  15 | `test_claim_str`                  |   ✅   | Claim str                             |
|  16 | `test_scope_allows`               |   ✅   | Scope allows                          |

</details>

---

## test_http_client - ✅ 15 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the outbound HTTP client's pure core: URL parsing, request_

|   # | Test                                       | Status | Description                                                        |
| --: | :----------------------------------------- | :----: | :----------------------------------------------------------------- |
|   1 | `test_url_edge_rejections`                 |   ✅   | Url edge rejections                                                |
|   2 | `test_build_edge_rejections`               |   ✅   | Build edge rejections                                              |
|   3 | `test_response_edge_rejections`            |   ✅   | Response edge rejections                                           |
|   4 | `test_host_transport_stubs`                |   ✅   | Host transport stubs                                               |
|   5 | `test_url_http_default`                    |   ✅   | Url http default                                                   |
|   6 | `test_url_https_port_nopath`               |   ✅   | Url https port nopath                                              |
|   7 | `test_url_bad_scheme`                      |   ✅   | Url bad scheme                                                     |
|   8 | `test_build_get`                           |   ✅   | Build get                                                          |
|   9 | `test_build_post_with_body_and_port`       |   ✅   | Build post with body and port                                      |
|  10 | `test_parse_content_length`                |   ✅   | Parse content length                                               |
|  11 | `test_parse_status_404`                    |   ✅   | Parse status 404                                                   |
|  12 | `test_parse_chunked`                       |   ✅   | two chunks "Wiki" (4) + "pedia" (5) -> "Wikipedia"                 |
|  13 | `test_parse_chunked_oversize_size_clamped` |   ✅   | Parse chunked oversize size clamped                                |
|  14 | `test_parse_connection_close_body`         |   ✅   | No Content-Length / chunked: body is everything after the headers. |
|  15 | `test_parse_malformed`                     |   ✅   | Parse malformed                                                    |

</details>

---

## test_compliance - ✅ 15 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_RFC-compliance suite. Built with production enforcement defaults_

|   # | Test                                                  | Status | Description                                                               |
| --: | :---------------------------------------------------- | :----: | :------------------------------------------------------------------------ |
|   1 | `test_http11_missing_host_rejected`                   |   ✅   | Http11 missing host rejected                                              |
|   2 | `test_http11_with_host_ok`                            |   ✅   | Http11 with host ok                                                       |
|   3 | `test_http10_missing_host_ok`                         |   ✅   | Host is not required for HTTP/1.0.                                        |
|   4 | `test_duplicate_host_rejected`                        |   ✅   | Duplicate host rejected                                                   |
|   5 | `test_duplicate_host_rejected_http10`                 |   ✅   | More than one Host is invalid regardless of version.                      |
|   6 | `test_host_beyond_max_headers_still_counted`          |   ✅   | A valid Host that appears after MAX_HEADERS other fields is still counted |
|   7 | `test_duplicate_host_with_one_beyond_cap_rejected`    |   ✅   | First Host is stored; a second Host pushed past MAX_HEADERS must still be |
|   8 | `test_content_length_non_digit_rejected`              |   ✅   | Content length non digit rejected                                         |
|   9 | `test_content_length_empty_rejected`                  |   ✅   | Content length empty rejected                                             |
|  10 | `test_content_length_conflicting_duplicate_rejected`  |   ✅   | Content length conflicting duplicate rejected                             |
|  11 | `test_content_length_matching_duplicate_ok`           |   ✅   | Two identical Content-Length values are not a conflict.                   |
|  12 | `test_content_length_valid_body`                      |   ✅   | Content length valid body                                                 |
|  13 | `test_transfer_encoding_chunked_rejected`             |   ✅   | Transfer encoding chunked rejected                                        |
|  14 | `test_transfer_encoding_with_content_length_rejected` |   ✅   | CL + TE present: the classic CL.TE smuggling desync - must be rejected.   |
|  15 | `test_transfer_encoding_case_insensitive_rejected`    |   ✅   | Header-name match must be case-insensitive (RFC 7230 §3.2).               |

</details>

---

## test_mqtt - ✅ 22 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host unit tests for the MQTT 3.1.1 packet codec (env:native_mqtt)._

|   # | Test                                          | Status | Description                                                                 |
| --: | :-------------------------------------------- | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_build_guards_and_overflow`              |   ✅   | Build guards and overflow                                                   |
|   2 | `test_parse_guards`                           |   ✅   | Parse guards                                                                |
|   3 | `test_host_transport_stubs`                   |   ✅   | Host transport stubs                                                        |
|   4 | `test_remlen_boundaries`                      |   ✅   | Remlen boundaries                                                           |
|   5 | `test_remlen_too_big`                         |   ✅   | Remlen too big                                                              |
|   6 | `test_remlen_decode_incomplete`               |   ✅   | Remlen decode incomplete                                                    |
|   7 | `test_remlen_decode_malformed`                |   ✅   | Remlen decode malformed                                                     |
|   8 | `test_connect_minimal`                        |   ✅   | Connect minimal                                                             |
|   9 | `test_connect_full`                           |   ✅   | Connect full                                                                |
|  10 | `test_publish_qos0_roundtrip`                 |   ✅   | Publish qos0 roundtrip                                                      |
|  11 | `test_publish_qos1_flags_and_id`              |   ✅   | Publish qos1 flags and id                                                   |
|  12 | `test_publish_topic_overflow_rejected`        |   ✅   | Publish topic overflow rejected                                             |
|  13 | `test_publish_qos3_rejected`                  |   ✅   | Publish qos3 rejected                                                       |
|  14 | `test_publish_wildcard_topic_rejected`        |   ✅   | Publish wildcard topic rejected                                             |
|  15 | `test_publish_topic_nul_or_bad_utf8_rejected` |   ✅   | topic length 2, bytes {0xC3,0x28} = invalid UTF-8 sequence, qos0 (flags 0). |
|  16 | `test_subscribe`                              |   ✅   | Subscribe                                                                   |
|  17 | `test_unsubscribe`                            |   ✅   | Unsubscribe                                                                 |
|  18 | `test_ack_packets`                            |   ✅   | Ack packets                                                                 |
|  19 | `test_connack`                                |   ✅   | Connack                                                                     |
|  20 | `test_suback`                                 |   ✅   | Suback                                                                      |
|  21 | `test_ping_disconnect`                        |   ✅   | Ping disconnect                                                             |
|  22 | `test_fixed_header_multibyte_remlen`          |   ✅   | Remaining length 300 -> 2-byte field {0xAC, 0x02}.                          |

</details>

---

## test_ws_client - ✅ 16 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host unit tests for the outbound WebSocket client codec (env:native_ws_client)._

|   # | Test                                | Status | Description                           |
| --: | :---------------------------------- | :----: | :------------------------------------ |
|   1 | `test_accept_for_key_guards`        |   ✅   | Accept for key guards                 |
|   2 | `test_build_handshake_guards`       |   ✅   | Build handshake guards                |
|   3 | `test_check_response_guards`        |   ✅   | Check response guards                 |
|   4 | `test_build_frame_guards_and_64bit` |   ✅   | Build frame guards and 64bit          |
|   5 | `test_parse_frame_edges`            |   ✅   | Parse frame edges                     |
|   6 | `test_host_transport_stubs`         |   ✅   | Host transport stubs                  |
|   7 | `test_accept_rfc_example`           |   ✅   | Accept rfc example                    |
|   8 | `test_build_handshake`              |   ✅   | Build handshake                       |
|   9 | `test_check_response_ok`            |   ✅   | Check response ok                     |
|  10 | `test_check_response_bad_accept`    |   ✅   | Check response bad accept             |
|  11 | `test_check_response_not_101`       |   ✅   | Check response not 101                |
|  12 | `test_build_frame_masked`           |   ✅   | Build frame masked                    |
|  13 | `test_build_frame_extended_len`     |   ✅   | Build frame extended len              |
|  14 | `test_parse_frame_server_text`      |   ✅   | Server (unmasked) text frame "hello". |
|  15 | `test_parse_frame_incomplete`       |   ✅   | Parse frame incomplete                |
|  16 | `test_parse_frame_extended_len`     |   ✅   | Parse frame extended len              |

</details>

---

## test_scratch - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the shared per-dispatch scratch arena (session/scratch): bump_

|   # | Test                                                    | Status | Description                                                           |
| --: | :------------------------------------------------------ | :----: | :-------------------------------------------------------------------- |
|   1 | `test_alloc_returns_nonnull_and_advances_used`          |   ✅   | Alloc returns nonnull and advances used                               |
|   2 | `test_sequential_allocs_are_distinct_and_ordered`       |   ✅   | Sequential allocs are distinct and ordered                            |
|   3 | `test_reset_frees_all_and_reuses_base`                  |   ✅   | Reset frees all and reuses base                                       |
|   4 | `test_alignment_is_honored`                             |   ✅   | Alignment is honored                                                  |
|   5 | `test_exhaustion_returns_null_without_corrupting_arena` |   ✅   | Exhaustion returns null without corrupting arena                      |
|   6 | `test_alloc_larger_than_capacity_returns_null`          |   ✅   | Alloc larger than capacity returns null                               |
|   7 | `test_alignment_padding_cannot_overflow_arena`          |   ✅   | Fill to one byte below capacity, then a large-alignment request whose |
|   8 | `test_high_water_bounds`                                |   ✅   | High water bounds                                                     |
|   9 | `test_zero_size_alloc_returns_nonnull_when_space`       |   ✅   | Zero size alloc returns nonnull when space                            |
|  10 | `test_mark_release_reclaims`                            |   ✅   | Mark release reclaims                                                 |
|  11 | `test_release_allows_reuse_of_same_region`              |   ✅   | Release allows reuse of same region                                   |
|  12 | `test_scratch_scope_releases_on_scope_exit`             |   ✅   | Scratch scope releases on scope exit                                  |
|  13 | `test_nested_scopes_reclaim_lifo`                       |   ✅   | Nested scopes reclaim lifo                                            |
|  14 | `test_sequential_scopes_do_not_accumulate`              |   ✅   | Mirrors ssh_pkt_recv's multi-packet loop: each iteration borrows then |

</details>

---

## test_snmp_trap - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host unit tests for the outbound SNMP notification builder (env:native_snmp_trap)._

|   # | Test                        | Status | Description          |
| --: | :-------------------------- | :----: | :------------------- |
|   1 | `test_trap_v2c_structure`   |   ✅   | Trap v2c structure   |
|   2 | `test_all_varbind_types`    |   ✅   | All varbind types    |
|   3 | `test_invalid_varbind_type` |   ✅   | Invalid varbind type |
|   4 | `test_build_v2c_null_args`  |   ✅   | Build v2c null args  |
|   5 | `test_host_transport_stubs` |   ✅   | Host transport stubs |
|   6 | `test_inform_tag`           |   ✅   | Inform tag           |
|   7 | `test_buffer_too_small`     |   ✅   | Buffer too small     |

</details>

---

## test_inflate - ✅ 12 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the RFC 1951 INFLATE core (network_drivers/presentation/inflate)._

|   # | Test                                      | Status | Description                                                            |
| --: | :---------------------------------------- | :----: | :--------------------------------------------------------------------- |
|   1 | `test_fixed_huffman`                      |   ✅   | Fixed huffman                                                          |
|   2 | `test_back_references`                    |   ✅   | Back references                                                        |
|   3 | `test_stored_block`                       |   ✅   | Stored block                                                           |
|   4 | `test_dynamic_huffman`                    |   ✅   | Dynamic huffman                                                        |
|   5 | `test_empty_message`                      |   ✅   | Empty message                                                          |
|   6 | `test_permessage_deflate_marker`          |   ✅   | Permessage deflate marker                                              |
|   7 | `test_permessage_deflate_back_references` |   ✅   | Permessage deflate back references                                     |
|   8 | `test_output_overflow_fails_closed`       |   ✅   | Output overflow fails closed                                           |
|   9 | `test_scratch_too_small_fails_closed`     |   ✅   | Scratch too small fails closed                                         |
|  10 | `test_truncated_input_is_malformed`       |   ✅   | Half of the fixed-Huffman stream: decode runs out of input mid-symbol. |
|  11 | `test_reserved_block_type_is_malformed`   |   ✅   | Reserved block type is malformed                                       |
|  12 | `test_corrupt_stored_nlen_is_malformed`   |   ✅   | Corrupt stored nlen is malformed                                       |

</details>

---

## test_deflate - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the RFC 1951 DEFLATE core (network_drivers/presentation/deflate)._

|   # | Test                                  | Status | Description                                                                 |
| --: | :------------------------------------ | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_roundtrip_text`                 |   ✅   | Roundtrip text                                                              |
|   2 | `test_roundtrip_empty`                |   ✅   | Roundtrip empty                                                             |
|   3 | `test_roundtrip_single_byte`          |   ✅   | Roundtrip single byte                                                       |
|   4 | `test_roundtrip_all_byte_values`      |   ✅   | Roundtrip all byte values                                                   |
|   5 | `test_compresses_repetitive`          |   ✅   | Compresses repetitive                                                       |
|   6 | `test_compresses_json`                |   ✅   | Compresses json                                                             |
|   7 | `test_fuzz_roundtrip`                 |   ✅   | Fuzz roundtrip                                                              |
|   8 | `test_fuzz_low_entropy_roundtrip`     |   ✅   | Fuzz low entropy roundtrip                                                  |
|   9 | `test_output_overflow_fails_closed`   |   ✅   | Incompressible data into a too-small buffer must report overflow, not write |
|  10 | `test_scratch_too_small_fails_closed` |   ✅   | Scratch too small fails closed                                              |

</details>

---

## test_time_source - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the multi-source time fallback matrix (services/time_source):_

|   # | Test                                       | Status | Description                         |
| --: | :----------------------------------------- | :----: | :---------------------------------- |
|   1 | `test_single_source`                       |   ✅   | Single source                       |
|   2 | `test_priority_order_lowest_value_wins`    |   ✅   | Priority order lowest value wins    |
|   3 | `test_falls_back_when_primary_unavailable` |   ✅   | Falls back when primary unavailable |
|   4 | `test_all_unavailable_returns_zero`        |   ✅   | All unavailable returns zero        |
|   5 | `test_first_valid_short_circuits`          |   ✅   | First valid short circuits          |
|   6 | `test_fallback_queries_in_priority_order`  |   ✅   | Fallback queries in priority order  |
|   7 | `test_table_full_rejects`                  |   ✅   | Table full rejects                  |
|   8 | `test_null_fn_rejected`                    |   ✅   | Null fn rejected                    |
|   9 | `test_reset_clears_sources`                |   ✅   | Reset clears sources                |

</details>

---

## test_config_store - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the typed NVS config store (services/config_store), exercised_

|   # | Test                                          | Status | Description                            |
| --: | :-------------------------------------------- | :----: | :------------------------------------- |
|   1 | `test_str_round_trip`                         |   ✅   | Str round trip                         |
|   2 | `test_str_default_when_missing`               |   ✅   | Str default when missing               |
|   3 | `test_str_overwrite`                          |   ✅   | Str overwrite                          |
|   4 | `test_str_truncates_to_capacity`              |   ✅   | Str truncates to capacity              |
|   5 | `test_u32_round_trip`                         |   ✅   | U32 round trip                         |
|   6 | `test_u32_default_when_missing`               |   ✅   | U32 default when missing               |
|   7 | `test_blob_round_trip`                        |   ✅   | Blob round trip                        |
|   8 | `test_blob_bounded_by_capacity`               |   ✅   | Blob bounded by capacity               |
|   9 | `test_blob_missing_returns_zero`              |   ✅   | Blob missing returns zero              |
|  10 | `test_erase_removes_key`                      |   ✅   | Erase removes key                      |
|  11 | `test_clear_wipes_namespace`                  |   ✅   | Clear wipes namespace                  |
|  12 | `test_table_full_rejects_new_key`             |   ✅   | Table full rejects new key             |
|  13 | `test_existing_key_overwrites_even_when_full` |   ✅   | Existing key overwrites even when full |
|  14 | `test_key_too_long_rejected`                  |   ✅   | Key too long rejected                  |

</details>

---

## test_device_id - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the MAC-derived device UUID (services/device_id). The expected_

|   # | Test                                       | Status | Description                         |
| --: | :----------------------------------------- | :----: | :---------------------------------- |
|   1 | `test_uuid_matches_reference_aabbccddeeff` |   ✅   | Uuid matches reference aabbccddeeff |
|   2 | `test_uuid_matches_reference_001122334455` |   ✅   | Uuid matches reference 001122334455 |
|   3 | `test_uuid_is_deterministic`               |   ✅   | Uuid is deterministic               |
|   4 | `test_uuid_version_and_variant_bits`       |   ✅   | Uuid version and variant bits       |

</details>

---

## test_auth_lockout - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the per-IP brute-force auth lockout (services/auth_lockout)._

|   # | Test                                    | Status | Description                      |
| --: | :-------------------------------------- | :----: | :------------------------------- |
|   1 | `test_below_threshold_not_locked`       |   ✅   | Below threshold not locked       |
|   2 | `test_locks_at_threshold`               |   ✅   | Locks at threshold               |
|   3 | `test_exponential_backoff`              |   ✅   | Exponential backoff              |
|   4 | `test_caps_at_max`                      |   ✅   | Caps at max                      |
|   5 | `test_expires_after_window`             |   ✅   | Expires after window             |
|   6 | `test_success_clears`                   |   ✅   | Success clears                   |
|   7 | `test_isolates_addresses`               |   ✅   | Isolates addresses               |
|   8 | `test_zero_ip_never_locked`             |   ✅   | Zero ip never locked             |
|   9 | `test_table_full_tracks_new_address`    |   ✅   | Table full tracks new address    |
|  10 | `test_active_lockout_survives_eviction` |   ✅   | Active lockout survives eviction |

</details>

---

## test_csrf - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the stateless HMAC-signed CSRF token (services/csrf). A fixed_

|   # | Test                                 | Status | Description                   |
| --: | :----------------------------------- | :----: | :---------------------------- |
|   1 | `test_issue_verify_roundtrip`        |   ✅   | Issue verify roundtrip        |
|   2 | `test_token_format_and_length`       |   ✅   | Token format and length       |
|   3 | `test_verify_rejects_tampered_sig`   |   ✅   | Verify rejects tampered sig   |
|   4 | `test_verify_rejects_tampered_nonce` |   ✅   | Verify rejects tampered nonce |
|   5 | `test_verify_rejects_garbage`        |   ✅   | Verify rejects garbage        |
|   6 | `test_different_secret_rejects`      |   ✅   | Different secret rejects      |
|   7 | `test_no_secret_fails_closed`        |   ✅   | No secret fails closed        |
|   8 | `test_issue_unique`                  |   ✅   | Issue unique                  |
|   9 | `test_issue_rejects_small_buffer`    |   ✅   | Issue rejects small buffer    |

</details>

---

## test_telemetry - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the telemetry math helpers (services/telemetry): moving-window_

|   # | Test                                 | Status | Description                   |
| --: | :----------------------------------- | :----: | :---------------------------- |
|   1 | `test_window_classic_stats`          |   ✅   | Window classic stats          |
|   2 | `test_window_empty`                  |   ✅   | Window empty                  |
|   3 | `test_window_single_sample`          |   ✅   | Window single sample          |
|   4 | `test_window_eviction`               |   ✅   | Window eviction               |
|   5 | `test_rate_basic`                    |   ✅   | Rate basic                    |
|   6 | `test_rate_zero_dt`                  |   ✅   | Rate zero dt                  |
|   7 | `test_totalizer_constant_rate`       |   ✅   | Totalizer constant rate       |
|   8 | `test_totalizer_trapezoid_and_reset` |   ✅   | Totalizer trapezoid and reset |

</details>

---

## test_dashboard - ✅ 15 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the dashboard widget-table JSON serializers (services/dashboard_

|   # | Test                                   | Status | Description                     |
| --: | :------------------------------------- | :----: | :------------------------------ |
|   1 | `test_layout_bar_sparkline_types`      |   ✅   | Layout bar sparkline types      |
|   2 | `test_null_widget_table_guards`        |   ✅   | Null widget table guards        |
|   3 | `test_json_overflow_paths`             |   ✅   | Json overflow paths             |
|   4 | `test_parse_control_edges`             |   ✅   | Parse control edges             |
|   5 | `test_layout_json`                     |   ✅   | Layout json                     |
|   6 | `test_values_json_initial_zero`        |   ✅   | Values json initial zero        |
|   7 | `test_set_and_values`                  |   ✅   | Set and values                  |
|   8 | `test_set_unknown_key`                 |   ✅   | Set unknown key                 |
|   9 | `test_configure_resets_values`         |   ✅   | Configure resets values         |
|  10 | `test_small_buffer_fails_closed`       |   ✅   | Small buffer fails closed       |
|  11 | `test_parse_control_ok`                |   ✅   | Parse control ok                |
|  12 | `test_parse_control_float`             |   ✅   | Parse control float             |
|  13 | `test_parse_control_rejects_malformed` |   ✅   | Parse control rejects malformed |
|  14 | `test_dispatch_control_invokes_cb`     |   ✅   | Dispatch control invokes cb     |
|  15 | `test_layout_control_types`            |   ✅   | Layout control types            |

</details>

---

## test_net_egress - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for egress-interface reporting (network_drivers/physical). The lwIP_

|   # | Test                    | Status | Description      |
| --: | :---------------------- | :----: | :--------------- |
|   1 | `test_classify_sta`     |   ✅   | Classify sta     |
|   2 | `test_classify_ap`      |   ✅   | Classify ap      |
|   3 | `test_classify_eth`     |   ✅   | Classify eth     |
|   4 | `test_classify_none`    |   ✅   | Classify none    |
|   5 | `test_egress_host_stub` |   ✅   | Egress host stub |
|   6 | `test_eth_host_stub`    |   ✅   | Eth host stub    |

</details>

---

## test_partition_monitor - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the partition-map core (services/partition_monitor): the_

|   # | Test                                  | Status | Description                    |
| --: | :------------------------------------ | :----: | :----------------------------- |
|   1 | `test_kind_app`                       |   ✅   | Kind app                       |
|   2 | `test_kind_data`                      |   ✅   | Kind data                      |
|   3 | `test_json`                           |   ✅   | Json                           |
|   4 | `test_json_small_buffer_fails_closed` |   ✅   | Json small buffer fails closed |
|   5 | `test_collect_host_stub`              |   ✅   | Collect host stub              |

</details>

---

## test_cbor - ✅ 18 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CBOR encoder (network_drivers/presentation/cbor). Expected_

|   # | Test                             | Status | Description               |
| --: | :------------------------------- | :----: | :------------------------ |
|   1 | `test_uint`                      |   ✅   | Uint                      |
|   2 | `test_peek_each_type`            |   ✅   | Peek each type            |
|   3 | `test_uint_8byte`                |   ✅   | Uint 8byte                |
|   4 | `test_read_double_encoded_float` |   ✅   | Read double encoded float |
|   5 | `test_read_map_type_mismatch`    |   ✅   | Read map type mismatch    |
|   6 | `test_int`                       |   ✅   | Int                       |
|   7 | `test_text`                      |   ✅   | Text                      |
|   8 | `test_bytes`                     |   ✅   | Bytes                     |
|   9 | `test_simple`                    |   ✅   | Simple                    |
|  10 | `test_float`                     |   ✅   | Float                     |
|  11 | `test_array_and_map`             |   ✅   | Array and map             |
|  12 | `test_overflow_fails_closed`     |   ✅   | Overflow fails closed     |
|  13 | `test_decode_uint`               |   ✅   | Decode uint               |
|  14 | `test_decode_int`                |   ✅   | Decode int                |
|  15 | `test_decode_float_roundtrip`    |   ✅   | Decode float roundtrip    |
|  16 | `test_decode_roundtrip_map`      |   ✅   | Decode roundtrip map      |
|  17 | `test_decode_truncated`          |   ✅   | Decode truncated          |
|  18 | `test_decode_type_mismatch`      |   ✅   | Decode type mismatch      |

</details>

---

## test_msgpack - ✅ 17 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the MessagePack encoder and decoder_

|   # | Test                            | Status | Description                                                                 |
| --: | :------------------------------ | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_uint`                     |   ✅   | Uint                                                                        |
|   2 | `test_wide_roundtrip`           |   ✅   | Wide roundtrip                                                              |
|   3 | `test_decode_wide_fails_closed` |   ✅   | str16 header claims 300 bytes, body absent                                  |
|   4 | `test_int`                      |   ✅   | Int                                                                         |
|   5 | `test_str`                      |   ✅   | Str                                                                         |
|   6 | `test_bytes`                    |   ✅   | Bytes                                                                       |
|   7 | `test_simple`                   |   ✅   | Simple                                                                      |
|   8 | `test_float`                    |   ✅   | Float                                                                       |
|   9 | `test_array_and_map`            |   ✅   | Array and map                                                               |
|  10 | `test_overflow_fails_closed`    |   ✅   | Overflow fails closed                                                       |
|  11 | `test_decode_uint`              |   ✅   | positive fixint, uint8, uint16, uint32, uint64                              |
|  12 | `test_decode_int`               |   ✅   | negative fixint (-1, -32), int8 (-128), int16 (-32768), int32 (-2147483648) |
|  13 | `test_decode_str_and_bytes`     |   ✅   | Decode str and bytes                                                        |
|  14 | `test_decode_simple_and_float`  |   ✅   | Decode simple and float                                                     |
|  15 | `test_decode_array_and_map`     |   ✅   | Decode array and map                                                        |
|  16 | `test_decode_roundtrip`         |   ✅   | Encode a small document, then decode it back and check each field.          |
|  17 | `test_decode_fails_closed`      |   ✅   | truncated uint16 (header says read 2 more bytes, only 1 present)            |

</details>

---

## test_gpio_map - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the GPIO pin-mapper core (services/gpio_map): the direction_

|   # | Test                                  | Status | Description                                                       |
| --: | :------------------------------------ | :----: | :---------------------------------------------------------------- |
|   1 | `test_dir_name`                       |   ✅   | Dir name                                                          |
|   2 | `test_json`                           |   ✅   | Json                                                              |
|   3 | `test_json_empty`                     |   ✅   | Json empty                                                        |
|   4 | `test_json_small_buffer_fails_closed` |   ✅   | Json small buffer fails closed                                    |
|   5 | `test_parse_set`                      |   ✅   | Parse set                                                         |
|   6 | `test_parse_set_rejects_partial`      |   ✅   | Parse set rejects partial                                         |
|   7 | `test_parse_set_no_prefix_match`      |   ✅   | "spin=2" must not satisfy the "pin" field (field-boundary check). |
|   8 | `test_is_output`                      |   ✅   | Is output                                                         |

</details>

---

## test_udp_telemetry - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the UDP telemetry line builder (services/udp_telemetry): the_

|   # | Test                                | Status | Description                  |
| --: | :---------------------------------- | :----: | :--------------------------- |
|   1 | `test_int_and_uint_fields`          |   ✅   | Int and uint fields          |
|   2 | `test_float_field`                  |   ✅   | Float field                  |
|   3 | `test_no_fields_not_ok`             |   ✅   | No fields not ok             |
|   4 | `test_overflow_fails_closed`        |   ✅   | Overflow fails closed        |
|   5 | `test_tags_and_timestamp`           |   ✅   | Tags and timestamp           |
|   6 | `test_tag_escaping`                 |   ✅   | Tag escaping                 |
|   7 | `test_tag_after_field_fails_closed` |   ✅   | Tag after field fails closed |

</details>

---

## test_guardrails - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the guardrails core (services/guardrails): the threshold_

|   # | Test                                  | Status | Description                    |
| --: | :------------------------------------ | :----: | :----------------------------- |
|   1 | `test_eval_all_clear`                 |   ✅   | Eval all clear                 |
|   2 | `test_eval_heap_breach`               |   ✅   | Eval heap breach               |
|   3 | `test_eval_frag_and_stack`            |   ✅   | Eval frag and stack            |
|   4 | `test_eval_all_breached`              |   ✅   | Eval all breached              |
|   5 | `test_json`                           |   ✅   | Json                           |
|   6 | `test_json_small_buffer_fails_closed` |   ✅   | Json small buffer fails closed |

</details>

---

## test_logbuf - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the rotating log ring (services/logbuf): append order, the_

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_append_and_order`      |   ✅   | Append and order      |
|   2 | `test_dump`                  |   ✅   | Dump                  |
|   3 | `test_rotation_drops_oldest` |   ✅   | Rotation drops oldest |
|   4 | `test_trap_threshold`        |   ✅   | Trap threshold        |

</details>

---

## test_config_io - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for schema-driven config export/restore (services/config_io) over_

|   # | Test                                | Status | Description                  |
| --: | :---------------------------------- | :----: | :--------------------------- |
|   1 | `test_export_format`                |   ✅   | Export format                |
|   2 | `test_round_trip`                   |   ✅   | Round trip                   |
|   3 | `test_import_skips_unknown_keys`    |   ✅   | Import skips unknown keys    |
|   4 | `test_export_overflow_fails_closed` |   ✅   | Export overflow fails closed |

</details>

---

## test_clock - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the pluggable monotonic clock (services/det_clock): the platform_

|   # | Test                                    | Status | Description                      |
| --: | :-------------------------------------- | :----: | :------------------------------- |
|   1 | `test_default_is_platform_millis`       |   ✅   | Default is platform millis       |
|   2 | `test_custom_clock_divides_to_1000hz`   |   ✅   | Custom clock divides to 1000hz   |
|   3 | `test_sub_khz_source_not_divided`       |   ✅   | Sub khz source not divided       |
|   4 | `test_revert_to_default`                |   ✅   | Revert to default                |
|   5 | `test_micros_custom_divides_to_1mhz`    |   ✅   | Micros custom divides to 1mhz    |
|   6 | `test_latency_stat_records_and_budgets` |   ✅   | Latency stat records and budgets |
|   7 | `test_latency_budget_zero_disables`     |   ✅   | Latency budget zero disables     |

</details>

---

## test_concurrency - ✅ 2 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Concurrency proof for the cross-thread slot fields (DetAtomic state / rx_head /_

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_spsc_ring_no_race`     |   ✅   | Spsc ring no race     |
|   2 | `test_state_handoff_no_race` |   ✅   | State handoff no race |

</details>

---

## test_concurrency - ✅ 2 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Concurrency proof for the cross-thread slot fields (DetAtomic state / rx_head /_

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_spsc_ring_no_race`     |   ✅   | Spsc ring no race     |
|   2 | `test_state_handoff_no_race` |   ✅   | State handoff no race |

</details>

---

## Raw Output

<details>
<summary>Expand full pio output</summary>

```
********************************************************************************
If you like PlatformIO, please:
- star it on GitHub > https://github.com/platformio/platformio-core
- follow us on LinkedIn to stay up-to-date on the latest project news > https://www.linkedin.com/company/platformio/
- try PlatformIO IDE for embedded development > https://platformio.org/platformio-ide
********************************************************************************

Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests
Platform Manager: Installing native
Downloading 0% 10%
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Platform Manager: native@1.2.1 has been installed!

Processing test_canopen in native_canopen environment
--------------------------------------------------------------------------------
Building...
Tool Manager: Installing platformio/tool-scons @ ~4.40801.0
Downloading 0% 10% 20% 30% 40%
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Tool Manager: tool-scons@4.40801.0 has been installed!
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Downloading 0% 10%
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_canopen/test_canopen.cpp:384: test_nmt_start_node             [PASSED]
test/test_canopen/test_canopen.cpp:385: test_sync                       [PASSED]
test/test_canopen/test_canopen.cpp:386: test_heartbeat_roundtrip        [PASSED]
test/test_canopen/test_canopen.cpp:387: test_emcy_roundtrip             [PASSED]
test/test_canopen/test_canopen.cpp:388: test_pdo_roundtrip              [PASSED]
test/test_canopen/test_canopen.cpp:389: test_sdo_read_request           [PASSED]
test/test_canopen/test_canopen.cpp:390: test_sdo_write_expedited        [PASSED]
test/test_canopen/test_canopen.cpp:391: test_sdo_upload_response_expedited [PASSED]
test/test_canopen/test_canopen.cpp:392: test_sdo_abort_roundtrip        [PASSED]
test/test_canopen/test_canopen.cpp:393: test_sdo_download_ack           [PASSED]
test/test_canopen/test_canopen.cpp:394: test_parse_classifies           [PASSED]
test/test_canopen/test_canopen.cpp:395: test_build_arg_validation       [PASSED]
test/test_canopen/test_canopen.cpp:396: test_emcy_build_null_msef       [PASSED]
test/test_canopen/test_canopen.cpp:397: test_parse_all_function_codes   [PASSED]
test/test_canopen/test_canopen.cpp:398: test_parse_emcy_rejections      [PASSED]
test/test_canopen/test_canopen.cpp:399: test_parse_heartbeat_rejections [PASSED]
test/test_canopen/test_canopen.cpp:400: test_parse_sdo_response_variants [PASSED]
----------- native_canopen:test_canopen [PASSED] Took 14.49 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_canopen  test_canopen  PASSED    00:00:14.494
================= 17 test cases: 17 succeeded in 00:00:14.494 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_det_primitives in native_det_primitives environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_det_primitives/test_det_primitives.cpp:94: test_strtol        [PASSED]
test/test_det_primitives/test_det_primitives.cpp:95: test_strtoul       [PASSED]
test/test_det_primitives/test_det_primitives.cpp:96: test_strtof        [PASSED]
test/test_det_primitives/test_det_primitives.cpp:97: test_utf8_valid    [PASSED]
test/test_det_primitives/test_det_primitives.cpp:98: test_utf8_invalid  [PASSED]
----- native_det_primitives:test_det_primitives [PASSED] Took 0.66 seconds -----

=================================== SUMMARY ===================================
Environment            Test                 Status    Duration
---------------------  -------------------  --------  ------------
native_det_primitives  test_det_primitives  PASSED    00:00:00.656
================== 5 test cases: 5 succeeded in 00:00:00.656 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_det_ip in native_det_ip environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_det_ip/test_det_ip.cpp:185: test_v4_round_trip                [PASSED]
test/test_det_ip/test_det_ip.cpp:186: test_from_v6_bytes                [PASSED]
test/test_det_ip/test_det_ip.cpp:187: test_ip_key                       [PASSED]
test/test_det_ip/test_det_ip.cpp:188: test_v6_canonical_5952            [PASSED]
test/test_det_ip/test_det_ip.cpp:189: test_v4_mapped                    [PASSED]
test/test_det_ip/test_det_ip.cpp:190: test_classify_v4                  [PASSED]
test/test_det_ip/test_det_ip.cpp:191: test_classify_v6                  [PASSED]
test/test_det_ip/test_det_ip.cpp:192: test_reject_malformed             [PASSED]
test/test_det_ip/test_det_ip.cpp:193: test_equal_and_from_v4            [PASSED]
------------- native_det_ip:test_det_ip [PASSED] Took 0.67 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_det_ip  test_det_ip  PASSED    00:00:00.674
================== 9 test cases: 9 succeeded in 00:00:00.674 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_promisc in native_promisc environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_promisc/test_promisc.cpp:162: test_beacon_mgmt                [PASSED]
test/test_promisc/test_promisc.cpp:163: test_data_from_ds               [PASSED]
test/test_promisc/test_promisc.cpp:164: test_data_to_ds                 [PASSED]
test/test_promisc/test_promisc.cpp:165: test_qos_data_header_len        [PASSED]
test/test_promisc/test_promisc.cpp:166: test_wds_four_address           [PASSED]
test/test_promisc/test_promisc.cpp:167: test_control_frame              [PASSED]
test/test_promisc/test_promisc.cpp:168: test_reject_short               [PASSED]
test/test_promisc/test_promisc.cpp:169: test_pcap_headers               [PASSED]
------------ native_promisc:test_promisc [PASSED] Took 0.66 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_promisc  test_promisc  PASSED    00:00:00.664
================== 8 test cases: 8 succeeded in 00:00:00.664 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_bus_capture in native_bus_capture environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_bus_capture/test_bus_capture.cpp:112: test_standard_data_frame [PASSED]
test/test_bus_capture/test_bus_capture.cpp:113: test_extended_id_sets_eff [PASSED]
test/test_bus_capture/test_bus_capture.cpp:114: test_rtr_flag_and_no_data [PASSED]
test/test_bus_capture/test_bus_capture.cpp:115: test_masks_and_bounds   [PASSED]
test/test_bus_capture/test_bus_capture.cpp:116: test_pcap_can_linktype  [PASSED]
-------- native_bus_capture:test_bus_capture [PASSED] Took 0.67 seconds --------

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_bus_capture  test_bus_capture  PASSED    00:00:00.669
================== 5 test cases: 5 succeeded in 00:00:00.669 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_j1939 in native_j1939 environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_j1939/test_j1939.cpp:160: test_id_pdu2_roundtrip              [PASSED]
test/test_j1939/test_j1939.cpp:161: test_id_pdu1_roundtrip              [PASSED]
test/test_j1939/test_j1939.cpp:162: test_encode_rejects_bad_args        [PASSED]
test/test_j1939/test_j1939.cpp:163: test_build_single_frame             [PASSED]
test/test_j1939/test_j1939.cpp:164: test_request_pgn                    [PASSED]
test/test_j1939/test_j1939.cpp:165: test_address_claim_name             [PASSED]
test/test_j1939/test_j1939.cpp:166: test_tp_num_packets                 [PASSED]
test/test_j1939/test_j1939.cpp:167: test_tp_bam_roundtrip               [PASSED]
test/test_j1939/test_j1939.cpp:168: test_tp_out_of_sequence_errors      [PASSED]
-------------- native_j1939:test_j1939 [PASSED] Took 0.67 seconds --------------

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_j1939   test_j1939  PASSED    00:00:00.672
================== 9 test cases: 9 succeeded in 00:00:00.672 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_devicenet in native_devicenet environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_devicenet/test_devicenet.cpp:143: test_id_group1              [PASSED]
test/test_devicenet/test_devicenet.cpp:144: test_id_group2              [PASSED]
test/test_devicenet/test_devicenet.cpp:145: test_id_group3_and_4        [PASSED]
test/test_devicenet/test_devicenet.cpp:146: test_header_and_frag_octets [PASSED]
test/test_devicenet/test_devicenet.cpp:147: test_build_explicit_single_frame [PASSED]
test/test_devicenet/test_devicenet.cpp:148: test_frag_non_fragmented    [PASSED]
test/test_devicenet/test_devicenet.cpp:149: test_frag_reassembly_roundtrip [PASSED]
test/test_devicenet/test_devicenet.cpp:150: test_frag_out_of_order_errors [PASSED]
---------- native_devicenet:test_devicenet [PASSED] Took 0.68 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_devicenet  test_devicenet  PASSED    00:00:00.675
================== 8 test cases: 8 succeeded in 00:00:00.675 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_nmea2000 in native_nmea2000 environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_nmea2000/test_nmea2000.cpp:122: test_num_frames               [PASSED]
test/test_nmea2000/test_nmea2000.cpp:123: test_single_frame             [PASSED]
test/test_nmea2000/test_nmea2000.cpp:124: test_fastpacket_roundtrip     [PASSED]
test/test_nmea2000/test_nmea2000.cpp:125: test_fastpacket_single_frame_completes [PASSED]
test/test_nmea2000/test_nmea2000.cpp:126: test_fastpacket_interleaved_sequence_ignored [PASSED]
test/test_nmea2000/test_nmea2000.cpp:127: test_fastpacket_out_of_order_errors [PASSED]
----------- native_nmea2000:test_nmea2000 [PASSED] Took 0.69 seconds -----------

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_nmea2000  test_nmea2000  PASSED    00:00:00.694
================== 6 test cases: 6 succeeded in 00:00:00.694 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_mbus in native_mbus environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_mbus/test_mbus.cpp:244: test_ack                              [PASSED]
test/test_mbus/test_mbus.cpp:245: test_short_frame_roundtrip            [PASSED]
test/test_mbus/test_mbus.cpp:246: test_req_ud2_fcb                      [PASSED]
test/test_mbus/test_mbus.cpp:247: test_long_frame_roundtrip             [PASSED]
test/test_mbus/test_mbus.cpp:248: test_parse_rejects_corruption         [PASSED]
test/test_mbus/test_mbus.cpp:249: test_dif_data_len                     [PASSED]
test/test_mbus/test_mbus.cpp:250: test_record_walk                      [PASSED]
test/test_mbus/test_mbus.cpp:251: test_record_truncated_fails           [PASSED]
test/test_mbus/test_mbus.cpp:252: test_build_and_parse_guards           [PASSED]
test/test_mbus/test_mbus.cpp:253: test_dif_data_len_remaining           [PASSED]
test/test_mbus/test_mbus.cpp:254: test_record_edges                     [PASSED]
--------------- native_mbus:test_mbus [PASSED] Took 0.66 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_mbus    test_mbus  PASSED    00:00:00.659
================= 11 test cases: 11 succeeded in 00:00:00.659 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_iec60870 in native_iec60870 environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_iec60870/test_iec60870.cpp:158: test_104_i_format_roundtrip   [PASSED]
test/test_iec60870/test_iec60870.cpp:159: test_104_s_format             [PASSED]
test/test_iec60870/test_iec60870.cpp:160: test_104_u_format             [PASSED]
test/test_iec60870/test_iec60870.cpp:161: test_104_sequence_numbers_15bit [PASSED]
test/test_iec60870/test_iec60870.cpp:162: test_asdu_header_roundtrip    [PASSED]
test/test_iec60870/test_iec60870.cpp:163: test_ioa_roundtrip            [PASSED]
test/test_iec60870/test_iec60870.cpp:164: test_101_fixed_frame          [PASSED]
test/test_iec60870/test_iec60870.cpp:165: test_101_variable_frame_roundtrip [PASSED]
----------- native_iec60870:test_iec60870 [PASSED] Took 0.70 seconds -----------

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_iec60870  test_iec60870  PASSED    00:00:00.696
================== 8 test cases: 8 succeeded in 00:00:00.696 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_sdi12 in native_sdi12 environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_sdi12/test_sdi12.cpp:114: test_command_builders               [PASSED]
test/test_sdi12/test_sdi12.cpp:115: test_parse_measure_m                [PASSED]
test/test_sdi12/test_sdi12.cpp:116: test_parse_measure_concurrent_two_digit_count [PASSED]
test/test_sdi12/test_sdi12.cpp:117: test_parse_values                   [PASSED]
test/test_sdi12/test_sdi12.cpp:118: test_crc_roundtrip                  [PASSED]
test/test_sdi12/test_sdi12.cpp:119: test_crc_encode_printable           [PASSED]
-------------- native_sdi12:test_sdi12 [PASSED] Took 0.67 seconds --------------

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_sdi12   test_sdi12  PASSED    00:00:00.668
================== 6 test cases: 6 succeeded in 00:00:00.668 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_dmx in native_dmx environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_dmx/test_dmx.cpp:130: test_dmx_build_and_get                  [PASSED]
test/test_dmx/test_dmx.cpp:131: test_rdm_uid                            [PASSED]
test/test_dmx/test_dmx.cpp:132: test_rdm_get_roundtrip                  [PASSED]
test/test_dmx/test_dmx.cpp:133: test_rdm_set_with_data                  [PASSED]
test/test_dmx/test_dmx.cpp:134: test_rdm_parse_rejects_bad              [PASSED]
---------------- native_dmx:test_dmx [PASSED] Took 0.69 seconds ----------------

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_dmx     test_dmx  PASSED    00:00:00.694
================== 5 test cases: 5 succeeded in 00:00:00.694 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_nmea0183 in native_nmea0183 environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_nmea0183/test_nmea0183.cpp:105: test_checksum_known_vector    [PASSED]
test/test_nmea0183/test_nmea0183.cpp:106: test_build                    [PASSED]
test/test_nmea0183/test_nmea0183.cpp:107: test_parse_gga                [PASSED]
test/test_nmea0183/test_nmea0183.cpp:108: test_field_helpers            [PASSED]
test/test_nmea0183/test_nmea0183.cpp:109: test_parse_rejects_bad_checksum [PASSED]
test/test_nmea0183/test_nmea0183.cpp:110: test_parse_rejects_no_dollar  [PASSED]
test/test_nmea0183/test_nmea0183.cpp:111: test_build_then_parse         [PASSED]
----------- native_nmea0183:test_nmea0183 [PASSED] Took 0.67 seconds -----------

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_nmea0183  test_nmea0183  PASSED    00:00:00.666
================== 7 test cases: 7 succeeded in 00:00:00.666 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_iolink in native_iolink environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_iolink/test_iolink.cpp:80: test_mc_octet                      [PASSED]
test/test_iolink/test_iolink.cpp:81: test_ckt_cks_octets                [PASSED]
test/test_iolink/test_iolink.cpp:82: test_checksum_known_vector         [PASSED]
test/test_iolink/test_iolink.cpp:83: test_finalize_preserves_type_and_detects_corruption [PASSED]
test/test_iolink/test_iolink.cpp:84: test_device_reply_cks_roundtrip    [PASSED]
------------- native_iolink:test_iolink [PASSED] Took 0.67 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_iolink  test_iolink  PASSED    00:00:00.668
================== 5 test cases: 5 succeeded in 00:00:00.668 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_sse in native environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
/usr/bin/ld: .pio/build/native/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
----------------- native:test_sse [ERRORED] Took 1.03 seconds -----------------

Processing test_session in native environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
--------------- native:test_session [ERRORED] Took 0.59 seconds ---------------

Processing test_presentation in native environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
------------- native:test_presentation [ERRORED] Took 0.64 seconds -------------

Processing test_transport in native environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
-------------- native:test_transport [ERRORED] Took 0.61 seconds --------------

Processing test_websocket in native environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
-------------- native:test_websocket [ERRORED] Took 0.63 seconds --------------

Processing test_http_parser in native environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
------------- native:test_http_parser [ERRORED] Took 0.61 seconds -------------

=================================== SUMMARY ===================================
Environment    Test               Status    Duration
-------------  -----------------  --------  ------------
native         test_sse           ERRORED   00:00:01.029
native         test_session       ERRORED   00:00:00.592
native         test_presentation  ERRORED   00:00:00.642
native         test_transport     ERRORED   00:00:00.606
native         test_websocket     ERRORED   00:00:00.629
native         test_http_parser   ERRORED   00:00:00.607
================== 6 test cases: 0 succeeded in 00:00:04.104 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_observability in native_observability environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
/usr/bin/ld: .pio/build/native_observability/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x1042): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_observability/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
----- native_observability:test_observability [ERRORED] Took 0.74 seconds -----

=================================== SUMMARY ===================================
Environment           Test                Status    Duration
--------------------  ------------------  --------  ------------
native_observability  test_observability  ERRORED   00:00:00.738
================== 1 test cases: 0 succeeded in 00:00:00.738 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_accept_gate in native_accept_gate environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
/usr/bin/ld: .pio/build/native_accept_gate/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_accept_gate/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
------- native_accept_gate:test_accept_gate [ERRORED] Took 0.99 seconds -------

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_accept_gate  test_accept_gate  ERRORED   00:00:00.986
================== 1 test cases: 0 succeeded in 00:00:00.986 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_http_ota in native_ota environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_http_ota/test_http_ota.cpp:95: test_large_body_streams_to_completion [PASSED]
test/test_http_ota/test_http_ota.cpp:96: test_no_hooks_large_body_is_413 [PASSED]
test/test_http_ota/test_http_ota.cpp:97: test_nonmatching_path_not_streamed [PASSED]
------------- native_ota:test_http_ota [PASSED] Took 0.69 seconds --------------

=================================== SUMMARY ===================================
Environment    Test           Status    Duration
-------------  -------------  --------  ------------
native_ota     test_http_ota  PASSED    00:00:00.693
================== 3 test cases: 3 succeeded in 00:00:00.693 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_provisioning in native_prov environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_provisioning/test_provisioning.cpp:66: test_plain_fields      [PASSED]
test/test_provisioning/test_provisioning.cpp:67: test_url_decoding      [PASSED]
test/test_provisioning/test_provisioning.cpp:68: test_missing_field     [PASSED]
test/test_provisioning/test_provisioning.cpp:69: test_no_substring_match [PASSED]
test/test_provisioning/test_provisioning.cpp:70: test_capacity_bound    [PASSED]
----------- native_prov:test_provisioning [PASSED] Took 0.70 seconds -----------

=================================== SUMMARY ===================================
Environment    Test               Status    Duration
-------------  -----------------  --------  ------------
native_prov    test_provisioning  PASSED    00:00:00.705
================== 5 test cases: 5 succeeded in 00:00:00.705 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_ssh_crypto in native_ssh environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_crypto/test_ssh_crypto.cpp:949: test_sha256_empty         [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:950: test_sha256_abc           [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:951: test_sha256_448bit        [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:952: test_sha256_streaming     [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:955: test_hmac_sha256_tc1      [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:956: test_hmac_sha256_tc2      [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:957: test_hmac_sha256_tc3      [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:958: test_hmac_sha256_streaming [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:961: test_aes256ctr_encrypt    [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:962: test_aes256ctr_decrypt    [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:963: test_aes256ctr_multi_block [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:964: test_aes256ctr_wipe       [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:967: test_bn_roundtrip         [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:968: test_bn_cmp_equal         [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:969: test_bn_cmp_less          [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:970: test_bn_cmp_greater       [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:971: test_bn_is_zero           [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:972: test_bn_dh_validate_rejects_zero [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:973: test_bn_dh_validate_rejects_one [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:974: test_bn_dh_validate_accepts_two [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:977: test_expmod_exp1          [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:978: test_expmod_exp2          [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:979: test_expmod_exp3          [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:980: test_expmod_commutative   [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:983: test_rsa_pkcs1_pad_structure [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:984: test_rsa_sign_verify_roundtrip [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:985: test_rsa_encode_pubkey    [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:986: test_rsa_verify_valid_signature [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:987: test_rsa_verify_rejects_tampered_signature [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:988: test_rsa_verify_rejects_wrong_message [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:991: test_pkt_send_recv_unencrypted [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:992: test_pkt_padding_alignment [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:993: test_pkt_seq_increments   [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:994: test_pkt_disconnect_zeroes_state [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:995: test_pkt_encrypted_roundtrip [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:996: test_pkt_encrypted_fragmented [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:997: test_pkt_encrypted_two_packets [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:998: test_ssh_kdf_canonical_mpint_k [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:999: test_ssh_kdf_extension_chain [PASSED]
------------ native_ssh:test_ssh_crypto [PASSED] Took 3.78 seconds -------------

Processing test_ssh_auth in native_ssh environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_ssh_auth/test_ssh_auth.cpp:561: test_service_request_errors   [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:562: test_build_response_guards    [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:563: test_parse_request_truncations [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:564: test_pubkey_blob_parse_failures [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:565: test_pubkey_oversized_signed_prefix [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:566: test_handle_request_index_and_parse_guards [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:567: test_service_request_accept   [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:568: test_service_request_rejects_unknown [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:569: test_parse_password_request   [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:570: test_parse_none_request       [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:571: test_handle_request_success   [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:572: test_handle_request_wrong_password_fails [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:573: test_handle_none_request_fails_without_auth [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:574: test_handle_request_no_callback_fails [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:575: test_pubkey_probe_returns_pk_ok [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:576: test_pubkey_valid_signature_succeeds [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:577: test_pubkey_tampered_signature_fails [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:578: test_pubkey_unauthorized_key_fails [PASSED]
------------- native_ssh:test_ssh_auth [PASSED] Took 0.63 seconds --------------

Processing test_ssh_server in native_ssh environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_ssh_server/test_ssh_server.cpp:607: test_ssh_pkt_index_and_cap_guards [PASSED]
test/test_ssh_server/test_ssh_server.cpp:608: test_ssh_pkt_recv_unencrypted_errors [PASSED]
test/test_ssh_server/test_ssh_server.cpp:609: test_ssh_pkt_seq_overflow_guards [PASSED]
test/test_ssh_server/test_ssh_server.cpp:610: test_ssh_pkt_encrypted_roundtrip_and_mac_fail [PASSED]
test/test_ssh_server/test_ssh_server.cpp:611: test_full_handshake_to_channel_data [PASSED]
test/test_ssh_server/test_ssh_server.cpp:612: test_extinfo_build_advertises_server_sig_algs [PASSED]
test/test_ssh_server/test_ssh_server.cpp:613: test_extinfo_not_sent_without_ext_info_c [PASSED]
test/test_ssh_server/test_ssh_server.cpp:614: test_inbound_ext_info_ignored [PASSED]
test/test_ssh_server/test_ssh_server.cpp:615: test_large_client_kexinit_accepted [PASSED]
test/test_ssh_server/test_ssh_server.cpp:616: test_channel_open_before_auth_rejected [PASSED]
test/test_ssh_server/test_ssh_server.cpp:617: test_disconnect_closes    [PASSED]
test/test_ssh_server/test_ssh_server.cpp:618: test_ignore_is_noop       [PASSED]
test/test_ssh_server/test_ssh_server.cpp:619: test_auth_bruteforce_disconnect [PASSED]
test/test_ssh_server/test_ssh_server.cpp:620: test_auth_success_after_failures [PASSED]
test/test_ssh_server/test_ssh_server.cpp:621: test_unimplemented_reply_for_unknown_message [PASSED]
test/test_ssh_server/test_ssh_server.cpp:622: test_inbound_close_emits_eof_then_close_separately [PASSED]
------------ native_ssh:test_ssh_server [PASSED] Took 0.89 seconds -------------

Processing test_ssh_transport in native_ssh environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_ssh_transport/test_ssh_transport.cpp:658: test_transport_index_guards [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:659: test_banner_and_build_caps [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:660: test_kexinit_parse_field_and_trunc [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:661: test_kexdh_parse_and_handle_errors [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:662: test_server_banner_format [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:663: test_recv_banner_complete [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:664: test_recv_banner_bare_lf [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:665: test_recv_banner_split_across_reads [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:666: test_recv_banner_skips_preamble_lines [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:667: test_kexinit_build_starts_with_msg_and_stores_is [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:668: test_kexinit_parse_accepts_supported [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:669: test_kexinit_parse_accepts_when_ours_listed_among_others [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:670: test_kexinit_parse_rejects_missing_kex [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:671: test_kexinit_parse_rejects_missing_cipher [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:672: test_kexinit_parse_rejects_truncated [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:673: test_exchange_hash_matches_independent_assembly [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:674: test_exchange_hash_changes_with_input [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:675: test_kexdh_parse_init_extracts_e_with_padding [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:676: test_kexdh_parse_init_extracts_small_e [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:677: test_kexdh_parse_init_rejects_wrong_type [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:678: test_kexdh_parse_init_rejects_oversized_e [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:679: test_kexdh_build_reply_structure [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:680: test_kexdh_handle_produces_reply_and_installs_keys [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:681: test_kexdh_handle_rejects_invalid_e [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:682: test_derive_keys_session_id_affects_output [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:683: test_rekey_needed_threshold [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:684: test_begin_rekey_preserves_session_and_auth [PASSED]
----------- native_ssh:test_ssh_transport [PASSED] Took 1.12 seconds -----------

Processing test_ssh_channel in native_ssh environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_ssh_channel/test_ssh_channel.cpp:492: test_open_session_confirms [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:493: test_open_unknown_type_fails [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:494: test_direct_tcpip_no_cb_prohibited [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:495: test_direct_tcpip_accept_confirms [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:496: test_direct_tcpip_refused_connect_failed [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:497: test_forward_data_routes_to_forward_cb [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:498: test_shell_request_success_with_reply [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:499: test_unknown_request_failure [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:500: test_request_no_reply_produces_nothing [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:501: test_inbound_data_invokes_callback [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:502: test_inbound_data_window_replenish [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:503: test_inbound_data_exceeding_window_rejected [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:504: test_outbound_data_frames_and_decrements_window [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:505: test_outbound_data_exceeding_peer_window_rejected [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:506: test_window_adjust_grows_peer_window [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:507: test_build_close_emits_eof_and_close [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:508: test_inbound_close_routes_to_channel [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:509: test_multiplex_two_channels_route_independently [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:510: test_pool_full_open_fails [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:511: test_data_to_unknown_channel_rejected [PASSED]
------------ native_ssh:test_ssh_channel [PASSED] Took 0.59 seconds ------------

=================================== SUMMARY ===================================
Environment    Test                Status    Duration
-------------  ------------------  --------  ------------
native_ssh     test_ssh_crypto     PASSED    00:00:03.785
native_ssh     test_ssh_auth       PASSED    00:00:00.627
native_ssh     test_ssh_server     PASSED    00:00:00.890
native_ssh     test_ssh_transport  PASSED    00:00:01.124
native_ssh     test_ssh_channel    PASSED    00:00:00.592
================ 120 test cases: 120 succeeded in 00:00:07.018 ================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_ssh_hardening in native_ssh_hardened environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_hardening/test_ssh_hardening.cpp:87: test_password_refused_even_with_correct_callback [PASSED]
test/test_ssh_hardening/test_ssh_hardening.cpp:88: test_failure_advertises_publickey_only [PASSED]
------ native_ssh_hardened:test_ssh_hardening [PASSED] Took 0.92 seconds -------

=================================== SUMMARY ===================================
Environment          Test                Status    Duration
-------------------  ------------------  --------  ------------
native_ssh_hardened  test_ssh_hardening  PASSED    00:00:00.918
================== 2 test cases: 2 succeeded in 00:00:00.918 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_ssh_conn in native_ssh_conn environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
/usr/bin/ld: .pio/build/native_ssh_conn/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_ssh_conn/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
---------- native_ssh_conn:test_ssh_conn [ERRORED] Took 1.20 seconds ----------

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_ssh_conn  test_ssh_conn  ERRORED   00:00:01.200
================== 1 test cases: 0 succeeded in 00:00:01.200 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_regex in native_app environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
src/DeterministicESPAsyncWebServer.cpp: In member function ‘void DetWebServer::serve_file_internal(uint8_t, bool, fs::FS&, const char*, const char*, const char*)’:
src/DeterministicESPAsyncWebServer.cpp:2915:76: warning: ‘snprintf’ output may be truncated before the last format character [-Wformat-truncation=]
 2915 |         snprintf(lastmod_line, sizeof(lastmod_line), "Last-Modified: %s\r\n", lm_date);
      |                                                                            ^
src/DeterministicESPAsyncWebServer.cpp:2915:17: note: ‘snprintf’ output between 18 and 57 bytes into a destination of size 56
 2915 |         snprintf(lastmod_line, sizeof(lastmod_line), "Last-Modified: %s\r\n", lm_date);
      |         ~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
-------------- native_app:test_regex [ERRORED] Took 1.20 seconds --------------

Processing test_template in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
------------- native_app:test_template [ERRORED] Took 0.67 seconds -------------

Processing test_path_params in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
----------- native_app:test_path_params [ERRORED] Took 0.65 seconds -----------

Processing test_digest_vectors in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
---------- native_app:test_digest_vectors [ERRORED] Took 0.59 seconds ----------

Processing test_form_params in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
----------- native_app:test_form_params [ERRORED] Took 0.63 seconds -----------

Processing test_iface in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
-------------- native_app:test_iface [ERRORED] Took 0.62 seconds --------------

Processing test_json in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
--------------- native_app:test_json [ERRORED] Took 0.59 seconds ---------------

Processing test_response_headers in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
--------- native_app:test_response_headers [ERRORED] Took 0.67 seconds ---------

Processing test_middleware in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
------------ native_app:test_middleware [ERRORED] Took 0.65 seconds ------------

Processing test_digest_auth in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
----------- native_app:test_digest_auth [ERRORED] Took 0.67 seconds -----------

Processing test_web_terminal in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
----------- native_app:test_web_terminal [ERRORED] Took 0.67 seconds -----------

Processing test_defer in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
-------------- native_app:test_defer [ERRORED] Took 0.62 seconds --------------

Processing test_multipart in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
------------ native_app:test_multipart [ERRORED] Took 0.64 seconds ------------

Processing test_auth in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
--------------- native_app:test_auth [ERRORED] Took 0.63 seconds ---------------

Processing test_file_serving in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
----------- native_app:test_file_serving [ERRORED] Took 0.65 seconds -----------

Processing test_dispatch in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
------------- native_app:test_dispatch [ERRORED] Took 0.65 seconds -------------

Processing test_chunked in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
------------- native_app:test_chunked [ERRORED] Took 0.66 seconds -------------

Processing test_application in native_app environment
--------------------------------------------------------------------------------
Building...
/usr/bin/ld: .pio/build/native_app/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_app/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
----------- native_app:test_application [ERRORED] Took 0.73 seconds -----------

=================================== SUMMARY ===================================
Environment    Test                   Status    Duration
-------------  ---------------------  --------  ------------
native_app     test_regex             ERRORED   00:00:01.204
native_app     test_template          ERRORED   00:00:00.669
native_app     test_path_params       ERRORED   00:00:00.646
native_app     test_digest_vectors    ERRORED   00:00:00.588
native_app     test_form_params       ERRORED   00:00:00.630
native_app     test_iface             ERRORED   00:00:00.625
native_app     test_json              ERRORED   00:00:00.593
native_app     test_response_headers  ERRORED   00:00:00.670
native_app     test_middleware        ERRORED   00:00:00.645
native_app     test_digest_auth       ERRORED   00:00:00.665
native_app     test_web_terminal      ERRORED   00:00:00.666
native_app     test_defer             ERRORED   00:00:00.618
native_app     test_multipart         ERRORED   00:00:00.637
native_app     test_auth              ERRORED   00:00:00.634
native_app     test_file_serving      ERRORED   00:00:00.653
native_app     test_dispatch          ERRORED   00:00:00.645
native_app     test_chunked           ERRORED   00:00:00.658
native_app     test_application       ERRORED   00:00:00.725
================== 18 test cases: 0 succeeded in 00:00:12.171 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_webdav_handler in native_webdav_handler environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
/usr/bin/ld: .pio/build/native_webdav_handler/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_webdav_handler/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
---- native_webdav_handler:test_webdav_handler [ERRORED] Took 1.23 seconds ----

=================================== SUMMARY ===================================
Environment            Test                 Status    Duration
---------------------  -------------------  --------  ------------
native_webdav_handler  test_webdav_handler  ERRORED   00:00:01.232
================== 1 test cases: 0 succeeded in 00:00:01.232 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_diag in native_diag environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
src/DeterministicESPAsyncWebServer.cpp: In member function ‘void DetWebServer::serve_file_internal(uint8_t, bool, fs::FS&, const char*, const char*, const char*)’:
src/DeterministicESPAsyncWebServer.cpp:2915:76: warning: ‘snprintf’ output may be truncated before the last format character [-Wformat-truncation=]
 2915 |         snprintf(lastmod_line, sizeof(lastmod_line), "Last-Modified: %s\r\n", lm_date);
      |                                                                            ^
src/DeterministicESPAsyncWebServer.cpp:2915:17: note: ‘snprintf’ output between 18 and 57 bytes into a destination of size 56
 2915 |         snprintf(lastmod_line, sizeof(lastmod_line), "Last-Modified: %s\r\n", lm_date);
      |         ~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/usr/bin/ld: .pio/build/native_diag/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_diag/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
-------------- native_diag:test_diag [ERRORED] Took 1.19 seconds --------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_diag    test_diag  ERRORED   00:00:01.194
================== 1 test cases: 0 succeeded in 00:00:01.194 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_snmp_ber in native_snmp environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_snmp_ber/test_snmp_ber.cpp:298: test_integer_vectors          [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:299: test_oid_vector               [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:300: test_octet_string_and_null    [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:301: test_counter32_keeps_unsigned [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:302: test_sequence_roundtrip       [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:303: test_oid_roundtrip            [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:304: test_large_arc_roundtrip      [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:305: test_oid_large_first_subidentifier_roundtrip [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:306: test_encoder_overflow_sets_not_ok [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:307: test_decoder_truncated_length_fails [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:308: test_decoder_longform_length_count_past_buffer_fails [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:309: test_decoder_longform_length_too_wide_fails [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:310: test_decoder_longform_length_content_past_buffer_fails [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:311: test_decoder_longform_length_max_uint32_fails [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:312: test_decoder_indefinite_length_fails [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:313: test_decoder_oversized_integer_fails [PASSED]
------------- native_snmp:test_snmp_ber [PASSED] Took 0.74 seconds -------------

Processing test_snmp_agent in native_snmp environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_snmp_agent/test_snmp_agent.cpp:518: test_registration_and_rw_edges [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:519: test_ipaddress_value_encodes [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:520: test_set_wrong_type_and_unknown [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:521: test_getbulk_variants     [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:522: test_dispatch_value_types_and_malformed [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:523: test_get_string_v2c       [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:524: test_get_unknown_v2c_exception [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:525: test_get_bad_instance_v2c_nosuchinstance [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:526: test_get_unknown_v1_error [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:527: test_getnext_walks_to_first [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:528: test_getnext_past_end_endofmibview [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:529: test_set_without_rw_community_denied [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:530: test_set_with_rw_community_invokes_setter [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:531: test_set_readonly_not_writable [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:532: test_getbulk_returns_multiple [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:533: test_dynamic_counter_value [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:534: test_uptime_is_timeticks  [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:535: test_unknown_community_no_response [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:536: test_v3_message_dropped   [PASSED]
------------ native_snmp:test_snmp_agent [PASSED] Took 0.56 seconds ------------

=================================== SUMMARY ===================================
Environment    Test             Status    Duration
-------------  ---------------  --------  ------------
native_snmp    test_snmp_ber    PASSED    00:00:00.741
native_snmp    test_snmp_agent  PASSED    00:00:00.561
================= 35 test cases: 35 succeeded in 00:00:01.302 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_snmp_v3 in native_snmp_v3 environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_snmp_v3/test_snmp_v3.cpp:604: test_v3_message_structure_rejections [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:605: test_v3_init_and_boots_accessors [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:606: test_v3_discovery_variants      [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:607: test_v3_priv_not_configured     [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:608: test_v3_notify_paths            [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:609: test_localize_key_sha256_vector [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:610: test_aes128_fips197_vector      [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:611: test_aes_cfb_roundtrip_partial_block [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:612: test_discovery_reports_engine_id [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:613: test_authnopriv_get             [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:614: test_authpriv_get               [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:615: test_wrong_auth_password_reports_wrong_digest [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:616: test_unknown_user_reports       [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:617: test_not_in_time_window_reports [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:618: test_inform_v3_builds_informrequest [PASSED]
------------ native_snmp_v3:test_snmp_v3 [PASSED] Took 2.12 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_snmp_v3  test_snmp_v3  PASSED    00:00:02.117
================= 15 test cases: 15 succeeded in 00:00:02.117 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_telnet in native_telnet environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
/usr/bin/ld: .pio/build/native_telnet/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_telnet/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
------------ native_telnet:test_telnet [ERRORED] Took 0.81 seconds ------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_telnet  test_telnet  ERRORED   00:00:00.811
================== 1 test cases: 0 succeeded in 00:00:00.811 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_coap in native_coap environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_coap/test_coap.cpp:1037: test_add_resource_limits             [PASSED]
test/test_coap/test_coap.cpp:1038: test_short_and_truncated_token       [PASSED]
test/test_coap/test_coap.cpp:1039: test_malformed_options_bad_request   [PASSED]
test/test_coap/test_coap.cpp:1040: test_extended_delta_and_length_ignored [PASSED]
test/test_coap/test_coap.cpp:1041: test_oversized_path_and_query        [PASSED]
test/test_coap/test_coap.cpp:1042: test_block_option_too_wide           [PASSED]
test/test_coap/test_coap.cpp:1043: test_block1_reserved_szx             [PASSED]
test/test_coap/test_coap.cpp:1044: test_block1_continue_no_space        [PASSED]
test/test_coap/test_coap.cpp:1045: test_response_payload_clamped        [PASSED]
test/test_coap/test_coap.cpp:1046: test_response_buffer_too_small       [PASSED]
test/test_coap/test_coap.cpp:1047: test_well_known_core_truncates       [PASSED]
test/test_coap/test_coap.cpp:1048: test_observe_large_seq_encoding      [PASSED]
test/test_coap/test_coap.cpp:1049: test_block2_explicit_paging          [PASSED]
test/test_coap/test_coap.cpp:1050: test_block2_auto_when_large          [PASSED]
test/test_coap/test_coap.cpp:1051: test_block2_szx_clamped              [PASSED]
test/test_coap/test_coap.cpp:1052: test_block2_absent_for_small         [PASSED]
test/test_coap/test_coap.cpp:1053: test_block2_out_of_range             [PASSED]
test/test_coap/test_coap.cpp:1054: test_block2_reserved_szx             [PASSED]
test/test_coap/test_coap.cpp:1055: test_block1_upload_two_blocks        [PASSED]
test/test_coap/test_coap.cpp:1056: test_block1_out_of_order             [PASSED]
test/test_coap/test_coap.cpp:1057: test_block1_too_large                [PASSED]
test/test_coap/test_coap.cpp:1058: test_observe_option_in_response      [PASSED]
test/test_coap/test_coap.cpp:1059: test_no_observe_option_when_seq_negative [PASSED]
test/test_coap/test_coap.cpp:1060: test_get_content                     [PASSED]
test/test_coap/test_coap.cpp:1061: test_not_found                       [PASSED]
test/test_coap/test_coap.cpp:1062: test_method_not_allowed              [PASSED]
test/test_coap/test_coap.cpp:1063: test_non_request_type                [PASSED]
test/test_coap/test_coap.cpp:1064: test_put_with_payload                [PASSED]
test/test_coap/test_coap.cpp:1065: test_multi_segment_path              [PASSED]
test/test_coap/test_coap.cpp:1066: test_uri_query                       [PASSED]
test/test_coap/test_coap.cpp:1067: test_empty_con_ping_rst              [PASSED]
test/test_coap/test_coap.cpp:1068: test_bad_version_rst                 [PASSED]
test/test_coap/test_coap.cpp:1069: test_delete                          [PASSED]
test/test_coap/test_coap.cpp:1070: test_token_8_bytes                   [PASSED]
test/test_coap/test_coap.cpp:1071: test_extended_option_length          [PASSED]
test/test_coap/test_coap.cpp:1072: test_ack_ignored                     [PASSED]
test/test_coap/test_coap.cpp:1073: test_root_path                       [PASSED]
test/test_coap/test_coap.cpp:1074: test_unknown_method_not_allowed      [PASSED]
test/test_coap/test_coap.cpp:1075: test_unknown_critical_option_bad_option [PASSED]
test/test_coap/test_coap.cpp:1076: test_well_known_core_discovery       [PASSED]
test/test_coap/test_coap.cpp:1077: test_well_known_core_rejects_post    [PASSED]
--------------- native_coap:test_coap [PASSED] Took 0.99 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_coap    test_coap  PASSED    00:00:00.991
================= 41 test cases: 41 succeeded in 00:00:00.991 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_coap in native_coap_observe environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_coap/test_coap.cpp:1037: test_add_resource_limits             [PASSED]
test/test_coap/test_coap.cpp:1038: test_short_and_truncated_token       [PASSED]
test/test_coap/test_coap.cpp:1039: test_malformed_options_bad_request   [PASSED]
test/test_coap/test_coap.cpp:1040: test_extended_delta_and_length_ignored [PASSED]
test/test_coap/test_coap.cpp:1041: test_oversized_path_and_query        [PASSED]
test/test_coap/test_coap.cpp:1042: test_block_option_too_wide           [PASSED]
test/test_coap/test_coap.cpp:1043: test_block1_reserved_szx             [PASSED]
test/test_coap/test_coap.cpp:1044: test_block1_continue_no_space        [PASSED]
test/test_coap/test_coap.cpp:1045: test_response_payload_clamped        [PASSED]
test/test_coap/test_coap.cpp:1046: test_response_buffer_too_small       [PASSED]
test/test_coap/test_coap.cpp:1047: test_well_known_core_truncates       [PASSED]
test/test_coap/test_coap.cpp:1048: test_observe_large_seq_encoding      [PASSED]
test/test_coap/test_coap.cpp:1049: test_block2_explicit_paging          [PASSED]
test/test_coap/test_coap.cpp:1050: test_block2_auto_when_large          [PASSED]
test/test_coap/test_coap.cpp:1051: test_block2_szx_clamped              [PASSED]
test/test_coap/test_coap.cpp:1052: test_block2_absent_for_small         [PASSED]
test/test_coap/test_coap.cpp:1053: test_block2_out_of_range             [PASSED]
test/test_coap/test_coap.cpp:1054: test_block2_reserved_szx             [PASSED]
test/test_coap/test_coap.cpp:1055: test_block1_upload_two_blocks        [PASSED]
test/test_coap/test_coap.cpp:1056: test_block1_out_of_order             [PASSED]
test/test_coap/test_coap.cpp:1057: test_block1_too_large                [PASSED]
test/test_coap/test_coap.cpp:1058: test_observe_option_in_response      [PASSED]
test/test_coap/test_coap.cpp:1059: test_no_observe_option_when_seq_negative [PASSED]
test/test_coap/test_coap.cpp:1060: test_get_content                     [PASSED]
test/test_coap/test_coap.cpp:1061: test_not_found                       [PASSED]
test/test_coap/test_coap.cpp:1062: test_method_not_allowed              [PASSED]
test/test_coap/test_coap.cpp:1063: test_non_request_type                [PASSED]
test/test_coap/test_coap.cpp:1064: test_put_with_payload                [PASSED]
test/test_coap/test_coap.cpp:1065: test_multi_segment_path              [PASSED]
test/test_coap/test_coap.cpp:1066: test_uri_query                       [PASSED]
test/test_coap/test_coap.cpp:1067: test_empty_con_ping_rst              [PASSED]
test/test_coap/test_coap.cpp:1068: test_bad_version_rst                 [PASSED]
test/test_coap/test_coap.cpp:1069: test_delete                          [PASSED]
test/test_coap/test_coap.cpp:1070: test_token_8_bytes                   [PASSED]
test/test_coap/test_coap.cpp:1071: test_extended_option_length          [PASSED]
test/test_coap/test_coap.cpp:1072: test_ack_ignored                     [PASSED]
test/test_coap/test_coap.cpp:1073: test_root_path                       [PASSED]
test/test_coap/test_coap.cpp:1074: test_unknown_method_not_allowed      [PASSED]
test/test_coap/test_coap.cpp:1075: test_unknown_critical_option_bad_option [PASSED]
test/test_coap/test_coap.cpp:1076: test_well_known_core_discovery       [PASSED]
test/test_coap/test_coap.cpp:1077: test_well_known_core_rejects_post    [PASSED]
----------- native_coap_observe:test_coap [PASSED] Took 0.79 seconds -----------

=================================== SUMMARY ===================================
Environment          Test       Status    Duration
-------------------  ---------  --------  ------------
native_coap_observe  test_coap  PASSED    00:00:00.794
================= 41 test cases: 41 succeeded in 00:00:00.794 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_webdav in native_webdav environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_webdav/test_webdav.cpp:291: test_method_classification        [PASSED]
test/test_webdav/test_webdav.cpp:292: test_depth_parsing                [PASSED]
test/test_webdav/test_webdav.cpp:293: test_xml_escape                   [PASSED]
test/test_webdav/test_webdav.cpp:294: test_xml_escape_truncates_safely  [PASSED]
test/test_webdav/test_webdav.cpp:295: test_dest_absolute_uri            [PASSED]
test/test_webdav/test_webdav.cpp:296: test_dest_percent_decoded         [PASSED]
test/test_webdav/test_webdav.cpp:297: test_dest_abs_path                [PASSED]
test/test_webdav/test_webdav.cpp:298: test_dest_rejects_malformed       [PASSED]
test/test_webdav/test_webdav.cpp:299: test_multistatus_file_and_collection [PASSED]
test/test_webdav/test_webdav.cpp:300: test_multistatus_escapes_href     [PASSED]
test/test_webdav/test_webdav.cpp:301: test_multistatus_entry_stops_when_full [PASSED]
test/test_webdav/test_webdav.cpp:302: test_proppatch_windows_timestamp  [PASSED]
test/test_webdav/test_webdav.cpp:303: test_proppatch_multiple_and_self_closed [PASSED]
test/test_webdav/test_webdav.cpp:304: test_proppatch_remove_block       [PASSED]
test/test_webdav/test_webdav.cpp:305: test_proppatch_escapes_href       [PASSED]
test/test_webdav/test_webdav.cpp:306: test_proppatch_empty_body_is_valid [PASSED]
test/test_webdav/test_webdav.cpp:307: test_proppatch_rejects_injection  [PASSED]
test/test_webdav/test_webdav.cpp:308: test_proppatch_fuzz_bounded       [PASSED]
test/test_webdav/test_webdav.cpp:309: test_proppatch_stops_when_full    [PASSED]
------------- native_webdav:test_webdav [PASSED] Took 0.70 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_webdav  test_webdav  PASSED    00:00:00.701
================= 19 test cases: 19 succeeded in 00:00:00.701 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_modbus in native_modbus environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_modbus/test_modbus.cpp:424: test_read_holding_registers       [PASSED]
test/test_modbus/test_modbus.cpp:425: test_read_input_registers         [PASSED]
test/test_modbus/test_modbus.cpp:426: test_read_coils_packs_bits        [PASSED]
test/test_modbus/test_modbus.cpp:427: test_write_single_coil            [PASSED]
test/test_modbus/test_modbus.cpp:428: test_write_single_register        [PASSED]
test/test_modbus/test_modbus.cpp:429: test_write_multiple_registers     [PASSED]
test/test_modbus/test_modbus.cpp:430: test_write_multiple_coils         [PASSED]
test/test_modbus/test_modbus.cpp:431: test_exception_illegal_function   [PASSED]
test/test_modbus/test_modbus.cpp:432: test_exception_illegal_address    [PASSED]
test/test_modbus/test_modbus.cpp:433: test_exception_illegal_value      [PASSED]
test/test_modbus/test_modbus.cpp:434: test_write_single_coil_bad_value  [PASSED]
test/test_modbus/test_modbus.cpp:435: test_non_modbus_protocol_id_ignored [PASSED]
test/test_modbus/test_modbus.cpp:436: test_truncated_frame_ignored      [PASSED]
test/test_modbus/test_modbus.cpp:437: test_discrete_and_input_accessors [PASSED]
test/test_modbus/test_modbus.cpp:438: test_exceptions_per_function      [PASSED]
test/test_modbus/test_modbus.cpp:439: test_small_response_buffer        [PASSED]
test/test_modbus/test_modbus.cpp:441: test_rtu_crc16_known_vector       [PASSED]
test/test_modbus/test_modbus.cpp:442: test_rtu_read_holding_roundtrip   [PASSED]
test/test_modbus/test_modbus.cpp:443: test_rtu_bad_crc_dropped          [PASSED]
test/test_modbus/test_modbus.cpp:444: test_rtu_wrong_address_dropped    [PASSED]
test/test_modbus/test_modbus.cpp:445: test_rtu_broadcast_executes_without_reply [PASSED]
test/test_modbus/test_modbus.cpp:446: test_rtu_edge_cases               [PASSED]
------------- native_modbus:test_modbus [PASSED] Took 0.69 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_modbus  test_modbus  PASSED    00:00:00.692
================= 22 test cases: 22 succeeded in 00:00:00.692 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_cloudevents in native_cloudevents environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_cloudevents/test_cloudevents.cpp:130: test_build_minimal      [PASSED]
test/test_cloudevents/test_cloudevents.cpp:131: test_build_requires_id_source_type [PASSED]
test/test_cloudevents/test_cloudevents.cpp:132: test_build_with_json_data [PASSED]
test/test_cloudevents/test_cloudevents.cpp:133: test_build_with_string_data [PASSED]
test/test_cloudevents/test_cloudevents.cpp:134: test_build_overflow_fails_closed [PASSED]
test/test_cloudevents/test_cloudevents.cpp:135: test_from_headers_binary_mode [PASSED]
test/test_cloudevents/test_cloudevents.cpp:136: test_from_headers_missing_required [PASSED]
-------- native_cloudevents:test_cloudevents [PASSED] Took 0.74 seconds --------

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_cloudevents  test_cloudevents  PASSED    00:00:00.744
================== 7 test cases: 7 succeeded in 00:00:00.744 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_redis_resp in native_redis environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_redis_resp/test_redis_resp.cpp:127: test_encode_command       [PASSED]
test/test_redis_resp/test_redis_resp.cpp:128: test_encode_binary_safe   [PASSED]
test/test_redis_resp/test_redis_resp.cpp:129: test_encode_overflow_fails_closed [PASSED]
test/test_redis_resp/test_redis_resp.cpp:130: test_parse_simple_and_error [PASSED]
test/test_redis_resp/test_redis_resp.cpp:131: test_parse_integer        [PASSED]
test/test_redis_resp/test_redis_resp.cpp:132: test_parse_bulk_and_nil   [PASSED]
test/test_redis_resp/test_redis_resp.cpp:133: test_parse_array_cursor   [PASSED]
test/test_redis_resp/test_redis_resp.cpp:134: test_parse_incomplete_and_malformed [PASSED]
----------- native_redis:test_redis_resp [PASSED] Took 0.72 seconds ------------

=================================== SUMMARY ===================================
Environment    Test             Status    Duration
-------------  ---------------  --------  ------------
native_redis   test_redis_resp  PASSED    00:00:00.716
================== 8 test cases: 8 succeeded in 00:00:00.716 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_stomp in native_stomp environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_stomp/test_stomp.cpp:214: test_build_send                     [PASSED]
test/test_stomp/test_stomp.cpp:215: test_build_cr_escape_and_guards     [PASSED]
test/test_stomp/test_stomp.cpp:216: test_parse_more_edges               [PASSED]
test/test_stomp/test_stomp.cpp:217: test_header_and_unescape_null       [PASSED]
test/test_stomp/test_stomp.cpp:218: test_build_no_headers_no_body       [PASSED]
test/test_stomp/test_stomp.cpp:219: test_build_escapes_header           [PASSED]
test/test_stomp/test_stomp.cpp:220: test_build_overflow_fails_closed    [PASSED]
test/test_stomp/test_stomp.cpp:221: test_round_trip                     [PASSED]
test/test_stomp/test_stomp.cpp:222: test_parse_message_crlf             [PASSED]
test/test_stomp/test_stomp.cpp:223: test_parse_content_length_body_with_nul [PASSED]
test/test_stomp/test_stomp.cpp:224: test_parse_skips_leading_heartbeats [PASSED]
test/test_stomp/test_stomp.cpp:225: test_parse_incomplete_and_malformed [PASSED]
test/test_stomp/test_stomp.cpp:226: test_unescape                       [PASSED]
test/test_stomp/test_stomp.cpp:227: test_unescape_rejects_bad           [PASSED]
-------------- native_stomp:test_stomp [PASSED] Took 0.70 seconds --------------

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_stomp   test_stomp  PASSED    00:00:00.698
================= 14 test cases: 14 succeeded in 00:00:00.698 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_mqtt_sn in native_mqtt_sn environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_mqtt_sn/test_mqtt_sn.cpp:288: test_make_flags                 [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:289: test_build_connect_bytes        [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:290: test_build_publish_bytes        [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:291: test_register_round_trip        [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:292: test_parse_connack_regack_suback_publish [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:293: test_three_octet_length         [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:294: test_optional_fields            [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:295: test_overflow_and_malformed     [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:296: test_build_regack_puback        [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:297: test_build_subscribe_variants   [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:298: test_pingreq_with_client_id     [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:299: test_build_guards               [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:300: test_parse_typed_rejections     [PASSED]
------------ native_mqtt_sn:test_mqtt_sn [PASSED] Took 0.72 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_mqtt_sn  test_mqtt_sn  PASSED    00:00:00.720
================= 13 test cases: 13 succeeded in 00:00:00.720 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_flow_export in native_flow_export environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_flow_export/test_flow_export.cpp:158: test_v5_header_bytes    [PASSED]
test/test_flow_export/test_flow_export.cpp:159: test_v5_record_bytes    [PASSED]
test/test_flow_export/test_flow_export.cpp:160: test_v5_overflow_fails_closed [PASSED]
test/test_flow_export/test_flow_export.cpp:161: test_ipfix_message_bytes [PASSED]
test/test_flow_export/test_flow_export.cpp:162: test_v9_count_and_padding [PASSED]
test/test_flow_export/test_flow_export.cpp:163: test_finish_overflow_fails_closed [PASSED]
-------- native_flow_export:test_flow_export [PASSED] Took 0.69 seconds --------

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_flow_export  test_flow_export  PASSED    00:00:00.694
================== 6 test cases: 6 succeeded in 00:00:00.694 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_protobuf in native_protobuf environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_protobuf/test_protobuf.cpp:285: test_writer_error_paths       [PASSED]
test/test_protobuf/test_protobuf.cpp:286: test_reader_error_paths       [PASSED]
test/test_protobuf/test_protobuf.cpp:287: test_float_bits_helper        [PASSED]
test/test_protobuf/test_protobuf.cpp:288: test_vector_field1_150        [PASSED]
test/test_protobuf/test_protobuf.cpp:289: test_vector_string_testing    [PASSED]
test/test_protobuf/test_protobuf.cpp:290: test_zigzag_mapping           [PASSED]
test/test_protobuf/test_protobuf.cpp:291: test_fixed_and_float_bytes    [PASSED]
test/test_protobuf/test_protobuf.cpp:292: test_round_trip_reader        [PASSED]
test/test_protobuf/test_protobuf.cpp:293: test_int64_negative           [PASSED]
test/test_protobuf/test_protobuf.cpp:294: test_varint_and_overflow      [PASSED]
test/test_protobuf/test_protobuf.cpp:295: test_malformed_reads          [PASSED]
test/test_protobuf/test_protobuf.cpp:296: test_varint_width_boundary    [PASSED]
test/test_protobuf/test_protobuf.cpp:297: test_empty_length_field       [PASSED]
----------- native_protobuf:test_protobuf [PASSED] Took 0.70 seconds -----------

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_protobuf  test_protobuf  PASSED    00:00:00.696
================= 13 test cases: 13 succeeded in 00:00:00.696 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_preempt_queue in native_preempt_queue environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_preempt_queue/test_preempt_queue.cpp:213: test_start_validates_and_runs [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:214: test_fifo_order     [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:215: test_urgent_goes_to_front [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:216: test_fail_closed_when_full [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:217: test_high_water_tracks_peak [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:218: test_from_isr_enqueues [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:219: test_drain_empties_and_reuses [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:220: test_internal_lanes_outrank_user [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:221: test_lanes_are_isolated [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:222: test_lane_start_stop_running_independent [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:223: test_lane_high_water_is_per_lane [PASSED]
------ native_preempt_queue:test_preempt_queue [PASSED] Took 0.74 seconds ------

=================================== SUMMARY ===================================
Environment           Test                Status    Duration
--------------------  ------------------  --------  ------------
native_preempt_queue  test_preempt_queue  PASSED    00:00:00.737
================= 11 test cases: 11 succeeded in 00:00:00.737 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_dma in native_dma environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_dma/test_dma.cpp:251: test_open_validates                     [PASSED]
test/test_dma/test_dma.cpp:252: test_ingress_emits_rx_event             [PASSED]
test/test_dma/test_dma.cpp:253: test_buffer_fills_then_partial_flush    [PASSED]
test/test_dma/test_dma.cpp:254: test_ping_pong_flips_buffer             [PASSED]
test/test_dma/test_dma.cpp:255: test_egress_captures_tx                 [PASSED]
test/test_dma/test_dma.cpp:256: test_tx_one_in_flight_fail_closed       [PASSED]
test/test_dma/test_dma.cpp:257: test_tx_rejects_bad_len                 [PASSED]
test/test_dma/test_dma.cpp:258: test_loopback_round_trip                [PASSED]
test/test_dma/test_dma.cpp:259: test_feed_fail_closed_when_full         [PASSED]
test/test_dma/test_dma.cpp:260: test_closed_channel_is_inert            [PASSED]
test/test_dma/test_dma.cpp:261: test_two_channels_independent           [PASSED]
---------------- native_dma:test_dma [PASSED] Took 0.80 seconds ----------------

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_dma     test_dma  PASSED    00:00:00.796
================= 11 test cases: 11 succeeded in 00:00:00.796 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_forward in native_forward environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_forward/test_forward.cpp:256: test_default_deny               [PASSED]
test/test_forward/test_forward.cpp:257: test_allow_forwards             [PASSED]
test/test_forward/test_forward.cpp:258: test_no_self_forward            [PASSED]
test/test_forward/test_forward.cpp:259: test_deny_wins_over_allow       [PASSED]
test/test_forward/test_forward.cpp:260: test_multi_destination_fanout   [PASSED]
test/test_forward/test_forward.cpp:261: test_rate_cap_drops_then_reopens [PASSED]
test/test_forward/test_forward.cpp:262: test_send_failure_counted       [PASSED]
test/test_forward/test_forward.cpp:263: test_add_if_validation_and_table_full [PASSED]
test/test_forward/test_forward.cpp:264: test_add_rule_table_full        [PASSED]
test/test_forward/test_forward.cpp:265: test_unregistered_destination_is_inert [PASSED]
test/test_forward/test_forward.cpp:266: test_acl_deny_by_byte_pattern   [PASSED]
test/test_forward/test_forward.cpp:267: test_acl_allowlist_default_deny [PASSED]
test/test_forward/test_forward.cpp:268: test_acl_first_match_wins       [PASSED]
test/test_forward/test_forward.cpp:269: test_acl_src_any_content_wildcard [PASSED]
test/test_forward/test_forward.cpp:270: test_acl_short_frame_skips_entry [PASSED]
test/test_forward/test_forward.cpp:271: test_acl_add_validation_and_table_full [PASSED]
------------ native_forward:test_forward [PASSED] Took 0.87 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_forward  test_forward  PASSED    00:00:00.871
================= 16 test cases: 16 succeeded in 00:00:00.871 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_gateway in native_gateway environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_gateway/test_gateway.cpp:234: test_uplink_envelopes_and_publishes [PASSED]
test/test_gateway/test_gateway.cpp:235: test_uplink_no_sink_drops       [PASSED]
test/test_gateway/test_gateway.cpp:236: test_uplink_unknown_port_drops  [PASSED]
test/test_gateway/test_gateway.cpp:237: test_uplink_rate_cap            [PASSED]
test/test_gateway/test_gateway.cpp:238: test_uplink_sink_refusal_counted [PASSED]
test/test_gateway/test_gateway.cpp:239: test_downlink_transmits         [PASSED]
test/test_gateway/test_gateway.cpp:240: test_downlink_no_tx_or_unknown_port_drops [PASSED]
test/test_gateway/test_gateway.cpp:241: test_downlink_tx_refusal_counted [PASSED]
test/test_gateway/test_gateway.cpp:242: test_topic_format               [PASSED]
test/test_gateway/test_gateway.cpp:243: test_add_port_validation_and_table_full [PASSED]
test/test_gateway/test_gateway.cpp:244: test_seq_increments_per_uplink  [PASSED]
------------ native_gateway:test_gateway [PASSED] Took 0.79 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_gateway  test_gateway  PASSED    00:00:00.791
================= 11 test cases: 11 succeeded in 00:00:00.791 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_lora in native_lora environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_lora/test_lora.cpp:210: test_frame_build_then_parse           [PASSED]
test/test_lora/test_lora.cpp:211: test_frame_parse_rejects_short        [PASSED]
test/test_lora/test_lora.cpp:212: test_frame_build_bounds               [PASSED]
test/test_lora/test_lora.cpp:213: test_init_verifies_chip_and_lands_in_standby [PASSED]
test/test_lora/test_lora.cpp:214: test_init_fails_on_wrong_version      [PASSED]
test/test_lora/test_lora.cpp:215: test_init_programs_frequency          [PASSED]
test/test_lora/test_lora.cpp:216: test_send_loads_fifo_and_starts_tx    [PASSED]
test/test_lora/test_lora.cpp:217: test_tx_done_flag                     [PASSED]
test/test_lora/test_lora.cpp:218: test_set_rx_enters_continuous         [PASSED]
test/test_lora/test_lora.cpp:219: test_recv_reads_frame_and_rssi        [PASSED]
test/test_lora/test_lora.cpp:220: test_recv_no_packet                   [PASSED]
test/test_lora/test_lora.cpp:221: test_recv_crc_error_dropped           [PASSED]
test/test_lora/test_lora.cpp:222: test_recv_truncates_to_cap            [PASSED]
--------------- native_lora:test_lora [PASSED] Took 0.68 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_lora    test_lora  PASSED    00:00:00.681
================= 13 test cases: 13 succeeded in 00:00:00.681 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_nrf24 in native_nrf24 environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_nrf24/test_nrf24.cpp:188: test_init_configures_and_powers_up  [PASSED]
test/test_nrf24/test_nrf24.cpp:189: test_init_fails_when_absent         [PASSED]
test/test_nrf24/test_nrf24.cpp:190: test_send_pads_to_width_and_keys_tx [PASSED]
test/test_nrf24/test_nrf24.cpp:191: test_send_rejects_oversize          [PASSED]
test/test_nrf24/test_nrf24.cpp:192: test_tx_done_flag                   [PASSED]
test/test_nrf24/test_nrf24.cpp:193: test_set_rx_enters_prx              [PASSED]
test/test_nrf24/test_nrf24.cpp:194: test_recv_reads_payload_and_pipe    [PASSED]
test/test_nrf24/test_nrf24.cpp:195: test_recv_no_packet                 [PASSED]
test/test_nrf24/test_nrf24.cpp:196: test_recv_fifo_empty_pipe           [PASSED]
test/test_nrf24/test_nrf24.cpp:197: test_recv_truncates_to_cap          [PASSED]
-------------- native_nrf24:test_nrf24 [PASSED] Took 0.70 seconds --------------

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_nrf24   test_nrf24  PASSED    00:00:00.697
================= 10 test cases: 10 succeeded in 00:00:00.697 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_enocean in native_enocean environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_enocean/test_enocean.cpp:120: test_crc8_known_answers         [PASSED]
test/test_enocean/test_enocean.cpp:121: test_build_then_parse_round_trip [PASSED]
test/test_enocean/test_enocean.cpp:122: test_parse_rejects_bad_sync     [PASSED]
test/test_enocean/test_enocean.cpp:123: test_parse_rejects_bad_header_crc [PASSED]
test/test_enocean/test_enocean.cpp:124: test_parse_rejects_bad_data_crc [PASSED]
test/test_enocean/test_enocean.cpp:125: test_parse_needs_more_bytes     [PASSED]
test/test_enocean/test_enocean.cpp:126: test_parse_rejects_over_length  [PASSED]
test/test_enocean/test_enocean.cpp:127: test_parse_resynchronises_after_junk [PASSED]
test/test_enocean/test_enocean.cpp:128: test_build_bounds               [PASSED]
------------ native_enocean:test_enocean [PASSED] Took 0.70 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_enocean  test_enocean  PASSED    00:00:00.700
================== 9 test cases: 9 succeeded in 00:00:00.700 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_pn532 in native_pn532 environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_pn532/test_pn532.cpp:139: test_build_getfirmwareversion_kat   [PASSED]
test/test_pn532/test_pn532.cpp:140: test_parse_getfirmwareversion_response_kat [PASSED]
test/test_pn532/test_pn532.cpp:141: test_build_then_parse_round_trip    [PASSED]
test/test_pn532/test_pn532.cpp:142: test_parse_rejects_bad_preamble_and_start [PASSED]
test/test_pn532/test_pn532.cpp:143: test_parse_rejects_bad_lcs          [PASSED]
test/test_pn532/test_pn532.cpp:144: test_parse_rejects_bad_dcs          [PASSED]
test/test_pn532/test_pn532.cpp:145: test_parse_needs_more_bytes         [PASSED]
test/test_pn532/test_pn532.cpp:146: test_parse_rejects_over_length      [PASSED]
test/test_pn532/test_pn532.cpp:147: test_ack_frame                      [PASSED]
test/test_pn532/test_pn532.cpp:148: test_build_bounds                   [PASSED]
-------------- native_pn532:test_pn532 [PASSED] Took 0.70 seconds --------------

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_pn532   test_pn532  PASSED    00:00:00.701
================= 10 test cases: 10 succeeded in 00:00:00.701 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_sigfox in native_sigfox environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_sigfox/test_sigfox.cpp:74: test_build_uplink_hex_encode       [PASSED]
test/test_sigfox/test_sigfox.cpp:75: test_build_uplink_single_byte      [PASSED]
test/test_sigfox/test_sigfox.cpp:76: test_build_uplink_bounds           [PASSED]
test/test_sigfox/test_sigfox.cpp:77: test_parse_response_ok             [PASSED]
test/test_sigfox/test_sigfox.cpp:78: test_parse_response_error          [PASSED]
test/test_sigfox/test_sigfox.cpp:79: test_parse_response_pending        [PASSED]
test/test_sigfox/test_sigfox.cpp:80: test_parse_response_error_wins     [PASSED]
------------- native_sigfox:test_sigfox [PASSED] Took 0.69 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_sigfox  test_sigfox  PASSED    00:00:00.685
================== 7 test cases: 7 succeeded in 00:00:00.685 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_zwave in native_zwave environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_zwave/test_zwave.cpp:109: test_build_getversion_kat           [PASSED]
test/test_zwave/test_zwave.cpp:110: test_build_then_parse_round_trip    [PASSED]
test/test_zwave/test_zwave.cpp:111: test_parse_getversion_kat           [PASSED]
test/test_zwave/test_zwave.cpp:112: test_parse_rejects_bad_sof          [PASSED]
test/test_zwave/test_zwave.cpp:113: test_parse_rejects_bad_checksum     [PASSED]
test/test_zwave/test_zwave.cpp:114: test_parse_needs_more_bytes         [PASSED]
test/test_zwave/test_zwave.cpp:115: test_parse_rejects_over_length      [PASSED]
test/test_zwave/test_zwave.cpp:116: test_control_bytes                  [PASSED]
test/test_zwave/test_zwave.cpp:117: test_build_bounds                   [PASSED]
-------------- native_zwave:test_zwave [PASSED] Took 0.70 seconds --------------

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_zwave   test_zwave  PASSED    00:00:00.701
================== 9 test cases: 9 succeeded in 00:00:00.701 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_zigbee in native_zigbee environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_zigbee/test_zigbee.cpp:125: test_crc16_rst_kat                [PASSED]
test/test_zigbee/test_zigbee.cpp:126: test_encode_rst_frame_kat         [PASSED]
test/test_zigbee/test_zigbee.cpp:127: test_encode_decode_round_trip     [PASSED]
test/test_zigbee/test_zigbee.cpp:128: test_byte_stuffing_round_trip     [PASSED]
test/test_zigbee/test_zigbee.cpp:129: test_decode_needs_more_without_flag [PASSED]
test/test_zigbee/test_zigbee.cpp:130: test_decode_rejects_bad_crc       [PASSED]
test/test_zigbee/test_zigbee.cpp:131: test_decode_rejects_dangling_escape [PASSED]
test/test_zigbee/test_zigbee.cpp:132: test_decode_rejects_small_payload_buffer [PASSED]
test/test_zigbee/test_zigbee.cpp:133: test_encode_bounds                [PASSED]
------------- native_zigbee:test_zigbee [PASSED] Took 0.69 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_zigbee  test_zigbee  PASSED    00:00:00.687
================== 9 test cases: 9 succeeded in 00:00:00.687 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_thread in native_thread environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_thread/test_thread.cpp:199: test_fcs_x25_check_value          [PASSED]
test/test_thread/test_thread.cpp:200: test_encode_decode_round_trip     [PASSED]
test/test_thread/test_thread.cpp:201: test_byte_stuffing_round_trip     [PASSED]
test/test_thread/test_thread.cpp:202: test_decode_needs_more_without_flag [PASSED]
test/test_thread/test_thread.cpp:203: test_decode_rejects_bad_fcs       [PASSED]
test/test_thread/test_thread.cpp:204: test_decode_rejects_dangling_escape [PASSED]
test/test_thread/test_thread.cpp:205: test_decode_rejects_small_payload_buffer [PASSED]
test/test_thread/test_thread.cpp:206: test_encode_bounds                [PASSED]
test/test_thread/test_thread.cpp:207: test_spinel_pack_uint_kats        [PASSED]
test/test_thread/test_thread.cpp:208: test_spinel_pack_unpack_round_trip [PASSED]
test/test_thread/test_thread.cpp:209: test_spinel_unpack_needs_more_and_overflow [PASSED]
test/test_thread/test_thread.cpp:210: test_spinel_command_build_parse_round_trip [PASSED]
test/test_thread/test_thread.cpp:211: test_spinel_command_through_hdlc  [PASSED]
------------- native_thread:test_thread [PASSED] Took 0.68 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_thread  test_thread  PASSED    00:00:00.682
================= 13 test cases: 13 succeeded in 00:00:00.682 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_wamp in native_wamp environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_wamp/test_wamp.cpp:171: test_build_hello                      [PASSED]
test/test_wamp/test_wamp.cpp:172: test_build_subscribe_default_options  [PASSED]
test/test_wamp/test_wamp.cpp:173: test_build_publish_with_args          [PASSED]
test/test_wamp/test_wamp.cpp:174: test_build_publish_kwargs_only        [PASSED]
test/test_wamp/test_wamp.cpp:175: test_build_call_and_register_and_yield [PASSED]
test/test_wamp/test_wamp.cpp:176: test_build_unsubscribe_and_goodbye    [PASSED]
test/test_wamp/test_wamp.cpp:177: test_build_overflow_fails_closed      [PASSED]
test/test_wamp/test_wamp.cpp:178: test_parse_type_and_id                [PASSED]
test/test_wamp/test_wamp.cpp:179: test_parse_event_positions            [PASSED]
test/test_wamp/test_wamp.cpp:180: test_parse_get_uri_and_nesting        [PASSED]
test/test_wamp/test_wamp.cpp:181: test_parse_malformed                  [PASSED]
test/test_wamp/test_wamp.cpp:182: test_get_uri_dest_bounds              [PASSED]
--------------- native_wamp:test_wamp [PASSED] Took 0.70 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_wamp    test_wamp  PASSED    00:00:00.698
================= 12 test cases: 12 succeeded in 00:00:00.698 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_sunspec in native_sunspec environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_sunspec/test_sunspec.cpp:130: test_build_and_walk             [PASSED]
test/test_sunspec/test_sunspec.cpp:131: test_two_models                 [PASSED]
test/test_sunspec/test_sunspec.cpp:132: test_string_point               [PASSED]
test/test_sunspec/test_sunspec.cpp:133: test_marker_and_truncation      [PASSED]
test/test_sunspec/test_sunspec.cpp:134: test_writer_overflow_fails_closed [PASSED]
------------ native_sunspec:test_sunspec [PASSED] Took 0.66 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_sunspec  test_sunspec  PASSED    00:00:00.661
================== 5 test cases: 5 succeeded in 00:00:00.661 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_c37118 in native_c37118 environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_c37118/test_c37118.cpp:117: test_crc_check_value              [PASSED]
test/test_c37118/test_c37118.cpp:118: test_build_command_bytes          [PASSED]
test/test_c37118/test_c37118.cpp:119: test_command_round_trip           [PASSED]
test/test_c37118/test_c37118.cpp:120: test_data_frame_payload           [PASSED]
test/test_c37118/test_c37118.cpp:121: test_parse_rejects_bad            [PASSED]
test/test_c37118/test_c37118.cpp:122: test_build_overflow_fails_closed  [PASSED]
------------- native_c37118:test_c37118 [PASSED] Took 0.70 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_c37118  test_c37118  PASSED    00:00:00.696
================== 6 test cases: 6 succeeded in 00:00:00.696 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_dnp3 in native_dnp3 environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_dnp3/test_dnp3.cpp:157: test_dnp3_parse_guards                [PASSED]
test/test_dnp3/test_dnp3.cpp:158: test_crc_check_value                  [PASSED]
test/test_dnp3/test_dnp3.cpp:159: test_build_header_bytes               [PASSED]
test/test_dnp3/test_dnp3.cpp:160: test_round_trip_single_block          [PASSED]
test/test_dnp3/test_dnp3.cpp:161: test_round_trip_multi_block           [PASSED]
test/test_dnp3/test_dnp3.cpp:162: test_header_only_frame                [PASSED]
test/test_dnp3/test_dnp3.cpp:163: test_parse_rejects_bad                [PASSED]
test/test_dnp3/test_dnp3.cpp:164: test_build_overflow_fails_closed      [PASSED]
--------------- native_dnp3:test_dnp3 [PASSED] Took 0.68 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_dnp3    test_dnp3  PASSED    00:00:00.684
================== 8 test cases: 8 succeeded in 00:00:00.684 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_grpcweb in native_grpcweb environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_grpcweb/test_grpcweb.cpp:116: test_frame_message_bytes        [PASSED]
test/test_grpcweb/test_grpcweb.cpp:117: test_compressed_flag            [PASSED]
test/test_grpcweb/test_grpcweb.cpp:118: test_trailer_frame              [PASSED]
test/test_grpcweb/test_grpcweb.cpp:119: test_trailer_status_only        [PASSED]
test/test_grpcweb/test_grpcweb.cpp:120: test_parse_stream               [PASSED]
test/test_grpcweb/test_grpcweb.cpp:121: test_parse_incomplete           [PASSED]
test/test_grpcweb/test_grpcweb.cpp:122: test_frame_overflow_fails_closed [PASSED]
------------ native_grpcweb:test_grpcweb [PASSED] Took 0.67 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_grpcweb  test_grpcweb  PASSED    00:00:00.670
================== 7 test cases: 7 succeeded in 00:00:00.670 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_lwm2m_tlv in native_lwm2m_tlv environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:266: test_write_int_1byte        [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:267: test_write_int_2byte        [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:268: test_write_string_8bit_length [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:269: test_write_16bit_id         [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:270: test_round_trip_and_value_int [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:271: test_object_instance_nested [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:272: test_write_16bit_length     [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:273: test_read_24bit_length      [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:274: test_value_int_4_and_8_byte [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:275: test_zero_length_value      [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:276: test_overflow_and_malformed [PASSED]
---------- native_lwm2m_tlv:test_lwm2m_tlv [PASSED] Took 0.68 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_lwm2m_tlv  test_lwm2m_tlv  PASSED    00:00:00.679
================= 11 test cases: 11 succeeded in 00:00:00.679 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_fins in native_fins environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_fins/test_fins.cpp:135: test_build_command_bytes              [PASSED]
test/test_fins/test_fins.cpp:136: test_memory_area_read                 [PASSED]
test/test_fins/test_fins.cpp:137: test_parse_command                    [PASSED]
test/test_fins/test_fins.cpp:138: test_parse_response_ok                [PASSED]
test/test_fins/test_fins.cpp:139: test_parse_response_error             [PASSED]
test/test_fins/test_fins.cpp:140: test_overflow_and_truncation          [PASSED]
--------------- native_fins:test_fins [PASSED] Took 0.66 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_fins    test_fins  PASSED    00:00:00.663
================== 6 test cases: 6 succeeded in 00:00:00.663 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_hostlink in native_hostlink environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_hostlink/test_hostlink.cpp:112: test_fcs_vector               [PASSED]
test/test_hostlink/test_hostlink.cpp:113: test_build_dm_read            [PASSED]
test/test_hostlink/test_hostlink.cpp:114: test_build_node_digits        [PASSED]
test/test_hostlink/test_hostlink.cpp:115: test_round_trip               [PASSED]
test/test_hostlink/test_hostlink.cpp:116: test_parse_response_end_code  [PASSED]
test/test_hostlink/test_hostlink.cpp:117: test_parse_rejects_bad        [PASSED]
test/test_hostlink/test_hostlink.cpp:118: test_build_overflow_fails_closed [PASSED]
----------- native_hostlink:test_hostlink [PASSED] Took 0.70 seconds -----------

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_hostlink  test_hostlink  PASSED    00:00:00.697
================== 7 test cases: 7 succeeded in 00:00:00.697 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_senml in native_senml environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_senml/test_senml.cpp:239: test_json_canonical                 [PASSED]
test/test_senml/test_senml.cpp:240: test_json_base_time_and_none        [PASSED]
test/test_senml/test_senml.cpp:241: test_cbor_all_kinds                 [PASSED]
test/test_senml/test_senml.cpp:242: test_senml_null_args                [PASSED]
test/test_senml/test_senml.cpp:243: test_json_multi_record              [PASSED]
test/test_senml/test_senml.cpp:244: test_json_string_bool_time          [PASSED]
test/test_senml/test_senml.cpp:245: test_cbor_round_trip                [PASSED]
test/test_senml/test_senml.cpp:246: test_cbor_base_name_key             [PASSED]
test/test_senml/test_senml.cpp:247: test_overflow_fails_closed          [PASSED]
-------------- native_senml:test_senml [PASSED] Took 0.76 seconds --------------

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_senml   test_senml  PASSED    00:00:00.759
================== 9 test cases: 9 succeeded in 00:00:00.759 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_df1 in native_df1 environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_df1/test_df1.cpp:131: test_bcc_vector                         [PASSED]
test/test_df1/test_df1.cpp:132: test_crc_vector                         [PASSED]
test/test_df1/test_df1.cpp:133: test_build_bcc_frame                    [PASSED]
test/test_df1/test_df1.cpp:134: test_build_dle_stuffing                 [PASSED]
test/test_df1/test_df1.cpp:135: test_round_trip_bcc                     [PASSED]
test/test_df1/test_df1.cpp:136: test_round_trip_crc                     [PASSED]
test/test_df1/test_df1.cpp:137: test_empty_data_frame                   [PASSED]
test/test_df1/test_df1.cpp:138: test_parse_rejects_bad                  [PASSED]
test/test_df1/test_df1.cpp:139: test_build_overflow_fails_closed        [PASSED]
---------------- native_df1:test_df1 [PASSED] Took 0.73 seconds ----------------

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_df1     test_df1  PASSED    00:00:00.732
================== 9 test cases: 9 succeeded in 00:00:00.732 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_cotp in native_cotp environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_cotp/test_cotp.cpp:122: test_tpkt_bytes                       [PASSED]
test/test_cotp/test_cotp.cpp:123: test_cotp_dt_bytes                    [PASSED]
test/test_cotp/test_cotp.cpp:124: test_cotp_cr_bytes                    [PASSED]
test/test_cotp/test_cotp.cpp:125: test_cotp_cr_with_tsaps               [PASSED]
test/test_cotp/test_cotp.cpp:126: test_full_stack                       [PASSED]
test/test_cotp/test_cotp.cpp:127: test_parse_rejects_bad                [PASSED]
--------------- native_cotp:test_cotp [PASSED] Took 0.70 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_cotp    test_cotp  PASSED    00:00:00.698
================== 6 test cases: 6 succeeded in 00:00:00.698 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_s7comm in native_s7comm environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_s7comm/test_s7comm.cpp:158: test_build_setup                  [PASSED]
test/test_s7comm/test_s7comm.cpp:159: test_build_read_request           [PASSED]
test/test_s7comm/test_s7comm.cpp:160: test_read_request_bit_address     [PASSED]
test/test_s7comm/test_s7comm.cpp:161: test_parse_response_single        [PASSED]
test/test_s7comm/test_s7comm.cpp:162: test_parse_response_padding       [PASSED]
test/test_s7comm/test_s7comm.cpp:163: test_parse_octet_and_error        [PASSED]
test/test_s7comm/test_s7comm.cpp:164: test_parse_rejects_bad            [PASSED]
test/test_s7comm/test_s7comm.cpp:165: test_build_overflow_fails_closed  [PASSED]
------------- native_s7comm:test_s7comm [PASSED] Took 0.71 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_s7comm  test_s7comm  PASSED    00:00:00.708
================== 8 test cases: 8 succeeded in 00:00:00.708 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_melsec in native_melsec environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_melsec/test_melsec.cpp:101: test_build_read_bytes             [PASSED]
test/test_melsec/test_melsec.cpp:102: test_head_device_24bit            [PASSED]
test/test_melsec/test_melsec.cpp:103: test_parse_response_ok            [PASSED]
test/test_melsec/test_melsec.cpp:104: test_parse_response_error         [PASSED]
test/test_melsec/test_melsec.cpp:105: test_parse_rejects_bad            [PASSED]
test/test_melsec/test_melsec.cpp:106: test_build_overflow_fails_closed  [PASSED]
------------- native_melsec:test_melsec [PASSED] Took 0.70 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_melsec  test_melsec  PASSED    00:00:00.701
================== 6 test cases: 6 succeeded in 00:00:00.701 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_bacnet in native_bacnet environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_bacnet/test_bacnet.cpp:180: test_bacnet_guards_and_truncations [PASSED]
test/test_bacnet/test_bacnet.cpp:181: test_bvlc_bytes                   [PASSED]
test/test_bacnet/test_bacnet.cpp:182: test_npdu_local                   [PASSED]
test/test_bacnet/test_bacnet.cpp:183: test_npdu_dest                    [PASSED]
test/test_bacnet/test_bacnet.cpp:184: test_npdu_broadcast               [PASSED]
test/test_bacnet/test_bacnet.cpp:185: test_npdu_parse_with_source       [PASSED]
test/test_bacnet/test_bacnet.cpp:186: test_full_stack                   [PASSED]
test/test_bacnet/test_bacnet.cpp:187: test_parse_rejects_bad            [PASSED]
test/test_bacnet/test_bacnet.cpp:188: test_overflow_fails_closed        [PASSED]
------------- native_bacnet:test_bacnet [PASSED] Took 0.73 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_bacnet  test_bacnet  PASSED    00:00:00.726
================== 9 test cases: 9 succeeded in 00:00:00.726 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_enip in native_enip environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_enip/test_enip.cpp:135: test_header_round_trip                [PASSED]
test/test_enip/test_enip.cpp:136: test_register_session                 [PASSED]
test/test_enip/test_enip.cpp:137: test_send_rr_data_bytes               [PASSED]
test/test_enip/test_enip.cpp:138: test_send_rr_data_round_trip          [PASSED]
test/test_enip/test_enip.cpp:139: test_parse_rejects_bad                [PASSED]
test/test_enip/test_enip.cpp:140: test_build_overflow_fails_closed      [PASSED]
--------------- native_enip:test_enip [PASSED] Took 0.71 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_enip    test_enip  PASSED    00:00:00.708
================== 6 test cases: 6 succeeded in 00:00:00.708 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_amqp in native_amqp environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_amqp/test_amqp.cpp:134: test_protocol_header                  [PASSED]
test/test_amqp/test_amqp.cpp:135: test_build_method_bytes               [PASSED]
test/test_amqp/test_amqp.cpp:136: test_method_round_trip                [PASSED]
test/test_amqp/test_amqp.cpp:137: test_heartbeat                        [PASSED]
test/test_amqp/test_amqp.cpp:138: test_parse_stream                     [PASSED]
test/test_amqp/test_amqp.cpp:139: test_parse_rejects_bad                [PASSED]
test/test_amqp/test_amqp.cpp:140: test_build_overflow_fails_closed      [PASSED]
--------------- native_amqp:test_amqp [PASSED] Took 0.70 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_amqp    test_amqp  PASSED    00:00:00.699
================== 7 test cases: 7 succeeded in 00:00:00.699 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_cip in native_cip environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_cip/test_cip.cpp:128: test_cip_build_guards                   [PASSED]
test/test_cip/test_cip.cpp:129: test_epath_8bit                         [PASSED]
test/test_cip/test_cip.cpp:130: test_epath_16bit                        [PASSED]
test/test_cip/test_cip.cpp:131: test_get_attr_single                    [PASSED]
test/test_cip/test_cip.cpp:132: test_build_request_with_data            [PASSED]
test/test_cip/test_cip.cpp:133: test_parse_response_ok                  [PASSED]
test/test_cip/test_cip.cpp:134: test_parse_response_additional_status   [PASSED]
test/test_cip/test_cip.cpp:135: test_parse_response_error               [PASSED]
test/test_cip/test_cip.cpp:136: test_rejects_bad                        [PASSED]
---------------- native_cip:test_cip [PASSED] Took 0.71 seconds ----------------

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_cip     test_cip  PASSED    00:00:00.714
================== 9 test cases: 9 succeeded in 00:00:00.714 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_nats in native_nats environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_nats/test_nats.cpp:186: test_build_connect                    [PASSED]
test/test_nats/test_nats.cpp:187: test_build_ping_pong                  [PASSED]
test/test_nats/test_nats.cpp:188: test_build_null_args                  [PASSED]
test/test_nats/test_nats.cpp:189: test_build_overflow_put_ch            [PASSED]
test/test_nats/test_nats.cpp:190: test_parse_edges                      [PASSED]
test/test_nats/test_nats.cpp:191: test_build_pub                        [PASSED]
test/test_nats/test_nats.cpp:192: test_build_pub_with_reply             [PASSED]
test/test_nats/test_nats.cpp:193: test_build_pub_empty_payload          [PASSED]
test/test_nats/test_nats.cpp:194: test_build_sub_and_unsub              [PASSED]
test/test_nats/test_nats.cpp:195: test_parse_msg                        [PASSED]
test/test_nats/test_nats.cpp:196: test_parse_msg_with_reply             [PASSED]
test/test_nats/test_nats.cpp:197: test_parse_control_lines              [PASSED]
test/test_nats/test_nats.cpp:198: test_parse_incomplete                 [PASSED]
test/test_nats/test_nats.cpp:199: test_build_overflow_fails_closed      [PASSED]
--------------- native_nats:test_nats [PASSED] Took 0.74 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_nats    test_nats  PASSED    00:00:00.742
================= 14 test cases: 14 succeeded in 00:00:00.742 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_proxy_protocol in native_proxy_protocol environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_proxy_protocol/test_proxy_protocol.cpp:120: test_v1_build     [PASSED]
test/test_proxy_protocol/test_proxy_protocol.cpp:121: test_v1_round_trip [PASSED]
test/test_proxy_protocol/test_proxy_protocol.cpp:122: test_v2_build_bytes [PASSED]
test/test_proxy_protocol/test_proxy_protocol.cpp:123: test_v2_round_trip [PASSED]
test/test_proxy_protocol/test_proxy_protocol.cpp:124: test_v1_unknown   [PASSED]
test/test_proxy_protocol/test_proxy_protocol.cpp:125: test_not_a_proxy_header [PASSED]
test/test_proxy_protocol/test_proxy_protocol.cpp:126: test_incomplete   [PASSED]
test/test_proxy_protocol/test_proxy_protocol.cpp:127: test_build_overflow_fails_closed [PASSED]
----- native_proxy_protocol:test_proxy_protocol [PASSED] Took 0.71 seconds -----

=================================== SUMMARY ===================================
Environment            Test                 Status    Duration
---------------------  -------------------  --------  ------------
native_proxy_protocol  test_proxy_protocol  PASSED    00:00:00.712
================== 8 test cases: 8 succeeded in 00:00:00.712 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_sparkplug in native_sparkplug environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_sparkplug/test_sparkplug.cpp:201: test_spb_error_and_kind_paths [PASSED]
test/test_sparkplug/test_sparkplug.cpp:202: test_topic                  [PASSED]
test/test_sparkplug/test_sparkplug.cpp:203: test_metric_bytes           [PASSED]
test/test_sparkplug/test_sparkplug.cpp:204: test_payload_round_trip     [PASSED]
test/test_sparkplug/test_sparkplug.cpp:205: test_metric_int_and_string  [PASSED]
test/test_sparkplug/test_sparkplug.cpp:206: test_metric_alias           [PASSED]
test/test_sparkplug/test_sparkplug.cpp:207: test_overflow_fails_closed  [PASSED]
---------- native_sparkplug:test_sparkplug [PASSED] Took 0.72 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_sparkplug  test_sparkplug  PASSED    00:00:00.716
================== 7 test cases: 7 succeeded in 00:00:00.716 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_modbus_master in native_modbus_master environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_modbus_master/test_modbus_master.cpp:85: test_build_read_bytes [PASSED]
test/test_modbus_master/test_modbus_master.cpp:86: test_build_rejects_bad_args [PASSED]
test/test_modbus_master/test_modbus_master.cpp:87: test_round_trip_holding_regs [PASSED]
test/test_modbus_master/test_modbus_master.cpp:88: test_round_trip_exception [PASSED]
test/test_modbus_master/test_modbus_master.cpp:89: test_parse_short_frame_fails [PASSED]
------ native_modbus_master:test_modbus_master [PASSED] Took 0.72 seconds ------

=================================== SUMMARY ===================================
Environment           Test                Status    Duration
--------------------  ------------------  --------  ------------
native_modbus_master  test_modbus_master  PASSED    00:00:00.720
================== 5 test cases: 5 succeeded in 00:00:00.720 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_ota_rollback in native_ota_rollback environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ota_rollback/test_ota_rollback.cpp:49: test_not_pending_waits [PASSED]
test/test_ota_rollback/test_ota_rollback.cpp:50: test_pending_self_test_ok_commits [PASSED]
test/test_ota_rollback/test_ota_rollback.cpp:51: test_pending_within_window_waits [PASSED]
test/test_ota_rollback/test_ota_rollback.cpp:52: test_pending_window_elapsed_rolls_back [PASSED]
test/test_ota_rollback/test_ota_rollback.cpp:53: test_self_test_ok_beats_window [PASSED]
------- native_ota_rollback:test_ota_rollback [PASSED] Took 0.71 seconds -------

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_ota_rollback  test_ota_rollback  PASSED    00:00:00.706
================== 5 test cases: 5 succeeded in 00:00:00.706 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_totp in native_totp environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_totp/test_totp.cpp:68: test_rfc6238_vectors                   [PASSED]
test/test_totp/test_totp.cpp:69: test_verify_window                     [PASSED]
test/test_totp/test_totp.cpp:70: test_base32_decode                     [PASSED]
test/test_totp/test_totp.cpp:71: test_base32_rejects_invalid            [PASSED]
--------------- native_totp:test_totp [PASSED] Took 0.75 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_totp    test_totp  PASSED    00:00:00.746
================== 4 test cases: 4 succeeded in 00:00:00.746 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_webhook in native_webhook environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_webhook/test_webhook.cpp:65: test_ifttt_url                   [PASSED]
test/test_webhook/test_webhook.cpp:66: test_payload_three_values        [PASSED]
test/test_webhook/test_webhook.cpp:67: test_payload_omits_nulls         [PASSED]
test/test_webhook/test_webhook.cpp:68: test_payload_escapes_json        [PASSED]
test/test_webhook/test_webhook.cpp:69: test_overflow_fails_closed       [PASSED]
------------ native_webhook:test_webhook [PASSED] Took 0.71 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_webhook  test_webhook  PASSED    00:00:00.713
================== 5 test cases: 5 succeeded in 00:00:00.713 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_radio_power in native_radio_power environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_radio_power/test_radio_power.cpp:34: test_ps_names            [PASSED]
test/test_radio_power/test_radio_power.cpp:35: test_apply_is_noop_on_host [PASSED]
-------- native_radio_power:test_radio_power [PASSED] Took 0.68 seconds --------

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_radio_power  test_radio_power  PASSED    00:00:00.678
================== 2 test cases: 2 succeeded in 00:00:00.678 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_dns_resolver in native_dns_resolver environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_dns_resolver/test_dns_resolver.cpp:59: test_classify          [PASSED]
test/test_dns_resolver/test_dns_resolver.cpp:60: test_verify_rejects_suspicious [PASSED]
test/test_dns_resolver/test_dns_resolver.cpp:61: test_verify_accepts_plausible [PASSED]
test/test_dns_resolver/test_dns_resolver.cpp:62: test_resolve_is_noop_on_host [PASSED]
------- native_dns_resolver:test_dns_resolver [PASSED] Took 0.68 seconds -------

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_dns_resolver  test_dns_resolver  PASSED    00:00:00.685
================== 4 test cases: 4 succeeded in 00:00:00.685 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_audit_log in native_audit_log environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_audit_log/test_audit_log.cpp:246: test_append_assigns_monotonic_seq [PASSED]
test/test_audit_log/test_audit_log.cpp:247: test_chain_verifies_when_untouched [PASSED]
test/test_audit_log/test_audit_log.cpp:248: test_tampered_message_breaks_chain [PASSED]
test/test_audit_log/test_audit_log.cpp:249: test_tampered_hash_breaks_chain [PASSED]
test/test_audit_log/test_audit_log.cpp:250: test_tampered_category_breaks_chain [PASSED]
test/test_audit_log/test_audit_log.cpp:251: test_ring_evicts_oldest_and_still_verifies [PASSED]
test/test_audit_log/test_audit_log.cpp:252: test_tamper_after_wrap_detected_at_oldest [PASSED]
test/test_audit_log/test_audit_log.cpp:253: test_reset_clears_everything [PASSED]
test/test_audit_log/test_audit_log.cpp:254: test_sink_receives_each_record [PASSED]
test/test_audit_log/test_audit_log.cpp:255: test_format_and_dump_json   [PASSED]
test/test_audit_log/test_audit_log.cpp:256: test_dump_json_reports_broken_chain [PASSED]
test/test_audit_log/test_audit_log.cpp:257: test_format_fails_closed_on_small_buffer [PASSED]
test/test_audit_log/test_audit_log.cpp:258: test_null_msg_and_categories [PASSED]
test/test_audit_log/test_audit_log.cpp:259: test_json_escape_all_chars  [PASSED]
test/test_audit_log/test_audit_log.cpp:260: test_format_fails_closed_all_stages [PASSED]
test/test_audit_log/test_audit_log.cpp:261: test_dump_fails_closed_all_stages [PASSED]
---------- native_audit_log:test_audit_log [PASSED] Took 0.71 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_audit_log  test_audit_log  PASSED    00:00:00.713
================= 16 test cases: 16 succeeded in 00:00:00.713 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_oidc in native_oidc environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_oidc/test_oidc.cpp:252: test_jwks_malformed_keys              [PASSED]
test/test_oidc/test_oidc.cpp:253: test_token_kid_guards                 [PASSED]
test/test_oidc/test_oidc.cpp:254: test_jwks_find_guards                 [PASSED]
test/test_oidc/test_oidc.cpp:255: test_verify_guards_and_malformed      [PASSED]
test/test_oidc/test_oidc.cpp:256: test_token_kid                        [PASSED]
test/test_oidc/test_oidc.cpp:257: test_jwks_find                        [PASSED]
test/test_oidc/test_oidc.cpp:258: test_jwks_find_missing_kid_fails      [PASSED]
test/test_oidc/test_oidc.cpp:259: test_verify_valid_token_and_claims    [PASSED]
test/test_oidc/test_oidc.cpp:260: test_verify_aud_array                 [PASSED]
test/test_oidc/test_oidc.cpp:261: test_reject_expired                   [PASSED]
test/test_oidc/test_oidc.cpp:262: test_reject_wrong_issuer              [PASSED]
test/test_oidc/test_oidc.cpp:263: test_reject_wrong_audience            [PASSED]
test/test_oidc/test_oidc.cpp:264: test_reject_non_rs256_header          [PASSED]
test/test_oidc/test_oidc.cpp:265: test_reject_tampered_payload          [PASSED]
test/test_oidc/test_oidc.cpp:266: test_reject_tampered_signature        [PASSED]
test/test_oidc/test_oidc.cpp:267: test_reject_unknown_key               [PASSED]
test/test_oidc/test_oidc.cpp:268: test_reject_malformed                 [PASSED]
--------------- native_oidc:test_oidc [PASSED] Took 0.94 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_oidc    test_oidc  PASSED    00:00:00.944
================= 17 test cases: 17 succeeded in 00:00:00.944 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_vfs in native_vfs environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_vfs/test_vfs.cpp:174: test_write_then_read_file               [PASSED]
test/test_vfs/test_vfs.cpp:175: test_streamed_write_and_read            [PASSED]
test/test_vfs/test_vfs.cpp:176: test_write_mode_truncates               [PASSED]
test/test_vfs/test_vfs.cpp:177: test_append_extends                     [PASSED]
test/test_vfs/test_vfs.cpp:178: test_remove_and_rename                  [PASSED]
test/test_vfs/test_vfs.cpp:179: test_missing_file_fails_closed          [PASSED]
test/test_vfs/test_vfs.cpp:180: test_read_buffer_too_small_fails_closed [PASSED]
test/test_vfs/test_vfs.cpp:181: test_file_full_is_bounded               [PASSED]
test/test_vfs/test_vfs.cpp:182: test_file_pool_exhaustion               [PASSED]
test/test_vfs/test_vfs.cpp:183: test_handle_pool_exhaustion             [PASSED]
test/test_vfs/test_vfs.cpp:184: test_unmounted_fails_closed             [PASSED]
---------------- native_vfs:test_vfs [PASSED] Took 0.73 seconds ----------------

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_vfs     test_vfs  PASSED    00:00:00.729
================= 11 test cases: 11 succeeded in 00:00:00.729 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_graphql in native_graphql environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_graphql/test_graphql.cpp:396: test_malformed_tokens_fail      [PASSED]
test/test_graphql/test_graphql.cpp:397: test_query_keyword_forms_fail   [PASSED]
test/test_graphql/test_graphql.cpp:398: test_pool_limits                [PASSED]
test/test_graphql/test_graphql.cpp:399: test_string_pool_exhaustion     [PASSED]
test/test_graphql/test_graphql.cpp:400: test_resolver_null_typed_value  [PASSED]
test/test_graphql/test_graphql.cpp:401: test_resolver_path_overflow     [PASSED]
test/test_graphql/test_graphql.cpp:402: test_arg_accessors_edges        [PASSED]
test/test_graphql/test_graphql.cpp:403: test_flat_selection             [PASSED]
test/test_graphql/test_graphql.cpp:404: test_string_escapes_decoded     [PASSED]
test/test_graphql/test_graphql.cpp:405: test_number_arg_variants_parse  [PASSED]
test/test_graphql/test_graphql.cpp:406: test_bool_args                  [PASSED]
test/test_graphql/test_graphql.cpp:407: test_null_arg_value             [PASSED]
test/test_graphql/test_graphql.cpp:408: test_control_char_is_unicode_escaped [PASSED]
test/test_graphql/test_graphql.cpp:409: test_unterminated_string_arg_fails [PASSED]
test/test_graphql/test_graphql.cpp:410: test_arg_missing_colon_fails    [PASSED]
test/test_graphql/test_graphql.cpp:411: test_bad_arg_value_fails        [PASSED]
test/test_graphql/test_graphql.cpp:412: test_trailing_junk_fails        [PASSED]
test/test_graphql/test_graphql.cpp:413: test_long_field_name_hits_limit [PASSED]
test/test_graphql/test_graphql.cpp:414: test_null_inputs_fail_closed    [PASSED]
test/test_graphql/test_graphql.cpp:415: test_unknown_operation_keyword_fails [PASSED]
test/test_graphql/test_graphql.cpp:416: test_selection_is_honored       [PASSED]
test/test_graphql/test_graphql.cpp:417: test_nested_object              [PASSED]
test/test_graphql/test_graphql.cpp:418: test_args_collected_along_path  [PASSED]
test/test_graphql/test_graphql.cpp:419: test_scalar_types               [PASSED]
test/test_graphql/test_graphql.cpp:420: test_string_arg_and_escaping    [PASSED]
test/test_graphql/test_graphql.cpp:421: test_unresolved_field_is_null   [PASSED]
test/test_graphql/test_graphql.cpp:422: test_query_keyword_and_name     [PASSED]
test/test_graphql/test_graphql.cpp:423: test_comments_and_commas        [PASSED]
test/test_graphql/test_graphql.cpp:424: test_parse_error_reports_errors [PASSED]
test/test_graphql/test_graphql.cpp:425: test_mutation_rejected          [PASSED]
test/test_graphql/test_graphql.cpp:426: test_depth_limit                [PASSED]
test/test_graphql/test_graphql.cpp:427: test_overflow_fails_closed      [PASSED]
------------ native_graphql:test_graphql [PASSED] Took 0.69 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_graphql  test_graphql  PASSED    00:00:00.689
================= 32 test cases: 32 succeeded in 00:00:00.689 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_espnow in native_espnow environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_espnow/test_espnow.cpp:121: test_encode_decode_roundtrip      [PASSED]
test/test_espnow/test_espnow.cpp:122: test_encode_zero_length           [PASSED]
test/test_espnow/test_espnow.cpp:123: test_encode_rejects_oversize_and_small_buffer [PASSED]
test/test_espnow/test_espnow.cpp:124: test_decode_rejects_corrupt       [PASSED]
test/test_espnow/test_espnow.cpp:125: test_peer_registry                [PASSED]
test/test_espnow/test_espnow.cpp:126: test_peer_table_full_fails_closed [PASSED]
test/test_espnow/test_espnow.cpp:127: test_broadcast_address            [PASSED]
------------- native_espnow:test_espnow [PASSED] Took 0.70 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_espnow  test_espnow  PASSED    00:00:00.705
================== 7 test cases: 7 succeeded in 00:00:00.705 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_oauth2 in native_oauth2 environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_oauth2/test_oauth2.cpp:99: test_build_code_request_minimal    [PASSED]
test/test_oauth2/test_oauth2.cpp:100: test_build_code_request_with_secret_encodes_specials [PASSED]
test/test_oauth2/test_oauth2.cpp:101: test_build_code_request_pkce      [PASSED]
test/test_oauth2/test_oauth2.cpp:102: test_build_refresh_request        [PASSED]
test/test_oauth2/test_oauth2.cpp:103: test_build_overflows_fail_closed  [PASSED]
test/test_oauth2/test_oauth2.cpp:104: test_parse_token_response         [PASSED]
test/test_oauth2/test_oauth2.cpp:105: test_parse_minimal_response       [PASSED]
test/test_oauth2/test_oauth2.cpp:106: test_parse_error_response_fails   [PASSED]
------------- native_oauth2:test_oauth2 [PASSED] Took 0.71 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_oauth2  test_oauth2  PASSED    00:00:00.711
================== 8 test cases: 8 succeeded in 00:00:00.711 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_opcua in native_opcua environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_opcua/test_opcua.cpp:1365: test_parse_read_optional_fields    [PASSED]
test/test_opcua/test_opcua.cpp:1366: test_parse_rejections              [PASSED]
test/test_opcua/test_opcua.cpp:1367: test_build_guards_and_overflow     [PASSED]
test/test_opcua/test_opcua.cpp:1368: test_setters_and_endpoint_url      [PASSED]
test/test_opcua/test_opcua.cpp:1369: test_variant_scalar_types          [PASSED]
test/test_opcua/test_opcua.cpp:1370: test_variant_errors                [PASSED]
test/test_opcua/test_opcua.cpp:1371: test_datavalue_all_masks           [PASSED]
test/test_opcua/test_opcua.cpp:1372: test_nodeid_encodings              [PASSED]
test/test_opcua/test_opcua.cpp:1373: test_reader_underruns              [PASSED]
test/test_opcua/test_opcua.cpp:1374: test_codec_roundtrip               [PASSED]
test/test_opcua/test_opcua.cpp:1375: test_string_null_roundtrip         [PASSED]
test/test_opcua/test_opcua.cpp:1376: test_reader_underrun_latches       [PASSED]
test/test_opcua/test_opcua.cpp:1377: test_writer_overflow_fails_closed  [PASSED]
test/test_opcua/test_opcua.cpp:1378: test_parse_header                  [PASSED]
test/test_opcua/test_opcua.cpp:1379: test_parse_hello                   [PASSED]
test/test_opcua/test_opcua.cpp:1380: test_parse_hello_rejects_short     [PASSED]
test/test_opcua/test_opcua.cpp:1381: test_build_ack_negotiates          [PASSED]
test/test_opcua/test_opcua.cpp:1382: test_nodeid_roundtrip              [PASSED]
test/test_opcua/test_opcua.cpp:1383: test_filetime_from_unix            [PASSED]
test/test_opcua/test_opcua.cpp:1384: test_parse_open                    [PASSED]
test/test_opcua/test_opcua.cpp:1385: test_parse_open_rejects_wrong_type [PASSED]
test/test_opcua/test_opcua.cpp:1386: test_build_open_response           [PASSED]
test/test_opcua/test_opcua.cpp:1387: test_parse_msg                     [PASSED]
test/test_opcua/test_opcua.cpp:1388: test_parse_msg_rejects_non_msg     [PASSED]
test/test_opcua/test_opcua.cpp:1389: test_build_create_session_response [PASSED]
test/test_opcua/test_opcua.cpp:1390: test_build_activate_session_response [PASSED]
test/test_opcua/test_opcua.cpp:1391: test_datavalue_good_int32          [PASSED]
test/test_opcua/test_opcua.cpp:1392: test_datavalue_bad_status          [PASSED]
test/test_opcua/test_opcua.cpp:1393: test_parse_read                    [PASSED]
test/test_opcua/test_opcua.cpp:1394: test_build_read_response           [PASSED]
test/test_opcua/test_opcua.cpp:1395: test_parse_browse                  [PASSED]
test/test_opcua/test_opcua.cpp:1396: test_build_browse_response         [PASSED]
test/test_opcua/test_opcua.cpp:1397: test_build_browse_response_unknown [PASSED]
test/test_opcua/test_opcua.cpp:1398: test_build_close_session_response  [PASSED]
test/test_opcua/test_opcua.cpp:1399: test_build_get_endpoints           [PASSED]
test/test_opcua/test_opcua.cpp:1400: test_build_service_fault           [PASSED]
test/test_opcua/test_opcua.cpp:1401: test_datavalue_roundtrip           [PASSED]
test/test_opcua/test_opcua.cpp:1402: test_parse_and_build_write         [PASSED]
-------------- native_opcua:test_opcua [PASSED] Took 0.71 seconds --------------

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_opcua   test_opcua  PASSED    00:00:00.713
================= 38 test cases: 38 succeeded in 00:00:00.713 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_opcua_client in native_opcua_client environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_opcua_client/test_opcua_client.cpp:413: test_on_read_all_variant_types [PASSED]
test/test_opcua_client/test_opcua_client.cpp:414: test_client_parsers_reject_fault [PASSED]
test/test_opcua_client/test_opcua_client.cpp:415: test_client_parsers_reject_malformed [PASSED]
test/test_opcua_client/test_opcua_client.cpp:416: test_hello_ack_roundtrip [PASSED]
test/test_opcua_client/test_opcua_client.cpp:417: test_open_roundtrip   [PASSED]
test/test_opcua_client/test_opcua_client.cpp:418: test_session_roundtrip [PASSED]
test/test_opcua_client/test_opcua_client.cpp:419: test_get_endpoints_roundtrip [PASSED]
test/test_opcua_client/test_opcua_client.cpp:420: test_service_fault_rejected_by_parsers [PASSED]
test/test_opcua_client/test_opcua_client.cpp:421: test_read_roundtrip   [PASSED]
test/test_opcua_client/test_opcua_client.cpp:422: test_browse_roundtrip [PASSED]
test/test_opcua_client/test_opcua_client.cpp:423: test_write_roundtrip  [PASSED]
test/test_opcua_client/test_opcua_client.cpp:424: test_close_session_roundtrip [PASSED]
test/test_opcua_client/test_opcua_client.cpp:425: test_close_channel_is_clo [PASSED]
test/test_opcua_client/test_opcua_client.cpp:426: test_seq_and_request_id_increment [PASSED]
------- native_opcua_client:test_opcua_client [PASSED] Took 0.73 seconds -------

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_opcua_client  test_opcua_client  PASSED    00:00:00.727
================= 14 test cases: 14 succeeded in 00:00:00.727 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_keepalive in native_keepalive environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
/usr/bin/ld: .pio/build/native_keepalive/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_keepalive/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
--------- native_keepalive:test_keepalive [ERRORED] Took 1.13 seconds ---------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_keepalive  test_keepalive  ERRORED   00:00:01.127
================== 1 test cases: 0 succeeded in 00:00:01.127 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_range in native_range environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
/usr/bin/ld: .pio/build/native_range/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_range/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
------------- native_range:test_range [ERRORED] Took 1.09 seconds -------------

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_range   test_range  ERRORED   00:00:01.092
================== 1 test cases: 0 succeeded in 00:00:01.092 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_syslog in native_syslog environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_syslog/test_syslog.cpp:125: test_pri_local0_info              [PASSED]
test/test_syslog/test_syslog.cpp:126: test_pri_computation_varies       [PASSED]
test/test_syslog/test_syslog.cpp:127: test_nilvalue_for_empty_fields    [PASSED]
test/test_syslog/test_syslog.cpp:128: test_empty_message_ok             [PASSED]
test/test_syslog/test_syslog.cpp:129: test_overflow_returns_zero        [PASSED]
test/test_syslog/test_syslog.cpp:130: test_length_matches_strlen        [PASSED]
test/test_syslog/test_syslog.cpp:131: test_init_and_log_captured        [PASSED]
test/test_syslog/test_syslog.cpp:132: test_log_not_ready_when_no_server [PASSED]
test/test_syslog/test_syslog.cpp:133: test_format_null_and_pri_clamp    [PASSED]
test/test_syslog/test_syslog.cpp:134: test_init_truncates_long_fields   [PASSED]
------------- native_syslog:test_syslog [PASSED] Took 0.72 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_syslog  test_syslog  PASSED    00:00:00.716
================= 10 test cases: 10 succeeded in 00:00:00.716 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_jwt in native_jwt environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_jwt/test_jwt.cpp:266: test_base64url_strict_alphabet          [PASSED]
test/test_jwt/test_jwt.cpp:267: test_verify_malformed_headers           [PASSED]
test/test_jwt/test_jwt.cpp:268: test_bearer_extra_spaces                [PASSED]
test/test_jwt/test_jwt.cpp:269: test_claim_int_edges                    [PASSED]
test/test_jwt/test_jwt.cpp:270: test_claim_str_edges                    [PASSED]
test/test_jwt/test_jwt.cpp:271: test_valid_token_accepts                [PASSED]
test/test_jwt/test_jwt.cpp:272: test_wrong_secret_rejects               [PASSED]
test/test_jwt/test_jwt.cpp:273: test_tampered_payload_rejects           [PASSED]
test/test_jwt/test_jwt.cpp:274: test_tampered_signature_rejects         [PASSED]
test/test_jwt/test_jwt.cpp:275: test_malformed_rejected                 [PASSED]
test/test_jwt/test_jwt.cpp:276: test_alg_not_hs256_rejected             [PASSED]
test/test_jwt/test_jwt.cpp:277: test_bearer_header                      [PASSED]
test/test_jwt/test_jwt.cpp:278: test_claim_int                          [PASSED]
test/test_jwt/test_jwt.cpp:279: test_claim_missing                      [PASSED]
test/test_jwt/test_jwt.cpp:280: test_claim_str                          [PASSED]
test/test_jwt/test_jwt.cpp:281: test_scope_allows                       [PASSED]
---------------- native_jwt:test_jwt [PASSED] Took 0.73 seconds ----------------

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_jwt     test_jwt  PASSED    00:00:00.730
================= 16 test cases: 16 succeeded in 00:00:00.730 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_upload in native_upload environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
/usr/bin/ld: .pio/build/native_upload/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_upload/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
------------ native_upload:test_upload [ERRORED] Took 1.13 seconds ------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_upload  test_upload  ERRORED   00:00:01.129
================== 1 test cases: 0 succeeded in 00:00:01.129 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_http_client in native_http_client environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_http_client/test_http_client.cpp:228: test_url_edge_rejections [PASSED]
test/test_http_client/test_http_client.cpp:229: test_build_edge_rejections [PASSED]
test/test_http_client/test_http_client.cpp:230: test_response_edge_rejections [PASSED]
test/test_http_client/test_http_client.cpp:231: test_host_transport_stubs [PASSED]
test/test_http_client/test_http_client.cpp:232: test_url_http_default   [PASSED]
test/test_http_client/test_http_client.cpp:233: test_url_https_port_nopath [PASSED]
test/test_http_client/test_http_client.cpp:234: test_url_bad_scheme     [PASSED]
test/test_http_client/test_http_client.cpp:235: test_build_get          [PASSED]
test/test_http_client/test_http_client.cpp:236: test_build_post_with_body_and_port [PASSED]
test/test_http_client/test_http_client.cpp:237: test_parse_content_length [PASSED]
test/test_http_client/test_http_client.cpp:238: test_parse_status_404   [PASSED]
test/test_http_client/test_http_client.cpp:239: test_parse_chunked      [PASSED]
test/test_http_client/test_http_client.cpp:240: test_parse_chunked_oversize_size_clamped [PASSED]
test/test_http_client/test_http_client.cpp:241: test_parse_connection_close_body [PASSED]
test/test_http_client/test_http_client.cpp:242: test_parse_malformed    [PASSED]
-------- native_http_client:test_http_client [PASSED] Took 0.69 seconds --------

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_http_client  test_http_client  PASSED    00:00:00.690
================= 15 test cases: 15 succeeded in 00:00:00.690 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_compliance in native_compliance environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_compliance/test_compliance.cpp:168: test_http11_missing_host_rejected [PASSED]
test/test_compliance/test_compliance.cpp:169: test_http11_with_host_ok  [PASSED]
test/test_compliance/test_compliance.cpp:170: test_http10_missing_host_ok [PASSED]
test/test_compliance/test_compliance.cpp:171: test_duplicate_host_rejected [PASSED]
test/test_compliance/test_compliance.cpp:172: test_duplicate_host_rejected_http10 [PASSED]
test/test_compliance/test_compliance.cpp:173: test_host_beyond_max_headers_still_counted [PASSED]
test/test_compliance/test_compliance.cpp:174: test_duplicate_host_with_one_beyond_cap_rejected [PASSED]
test/test_compliance/test_compliance.cpp:176: test_content_length_non_digit_rejected [PASSED]
test/test_compliance/test_compliance.cpp:177: test_content_length_empty_rejected [PASSED]
test/test_compliance/test_compliance.cpp:178: test_content_length_conflicting_duplicate_rejected [PASSED]
test/test_compliance/test_compliance.cpp:179: test_content_length_matching_duplicate_ok [PASSED]
test/test_compliance/test_compliance.cpp:180: test_content_length_valid_body [PASSED]
test/test_compliance/test_compliance.cpp:182: test_transfer_encoding_chunked_rejected [PASSED]
test/test_compliance/test_compliance.cpp:183: test_transfer_encoding_with_content_length_rejected [PASSED]
test/test_compliance/test_compliance.cpp:184: test_transfer_encoding_case_insensitive_rejected [PASSED]
--------- native_compliance:test_compliance [PASSED] Took 0.70 seconds ---------

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_compliance  test_compliance  PASSED    00:00:00.698
================= 15 test cases: 15 succeeded in 00:00:00.698 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_mqtt in native_mqtt environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_mqtt/test_mqtt.cpp:410: test_build_guards_and_overflow        [PASSED]
test/test_mqtt/test_mqtt.cpp:411: test_parse_guards                     [PASSED]
test/test_mqtt/test_mqtt.cpp:412: test_host_transport_stubs             [PASSED]
test/test_mqtt/test_mqtt.cpp:413: test_remlen_boundaries                [PASSED]
test/test_mqtt/test_mqtt.cpp:414: test_remlen_too_big                   [PASSED]
test/test_mqtt/test_mqtt.cpp:415: test_remlen_decode_incomplete         [PASSED]
test/test_mqtt/test_mqtt.cpp:416: test_remlen_decode_malformed          [PASSED]
test/test_mqtt/test_mqtt.cpp:417: test_connect_minimal                  [PASSED]
test/test_mqtt/test_mqtt.cpp:418: test_connect_full                     [PASSED]
test/test_mqtt/test_mqtt.cpp:419: test_publish_qos0_roundtrip           [PASSED]
test/test_mqtt/test_mqtt.cpp:420: test_publish_qos1_flags_and_id        [PASSED]
test/test_mqtt/test_mqtt.cpp:421: test_publish_topic_overflow_rejected  [PASSED]
test/test_mqtt/test_mqtt.cpp:422: test_publish_qos3_rejected            [PASSED]
test/test_mqtt/test_mqtt.cpp:423: test_publish_wildcard_topic_rejected  [PASSED]
test/test_mqtt/test_mqtt.cpp:424: test_publish_topic_nul_or_bad_utf8_rejected [PASSED]
test/test_mqtt/test_mqtt.cpp:425: test_subscribe                        [PASSED]
test/test_mqtt/test_mqtt.cpp:426: test_unsubscribe                      [PASSED]
test/test_mqtt/test_mqtt.cpp:427: test_ack_packets                      [PASSED]
test/test_mqtt/test_mqtt.cpp:428: test_connack                          [PASSED]
test/test_mqtt/test_mqtt.cpp:429: test_suback                           [PASSED]
test/test_mqtt/test_mqtt.cpp:430: test_ping_disconnect                  [PASSED]
test/test_mqtt/test_mqtt.cpp:431: test_fixed_header_multibyte_remlen    [PASSED]
--------------- native_mqtt:test_mqtt [PASSED] Took 0.68 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_mqtt    test_mqtt  PASSED    00:00:00.676
================= 22 test cases: 22 succeeded in 00:00:00.676 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_ws_client in native_ws_client environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ws_client/test_ws_client.cpp:246: test_accept_for_key_guards  [PASSED]
test/test_ws_client/test_ws_client.cpp:247: test_build_handshake_guards [PASSED]
test/test_ws_client/test_ws_client.cpp:248: test_check_response_guards  [PASSED]
test/test_ws_client/test_ws_client.cpp:249: test_build_frame_guards_and_64bit [PASSED]
test/test_ws_client/test_ws_client.cpp:250: test_parse_frame_edges      [PASSED]
test/test_ws_client/test_ws_client.cpp:251: test_host_transport_stubs   [PASSED]
test/test_ws_client/test_ws_client.cpp:252: test_accept_rfc_example     [PASSED]
test/test_ws_client/test_ws_client.cpp:253: test_build_handshake        [PASSED]
test/test_ws_client/test_ws_client.cpp:254: test_check_response_ok      [PASSED]
test/test_ws_client/test_ws_client.cpp:255: test_check_response_bad_accept [PASSED]
test/test_ws_client/test_ws_client.cpp:256: test_check_response_not_101 [PASSED]
test/test_ws_client/test_ws_client.cpp:257: test_build_frame_masked     [PASSED]
test/test_ws_client/test_ws_client.cpp:258: test_build_frame_extended_len [PASSED]
test/test_ws_client/test_ws_client.cpp:259: test_parse_frame_server_text [PASSED]
test/test_ws_client/test_ws_client.cpp:260: test_parse_frame_incomplete [PASSED]
test/test_ws_client/test_ws_client.cpp:261: test_parse_frame_extended_len [PASSED]
---------- native_ws_client:test_ws_client [PASSED] Took 0.73 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_ws_client  test_ws_client  PASSED    00:00:00.731
================= 16 test cases: 16 succeeded in 00:00:00.731 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_scratch in native_scratch environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_scratch/test_scratch.cpp:160: test_alloc_returns_nonnull_and_advances_used [PASSED]
test/test_scratch/test_scratch.cpp:161: test_sequential_allocs_are_distinct_and_ordered [PASSED]
test/test_scratch/test_scratch.cpp:162: test_reset_frees_all_and_reuses_base [PASSED]
test/test_scratch/test_scratch.cpp:163: test_alignment_is_honored       [PASSED]
test/test_scratch/test_scratch.cpp:164: test_exhaustion_returns_null_without_corrupting_arena [PASSED]
test/test_scratch/test_scratch.cpp:165: test_alloc_larger_than_capacity_returns_null [PASSED]
test/test_scratch/test_scratch.cpp:166: test_alignment_padding_cannot_overflow_arena [PASSED]
test/test_scratch/test_scratch.cpp:167: test_high_water_bounds          [PASSED]
test/test_scratch/test_scratch.cpp:168: test_zero_size_alloc_returns_nonnull_when_space [PASSED]
test/test_scratch/test_scratch.cpp:169: test_mark_release_reclaims      [PASSED]
test/test_scratch/test_scratch.cpp:170: test_release_allows_reuse_of_same_region [PASSED]
test/test_scratch/test_scratch.cpp:171: test_scratch_scope_releases_on_scope_exit [PASSED]
test/test_scratch/test_scratch.cpp:172: test_nested_scopes_reclaim_lifo [PASSED]
test/test_scratch/test_scratch.cpp:173: test_sequential_scopes_do_not_accumulate [PASSED]
------------ native_scratch:test_scratch [PASSED] Took 0.71 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_scratch  test_scratch  PASSED    00:00:00.713
================= 14 test cases: 14 succeeded in 00:00:00.713 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_snmp_trap in native_snmp_trap environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_snmp_trap/test_snmp_trap.cpp:226: test_trap_v2c_structure     [PASSED]
test/test_snmp_trap/test_snmp_trap.cpp:227: test_all_varbind_types      [PASSED]
test/test_snmp_trap/test_snmp_trap.cpp:228: test_invalid_varbind_type   [PASSED]
test/test_snmp_trap/test_snmp_trap.cpp:229: test_build_v2c_null_args    [PASSED]
test/test_snmp_trap/test_snmp_trap.cpp:230: test_host_transport_stubs   [PASSED]
test/test_snmp_trap/test_snmp_trap.cpp:231: test_inform_tag             [PASSED]
test/test_snmp_trap/test_snmp_trap.cpp:232: test_buffer_too_small       [PASSED]
---------- native_snmp_trap:test_snmp_trap [PASSED] Took 0.72 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_snmp_trap  test_snmp_trap  PASSED    00:00:00.720
================== 7 test cases: 7 succeeded in 00:00:00.720 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_inflate in native_inflate environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_inflate/test_inflate.cpp:169: test_fixed_huffman              [PASSED]
test/test_inflate/test_inflate.cpp:170: test_back_references            [PASSED]
test/test_inflate/test_inflate.cpp:171: test_stored_block               [PASSED]
test/test_inflate/test_inflate.cpp:172: test_dynamic_huffman            [PASSED]
test/test_inflate/test_inflate.cpp:173: test_empty_message              [PASSED]
test/test_inflate/test_inflate.cpp:174: test_permessage_deflate_marker  [PASSED]
test/test_inflate/test_inflate.cpp:175: test_permessage_deflate_back_references [PASSED]
test/test_inflate/test_inflate.cpp:176: test_output_overflow_fails_closed [PASSED]
test/test_inflate/test_inflate.cpp:177: test_scratch_too_small_fails_closed [PASSED]
test/test_inflate/test_inflate.cpp:178: test_truncated_input_is_malformed [PASSED]
test/test_inflate/test_inflate.cpp:179: test_reserved_block_type_is_malformed [PASSED]
test/test_inflate/test_inflate.cpp:180: test_corrupt_stored_nlen_is_malformed [PASSED]
------------ native_inflate:test_inflate [PASSED] Took 0.68 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_inflate  test_inflate  PASSED    00:00:00.681
================= 12 test cases: 12 succeeded in 00:00:00.681 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_deflate in native_deflate environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_deflate/test_deflate.cpp:161: test_roundtrip_text             [PASSED]
test/test_deflate/test_deflate.cpp:162: test_roundtrip_empty            [PASSED]
test/test_deflate/test_deflate.cpp:163: test_roundtrip_single_byte      [PASSED]
test/test_deflate/test_deflate.cpp:164: test_roundtrip_all_byte_values  [PASSED]
test/test_deflate/test_deflate.cpp:165: test_compresses_repetitive      [PASSED]
test/test_deflate/test_deflate.cpp:166: test_compresses_json            [PASSED]
test/test_deflate/test_deflate.cpp:167: test_fuzz_roundtrip             [PASSED]
test/test_deflate/test_deflate.cpp:168: test_fuzz_low_entropy_roundtrip [PASSED]
test/test_deflate/test_deflate.cpp:169: test_output_overflow_fails_closed [PASSED]
test/test_deflate/test_deflate.cpp:170: test_scratch_too_small_fails_closed [PASSED]
------------ native_deflate:test_deflate [PASSED] Took 0.73 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_deflate  test_deflate  PASSED    00:00:00.731
================= 10 test cases: 10 succeeded in 00:00:00.731 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_websocket in native_ws_deflate environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
/usr/bin/ld: .pio/build/native_ws_deflate/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_ws_deflate/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
--------- native_ws_deflate:test_websocket [ERRORED] Took 1.04 seconds ---------

=================================== SUMMARY ===================================
Environment        Test            Status    Duration
-----------------  --------------  --------  ------------
native_ws_deflate  test_websocket  ERRORED   00:00:01.042
================== 1 test cases: 0 succeeded in 00:00:01.042 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_time_source in native_time_source environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_time_source/test_time_source.cpp:129: test_single_source      [PASSED]
test/test_time_source/test_time_source.cpp:130: test_priority_order_lowest_value_wins [PASSED]
test/test_time_source/test_time_source.cpp:131: test_falls_back_when_primary_unavailable [PASSED]
test/test_time_source/test_time_source.cpp:132: test_all_unavailable_returns_zero [PASSED]
test/test_time_source/test_time_source.cpp:133: test_first_valid_short_circuits [PASSED]
test/test_time_source/test_time_source.cpp:134: test_fallback_queries_in_priority_order [PASSED]
test/test_time_source/test_time_source.cpp:135: test_table_full_rejects [PASSED]
test/test_time_source/test_time_source.cpp:136: test_null_fn_rejected   [PASSED]
test/test_time_source/test_time_source.cpp:137: test_reset_clears_sources [PASSED]
-------- native_time_source:test_time_source [PASSED] Took 0.68 seconds --------

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_time_source  test_time_source  PASSED    00:00:00.676
================== 9 test cases: 9 succeeded in 00:00:00.676 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_config_store in native_config_store environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_config_store/test_config_store.cpp:149: test_str_round_trip   [PASSED]
test/test_config_store/test_config_store.cpp:150: test_str_default_when_missing [PASSED]
test/test_config_store/test_config_store.cpp:151: test_str_overwrite    [PASSED]
test/test_config_store/test_config_store.cpp:152: test_str_truncates_to_capacity [PASSED]
test/test_config_store/test_config_store.cpp:153: test_u32_round_trip   [PASSED]
test/test_config_store/test_config_store.cpp:154: test_u32_default_when_missing [PASSED]
test/test_config_store/test_config_store.cpp:155: test_blob_round_trip  [PASSED]
test/test_config_store/test_config_store.cpp:156: test_blob_bounded_by_capacity [PASSED]
test/test_config_store/test_config_store.cpp:157: test_blob_missing_returns_zero [PASSED]
test/test_config_store/test_config_store.cpp:158: test_erase_removes_key [PASSED]
test/test_config_store/test_config_store.cpp:159: test_clear_wipes_namespace [PASSED]
test/test_config_store/test_config_store.cpp:160: test_table_full_rejects_new_key [PASSED]
test/test_config_store/test_config_store.cpp:161: test_existing_key_overwrites_even_when_full [PASSED]
test/test_config_store/test_config_store.cpp:162: test_key_too_long_rejected [PASSED]
------- native_config_store:test_config_store [PASSED] Took 0.69 seconds -------

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_config_store  test_config_store  PASSED    00:00:00.690
================= 14 test cases: 14 succeeded in 00:00:00.690 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_device_id in native_device_id environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_device_id/test_device_id.cpp:58: test_uuid_matches_reference_aabbccddeeff [PASSED]
test/test_device_id/test_device_id.cpp:59: test_uuid_matches_reference_001122334455 [PASSED]
test/test_device_id/test_device_id.cpp:60: test_uuid_is_deterministic   [PASSED]
test/test_device_id/test_device_id.cpp:61: test_uuid_version_and_variant_bits [PASSED]
---------- native_device_id:test_device_id [PASSED] Took 0.68 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_device_id  test_device_id  PASSED    00:00:00.685
================== 4 test cases: 4 succeeded in 00:00:00.685 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_auth_lockout in native_auth_lockout environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_auth_lockout/test_auth_lockout.cpp:144: test_below_threshold_not_locked [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:145: test_locks_at_threshold [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:146: test_exponential_backoff [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:147: test_caps_at_max      [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:148: test_expires_after_window [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:149: test_success_clears   [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:150: test_isolates_addresses [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:151: test_zero_ip_never_locked [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:152: test_table_full_tracks_new_address [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:153: test_active_lockout_survives_eviction [PASSED]
------- native_auth_lockout:test_auth_lockout [PASSED] Took 0.68 seconds -------

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_auth_lockout  test_auth_lockout  PASSED    00:00:00.681
================= 10 test cases: 10 succeeded in 00:00:00.681 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_csrf in native_csrf environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_csrf/test_csrf.cpp:114: test_issue_verify_roundtrip           [PASSED]
test/test_csrf/test_csrf.cpp:115: test_token_format_and_length          [PASSED]
test/test_csrf/test_csrf.cpp:116: test_verify_rejects_tampered_sig      [PASSED]
test/test_csrf/test_csrf.cpp:117: test_verify_rejects_tampered_nonce    [PASSED]
test/test_csrf/test_csrf.cpp:118: test_verify_rejects_garbage           [PASSED]
test/test_csrf/test_csrf.cpp:119: test_different_secret_rejects         [PASSED]
test/test_csrf/test_csrf.cpp:120: test_no_secret_fails_closed           [PASSED]
test/test_csrf/test_csrf.cpp:121: test_issue_unique                     [PASSED]
test/test_csrf/test_csrf.cpp:122: test_issue_rejects_small_buffer       [PASSED]
--------------- native_csrf:test_csrf [PASSED] Took 0.74 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_csrf    test_csrf  PASSED    00:00:00.741
================== 9 test cases: 9 succeeded in 00:00:00.741 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_telemetry in native_telemetry environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_telemetry/test_telemetry.cpp:121: test_window_classic_stats   [PASSED]
test/test_telemetry/test_telemetry.cpp:122: test_window_empty           [PASSED]
test/test_telemetry/test_telemetry.cpp:123: test_window_single_sample   [PASSED]
test/test_telemetry/test_telemetry.cpp:124: test_window_eviction        [PASSED]
test/test_telemetry/test_telemetry.cpp:125: test_rate_basic             [PASSED]
test/test_telemetry/test_telemetry.cpp:126: test_rate_zero_dt           [PASSED]
test/test_telemetry/test_telemetry.cpp:127: test_totalizer_constant_rate [PASSED]
test/test_telemetry/test_telemetry.cpp:128: test_totalizer_trapezoid_and_reset [PASSED]
---------- native_telemetry:test_telemetry [PASSED] Took 0.69 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_telemetry  test_telemetry  PASSED    00:00:00.687
================== 8 test cases: 8 succeeded in 00:00:00.687 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_dashboard in native_dashboard environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_dashboard/test_dashboard.cpp:220: test_layout_bar_sparkline_types [PASSED]
test/test_dashboard/test_dashboard.cpp:221: test_null_widget_table_guards [PASSED]
test/test_dashboard/test_dashboard.cpp:222: test_json_overflow_paths    [PASSED]
test/test_dashboard/test_dashboard.cpp:223: test_parse_control_edges    [PASSED]
test/test_dashboard/test_dashboard.cpp:224: test_layout_json            [PASSED]
test/test_dashboard/test_dashboard.cpp:225: test_values_json_initial_zero [PASSED]
test/test_dashboard/test_dashboard.cpp:226: test_set_and_values         [PASSED]
test/test_dashboard/test_dashboard.cpp:227: test_set_unknown_key        [PASSED]
test/test_dashboard/test_dashboard.cpp:228: test_configure_resets_values [PASSED]
test/test_dashboard/test_dashboard.cpp:229: test_small_buffer_fails_closed [PASSED]
test/test_dashboard/test_dashboard.cpp:230: test_parse_control_ok       [PASSED]
test/test_dashboard/test_dashboard.cpp:231: test_parse_control_float    [PASSED]
test/test_dashboard/test_dashboard.cpp:232: test_parse_control_rejects_malformed [PASSED]
test/test_dashboard/test_dashboard.cpp:233: test_dispatch_control_invokes_cb [PASSED]
test/test_dashboard/test_dashboard.cpp:234: test_layout_control_types   [PASSED]
---------- native_dashboard:test_dashboard [PASSED] Took 0.70 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_dashboard  test_dashboard  PASSED    00:00:00.697
================= 15 test cases: 15 succeeded in 00:00:00.697 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_net_egress in native_net_egress environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_net_egress/test_net_egress.cpp:60: test_classify_sta          [PASSED]
test/test_net_egress/test_net_egress.cpp:61: test_classify_ap           [PASSED]
test/test_net_egress/test_net_egress.cpp:62: test_classify_eth          [PASSED]
test/test_net_egress/test_net_egress.cpp:63: test_classify_none         [PASSED]
test/test_net_egress/test_net_egress.cpp:64: test_egress_host_stub      [PASSED]
test/test_net_egress/test_net_egress.cpp:65: test_eth_host_stub         [PASSED]
--------- native_net_egress:test_net_egress [PASSED] Took 0.68 seconds ---------

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_net_egress  test_net_egress  PASSED    00:00:00.683
================== 6 test cases: 6 succeeded in 00:00:00.683 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_partition_monitor in native_partition environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_partition_monitor/test_partition_monitor.cpp:71: test_kind_app [PASSED]
test/test_partition_monitor/test_partition_monitor.cpp:72: test_kind_data [PASSED]
test/test_partition_monitor/test_partition_monitor.cpp:73: test_json    [PASSED]
test/test_partition_monitor/test_partition_monitor.cpp:74: test_json_small_buffer_fails_closed [PASSED]
test/test_partition_monitor/test_partition_monitor.cpp:75: test_collect_host_stub [PASSED]
------ native_partition:test_partition_monitor [PASSED] Took 0.70 seconds ------

=================================== SUMMARY ===================================
Environment       Test                    Status    Duration
----------------  ----------------------  --------  ------------
native_partition  test_partition_monitor  PASSED    00:00:00.695
================== 5 test cases: 5 succeeded in 00:00:00.695 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_cbor in native_cbor environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_cbor/test_cbor.cpp:372: test_uint                             [PASSED]
test/test_cbor/test_cbor.cpp:373: test_peek_each_type                   [PASSED]
test/test_cbor/test_cbor.cpp:374: test_uint_8byte                       [PASSED]
test/test_cbor/test_cbor.cpp:375: test_read_double_encoded_float        [PASSED]
test/test_cbor/test_cbor.cpp:376: test_read_map_type_mismatch           [PASSED]
test/test_cbor/test_cbor.cpp:377: test_int                              [PASSED]
test/test_cbor/test_cbor.cpp:378: test_text                             [PASSED]
test/test_cbor/test_cbor.cpp:379: test_bytes                            [PASSED]
test/test_cbor/test_cbor.cpp:380: test_simple                           [PASSED]
test/test_cbor/test_cbor.cpp:381: test_float                            [PASSED]
test/test_cbor/test_cbor.cpp:382: test_array_and_map                    [PASSED]
test/test_cbor/test_cbor.cpp:383: test_overflow_fails_closed            [PASSED]
test/test_cbor/test_cbor.cpp:384: test_decode_uint                      [PASSED]
test/test_cbor/test_cbor.cpp:385: test_decode_int                       [PASSED]
test/test_cbor/test_cbor.cpp:386: test_decode_float_roundtrip           [PASSED]
test/test_cbor/test_cbor.cpp:387: test_decode_roundtrip_map             [PASSED]
test/test_cbor/test_cbor.cpp:388: test_decode_truncated                 [PASSED]
test/test_cbor/test_cbor.cpp:389: test_decode_type_mismatch             [PASSED]
--------------- native_cbor:test_cbor [PASSED] Took 0.68 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_cbor    test_cbor  PASSED    00:00:00.683
================= 18 test cases: 18 succeeded in 00:00:00.683 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_msgpack in native_msgpack environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_msgpack/test_msgpack.cpp:495: test_uint                       [PASSED]
test/test_msgpack/test_msgpack.cpp:496: test_wide_roundtrip             [PASSED]
test/test_msgpack/test_msgpack.cpp:497: test_decode_wide_fails_closed   [PASSED]
test/test_msgpack/test_msgpack.cpp:498: test_int                        [PASSED]
test/test_msgpack/test_msgpack.cpp:499: test_str                        [PASSED]
test/test_msgpack/test_msgpack.cpp:500: test_bytes                      [PASSED]
test/test_msgpack/test_msgpack.cpp:501: test_simple                     [PASSED]
test/test_msgpack/test_msgpack.cpp:502: test_float                      [PASSED]
test/test_msgpack/test_msgpack.cpp:503: test_array_and_map              [PASSED]
test/test_msgpack/test_msgpack.cpp:504: test_overflow_fails_closed      [PASSED]
test/test_msgpack/test_msgpack.cpp:505: test_decode_uint                [PASSED]
test/test_msgpack/test_msgpack.cpp:506: test_decode_int                 [PASSED]
test/test_msgpack/test_msgpack.cpp:507: test_decode_str_and_bytes       [PASSED]
test/test_msgpack/test_msgpack.cpp:508: test_decode_simple_and_float    [PASSED]
test/test_msgpack/test_msgpack.cpp:509: test_decode_array_and_map       [PASSED]
test/test_msgpack/test_msgpack.cpp:510: test_decode_roundtrip           [PASSED]
test/test_msgpack/test_msgpack.cpp:511: test_decode_fails_closed        [PASSED]
------------ native_msgpack:test_msgpack [PASSED] Took 0.69 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_msgpack  test_msgpack  PASSED    00:00:00.691
================= 17 test cases: 17 succeeded in 00:00:00.691 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_gpio_map in native_gpio_map environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_gpio_map/test_gpio_map.cpp:107: test_dir_name                 [PASSED]
test/test_gpio_map/test_gpio_map.cpp:108: test_json                     [PASSED]
test/test_gpio_map/test_gpio_map.cpp:109: test_json_empty               [PASSED]
test/test_gpio_map/test_gpio_map.cpp:110: test_json_small_buffer_fails_closed [PASSED]
test/test_gpio_map/test_gpio_map.cpp:111: test_parse_set                [PASSED]
test/test_gpio_map/test_gpio_map.cpp:112: test_parse_set_rejects_partial [PASSED]
test/test_gpio_map/test_gpio_map.cpp:113: test_parse_set_no_prefix_match [PASSED]
test/test_gpio_map/test_gpio_map.cpp:114: test_is_output                [PASSED]
----------- native_gpio_map:test_gpio_map [PASSED] Took 0.69 seconds -----------

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_gpio_map  test_gpio_map  PASSED    00:00:00.689
================== 8 test cases: 8 succeeded in 00:00:00.689 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_udp_telemetry in native_udp_telemetry environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_udp_telemetry/test_udp_telemetry.cpp:101: test_int_and_uint_fields [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:102: test_float_field    [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:103: test_no_fields_not_ok [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:104: test_overflow_fails_closed [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:105: test_tags_and_timestamp [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:106: test_tag_escaping   [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:107: test_tag_after_field_fails_closed [PASSED]
------ native_udp_telemetry:test_udp_telemetry [PASSED] Took 0.70 seconds ------

=================================== SUMMARY ===================================
Environment           Test                Status    Duration
--------------------  ------------------  --------  ------------
native_udp_telemetry  test_udp_telemetry  PASSED    00:00:00.702
================== 7 test cases: 7 succeeded in 00:00:00.702 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_guardrails in native_guardrails environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_guardrails/test_guardrails.cpp:63: test_eval_all_clear        [PASSED]
test/test_guardrails/test_guardrails.cpp:64: test_eval_heap_breach      [PASSED]
test/test_guardrails/test_guardrails.cpp:65: test_eval_frag_and_stack   [PASSED]
test/test_guardrails/test_guardrails.cpp:66: test_eval_all_breached     [PASSED]
test/test_guardrails/test_guardrails.cpp:67: test_json                  [PASSED]
test/test_guardrails/test_guardrails.cpp:68: test_json_small_buffer_fails_closed [PASSED]
--------- native_guardrails:test_guardrails [PASSED] Took 0.68 seconds ---------

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_guardrails  test_guardrails  PASSED    00:00:00.676
================== 6 test cases: 6 succeeded in 00:00:00.676 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_logbuf in native_logbuf environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_logbuf/test_logbuf.cpp:81: test_append_and_order              [PASSED]
test/test_logbuf/test_logbuf.cpp:82: test_dump                          [PASSED]
test/test_logbuf/test_logbuf.cpp:83: test_rotation_drops_oldest         [PASSED]
test/test_logbuf/test_logbuf.cpp:84: test_trap_threshold                [PASSED]
------------- native_logbuf:test_logbuf [PASSED] Took 0.70 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_logbuf  test_logbuf  PASSED    00:00:00.697
================== 4 test cases: 4 succeeded in 00:00:00.697 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_config_io in native_config_io environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_config_io/test_config_io.cpp:81: test_export_format           [PASSED]
test/test_config_io/test_config_io.cpp:82: test_round_trip              [PASSED]
test/test_config_io/test_config_io.cpp:83: test_import_skips_unknown_keys [PASSED]
test/test_config_io/test_config_io.cpp:84: test_export_overflow_fails_closed [PASSED]
---------- native_config_io:test_config_io [PASSED] Took 0.71 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_config_io  test_config_io  PASSED    00:00:00.709
================== 4 test cases: 4 succeeded in 00:00:00.709 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_workers in native_workers environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
/usr/bin/ld: .pio/build/native_workers/src/network_drivers/transport/transport.o: in function `det_conn_remote_key(unsigned char)':
transport.cpp:(.text+0x9ae): undefined reference to `det_ip_key(DetIp const*)'
collect2: error: ld returned 1 exit status
*** [.pio/build/native_workers/program] Error 1
Building stage has failed, see errors above. Use `pio test -vvv` option to enable verbose output.
----------- native_workers:test_workers [ERRORED] Took 0.79 seconds -----------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_workers  test_workers  ERRORED   00:00:00.789
================== 1 test cases: 0 succeeded in 00:00:00.789 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_clock in native_clock environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_clock/test_clock.cpp:126: test_default_is_platform_millis     [PASSED]
test/test_clock/test_clock.cpp:127: test_custom_clock_divides_to_1000hz [PASSED]
test/test_clock/test_clock.cpp:128: test_sub_khz_source_not_divided     [PASSED]
test/test_clock/test_clock.cpp:129: test_revert_to_default              [PASSED]
test/test_clock/test_clock.cpp:130: test_micros_custom_divides_to_1mhz  [PASSED]
test/test_clock/test_clock.cpp:131: test_latency_stat_records_and_budgets [PASSED]
test/test_clock/test_clock.cpp:132: test_latency_budget_zero_disables   [PASSED]
-------------- native_clock:test_clock [PASSED] Took 0.66 seconds --------------

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_clock   test_clock  PASSED    00:00:00.662
================== 7 test cases: 7 succeeded in 00:00:00.662 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_concurrency in native_concurrency environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_concurrency/test_concurrency.cpp:109: test_spsc_ring_no_race  [PASSED]
test/test_concurrency/test_concurrency.cpp:110: test_state_handoff_no_race [PASSED]
-------- native_concurrency:test_concurrency [PASSED] Took 0.85 seconds --------

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_concurrency  test_concurrency  PASSED    00:00:00.848
================== 2 test cases: 2 succeeded in 00:00:00.848 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 143 tests

Processing test_concurrency in native_tsan environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_concurrency/test_concurrency.cpp:109: test_spsc_ring_no_race  [PASSED]
test/test_concurrency/test_concurrency.cpp:110: test_state_handoff_no_race [PASSED]
----------- native_tsan:test_concurrency [PASSED] Took 1.22 seconds ------------

=================================== SUMMARY ===================================
Environment    Test              Status    Duration
-------------  ----------------  --------  ------------
native_tsan    test_concurrency  PASSED    00:00:01.219
================== 2 test cases: 2 succeeded in 00:00:01.219 ==================
```

</details>
