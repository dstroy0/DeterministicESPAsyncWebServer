# Test Report

**Generated:** 2026-07-19 20:15:01
**Command:** `pio test` over 247 auto-discovered native envs (excludes native_pentest, native_codeql)
**Result:** ✅ 3611 passed - 279s

---

## Summary

| Suite                    | Environment              | Tests | Status |     Duration |
| :----------------------- | :----------------------- | ----: | :----: | -----------: |
| `test_canopen`           | `native_canopen`         |    17 |   ✅   | 00:00:04.991 |
| `test_cia402`            | `native_cia402`          |    15 |   ✅   | 00:00:00.708 |
| `test_control`           | `native_control`         |    17 |   ✅   | 00:00:00.685 |
| `test_dbm`               | `native_dbm`             |    13 |   ✅   | 00:00:00.752 |
| `test_docstore`          | `native_docstore`        |     7 |   ✅   | 00:00:00.799 |
| `test_dnc`               | `native_dnc`             |    14 |   ✅   | 00:00:00.710 |
| `test_dnc_stream`        | `native_dnc`             |     8 |   ✅   | 00:00:00.534 |
| `test_ftp`               | `native_ftp`             |    21 |   ✅   | 00:00:00.728 |
| `test_httpcache`         | `native_httpcache`       |    15 |   ✅   | 00:00:00.697 |
| `test_edge_cache`        | `native_edge_cache`      |    27 |   ✅   | 00:00:00.965 |
| `test_edge_fetch`        | `native_edge_cache`      |     7 |   ✅   | 00:00:00.548 |
| `test_edge_cache_sd`     | `native_edge_cache_sd`   |    15 |   ✅   | 00:00:00.915 |
| `test_edge_mesh`         | `native_edge_mesh`       |    14 |   ✅   | 00:00:00.955 |
| `test_det_primitives`    | `native_det_primitives`  |     5 |   ✅   | 00:00:00.655 |
| `test_det_ip`            | `native_det_ip`          |    11 |   ✅   | 00:00:00.673 |
| `test_det_arena`         | `native_det_arena`       |    19 |   ✅   | 00:00:00.744 |
| `test_ssh_ed25519`       | `native_ssh_ed25519`     |    19 |   ✅   | 00:00:03.440 |
| `test_crypto_kat`        | `native_crypto_kat`      |     9 |   ✅   | 00:00:01.965 |
| `test_promisc`           | `native_promisc`         |     9 |   ✅   | 00:00:00.684 |
| `test_bus_capture`       | `native_bus_capture`     |     7 |   ✅   | 00:00:00.682 |
| `test_j1939`             | `native_j1939`           |    11 |   ✅   | 00:00:00.697 |
| `test_devicenet`         | `native_devicenet`       |    11 |   ✅   | 00:00:00.697 |
| `test_nmea2000`          | `native_nmea2000`        |     7 |   ✅   | 00:00:00.906 |
| `test_mbus`              | `native_mbus`            |    12 |   ✅   | 00:00:00.677 |
| `test_iec60870`          | `native_iec60870`        |    13 |   ✅   | 00:00:00.671 |
| `test_sdi12`             | `native_sdi12`           |     7 |   ✅   | 00:00:00.675 |
| `test_dmx`               | `native_dmx`             |     6 |   ✅   | 00:00:00.684 |
| `test_nmea0183`          | `native_nmea0183`        |     8 |   ✅   | 00:00:00.757 |
| `test_iolink`            | `native_iolink`          |     6 |   ✅   | 00:00:01.106 |
| `test_presentation`      | `native`                 |    63 |   ✅   | 00:00:01.299 |
| `test_http_parser`       | `native`                 |    93 |   ✅   | 00:00:00.651 |
| `test_transport`         | `native`                 |    45 |   ✅   | 00:00:00.648 |
| `test_session`           | `native`                 |    19 |   ✅   | 00:00:00.608 |
| `test_websocket`         | `native`                 |    69 |   ✅   | 00:00:00.676 |
| `test_sse`               | `native`                 |    46 |   ✅   | 00:00:00.635 |
| `test_observability`     | `native_observability`   |    17 |   ✅   | 00:00:00.787 |
| `test_accept_gate`       | `native_accept_gate`     |    13 |   ✅   | 00:00:01.126 |
| `test_http_ota`          | `native_ota`             |     3 |   ✅   | 00:00:00.721 |
| `test_provisioning`      | `native_prov`            |     7 |   ✅   | 00:00:00.709 |
| `test_ssh_channel`       | `native_ssh`             |    40 |   ✅   | 00:00:01.323 |
| `test_ssh_auth`          | `native_ssh`             |    21 |   ✅   | 00:00:02.904 |
| `test_ssh_crypto`        | `native_ssh`             |    58 |   ✅   | 00:00:05.025 |
| `test_ssh_transport`     | `native_ssh`             |    47 |   ✅   | 00:00:03.027 |
| `test_ssh_server`        | `native_ssh`             |    27 |   ✅   | 00:00:01.063 |
| `test_ssh_pqc`           | `native_ssh_pqc`         |     4 |   ✅   | 00:00:01.304 |
| `test_ssh_hardening`     | `native_ssh_hardened`    |     2 |   ✅   | 00:00:01.201 |
| `test_ssh_conn`          | `native_ssh_conn`        |    16 |   ✅   | 00:00:01.976 |
| `test_ssh_sftp`          | `native_ssh_sftp`        |    14 |   ✅   | 00:00:00.695 |
| `test_scp`               | `native_scp`             |     6 |   ✅   | 00:00:00.689 |
| `test_middleware`        | `native_app`             |     9 |   ✅   | 00:00:09.453 |
| `test_application`       | `native_app`             |    72 |   ✅   | 00:00:00.948 |
| `test_digest_vectors`    | `native_app`             |     4 |   ✅   | 00:00:00.665 |
| `test_dispatch`          | `native_app`             |    11 |   ✅   | 00:00:00.770 |
| `test_web_terminal`      | `native_app`             |     9 |   ✅   | 00:00:00.759 |
| `test_response_headers`  | `native_app`             |    12 |   ✅   | 00:00:00.787 |
| `test_defer`             | `native_app`             |     3 |   ✅   | 00:00:00.749 |
| `test_template`          | `native_app`             |     6 |   ✅   | 00:00:00.752 |
| `test_regex`             | `native_app`             |    13 |   ✅   | 00:00:00.749 |
| `test_iface`             | `native_app`             |     7 |   ✅   | 00:00:00.758 |
| `test_file_serving`      | `native_app`             |    12 |   ✅   | 00:00:00.779 |
| `test_path_params`       | `native_app`             |     8 |   ✅   | 00:00:00.752 |
| `test_digest_auth`       | `native_app`             |    11 |   ✅   | 00:00:00.781 |
| `test_json`              | `native_app`             |    28 |   ✅   | 00:00:00.729 |
| `test_auth`              | `native_app`             |    13 |   ✅   | 00:00:00.772 |
| `test_multipart`         | `native_app`             |    25 |   ✅   | 00:00:00.788 |
| `test_chunked`           | `native_app`             |    14 |   ✅   | 00:00:00.783 |
| `test_form_params`       | `native_app`             |     5 |   ✅   | 00:00:00.744 |
| `test_webdav_handler`    | `native_webdav_handler`  |    29 |   ✅   | 00:00:02.040 |
| `test_diag`              | `native_diag`            |     2 |   ✅   | 00:00:01.936 |
| `test_snmp_ber`          | `native_snmp`            |    21 |   ✅   | 00:00:01.169 |
| `test_snmp_agent`        | `native_snmp`            |    28 |   ✅   | 00:00:00.585 |
| `test_snmp_v3`           | `native_snmp_v3`         |    22 |   ✅   | 00:00:02.567 |
| `test_telnet`            | `native_telnet`          |    15 |   ✅   | 00:00:00.858 |
| `test_coap`              | `native_coap`            |    44 |   ✅   | 00:00:00.796 |
| `test_coap`              | `native_coap_observe`    |    46 |   ✅   | 00:00:00.816 |
| `test_webdav`            | `native_webdav`          |    25 |   ✅   | 00:00:00.730 |
| `test_modbus`            | `native_modbus`          |    23 |   ✅   | 00:00:00.685 |
| `test_cloudevents`       | `native_cloudevents`     |     8 |   ✅   | 00:00:00.791 |
| `test_redis_resp`        | `native_redis`           |    14 |   ✅   | 00:00:00.681 |
| `test_sqlite`            | `native_sqlite`          |    23 |   ✅   | 00:00:00.707 |
| `test_stomp`             | `native_stomp`           |    14 |   ✅   | 00:00:00.677 |
| `test_mqtt_sn`           | `native_mqtt_sn`         |    13 |   ✅   | 00:00:00.691 |
| `test_flow_export`       | `native_flow_export`     |     8 |   ✅   | 00:00:00.688 |
| `test_protobuf`          | `native_protobuf`        |    13 |   ✅   | 00:00:00.681 |
| `test_preempt_queue`     | `native_preempt_queue`   |    12 |   ✅   | 00:00:00.727 |
| `test_dma`               | `native_dma`             |    12 |   ✅   | 00:00:00.811 |
| `test_forward`           | `native_forward`         |    26 |   ✅   | 00:00:00.919 |
| `test_gateway`           | `native_gateway`         |    12 |   ✅   | 00:00:00.811 |
| `test_lora`              | `native_lora`            |    14 |   ✅   | 00:00:00.696 |
| `test_nrf24`             | `native_nrf24`           |    11 |   ✅   | 00:00:00.687 |
| `test_enocean`           | `native_enocean`         |    10 |   ✅   | 00:00:00.691 |
| `test_pn532`             | `native_pn532`           |    11 |   ✅   | 00:00:00.672 |
| `test_sigfox`            | `native_sigfox`          |     7 |   ✅   | 00:00:00.675 |
| `test_zwave`             | `native_zwave`           |     9 |   ✅   | 00:00:00.681 |
| `test_zigbee`            | `native_zigbee`          |    10 |   ✅   | 00:00:00.674 |
| `test_thread`            | `native_thread`          |    15 |   ✅   | 00:00:00.672 |
| `test_wamp`              | `native_wamp`            |    15 |   ✅   | 00:00:00.704 |
| `test_sunspec`           | `native_sunspec`         |     7 |   ✅   | 00:00:00.697 |
| `test_c37118`            | `native_c37118`          |     6 |   ✅   | 00:00:00.683 |
| `test_dnp3`              | `native_dnp3`            |     8 |   ✅   | 00:00:00.678 |
| `test_grpcweb`           | `native_grpcweb`         |     9 |   ✅   | 00:00:00.681 |
| `test_lwm2m_tlv`         | `native_lwm2m_tlv`       |    14 |   ✅   | 00:00:00.691 |
| `test_fins`              | `native_fins`            |     6 |   ✅   | 00:00:00.692 |
| `test_hostlink`          | `native_hostlink`        |     8 |   ✅   | 00:00:00.693 |
| `test_scpi`              | `native_scpi`            |    24 |   ✅   | 00:00:00.714 |
| `test_hislip`            | `native_hislip`          |    11 |   ✅   | 00:00:00.704 |
| `test_vxi11`             | `native_vxi11`           |    10 |   ✅   | 00:00:00.698 |
| `test_gpib`              | `native_gpib`            |    10 |   ✅   | 00:00:00.698 |
| `test_haas_mdc`          | `native_haas_mdc`        |    10 |   ✅   | 00:00:00.704 |
| `test_lsv2`              | `native_lsv2`            |    12 |   ✅   | 00:00:00.703 |
| `test_ikev2`             | `native_ikev2`           |    16 |   ✅   | 00:00:00.734 |
| `test_senml`             | `native_senml`           |     9 |   ✅   | 00:00:00.769 |
| `test_df1`               | `native_df1`             |    10 |   ✅   | 00:00:00.680 |
| `test_cotp`              | `native_cotp`            |     7 |   ✅   | 00:00:00.716 |
| `test_s7comm`            | `native_s7comm`          |     9 |   ✅   | 00:00:00.686 |
| `test_melsec`            | `native_melsec`          |     7 |   ✅   | 00:00:00.680 |
| `test_ads`               | `native_ads`             |    17 |   ✅   | 00:00:00.692 |
| `test_focas`             | `native_focas`           |    11 |   ✅   | 00:00:00.693 |
| `test_pqc_sha3`          | `native_pqc`             |     4 |   ✅   | 00:00:00.714 |
| `test_pqc_mlkem`         | `native_pqc`             |     3 |   ✅   | 00:00:00.522 |
| `test_iface_bridge`      | `native_iface_bridge`    |     7 |   ✅   | 00:00:00.712 |
| `test_rtcm3`             | `native_rtcm3`           |    11 |   ✅   | 00:00:00.675 |
| `test_gnss_survey`       | `native_gnss_survey`     |    22 |   ✅   | 00:00:00.753 |
| `test_ntrip_caster`      | `native_ntrip_caster`    |    14 |   ✅   | 00:00:00.692 |
| `test_bacnet`            | `native_bacnet`          |     9 |   ✅   | 00:00:00.669 |
| `test_enip`              | `native_enip`            |     7 |   ✅   | 00:00:00.680 |
| `test_amqp`              | `native_amqp`            |     8 |   ✅   | 00:00:00.673 |
| `test_cip`               | `native_cip`             |     9 |   ✅   | 00:00:00.685 |
| `test_nats`              | `native_nats`            |    14 |   ✅   | 00:00:00.681 |
| `test_proxy_protocol`    | `native_proxy_protocol`  |    10 |   ✅   | 00:00:00.678 |
| `test_sparkplug`         | `native_sparkplug`       |     7 |   ✅   | 00:00:00.716 |
| `test_modbus_master`     | `native_modbus_master`   |    12 |   ✅   | 00:00:00.738 |
| `test_ota_rollback`      | `native_ota_rollback`    |     6 |   ✅   | 00:00:00.657 |
| `test_totp`              | `native_totp`            |     5 |   ✅   | 00:00:00.704 |
| `test_webhook`           | `native_webhook`         |     9 |   ✅   | 00:00:00.683 |
| `test_radio_power`       | `native_radio_power`     |     2 |   ✅   | 00:00:00.662 |
| `test_dns_resolver`      | `native_dns_resolver`    |     5 |   ✅   | 00:00:00.691 |
| `test_audit_log`         | `native_audit_log`       |    16 |   ✅   | 00:00:00.721 |
| `test_oidc`              | `native_oidc`            |    19 |   ✅   | 00:00:10.800 |
| `test_vfs`               | `native_vfs`             |    12 |   ✅   | 00:00:00.680 |
| `test_graphql`           | `native_graphql`         |    32 |   ✅   | 00:00:00.701 |
| `test_espnow`            | `native_espnow`          |     8 |   ✅   | 00:00:00.680 |
| `test_oauth2`            | `native_oauth2`          |     9 |   ✅   | 00:00:00.719 |
| `test_opcua`             | `native_opcua`           |    47 |   ✅   | 00:00:00.789 |
| `test_opcua_client`      | `native_opcua_client`    |    20 |   ✅   | 00:00:00.765 |
| `test_umati`             | `native_umati`           |    11 |   ✅   | 00:00:00.741 |
| `test_keepalive`         | `native_keepalive`       |    11 |   ✅   | 00:00:01.806 |
| `test_range`             | `native_range`           |    20 |   ✅   | 00:00:01.817 |
| `test_syslog`            | `native_syslog`          |    10 |   ✅   | 00:00:00.712 |
| `test_smb_client`        | `native_smb`             |    58 |   ✅   | 00:00:00.855 |
| `test_smb_crypto`        | `native_smb`             |     5 |   ✅   | 00:00:00.523 |
| `test_spnego`            | `native_smb`             |    14 |   ✅   | 00:00:00.537 |
| `test_ntlm`              | `native_smb`             |     8 |   ✅   | 00:00:00.532 |
| `test_ntlmssp`           | `native_smb`             |     5 |   ✅   | 00:00:00.527 |
| `test_smb2`              | `native_smb`             |    19 |   ✅   | 00:00:00.591 |
| `test_smtp`              | `native_smtp`            |    22 |   ✅   | 00:00:00.851 |
| `test_ntp_server`        | `native_ntp_server`      |     8 |   ✅   | 00:00:00.685 |
| `test_dns_server`        | `native_dns_server`      |    13 |   ✅   | 00:00:00.683 |
| `test_rtc`               | `native_rtc`             |     9 |   ✅   | 00:00:00.673 |
| `test_relay`             | `native_relay`           |     6 |   ✅   | 00:00:00.693 |
| `test_ld2410`            | `native_ld2410`          |     8 |   ✅   | 00:00:00.695 |
| `test_sen0192`           | `native_sen0192`         |     5 |   ✅   | 00:00:00.686 |
| `test_mpr121`            | `native_mpr121`          |     6 |   ✅   | 00:00:00.689 |
| `test_sht3x`             | `native_sht3x`           |     6 |   ✅   | 00:00:00.685 |
| `test_pca9685`           | `native_pca9685`         |     5 |   ✅   | 00:00:00.683 |
| `test_ads1115`           | `native_ads1115`         |     5 |   ✅   | 00:00:00.704 |
| `test_ina219`            | `native_ina219`          |     5 |   ✅   | 00:00:00.702 |
| `test_hpack`             | `native_hpack`           |    15 |   ✅   | 00:00:00.832 |
| `test_h2_frame`          | `native_h2frame`         |     7 |   ✅   | 00:00:00.798 |
| `test_h2_conn`           | `native_h2conn`          |    22 |   ✅   | 00:00:01.006 |
| `test_quic_varint`       | `native_quic_varint`     |     3 |   ✅   | 00:00:00.695 |
| `test_h3_frame`          | `native_h3frame`         |     7 |   ✅   | 00:00:00.718 |
| `test_jwt`               | `native_jwt`             |    22 |   ✅   | 00:00:00.755 |
| `test_upload`            | `native_upload`          |     8 |   ✅   | 00:00:01.827 |
| `test_http_client`       | `native_http_client`     |    15 |   ✅   | 00:00:00.703 |
| `test_compliance`        | `native_compliance`      |    15 |   ✅   | 00:00:00.722 |
| `test_mqtt`              | `native_mqtt`            |    22 |   ✅   | 00:00:00.685 |
| `test_ws_client`         | `native_ws_client`       |    17 |   ✅   | 00:00:00.744 |
| `test_scratch`           | `native_scratch`         |    15 |   ✅   | 00:00:00.718 |
| `test_snmp_trap`         | `native_snmp_trap`       |     7 |   ✅   | 00:00:00.703 |
| `test_inflate`           | `native_inflate`         |    14 |   ✅   | 00:00:00.687 |
| `test_deflate`           | `native_deflate`         |    10 |   ✅   | 00:00:00.738 |
| `test_ssh_zlib`          | `native_ssh_zlib`        |     9 |   ✅   | 00:00:00.748 |
| `test_ssh_comp`          | `native_ssh_comp`        |     8 |   ✅   | 00:00:01.287 |
| `test_websocket`         | `native_ws_deflate`      |    74 |   ✅   | 00:00:01.194 |
| `test_time_source`       | `native_time_source`     |    10 |   ✅   | 00:00:00.686 |
| `test_config_store`      | `native_config_store`    |    15 |   ✅   | 00:00:00.678 |
| `test_device_id`         | `native_device_id`       |     4 |   ✅   | 00:00:00.697 |
| `test_auth_lockout`      | `native_auth_lockout`    |    12 |   ✅   | 00:00:00.719 |
| `test_csrf`              | `native_csrf`            |    10 |   ✅   | 00:00:00.726 |
| `test_telemetry`         | `native_telemetry`       |     8 |   ✅   | 00:00:00.688 |
| `test_dashboard`         | `native_dashboard`       |    15 |   ✅   | 00:00:00.686 |
| `test_net_egress`        | `native_net_egress`      |     6 |   ✅   | 00:00:00.659 |
| `test_partition_monitor` | `native_partition`       |     6 |   ✅   | 00:00:00.696 |
| `test_cbor`              | `native_cbor`            |    21 |   ✅   | 00:00:00.688 |
| `test_msgpack`           | `native_msgpack`         |    23 |   ✅   | 00:00:00.697 |
| `test_gpio_map`          | `native_gpio_map`        |     9 |   ✅   | 00:00:00.675 |
| `test_udp_telemetry`     | `native_udp_telemetry`   |     8 |   ✅   | 00:00:00.670 |
| `test_statsd`            | `native_statsd`          |    10 |   ✅   | 00:00:00.718 |
| `test_guardrails`        | `native_guardrails`      |     9 |   ✅   | 00:00:00.675 |
| `test_failsafe`          | `native_failsafe`        |     7 |   ✅   | 00:00:00.687 |
| `test_sleep_sched`       | `native_sleep_sched`     |     8 |   ✅   | 00:00:00.677 |
| `test_wearlevel`         | `native_wearlevel`       |     5 |   ✅   | 00:00:00.667 |
| `test_netadapt`          | `native_netadapt`        |     6 |   ✅   | 00:00:00.658 |
| `test_dshot`             | `native_dshot`           |     8 |   ✅   | 00:00:00.672 |
| `test_hart`              | `native_hart`            |     7 |   ✅   | 00:00:00.678 |
| `test_nts`               | `native_nts`             |     5 |   ✅   | 00:00:00.689 |
| `test_dds`               | `native_dds`             |     5 |   ✅   | 00:00:00.687 |
| `test_xmpp`              | `native_xmpp`            |    11 |   ✅   | 00:00:00.684 |
| `test_rawl2`             | `native_rawl2`           |     5 |   ✅   | 00:00:00.693 |
| `test_spa_router`        | `native_spa_router`      |     2 |   ✅   | 00:00:00.663 |
| `test_goose`             | `native_goose`           |     4 |   ✅   | 00:00:00.676 |
| `test_mtconnect`         | `native_mtconnect`       |    12 |   ✅   | 00:00:00.679 |
| `test_wal`               | `native_wal`             |     6 |   ✅   | 00:00:00.714 |
| `test_wal_store`         | `native_wal`             |    29 |   ✅   | 00:00:00.571 |
| `test_j2735`             | `native_j2735`           |    11 |   ✅   | 00:00:00.687 |
| `test_nema_ts2`          | `native_nema_ts2`        |     4 |   ✅   | 00:00:00.685 |
| `test_snp`               | `native_snp`             |     5 |   ✅   | 00:00:00.678 |
| `test_directnet`         | `native_directnet`       |     5 |   ✅   | 00:00:00.683 |
| `test_sep2`              | `native_sep2`            |     5 |   ✅   | 00:00:00.682 |
| `test_profinet`          | `native_profinet`        |     5 |   ✅   | 00:00:00.698 |
| `test_ntcip`             | `native_ntcip`           |     3 |   ✅   | 00:00:00.675 |
| `test_openadr`           | `native_openadr`         |     5 |   ✅   | 00:00:00.682 |
| `test_mms`               | `native_mms`             |    11 |   ✅   | 00:00:00.686 |
| `test_cclink`            | `native_cclink`          |     5 |   ✅   | 00:00:00.687 |
| `test_powerlink`         | `native_powerlink`       |     4 |   ✅   | 00:00:00.679 |
| `test_sercos`            | `native_sercos`          |     4 |   ✅   | 00:00:00.675 |
| `test_profibus`          | `native_profibus`        |     5 |   ✅   | 00:00:00.679 |
| `test_lonworks`          | `native_lonworks`        |     5 |   ✅   | 00:00:00.673 |
| `test_mbplus`            | `native_mbplus`          |     6 |   ✅   | 00:00:00.675 |
| `test_interbus`          | `native_interbus`        |     5 |   ✅   | 00:00:00.689 |
| `test_iccp`              | `native_iccp`            |     5 |   ✅   | 00:00:00.685 |
| `test_wave`              | `native_wave`            |     9 |   ✅   | 00:00:00.681 |
| `test_utmc`              | `native_utmc`            |     6 |   ✅   | 00:00:00.689 |
| `test_ocit`              | `native_ocit`            |     4 |   ✅   | 00:00:00.680 |
| `test_atc`               | `native_atc`             |     5 |   ✅   | 00:00:00.689 |
| `test_southbound`        | `native_southbound`      |     6 |   ✅   | 00:00:00.676 |
| `test_exc_decoder`       | `native_exc_decoder`     |     7 |   ✅   | 00:00:00.678 |
| `test_http_delivery`     | `native_http_delivery`   |     8 |   ✅   | 00:00:00.684 |
| `test_hw_health`         | `native_hw_health`       |     7 |   ✅   | 00:00:00.681 |
| `test_mdns_adaptive`     | `native_mdns_adaptive`   |     5 |   ✅   | 00:00:00.686 |
| `test_sockpool`          | `native_sockpool`        |     6 |   ✅   | 00:00:00.680 |
| `test_psram_pool`        | `native_psram_pool`      |     5 |   ✅   | 00:00:00.673 |
| `test_happy_eyeballs`    | `native_happy_eyeballs`  |     5 |   ✅   | 00:00:00.697 |
| `test_wifi_sniffer`      | `native_wifi_sniffer`    |     6 |   ✅   | 00:00:00.676 |
| `test_link_manager`      | `native_link_manager`    |     7 |   ✅   | 00:00:00.684 |
| `test_cc1101`            | `native_cc1101`          |    18 |   ✅   | 00:00:00.690 |
| `test_fdc2214`           | `native_fdc2214`         |     4 |   ✅   | 00:00:00.683 |
| `test_ldc1614`           | `native_ldc1614`         |     4 |   ✅   | 00:00:00.687 |
| `test_vl53l0x`           | `native_vl53l0x`         |     3 |   ✅   | 00:00:00.661 |
| `test_radio_sniff`       | `native_radio_sniff`     |     4 |   ✅   | 00:00:00.679 |
| `test_ble_gatt`          | `native_ble_gatt`        |     7 |   ✅   | 00:00:00.697 |
| `test_tls_policy`        | `native_tls_policy`      |     4 |   ✅   | 00:00:00.685 |
| `test_wisun`             | `native_wisun`           |    10 |   ✅   | 00:00:00.742 |
| `test_logbuf`            | `native_logbuf`          |     5 |   ✅   | 00:00:00.687 |
| `test_config_io`         | `native_config_io`       |     5 |   ✅   | 00:00:00.727 |
| `test_workers`           | `native_workers`         |     6 |   ✅   | 00:00:00.824 |
| `test_clock`             | `native_clock`           |     7 |   ✅   | 00:00:00.665 |
| `test_concurrency`       | `native_concurrency`     |     2 |   ✅   | 00:00:00.810 |
| `test_concurrency`       | `native_tsan`            |     2 |   ✅   | 00:00:00.948 |
| `test_qpack`             | `native_qpack`           |    11 |   ✅   | 00:00:00.816 |
| `test_quic_packet`       | `native_quic_packet`     |     8 |   ✅   | 00:00:00.722 |
| `test_quic_frame`        | `native_quic_frame`      |    11 |   ✅   | 00:00:00.704 |
| `test_quic_crypto`       | `native_quic_crypto`     |    13 |   ✅   | 00:00:00.823 |
| `test_dtls_record`       | `native_dtls`            |    16 |   ✅   | 00:00:00.817 |
| `test_dtls_handshake`    | `native_dtls_hs`         |    15 |   ✅   | 00:00:00.737 |
| `test_dtls_tls13`        | `native_dtls_tls13`      |     6 |   ✅   | 00:00:00.799 |
| `test_dtls_conn`         | `native_dtls_conn`       |     8 |   ✅   | 00:00:01.153 |
| `test_coaps`             | `native_coaps`           |     6 |   ✅   | 00:00:01.128 |
| `test_coaps_server`      | `native_coaps_server`    |    15 |   ✅   | 00:00:01.259 |
| `test_tls13_kdf`         | `native_tls13_kdf`       |     6 |   ✅   | 00:00:01.022 |
| `test_quic_tp`           | `native_quic_tp`         |     8 |   ✅   | 00:00:00.708 |
| `test_tls13_msg`         | `native_tls13_msg`       |    11 |   ✅   | 00:00:00.808 |
| `test_quic_tls`          | `native_quic_tls`        |    13 |   ✅   | 00:00:01.141 |
| `test_quic_tls`          | `native_quic_tls_pqc`    |    14 |   ✅   | 00:00:01.278 |
| `test_quic_conn`         | `native_quic_conn`       |    27 |   ✅   | 00:00:01.832 |
| `test_h3_conn`           | `native_h3_conn`         |    11 |   ✅   | 00:00:01.085 |
| `test_h3_e2e`            | `native_h3_e2e`          |     1 |   ✅   | 00:00:01.112 |
| `test_quic_server`       | `native_quic_server`     |     4 |   ✅   | 00:00:01.160 |
| `test_h3_server`         | `native_h3_server`       |     1 |   ✅   | 00:00:02.486 |
| `test_ssh_chachapoly`    | `native_ssh_chachapoly`  |     5 |   ✅   | 00:00:00.727 |
| `test_ssh_aesgcm`        | `native_ssh_aesgcm`      |     3 |   ✅   | 00:00:00.682 |
| `test_ssh_ecdsa`         | `native_ssh_ecdsa`       |    11 |   ✅   | 00:00:21.021 |
| `test_forwarded_trust`   | `native_forwarded_trust` |    10 |   ✅   | 00:00:00.816 |

---

## test_canopen - native_canopen - ✅ 17 passed

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

## test_cia402 - native_cia402 - ✅ 15 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CiA 402 drive profile (services/cia402): the Statusword state decode, the_

|   # | Test                                  | Status | Description                                                                               |
| --: | :------------------------------------ | :----: | :---------------------------------------------------------------------------------------- |
|   1 | `test_state_decode`                   |   ✅   | State decode                                                                              |
|   2 | `test_state_decode_ignores_high_bits` |   ✅   | The upper Statusword bits (voltage, remote, target reached, warning, ...) must not change |
|   3 | `test_controlword_commands`           |   ✅   | Controlword commands                                                                      |
|   4 | `test_enable_sequence`                |   ✅   | Enable sequence                                                                           |
|   5 | `test_statusword_flags`               |   ✅   | Statusword flags                                                                          |
|   6 | `test_sdo_set_controlword`            |   ✅   | Sdo set controlword                                                                       |
|   7 | `test_sdo_set_targets`                |   ✅   | Sdo set targets                                                                           |
|   8 | `test_sdo_get_roundtrip`              |   ✅   | Build a read request, then decode a crafted SDO upload response for the Statusword.       |
|   9 | `test_pdo_pack_unpack`                |   ✅   | Pdo pack unpack                                                                           |
|  10 | `test_state_decode_unknown`           |   ✅   | State decode unknown                                                                      |
|  11 | `test_controlword_invalid_command`    |   ✅   | Controlword invalid command                                                               |
|  12 | `test_sdo_set_velocity_torque`        |   ✅   | Sdo set velocity torque                                                                   |
|  13 | `test_sdo_get_i32_roundtrip`          |   ✅   | Sdo get i32 roundtrip                                                                     |
|  14 | `test_sdo_upload_reject_paths`        |   ✅   | (a) parse failure: dlc < 8 makes dws_canopen_parse_sdo_response fail.                     |
|  15 | `test_pdo_null_guards`                |   ✅   | Pdo null guards                                                                           |

</details>

---

## test_control - native_control - ✅ 17 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the PID control law (services/control): P / I / D terms, output clamping,_

|   # | Test                               | Status | Description                                                                                 |
| --: | :--------------------------------- | :----: | :------------------------------------------------------------------------------------------ |
|   1 | `test_proportional_only`           |   ✅   | Proportional only                                                                           |
|   2 | `test_integral_accumulates`        |   ✅   | Integral accumulates                                                                        |
|   3 | `test_feedforward`                 |   ✅   | Feedforward                                                                                 |
|   4 | `test_output_clamp_and_antiwindup` |   ✅   | Output clamp and antiwindup                                                                 |
|   5 | `test_antiwindup_recovers`         |   ✅   | Once the error reverses, the (un-wound) integrator resumes normally.                        |
|   6 | `test_derivative_on_measurement`   |   ✅   | Derivative on measurement                                                                   |
|   7 | `test_setpoint_step_no_kick`       |   ✅   | A setpoint step must NOT produce a derivative kick (D acts on measurement only).            |
|   8 | `test_derivative_filter`           |   ✅   | Derivative filter                                                                           |
|   9 | `test_reset_and_guards`            |   ✅   | Reset and guards                                                                            |
|  10 | `test_batched_update`              |   ✅   | Batched update                                                                              |
|  11 | `test_fixed_rate_matches`          |   ✅   | pid_update_fixed(sp, meas) must equal pid_update(sp, meas, dt) once pid_set_rate caches dt. |
|  12 | `test_control_primitives`          |   ✅   | Control primitives                                                                          |
|  13 | `test_setter_null_guards`          |   ✅   | Setter null guards                                                                          |
|  14 | `test_integral_limits_take_effect` |   ✅   | Integral limits take effect                                                                 |
|  15 | `test_pid_update_n_null_guards`    |   ✅   | Pid update n null guards                                                                    |
|  16 | `test_pid_log_header_bytes`        |   ✅   | Pid log header bytes                                                                        |
|  17 | `test_pid_log_record_bytes`        |   ✅   | Pid log record bytes                                                                        |

</details>

---

## test_dbm - native_dbm - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/dbm: a log-structured hash KV over the WAL, exercised on a RAM-backed WalDev._

|   # | Test                                            | Status | Description                                                                                             |
| --: | :---------------------------------------------- | :----: | :------------------------------------------------------------------------------------------------------ |
|   1 | `test_put_get_overwrite`                        |   ✅   | Put get overwrite                                                                                       |
|   2 | `test_delete_and_contains`                      |   ✅   | Delete and contains                                                                                     |
|   3 | `test_persist_across_reboot_with_checkpoint`    |   ✅   | Persist across reboot with checkpoint                                                                   |
|   4 | `test_persist_across_reboot_without_checkpoint` |   ✅   | Persist across reboot without checkpoint                                                                |
|   5 | `test_delete_persists_across_reboot`            |   ✅   | Delete persists across reboot                                                                           |
|   6 | `test_many_keys_and_collisions`                 |   ✅   | Many keys and collisions                                                                                |
|   7 | `test_index_full_fails_closed`                  |   ✅   | Index full fails closed                                                                                 |
|   8 | `test_bounds_and_empty_value`                   |   ✅   | Bounds and empty value                                                                                  |
|   9 | `test_max_value_roundtrip`                      |   ✅   | Max value roundtrip                                                                                     |
|  10 | `test_compact_reclaims_space`                   |   ✅   | Compact reclaims space                                                                                  |
|  11 | `test_compact_dest_too_small_fails_closed`      |   ✅   | Compact dest too small fails closed                                                                     |
|  12 | `test_compact_source_read_failure`              |   ✅   | If reading a value back from the source log fails mid-compaction, compact must fail closed BEFORE       |
|  13 | `test_compact_checkpoint_failure`               |   ✅   | If the destination checkpoint (sync) fails after the live keys are copied, compact must fail closed and |

</details>

---

## test_docstore - native_docstore - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/docstore: a JSON document store on the WAL (via dbm), with top-level field_

|   # | Test                                   | Status | Description                     |
| --: | :------------------------------------- | :----: | :------------------------------ |
|   1 | `test_put_get_del`                     |   ✅   | Put get del                     |
|   2 | `test_find_by_field`                   |   ✅   | Find by field                   |
|   3 | `test_find_bool`                       |   ✅   | Find bool                       |
|   4 | `test_persist_and_query_across_reboot` |   ✅   | Persist and query across reboot |
|   5 | `test_find_early_stop`                 |   ✅   | Find early stop                 |
|   6 | `test_find_field_absent`               |   ✅   | Find field absent               |
|   7 | `test_find_count_only_null_cb`         |   ✅   | Find count only null cb         |

</details>

---

## test_dnc - native_dnc - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CNC RS-232 DNC drip-feed codec (services/dnc): the EIA RS-244_

|   # | Test                                    | Status | Description                      |
| --: | :-------------------------------------- | :----: | :------------------------------- |
|   1 | `test_eia_table_odd_parity_and_inverse` |   ✅   | Eia table odd parity and inverse |
|   2 | `test_eia_known_vectors`                |   ✅   | Eia known vectors                |
|   3 | `test_iso_even_parity`                  |   ✅   | Iso even parity                  |
|   4 | `test_encode_block_iso`                 |   ✅   | Encode block iso                 |
|   5 | `test_encode_block_eia`                 |   ✅   | Encode block eia                 |
|   6 | `test_encode_block_fail_closed`         |   ✅   | Encode block fail closed         |
|   7 | `test_encode_marker`                    |   ✅   | Encode marker                    |
|   8 | `test_encode_leader`                    |   ✅   | Encode leader                    |
|   9 | `test_flow_control`                     |   ✅   | Flow control                     |
|  10 | `test_roundtrip_program`                |   ✅   | Roundtrip program                |
|  11 | `test_decode_overflow_and_recovery`     |   ✅   | Decode overflow and recovery     |
|  12 | `test_decode_ignores_runout`            |   ✅   | Decode ignores runout            |
|  13 | `test_decode_eia_three_is_not_xoff`     |   ✅   | Decode eia three is not xoff     |
|  14 | `test_encode_overflow_paths`            |   ✅   | Encode overflow paths            |

</details>

---

## test_dnc_stream - native_dnc - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the DNC drip-feed engine (services/dnc/dnc_stream): stream a G-code program over a_

|   # | Test                     | Status | Description       |
| --: | :----------------------- | :----: | :---------------- |
|   1 | `test_iso_roundtrip`     |   ✅   | Iso roundtrip     |
|   2 | `test_eia_roundtrip`     |   ✅   | Eia roundtrip     |
|   3 | `test_crlf_and_parity`   |   ✅   | Crlf and parity   |
|   4 | `test_xoff_pacing`       |   ✅   | Xoff pacing       |
|   5 | `test_leader_trailer`    |   ✅   | Leader trailer    |
|   6 | `test_empty_program`     |   ✅   | Empty program     |
|   7 | `test_encode_error`      |   ✅   | Encode error      |
|   8 | `test_io_error_and_args` |   ✅   | Io error and args |

</details>

---

## test_ftp - native_ftp - ✅ 21 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the FTP client wire codec (services/ftp): command builders, the_

|   # | Test                                                | Status | Description                                                                     |
| --: | :-------------------------------------------------- | :----: | :------------------------------------------------------------------------------ |
|   1 | `test_build_command`                                |   ✅   | Build command                                                                   |
|   2 | `test_build_command_fail_closed`                    |   ✅   | Build command fail closed                                                       |
|   3 | `test_build_port_and_eprt`                          |   ✅   | Build port and eprt                                                             |
|   4 | `test_reply_single_line`                            |   ✅   | Reply single line                                                               |
|   5 | `test_reply_multiline_greeting`                     |   ✅   | real test.rebex.net greeting: continuation lines do NOT repeat the code         |
|   6 | `test_reply_multiline_feat`                         |   ✅   | real FEAT reply: many indented continuation lines, terminated by "211 End."     |
|   7 | `test_reply_incomplete_and_malformed`               |   ✅   | single line without its CRLF yet -> incomplete                                  |
|   8 | `test_reply_pipelined_advance`                      |   ✅   | two replies back-to-back; parse the first, advance by `used`, parse the second. |
|   9 | `test_reply_multiline_not_terminated_by_other_code` |   ✅   | Reply multiline not terminated by other code                                    |
|  10 | `test_parse_pasv`                                   |   ✅   | Parse pasv                                                                      |
|  11 | `test_parse_pasv_malformed`                         |   ✅   | Parse pasv malformed                                                            |
|  12 | `test_parse_epsv`                                   |   ✅   | Parse epsv                                                                      |
|  13 | `test_parse_epsv_malformed`                         |   ✅   | Parse epsv malformed                                                            |
|  14 | `test_reply_null_and_partial_multiline`             |   ✅   | Reply null and partial multiline                                                |
|  15 | `test_build_overflow_and_null`                      |   ✅   | Build overflow and null                                                         |
|  16 | `test_pasv_epsv_null_and_edges`                     |   ✅   | Pasv epsv null and edges                                                        |
|  17 | `test_build_null_args`                              |   ✅   | Build null args                                                                 |
|  18 | `test_reply_head_nondigit_edges`                    |   ✅   | Reply head nondigit edges                                                       |
|  19 | `test_reply_multiline_samecode_dash`                |   ✅   | Reply multiline samecode dash                                                   |
|  20 | `test_parse_pasv_edges`                             |   ✅   | Parse pasv edges                                                                |
|  21 | `test_parse_epsv_edges`                             |   ✅   | Parse epsv edges                                                                |

</details>

---

## test_httpcache - native_httpcache - ✅ 15 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the HTTP Cache-Control helpers (services/httpcache): the directive_

|   # | Test                                                 | Status | Description                                                                           |
| --: | :--------------------------------------------------- | :----: | :------------------------------------------------------------------------------------ |
|   1 | `test_preset_immutable`                              |   ✅   | Preset immutable                                                                      |
|   2 | `test_preset_no_store_and_shared_and_revalidatable`  |   ✅   | Preset no store and shared and revalidatable                                          |
|   3 | `test_build_manual_and_edges`                        |   ✅   | Build manual and edges                                                                |
|   4 | `test_parse_response_directives`                     |   ✅   | Parse response directives                                                             |
|   5 | `test_parse_case_insensitive_and_quoted_and_unknown` |   ✅   | case-insensitive names, a quoted delta, extra OWS, and an unknown directive to ignore |
|   6 | `test_parse_request_directives`                      |   ✅   | Parse request directives                                                              |
|   7 | `test_build_parse_roundtrip`                         |   ✅   | Build parse roundtrip                                                                 |
|   8 | `test_freshness_precedence`                          |   ✅   | Freshness precedence                                                                  |
|   9 | `test_build_all_directives`                          |   ✅   | Build all directives                                                                  |
|  10 | `test_parse_all_directives`                          |   ✅   | Parse all directives                                                                  |
|  11 | `test_parse_and_build_guards`                        |   ✅   | Parse and build guards                                                                |
|  12 | `test_preset_clamps`                                 |   ✅   | Preset clamps                                                                         |
|  13 | `test_build_boundaries`                              |   ✅   | Build boundaries                                                                      |
|  14 | `test_parse_ci_length_edges`                         |   ✅   | Parse ci length edges                                                                 |
|  15 | `test_parse_ows_and_empty`                           |   ✅   | Parse ows and empty                                                                   |

</details>

---

## test_edge_cache - native_edge_cache - ✅ 27 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Pure host tests for the CDN edge-cache engine (services/edge_cache): response header-field access,_

|   # | Test                                              | Status | Description                                                                              |
| --: | :------------------------------------------------ | :----: | :--------------------------------------------------------------------------------------- |
|   1 | `test_header_value_found`                         |   ✅   | Header value found                                                                       |
|   2 | `test_header_value_case_insensitive_and_ows_trim` |   ✅   | case-insensitive name; leading + trailing OWS on the value is stripped                   |
|   3 | `test_header_value_absent_and_too_small`          |   ✅   | Header value absent and too small                                                        |
|   4 | `test_http_date_all_three_formats`                |   ✅   | RFC 9110 sec 5.6.7 worked example: all three encode 1994-11-06 08:49:37 UTC = 784111777. |
|   5 | `test_http_date_epoch_zero_and_invalid`           |   ✅   | Http date epoch zero and invalid                                                         |
|   6 | `test_freshness_lifetime_precedence`              |   ✅   | Freshness lifetime precedence                                                            |
|   7 | `test_heuristic_lifetime`                         |   ✅   | Heuristic lifetime                                                                       |
|   8 | `test_initial_and_current_age`                    |   ✅   | no wall clock (response_time_epoch < 0) -> the Age header alone                          |
|   9 | `test_is_fresh`                                   |   ✅   | Is fresh                                                                                 |
|  10 | `test_key_canon`                                  |   ✅   | Key canon                                                                                |
|  11 | `test_key_digest_deterministic_and_distinct`      |   ✅   | Key digest deterministic and distinct                                                    |
|  12 | `test_vary_serialize_match_and_differ`            |   ✅   | Vary serialize match and differ                                                          |
|  13 | `test_vary_serialize_star_and_empty`              |   ✅   | Vary serialize star and empty                                                            |
|  14 | `test_store_alloc_lookup`                         |   ✅   | Store alloc lookup                                                                       |
|  15 | `test_store_lru_evict`                            |   ✅   | Store lru evict                                                                          |
|  16 | `test_store_ttl_sweep`                            |   ✅   | Store ttl sweep                                                                          |
|  17 | `test_store_purge`                                |   ✅   | Store purge                                                                              |
|  18 | `test_store_free_entry`                           |   ✅   | Store free entry                                                                         |
|  19 | `test_store_find_vary`                            |   ✅   | Store find vary                                                                          |
|  20 | `test_entry_freshness_resolution`                 |   ✅   | Entry freshness resolution                                                               |
|  21 | `test_storeability`                               |   ✅   | Storeability                                                                             |
|  22 | `test_build_conditional`                          |   ✅   | Build conditional                                                                        |
|  23 | `test_apply_304`                                  |   ✅   | Apply 304                                                                                |
|  24 | `test_range_explicit_and_open_ended`              |   ✅   | bytes=A-B -> inclusive window.                                                           |
|  25 | `test_range_suffix`                               |   ✅   | bytes=-N -> the last N bytes.                                                            |
|  26 | `test_range_unsatisfiable`                        |   ✅   | Range unsatisfiable                                                                      |
|  27 | `test_range_ignored_forms`                        |   ✅   | Range ignored forms                                                                      |

</details>

---

## test_edge_fetch - native_edge_cache - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for the CDN edge-cache async origin-fetch engine (services/edge_cache/edge_fetch): the_

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_fetch_content_length`  |   ✅   | Fetch content length  |
|   2 | `test_fetch_chunked`         |   ✅   | Fetch chunked         |
|   3 | `test_fetch_close_delimited` |   ✅   | Fetch close delimited |
|   4 | `test_fetch_oversize`        |   ✅   | Fetch oversize        |
|   5 | `test_fetch_timeout`         |   ✅   | Fetch timeout         |
|   6 | `test_fetch_open_fail`       |   ✅   | Fetch open fail       |
|   7 | `test_resp_complete_unit`    |   ✅   | Resp complete unit    |

</details>

---

## test_edge_cache_sd - native_edge_cache_sd - ✅ 15 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/edge_cache/edge_cache_sd: the CDN edge cache's L2 SD-persistence tier over a_

|   # | Test                                      | Status | Description                        |
| --: | :---------------------------------------- | :----: | :--------------------------------- |
|   1 | `test_serialize_roundtrip_all_fields`     |   ✅   | Serialize roundtrip all fields     |
|   2 | `test_serialize_max_body`                 |   ✅   | Serialize max body                 |
|   3 | `test_serialize_too_small_scratch_fails`  |   ✅   | Serialize too small scratch fails  |
|   4 | `test_deserialize_corrupt_fails_closed`   |   ✅   | Deserialize corrupt fails closed   |
|   5 | `test_put_get_roundtrip`                  |   ✅   | Put get roundtrip                  |
|   6 | `test_no_validator_not_spilled`           |   ✅   | No validator not spilled           |
|   7 | `test_oversize_body_stays_l1_only`        |   ✅   | Oversize body stays l1 only        |
|   8 | `test_spill_on_evict_and_promote`         |   ✅   | Spill on evict and promote         |
|   9 | `test_transient_entry_not_spilled`        |   ✅   | Transient entry not spilled        |
|  10 | `test_survives_reboot`                    |   ✅   | Survives reboot                    |
|  11 | `test_del`                                |   ✅   | Del                                |
|  12 | `test_purge_prefix`                       |   ✅   | Purge prefix                       |
|  13 | `test_purge_prefix_multipass`             |   ✅   | Purge prefix multipass             |
|  14 | `test_purge_all`                          |   ✅   | Purge all                          |
|  15 | `test_shared_dbm_foreign_value_untouched` |   ✅   | Shared dbm foreign value untouched |

</details>

---

## test_edge_mesh - native_edge_mesh - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/edge_cache/edge_mesh: the CDN edge cache's mesh (sibling-cache) wire codec and_

|   # | Test                                    | Status | Description                                                                                 |
| --: | :-------------------------------------- | :----: | :------------------------------------------------------------------------------------------ |
|   1 | `test_request_roundtrip`                |   ✅   | Request roundtrip                                                                           |
|   2 | `test_request_incomplete_then_complete` |   ✅   | Request incomplete then complete                                                            |
|   3 | `test_request_malformed`                |   ✅   | Request malformed                                                                           |
|   4 | `test_entry_frame_roundtrip`            |   ✅   | Entry frame roundtrip                                                                       |
|   5 | `test_age_propagation`                  |   ✅   | Age propagation                                                                             |
|   6 | `test_response_roundtrip`               |   ✅   | Response roundtrip                                                                          |
|   7 | `test_response_malformed`               |   ✅   | Response malformed                                                                          |
|   8 | `test_requester_hit`                    |   ✅   | Requester hit                                                                               |
|   9 | `test_requester_miss`                   |   ✅   | Requester miss                                                                              |
|  10 | `test_requester_open_fail`              |   ✅   | Requester open fail                                                                         |
|  11 | `test_requester_send_fail`              |   ✅   | Requester send fail                                                                         |
|  12 | `test_requester_timeout`                |   ✅   | A truncated frame that never completes and the peer never closes -> deadline drives FAILED. |
|  13 | `test_requester_peer_closed_early`      |   ✅   | Requester peer closed early                                                                 |
|  14 | `test_requester_malformed`              |   ✅   | Requester malformed                                                                         |

</details>

---

## test_det_primitives - native_det_primitives - ✅ 5 passed

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

## test_det_ip - native_det_ip - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the DWSIp address core (network_drivers/network/dws_ip): RFC 4291 text_

|   # | Test                                          | Status | Description                                                                 |
| --: | :-------------------------------------------- | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_v4_round_trip`                          |   ✅   | V4 round trip                                                               |
|   2 | `test_from_v6_bytes`                          |   ✅   | 2001:db8::1 as raw network-order bytes -> DWSIp -> canonical text.          |
|   3 | `test_is_unspecified`                         |   ✅   | Is unspecified                                                              |
|   4 | `test_prefix_match`                           |   ✅   | IPv4 CIDR containment (the allowlist primitive - full address, no hashing). |
|   5 | `test_v6_canonical_5952`                      |   ✅   | RFC 5952: lower-case, no leading zeros, longest zero run -> "::".           |
|   6 | `test_v4_mapped`                              |   ✅   | V4 mapped                                                                   |
|   7 | `test_classify_v4`                            |   ✅   | Classify v4                                                                 |
|   8 | `test_classify_v6`                            |   ✅   | Classify v6                                                                 |
|   9 | `test_reject_malformed`                       |   ✅   | Reject malformed                                                            |
|  10 | `test_equal_and_from_v4`                      |   ✅   | Equal and from v4                                                           |
|  11 | `test_ip_classify_equal_cidr_and_parse_edges` |   ✅   | classify: null and a DWSIpFamily::DWS_IP_NONE address are UNSPECIFIED.      |

</details>

---

## test_det_arena - native_det_arena - ✅ 19 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the unified double-ended arena (network_drivers/session/dws_arena):_

|   # | Test                                          | Status | Description                                                                             |
| --: | :-------------------------------------------- | :----: | :-------------------------------------------------------------------------------------- |
|   1 | `test_persist_basic_alloc`                    |   ✅   | Persist basic alloc                                                                     |
|   2 | `test_persist_zeroed`                         |   ✅   | Persist zeroed                                                                          |
|   3 | `test_persist_first_fit_reuse`                |   ✅   | Persist first fit reuse                                                                 |
|   4 | `test_persist_coalesce`                       |   ✅   | Persist coalesce                                                                        |
|   5 | `test_persist_free_shrinks_boundary`          |   ✅   | Persist free shrinks boundary                                                           |
|   6 | `test_scratch_bump_and_reset`                 |   ✅   | Scratch bump and reset                                                                  |
|   7 | `test_scratch_mark_release`                   |   ✅   | Scratch mark release                                                                    |
|   8 | `test_persist_and_scratch_no_overlap`         |   ✅   | Persist and scratch no overlap                                                          |
|   9 | `test_boundary_collision_fail_closed`         |   ✅   | Take most of the arena from the bottom, then from the top, until they nearly meet.      |
|  10 | `test_scratch_reset_frees_middle_for_persist` |   ✅   | Scratch reset frees middle for persist                                                  |
|  11 | `test_alignment_various_sizes`                |   ✅   | Alignment various sizes                                                                 |
|  12 | `test_scratch_alignment_16`                   |   ✅   | The real scratch callers (SSH, WS deflate) ask for 16-byte alignment.                   |
|  13 | `test_zero_size_and_null_free`                |   ✅   | Zero size and null free                                                                 |
|  14 | `test_set_add_limits`                         |   ✅   | Set add limits                                                                          |
|  15 | `test_set_persist_overflow_and_prefer`        |   ✅   | Set persist overflow and prefer                                                         |
|  16 | `test_set_persist_free_routes_by_address`     |   ✅   | Set persist free routes by address                                                      |
|  17 | `test_set_scratch_overflow_and_unwind`        |   ✅   | Set scratch overflow and unwind                                                         |
|  18 | `test_persist_split_and_max_align`            |   ✅   | A small alloc into a large non-terminal hole splits the hole (leaves a free remainder). |
|  19 | `test_set_exhaustion_and_free_bytes`          |   ✅   | Set exhaustion and free bytes                                                           |

</details>

---

## test_ssh_ed25519 - native_ssh_ed25519 - ✅ 19 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Known-answer tests for the modern SSH crypto suite (curve25519-sha256 KEX +_

|   # | Test                                               | Status | Description                                 |
| --: | :------------------------------------------------- | :----: | :------------------------------------------ |
|   1 | `test_sha512_empty`                                |   ✅   | Sha512 empty                                |
|   2 | `test_sha512_abc`                                  |   ✅   | Sha512 abc                                  |
|   3 | `test_sha512_one_block_boundary`                   |   ✅   | Sha512 one block boundary                   |
|   4 | `test_sha512_two_block_boundary`                   |   ✅   | Sha512 two block boundary                   |
|   5 | `test_sha512_million_a_streaming`                  |   ✅   | Sha512 million a streaming                  |
|   6 | `test_sha512_streaming_matches_oneshot`            |   ✅   | Sha512 streaming matches oneshot            |
|   7 | `test_x25519_rfc7748_vector1`                      |   ✅   | X25519 rfc7748 vector1                      |
|   8 | `test_x25519_rfc7748_vector2`                      |   ✅   | X25519 rfc7748 vector2                      |
|   9 | `test_x25519_iterated_1`                           |   ✅   | X25519 iterated 1                           |
|  10 | `test_x25519_iterated_1000`                        |   ✅   | X25519 iterated 1000                        |
|  11 | `test_x25519_dh_agreement`                         |   ✅   | X25519 dh agreement                         |
|  12 | `test_ed25519_vector_empty_msg`                    |   ✅   | Ed25519 vector empty msg                    |
|  13 | `test_ed25519_vector_rfc8032_test2`                |   ✅   | Ed25519 vector rfc8032 test2                |
|  14 | `test_ed25519_vector_zero_seed`                    |   ✅   | Ed25519 vector zero seed                    |
|  15 | `test_ed25519_verify_rejects_tampering`            |   ✅   | Ed25519 verify rejects tampering            |
|  16 | `test_ed25519_verify_rejects_noncanonical_s`       |   ✅   | Ed25519 verify rejects noncanonical s       |
|  17 | `test_ed25519_verify_rejects_invalid_pubkey_point` |   ✅   | Ed25519 verify rejects invalid pubkey point |
|  18 | `test_ed25519_roundtrip_long`                      |   ✅   | Ed25519 roundtrip long                      |
|  19 | `test_gf_mul_s16_model_matches_scalar`             |   ✅   | Gf mul s16 model matches scalar             |

</details>

---

## test_crypto_kat - native_crypto_kat - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Data-driven external known-answer tests (KAT) for the library's crypto_

|   # | Test                  | Status | Description    |
| --: | :-------------------- | :----: | :------------- |
|   1 | `test_hmac_sha256`    |   ✅   | Hmac sha256    |
|   2 | `test_hmac_sha512`    |   ✅   | Hmac sha512    |
|   3 | `test_aes128gcm`      |   ✅   | Aes128gcm      |
|   4 | `test_x25519`         |   ✅   | X25519         |
|   5 | `test_ed25519_verify` |   ✅   | Ed25519 verify |
|   6 | `test_ed25519_sign`   |   ✅   | Ed25519 sign   |
|   7 | `test_hkdf_extract`   |   ✅   | Hkdf extract   |
|   8 | `test_chacha20_block` |   ✅   | Chacha20 block |
|   9 | `test_poly1305`       |   ✅   | Poly1305       |

</details>

---

## test_promisc - native_promisc - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Wi-Fi promiscuous capture helpers (services/promisc): the pure 802.11 MAC_

|   # | Test                              | Status | Description                                                                    |
| --: | :-------------------------------- | :----: | :----------------------------------------------------------------------------- |
|   1 | `test_beacon_mgmt`                |   ✅   | Mgmt (type 0), Beacon (subtype 8): fc0 = (8<<4)                                | (0<<2) = 0x80; no DS bits.                      |
|   2 | `test_data_from_ds`               |   ✅   | Data (type 2), from the AP: fc0 = (0<<4)                                       | (2<<2) = 0x08; from_ds = 0x02.                  |
|   3 | `test_data_to_ds`                 |   ✅   | Data to the AP: to_ds = 0x01. a1 = BSSID, a2 = SRC, a3 = DST.                  |
|   4 | `test_qos_data_header_len`        |   ✅   | QoS Data subtype 8: fc0 = (8<<4)                                               | (2<<2) = 0x88. Adds a 2-byte QoS Control field. |
|   5 | `test_wds_four_address`           |   ✅   | WDS: to_ds & from_ds set (fc1 = 0x03). Addr4 at offset 24; DST = a3, SRC = a4. |
|   6 | `test_control_frame`              |   ✅   | ACK (type 1, subtype 13): fc0 = (13<<4)                                        | (1<<2) = 0xD4. Only Addr1 (RA), 10-byte header. |
|   7 | `test_reject_short`               |   ✅   | Reject short                                                                   |
|   8 | `test_pcap_headers`               |   ✅   | Pcap headers                                                                   |
|   9 | `test_host_stubs_and_short_frame` |   ✅   | Host stubs and short frame                                                     |

</details>

---

## test_bus_capture - native_bus_capture - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CAN listen-only capture framing (services/bus_capture): can_to_socketcan()_

|   # | Test                               | Status | Description                                                                           |
| --: | :--------------------------------- | :----: | :------------------------------------------------------------------------------------ |
|   1 | `test_standard_data_frame`         |   ✅   | Standard data frame                                                                   |
|   2 | `test_extended_id_sets_eff`        |   ✅   | Extended id sets eff                                                                  |
|   3 | `test_rtr_flag_and_no_data`        |   ✅   | Rtr flag and no data                                                                  |
|   4 | `test_masks_and_bounds`            |   ✅   | Masks and bounds                                                                      |
|   5 | `test_pcap_can_linktype`           |   ✅   | Pcap can linktype                                                                     |
|   6 | `test_host_twai_stubs_fail_closed` |   ✅   | On host there is no TWAI controller: begin fails closed and poll/end are safe no-ops. |
|   7 | `test_host_can_stubs`              |   ✅   | Host build: no TWAI/CAN peripheral. begin() fails; poll/end are no-ops.               |

</details>

---

## test_j1939 - native_j1939 - ✅ 11 passed

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
|  10 | `test_build_error_paths`         |   ✅   | Build error paths         |
|  11 | `test_tp_feed_error_paths`       |   ✅   | Tp feed error paths       |

</details>

---

## test_devicenet - native_devicenet - ✅ 11 passed

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
|   9 | `test_id_error_paths`              |   ✅   | Id error paths                                                       |
|  10 | `test_frag_reject_paths`           |   ✅   | Frag reject paths                                                    |
|  11 | `test_frag_overflow`               |   ✅   | Frag overflow                                                        |

</details>

---

## test_nmea2000 - native_nmea2000 - ✅ 7 passed

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
|   7 | `test_nmea2000_error_paths`                    |   ✅   | Nmea2000 error paths                    |

</details>

---

## test_mbus - native_mbus - ✅ 12 passed

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
|  12 | `test_record_vife_chain`        |   ✅   | Record vife chain        |

</details>

---

## test_iec60870 - native_iec60870 - ✅ 13 passed

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
|   9 | `test_104_build_guards`             |   ✅   | 104 build guards             |
|  10 | `test_104_parse_rejects`            |   ✅   | 104 parse rejects            |
|  11 | `test_asdu_ioa_guards`              |   ✅   | Asdu ioa guards              |
|  12 | `test_101_build_guards`             |   ✅   | 101 build guards             |
|  13 | `test_101_parse_rejects`            |   ✅   | 101 parse rejects            |

</details>

---

## test_sdi12 - native_sdi12 - ✅ 7 passed

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
|   7 | `test_sdi12_error_paths`                        |   ✅   | Sdi12 error paths                                                                    |

</details>

---

## test_dmx - native_dmx - ✅ 6 passed

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
|   6 | `test_dmx_rdm_error_paths`   |   ✅   | Dmx rdm error paths   |

</details>

---

## test_nmea0183 - native_nmea0183 - ✅ 8 passed

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
|   8 | `test_nmea0183_error_paths`       |   ✅   | Nmea0183 error paths      |

</details>

---

## test_iolink - native_iolink - ✅ 6 passed

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
|   6 | `test_iol_finalize_verify_guards`                     |   ✅   | Iol finalize verify guards                     |

</details>

---

## test_presentation - native - ✅ 63 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit, stress, and race-condition tests for Layer 6 (Presentation)._

|   # | Test                                                 | Status | Description                                                                                  |
| --: | :--------------------------------------------------- | :----: | :------------------------------------------------------------------------------------------- |
|   1 | `test_fn_reset_sets_parse_state_to_method`           |   ✅   | Fn reset sets parse state to method                                                          |
|   2 | `test_fn_reset_sets_slot_id`                         |   ✅   | Fn reset sets slot id                                                                        |
|   3 | `test_fn_reset_clears_method`                        |   ✅   | Fn reset clears method                                                                       |
|   4 | `test_fn_reset_clears_path_and_idx`                  |   ✅   | Fn reset clears path and idx                                                                 |
|   5 | `test_fn_reset_clears_query_raw_and_params`          |   ✅   | Fn reset clears query raw and params                                                         |
|   6 | `test_fn_reset_clears_all_header_slots`              |   ✅   | Fn reset clears all header slots                                                             |
|   7 | `test_fn_reset_clears_body_fields`                   |   ✅   | Fn reset clears body fields                                                                  |
|   8 | `test_fn_reset_out_of_range_is_nop`                  |   ✅   | Fn reset out of range is nop                                                                 |
|   9 | `test_fn_reset_is_idempotent`                        |   ✅   | Fn reset is idempotent                                                                       |
|  10 | `test_fn_get_header_null_when_no_headers`            |   ✅   | setUp already reset all slots - header_count is 0                                            |
|  11 | `test_fn_get_header_finds_single_header`             |   ✅   | Fn get header finds single header                                                            |
|  12 | `test_fn_get_header_finds_first_of_many`             |   ✅   | Fn get header finds first of many                                                            |
|  13 | `test_fn_get_header_finds_middle_of_many`            |   ✅   | Fn get header finds middle of many                                                           |
|  14 | `test_fn_get_header_finds_last_of_many`              |   ✅   | Fn get header finds last of many                                                             |
|  15 | `test_fn_get_header_case_insensitive_lowercase`      |   ✅   | Fn get header case insensitive lowercase                                                     |
|  16 | `test_fn_get_header_case_insensitive_uppercase`      |   ✅   | Fn get header case insensitive uppercase                                                     |
|  17 | `test_fn_get_header_returns_null_for_absent_key`     |   ✅   | Fn get header returns null for absent key                                                    |
|  18 | `test_fn_get_header_does_not_bleed_across_slots`     |   ✅   | Fn get header does not bleed across slots                                                    |
|  19 | `test_fn_get_query_null_when_no_params`              |   ✅   | Fn get query null when no params                                                             |
|  20 | `test_fn_get_query_finds_single_param`               |   ✅   | Fn get query finds single param                                                              |
|  21 | `test_fn_get_query_finds_first_param`                |   ✅   | Fn get query finds first param                                                               |
|  22 | `test_fn_get_query_finds_middle_param`               |   ✅   | Fn get query finds middle param                                                              |
|  23 | `test_fn_get_query_finds_last_param`                 |   ✅   | Fn get query finds last param                                                                |
|  24 | `test_fn_get_query_returns_null_for_absent_key`      |   ✅   | Fn get query returns null for absent key                                                     |
|  25 | `test_fn_get_query_empty_value`                      |   ✅   | Fn get query empty value                                                                     |
|  26 | `test_fn_get_query_does_not_bleed_across_slots`      |   ✅   | Fn get query does not bleed across slots                                                     |
|  27 | `test_get_parses_complete`                           |   ✅   | Get parses complete                                                                          |
|  28 | `test_post_body_stored`                              |   ✅   | Post body stored                                                                             |
|  29 | `test_put_parses_complete`                           |   ✅   | Put parses complete                                                                          |
|  30 | `test_delete_parses_complete`                        |   ✅   | Delete parses complete                                                                       |
|  31 | `test_patch_parses_complete`                         |   ✅   | Patch parses complete                                                                        |
|  32 | `test_head_parses_complete`                          |   ✅   | Head parses complete                                                                         |
|  33 | `test_query_single_param`                            |   ✅   | Query single param                                                                           |
|  34 | `test_query_multiple_params`                         |   ✅   | Query multiple params                                                                        |
|  35 | `test_body_null_terminated`                          |   ✅   | Body null terminated                                                                         |
|  36 | `test_body_over_buf_size_is_413`                     |   ✅   | Content-Length > BODY_BUF_SIZE → ParseState::PARSE_ENTITY_TOO_LARGE before any body is read. |
|  37 | `test_overflow_method_sets_error`                    |   ✅   | Overflow method sets error                                                                   |
|  38 | `test_overflow_path_sets_414`                        |   ✅   | Overflow path sets 414                                                                       |
|  39 | `test_bad_lf_after_cr_sets_error`                    |   ✅   | Null byte would terminate the C-string in push(), so use a visible non-LF byte.              |
|  40 | `test_headers_beyond_max_are_dropped`                |   ✅   | Headers beyond max are dropped                                                               |
|  41 | `test_query_params_beyond_max_are_dropped`           |   ✅   | Query params beyond max are dropped                                                          |
|  42 | `test_incremental_two_pushes_completes`              |   ✅   | Incremental two pushes completes                                                             |
|  43 | `test_body_starting_with_newline_stored`             |   ✅   | Body starting with newline stored                                                            |
|  44 | `test_put_body_stored`                               |   ✅   | Put body stored                                                                              |
|  45 | `test_content_length_header_stored_in_headers_array` |   ✅   | Content length header stored in headers array                                                |
|  46 | `stress_parse_reset_100_cycles`                      |   ✅   | Stress - Parse reset 100 cycles                                                              |
|  47 | `stress_all_slots_parse_simultaneously`              |   ✅   | Stress - All slots parse simultaneously                                                      |
|  48 | `stress_method_at_max_7_chars_no_error`              |   ✅   | Stress - Method at max 7 chars no error                                                      |
|  49 | `stress_path_at_exact_limit_no_error`                |   ✅   | Stress - Path at exact limit no error                                                        |
|  50 | `stress_body_exactly_buf_size_all_stored`            |   ✅   | Stress - Body exactly buf size all stored                                                    |
|  51 | `stress_exactly_max_headers_all_stored`              |   ✅   | Stress - Exactly max headers all stored                                                      |
|  52 | `stress_exactly_max_query_params_all_stored`         |   ✅   | Stress - Exactly max query params all stored                                                 |
|  53 | `stress_incremental_byte_by_byte_no_error`           |   ✅   | Stress - Incremental byte by byte no error                                                   |
|  54 | `stress_sequential_requests_no_state_leak`           |   ✅   | Stress - Sequential requests no state leak                                                   |
|  55 | `race_interleaved_producer_consumer_ring_buffer`     |   ✅   | Producer writes first 100 bytes                                                              |
|  56 | `race_ring_buffer_full_prevents_write`               |   ✅   | Race - Ring buffer full prevents write                                                       |
|  57 | `race_aba_slot_reuse_fresh_timestamp`                |   ✅   | Race - Aba slot reuse fresh timestamp                                                        |
|  58 | `race_double_free_is_nop`                            |   ✅   | Race - Double free is nop                                                                    |
|  59 | `race_concurrent_slot_parse_isolation`               |   ✅   | Slot 0: push a full request                                                                  |
|  60 | `race_reset_during_parse_header_val`                 |   ✅   | Race - Reset during parse header val                                                         |
|  61 | `race_reset_during_parse_query`                      |   ✅   | Race - Reset during parse query                                                              |
|  62 | `race_reset_during_parse_body`                       |   ✅   | Race - Reset during parse body                                                               |
|  63 | `race_parse_after_complete_is_nop`                   |   ✅   | Race - Parse after complete is nop                                                           |

</details>

---

## test_http_parser - native - ✅ 93 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Comprehensive unit tests for the standalone HTTP/1.1 parser._

|   # | Test                                                     | Status | Description                                                                              |
| --: | :------------------------------------------------------- | :----: | :--------------------------------------------------------------------------------------- |
|   1 | `test_accessor_null_guards`                              |   ✅   | Accessor null guards                                                                     |
|   2 | `test_cookie_parse_edges`                                |   ✅   | Cookie parse edges                                                                       |
|   3 | `test_forwarded_ip_whitespace_and_invalid`               |   ✅   | Forwarded ip whitespace and invalid                                                      |
|   4 | `test_reset_sets_parse_method_state`                     |   ✅   | Reset sets parse method state                                                            |
|   5 | `test_reset_preserves_slot_id`                           |   ✅   | Reset preserves slot id                                                                  |
|   6 | `test_reset_clears_method`                               |   ✅   | Reset clears method                                                                      |
|   7 | `test_reset_clears_path`                                 |   ✅   | Reset clears path                                                                        |
|   8 | `test_reset_clears_header_count`                         |   ✅   | Reset clears header count                                                                |
|   9 | `test_reset_clears_body`                                 |   ✅   | Reset clears body                                                                        |
|  10 | `test_reset_clears_query_count`                          |   ✅   | Reset clears query count                                                                 |
|  11 | `test_feed_after_complete_does_not_change_state`         |   ✅   | Feed after complete does not change state                                                |
|  12 | `test_feed_after_error_does_not_change_state`            |   ✅   | Feed after error does not change state                                                   |
|  13 | `test_feed_after_entity_too_large_does_not_change_state` |   ✅   | Feed after entity too large does not change state                                        |
|  14 | `test_method_get`                                        |   ✅   | Method get                                                                               |
|  15 | `test_method_post`                                       |   ✅   | Method post                                                                              |
|  16 | `test_method_put`                                        |   ✅   | Method put                                                                               |
|  17 | `test_method_delete`                                     |   ✅   | Method delete                                                                            |
|  18 | `test_method_patch`                                      |   ✅   | Method patch                                                                             |
|  19 | `test_method_head`                                       |   ✅   | Method head                                                                              |
|  20 | `test_method_options`                                    |   ✅   | Method options                                                                           |
|  21 | `test_method_overflow_is_error`                          |   ✅   | More than 7 chars (sizeof method - 1) before a space → ParseState::PARSE_ERROR           |
|  22 | `test_path_root`                                         |   ✅   | Path root                                                                                |
|  23 | `test_path_segments`                                     |   ✅   | Path segments                                                                            |
|  24 | `test_path_without_query`                                |   ✅   | Path without query                                                                       |
|  25 | `test_path_overflow_is_414`                              |   ✅   | Build a path longer than MAX_PATH_LEN                                                    |
|  26 | `test_single_query_param`                                |   ✅   | Single query param                                                                       |
|  27 | `test_two_query_params`                                  |   ✅   | Two query params                                                                         |
|  28 | `test_query_key_not_found_returns_null`                  |   ✅   | Query key not found returns null                                                         |
|  29 | `test_query_empty_value`                                 |   ✅   | Query empty value                                                                        |
|  30 | `test_single_header_stored`                              |   ✅   | Single header stored                                                                     |
|  31 | `test_header_lookup_case_insensitive`                    |   ✅   | Header lookup case insensitive                                                           |
|  32 | `test_cookie_basic_and_positions`                        |   ✅   | Cookie basic and positions                                                               |
|  33 | `test_cookie_missing_and_no_header`                      |   ✅   | Cookie missing and no header                                                             |
|  34 | `test_cookie_exact_name_not_substring`                   |   ✅   | Cookie exact name not substring                                                          |
|  35 | `test_cookie_quoted_and_value_with_equals`               |   ✅   | Cookie quoted and value with equals                                                      |
|  36 | `test_forwarded_rfc7239`                                 |   ✅   | Forwarded rfc7239                                                                        |
|  37 | `test_forwarded_leftmost_client`                         |   ✅   | Both header forms list the original client leftmost.                                     |
|  38 | `test_forwarded_strips_quotes_and_port`                  |   ✅   | Forwarded strips quotes and port                                                         |
|  39 | `test_forwarded_ipv6_recovered_unknown_rejected`         |   ✅   | RFC 7239 §6: an IPv6 for= value is DQUOTE-wrapped + bracketed, optional :port.           |
|  40 | `test_header_leading_space_stripped`                     |   ✅   | Header leading space stripped                                                            |
|  41 | `test_content_length_header_parsed`                      |   ✅   | Content length header parsed                                                             |
|  42 | `test_content_length_in_headers_array`                   |   ✅   | Content length in headers array                                                          |
|  43 | `test_multiple_headers_stored`                           |   ✅   | Multiple headers stored                                                                  |
|  44 | `test_missing_header_returns_null`                       |   ✅   | Missing header returns null                                                              |
|  45 | `test_get_no_body_completes`                             |   ✅   | Get no body completes                                                                    |
|  46 | `test_post_with_body`                                    |   ✅   | Post with body                                                                           |
|  47 | `test_put_with_body`                                     |   ✅   | Put with body                                                                            |
|  48 | `test_body_starting_with_newline`                        |   ✅   | Body starting with newline                                                               |
|  49 | `test_post_content_length_zero`                          |   ✅   | Post content length zero                                                                 |
|  50 | `test_body_exactly_at_buffer_limit`                      |   ✅   | Body of exactly BODY_BUF_SIZE bytes - should succeed                                     |
|  51 | `test_body_null_terminated_after_complete`               |   ✅   | Body null terminated after complete                                                      |
|  52 | `test_body_one_over_limit_is_413`                        |   ✅   | Content-Length == BODY_BUF_SIZE + 1 → ParseState::PARSE_ENTITY_TOO_LARGE                 |
|  53 | `test_body_far_over_limit_is_413`                        |   ✅   | Body far over limit is 413                                                               |
|  54 | `test_413_no_body_bytes_fed`                             |   ✅   | Even though we detected 413, no body bytes should have been stored                       |
|  55 | `test_413_header_still_stored`                           |   ✅   | Headers before the blank line must be accessible even when 413                           |
|  56 | `test_body_exactly_at_limit_is_not_413`                  |   ✅   | BODY_BUF_SIZE is the max that fits - should NOT trigger 413                              |
|  57 | `test_path_overflow_stops_feeding`                       |   ✅   | Bytes fed after URI_TOO_LONG are ignored - state must not change                         |
|  58 | `test_414_path_filled_to_capacity`                       |   ✅   | Buffer fills to MAX_PATH_LEN-1 chars before overflow is detected                         |
|  59 | `test_method_nul_byte_is_error`                          |   ✅   | Method nul byte is error                                                                 |
|  60 | `test_method_control_char_is_error`                      |   ✅   | Method control char is error                                                             |
|  61 | `test_method_del_byte_is_error`                          |   ✅   | Method del byte is error                                                                 |
|  62 | `test_method_non_tchar_symbol_is_error`                  |   ✅   | '(' is VCHAR but not tchar                                                               |
|  63 | `test_method_tchar_symbols_accepted`                     |   ✅   | '-' is a valid tchar; a custom method like "X-CMD" is valid per RFC 7230                 |
|  64 | `test_path_nul_byte_is_error`                            |   ✅   | Path nul byte is error                                                                   |
|  65 | `test_path_control_char_is_error`                        |   ✅   | Path control char is error                                                               |
|  66 | `test_path_del_byte_is_error`                            |   ✅   | Path del byte is error                                                                   |
|  67 | `test_query_nul_byte_is_error`                           |   ✅   | Query nul byte is error                                                                  |
|  68 | `test_query_control_char_is_error`                       |   ✅   | Query control char is error                                                              |
|  69 | `test_header_key_space_is_error`                         |   ✅   | Space in a field-name is not a valid tchar                                               |
|  70 | `test_header_key_nul_byte_is_error`                      |   ✅   | Header key nul byte is error                                                             |
|  71 | `test_header_key_control_char_is_error`                  |   ✅   | Header key control char is error                                                         |
|  72 | `test_header_key_mid_cr_is_error`                        |   ✅   | CR in the middle of a key name must be ParseState::PARSE_ERROR, not blank-line detection |
|  73 | `test_header_key_colon_at_start_skips_header`            |   ✅   | Empty key name (colon immediately after CRLF): transition to val with empty key          |
|  74 | `test_long_standard_header_key_accepted`                 |   ✅   | Regression: "Sec-WebSocket-Extensions" (24 chars) is a standard header that              |
|  75 | `test_overlong_header_key_truncated_not_error`           |   ✅   | A header name longer than MAX_KEY_LEN is capped (capacity), not rejected:                |
|  76 | `test_header_val_nul_byte_is_error`                      |   ✅   | Header val nul byte is error                                                             |
|  77 | `test_header_val_control_char_is_error`                  |   ✅   | Header val control char is error                                                         |
|  78 | `test_header_val_del_byte_is_error`                      |   ✅   | Header val del byte is error                                                             |
|  79 | `test_header_val_htab_mid_value_allowed`                 |   ✅   | HTAB is valid mid-value (RFC 7230 §3.2)                                                  |
|  80 | `test_header_val_leading_htab_stripped`                  |   ✅   | Leading HTAB (OWS) is stripped just like leading SP                                      |
|  81 | `test_header_val_obs_text_allowed`                       |   ✅   | obs-text bytes (%x80-FF) are allowed for legacy compatibility (RFC 7230 §3.2.6)          |
|  82 | `test_version_http11_recognized`                         |   ✅   | Version http11 recognized                                                                |
|  83 | `test_version_http10_recognized`                         |   ✅   | Version http10 recognized                                                                |
|  84 | `test_version_unknown_is_http_unknown`                   |   ✅   | Version unknown is http unknown                                                          |
|  85 | `test_version_reset_to_unknown`                          |   ✅   | Version reset to unknown                                                                 |
|  86 | `test_bad_expect_lf_is_error`                            |   ✅   | CRLF in version line replaced by CR + X (no LF)                                          |
|  87 | `test_blank_line_non_lf_is_error`                        |   ✅   | Header block ends with CR + non-LF in the blank line                                     |
|  88 | `test_slots_are_independent`                             |   ✅   | Slots are independent                                                                    |
|  89 | `test_incremental_byte_by_byte`                          |   ✅   | Incremental byte by byte                                                                 |
|  90 | `test_incremental_two_chunks`                            |   ✅   | Incremental two chunks                                                                   |
|  91 | `stress_many_requests_same_slot`                         |   ✅   | Stress - Many requests same slot                                                         |
|  92 | `stress_max_headers`                                     |   ✅   | Build a request with MAX_HEADERS header lines                                            |
|  93 | `stress_max_query_params`                                |   ✅   | Build a query string with MAX_QUERY_PARAMS parameters                                    |

</details>

---

## test_transport - native - ✅ 45 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit and stress tests for Layer 4 (Transport) - constants, pool invariants,_

|   # | Test                                             | Status | Description                                                                             |
| --: | :----------------------------------------------- | :----: | :-------------------------------------------------------------------------------------- |
|   1 | `test_pool_capacity_default_is_eight`            |   ✅   | The default connection pool is 8 (keep-alive/concurrency headroom; see ServerConfig.h). |
|   2 | `test_rx_buffer_size_is_one_kb`                  |   ✅   | Rx buffer size is one kb                                                                |
|   3 | `test_timeout_constant_is_5000ms`                |   ✅   | Timeout constant is 5000ms                                                              |
|   4 | `test_all_slots_free_after_init`                 |   ✅   | All slots free after init                                                               |
|   5 | `test_all_pcbs_null_after_init`                  |   ✅   | All pcbs null after init                                                                |
|   6 | `test_all_ring_buffers_empty_after_init`         |   ✅   | All ring buffers empty after init                                                       |
|   7 | `test_slot_ids_match_indices`                    |   ✅   | Slot ids match indices                                                                  |
|   8 | `test_ring_empty_when_head_equals_tail`          |   ✅   | Ring empty when head equals tail                                                        |
|   9 | `test_ring_wrap_at_boundary`                     |   ✅   | Ring wrap at boundary                                                                   |
|  10 | `test_ring_full_sentinel_one_slot_reserved`      |   ✅   | Ring full sentinel one slot reserved                                                    |
|  11 | `test_ring_can_store_size_minus_one_bytes`       |   ✅   | Ring can store size minus one bytes                                                     |
|  12 | `test_event_types_are_distinct`                  |   ✅   | Event types are distinct                                                                |
|  13 | `test_timeout_does_not_fire_on_free_slot`        |   ✅   | Timeout does not fire on free slot                                                      |
|  14 | `test_timeout_does_not_fire_before_deadline`     |   ✅   | Timeout does not fire before deadline                                                   |
|  15 | `test_timeout_fires_at_deadline`                 |   ✅   | Timeout fires at deadline                                                               |
|  16 | `test_timeout_fires_only_on_stale_slots`         |   ✅   | Timeout fires only on stale slots                                                       |
|  17 | `test_active_send_not_reaped`                    |   ✅   | Active send not reaped                                                                  |
|  18 | `test_init_succeeds_on_native`                   |   ✅   | Init succeeds on native                                                                 |
|  19 | `test_all_last_activity_ms_zero_after_init`      |   ✅   | All last activity ms zero after init                                                    |
|  20 | `test_queue_not_null_after_init`                 |   ✅   | Queue not null after init                                                               |
|  21 | `stress_ring_buffer_fill_drain_integrity`        |   ✅   | Write known pattern                                                                     |
|  22 | `stress_ring_buffer_multi_cycle_no_corruption`   |   ✅   | Stress - Ring buffer multi cycle no corruption                                          |
|  23 | `stress_all_slots_timeout_simultaneously`        |   ✅   | Stress - All slots timeout simultaneously                                               |
|  24 | `stress_timeout_arm_recover_cycle`               |   ✅   | Stress - Timeout arm recover cycle                                                      |
|  25 | `stress_check_timeouts_high_call_rate`           |   ✅   | Stress - Check timeouts high call rate                                                  |
|  26 | `stress_ring_buffer_byte_by_byte_fill_and_drain` |   ✅   | Stress - Ring buffer byte by byte fill and drain                                        |
|  27 | `test_accept_throttle_blocks_over_budget`        |   ✅   | Accept throttle blocks over budget                                                      |
|  28 | `test_accept_throttle_window_refills`            |   ✅   | Accept throttle window refills                                                          |
|  29 | `test_accept_throttle_handles_rollover`          |   ✅   | Accept throttle handles rollover                                                        |
|  30 | `test_per_ip_throttle_blocks_over_budget`        |   ✅   | Per ip throttle blocks over budget                                                      |
|  31 | `test_per_ip_throttle_isolates_addresses`        |   ✅   | Per ip throttle isolates addresses                                                      |
|  32 | `test_per_ip_throttle_window_refills`            |   ✅   | Per ip throttle window refills                                                          |
|  33 | `test_per_ip_throttle_evicts_when_full`          |   ✅   | Per ip throttle evicts when full                                                        |
|  34 | `test_per_ip_throttle_zero_ip_always_allowed`    |   ✅   | Per ip throttle zero ip always allowed                                                  |
|  35 | `test_per_ip_throttle_v6_distinct`               |   ✅   | Per ip throttle v6 distinct                                                             |
|  36 | `test_per_ip_throttle_handles_rollover`          |   ✅   | Per ip throttle handles rollover                                                        |
|  37 | `test_ip_allowlist_empty_allows_all`             |   ✅   | Ip allowlist empty allows all                                                           |
|  38 | `test_ip_allowlist_host_match`                   |   ✅   | Ip allowlist host match                                                                 |
|  39 | `test_ip_allowlist_cidr_match`                   |   ✅   | Ip allowlist cidr match                                                                 |
|  40 | `test_ip_allowlist_masks_host_bits`              |   ✅   | Ip allowlist masks host bits                                                            |
|  41 | `test_ip_allowlist_multiple_rules`               |   ✅   | Ip allowlist multiple rules                                                             |
|  42 | `test_ip_allowlist_zero_prefix_matches_all`      |   ✅   | Ip allowlist zero prefix matches all                                                    |
|  43 | `test_ip_allowlist_v6_cidr`                      |   ✅   | Ip allowlist v6 cidr                                                                    |
|  44 | `test_ip_allowlist_rejects_bad_prefix`           |   ✅   | Ip allowlist rejects bad prefix                                                         |
|  45 | `test_ip_allowlist_table_full`                   |   ✅   | Ip allowlist table full                                                                 |

</details>

---

## test_session - native - ✅ 19 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit, stress, and race-condition tests for Layer 5 (Session)._

|   # | Test                                               | Status | Description                                           |
| --: | :------------------------------------------------- | :----: | :---------------------------------------------------- |
|   1 | `test_empty_queue_does_not_crash`                  |   ✅   | Empty queue does not crash                            |
|   2 | `test_pool_initializes_to_parse_method`            |   ✅   | Pool initializes to parse method                      |
|   3 | `test_reset_clears_mid_parse_state`                |   ✅   | Reset clears mid parse state                          |
|   4 | `test_tick_fires_check_timeouts_stale_slot_freed`  |   ✅   | Tick fires check timeouts stale slot freed            |
|   5 | `test_tick_does_not_free_fresh_connection`         |   ✅   | Tick does not free fresh connection                   |
|   6 | `test_fn_tick_timeout_before_event_drain_ordering` |   ✅   | Fn tick timeout before event drain ordering           |
|   7 | `test_fn_tick_only_active_slots_expire`            |   ✅   | Fn tick only active slots expire                      |
|   8 | `stress_1000_idle_ticks_stable`                    |   ✅   | Stress - 1000 idle ticks stable                       |
|   9 | `stress_timeout_all_slots_10_cycles`               |   ✅   | Stress - Timeout all slots 10 cycles                  |
|  10 | `stress_mixed_fresh_stale_slots_many_ticks`        |   ✅   | Stress - Mixed fresh stale slots many ticks           |
|  11 | `test_evt_connect_calls_http_reset`                |   ✅   | Evt connect calls http reset                          |
|  12 | `test_evt_disconnect_calls_http_reset`             |   ✅   | Evt disconnect calls http reset                       |
|  13 | `test_evt_error_calls_http_reset`                  |   ✅   | Evt error calls http reset                            |
|  14 | `test_evt_data_calls_http_parse`                   |   ✅   | Evt data calls http parse                             |
|  15 | `test_multiple_events_drained_in_one_tick`         |   ✅   | Slot 0: dirty state → EvtType::EVT_CONNECT → reset    |
|  16 | `race_external_free_between_ticks`                 |   ✅   | First tick: slot expires inside check_timeouts        |
|  17 | `race_activity_update_saves_slot_from_timeout`     |   ✅   | Race - Activity update saves slot from timeout        |
|  18 | `race_all_expire_then_idle_tick`                   |   ✅   | Race - All expire then idle tick                      |
|  19 | `race_millis_wraparound_no_spurious_timeout`       |   ✅   | last_activity close to UINT32_MAX, now just past wrap |

</details>

---

## test_websocket - native - ✅ 69 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit and stress tests for SHA-1, Base64, and the WebSocket frame parser._

|   # | Test                                                   | Status | Description                                                               |
| --: | :----------------------------------------------------- | :----: | :------------------------------------------------------------------------ |
|   1 | `test_sha1_empty_string`                               |   ✅   | Sha1 empty string                                                         |
|   2 | `test_sha1_abc`                                        |   ✅   | Sha1 abc                                                                  |
|   3 | `test_sha1_rfc6455_handshake_key`                      |   ✅   | Client sends: Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==                 |
|   4 | `test_sha1_different_inputs_different_digests`         |   ✅   | Sha1 different inputs different digests                                   |
|   5 | `test_base64_encode_one_byte`                          |   ✅   | Base64 encode one byte                                                    |
|   6 | `test_base64_encode_two_bytes`                         |   ✅   | Base64 encode two bytes                                                   |
|   7 | `test_base64_encode_three_bytes`                       |   ✅   | Base64 encode three bytes                                                 |
|   8 | `test_base64_encode_ws_accept_key`                     |   ✅   | Base64 encode ws accept key                                               |
|   9 | `test_base64_decode_one_byte`                          |   ✅   | Base64 decode one byte                                                    |
|  10 | `test_base64_decode_two_bytes`                         |   ✅   | Base64 decode two bytes                                                   |
|  11 | `test_base64_decode_three_bytes`                       |   ✅   | Base64 decode three bytes                                                 |
|  12 | `test_base64_decode_ws_accept_key`                     |   ✅   | Base64 decode ws accept key                                               |
|  13 | `test_base64_decode_rejects_misplaced_padding`         |   ✅   | Base64 decode rejects misplaced padding                                   |
|  14 | `test_base64_decode_respects_capacity`                 |   ✅   | "TWFu" decodes to 3 bytes ("Man"); a 2-byte buffer is too small.          |
|  15 | `test_base64_round_trip`                               |   ✅   | Base64 round trip                                                         |
|  16 | `test_ws_pool_size`                                    |   ✅   | Ws pool size                                                              |
|  17 | `test_ws_ids_match_indices_after_init`                 |   ✅   | Ws ids match indices after init                                           |
|  18 | `test_ws_all_inactive_after_init`                      |   ✅   | Ws all inactive after init                                                |
|  19 | `test_ws_alloc_returns_non_null`                       |   ✅   | Ws alloc returns non null                                                 |
|  20 | `test_ws_alloc_sets_active`                            |   ✅   | Ws alloc sets active                                                      |
|  21 | `test_ws_alloc_sets_slot_id`                           |   ✅   | Ws alloc sets slot id                                                     |
|  22 | `test_ws_alloc_sets_parse_state_header1`               |   ✅   | Ws alloc sets parse state header1                                         |
|  23 | `test_ws_alloc_pool_full_returns_null`                 |   ✅   | Ws alloc pool full returns null                                           |
|  24 | `test_ws_find_returns_correct_conn`                    |   ✅   | Ws find returns correct conn                                              |
|  25 | `test_ws_find_returns_null_when_empty`                 |   ✅   | Ws find returns null when empty                                           |
|  26 | `test_ws_find_returns_null_for_different_slot`         |   ✅   | Ws find returns null for different slot                                   |
|  27 | `test_ws_find_after_both_slots_allocated`              |   ✅   | Ws find after both slots allocated                                        |
|  28 | `test_ws_free_deactivates_slot`                        |   ✅   | Ws free deactivates slot                                                  |
|  29 | `test_ws_free_restores_ws_id`                          |   ✅   | Ws free restores ws id                                                    |
|  30 | `test_ws_free_makes_slot_findable_as_null`             |   ✅   | Ws free makes slot findable as null                                       |
|  31 | `test_ws_free_nop_on_unallocated`                      |   ✅   | Ws free nop on unallocated                                                |
|  32 | `test_ws_alloc_after_free_succeeds`                    |   ✅   | Ws alloc after free succeeds                                              |
|  33 | `test_ws_parse_text_frame_sets_ready`                  |   ✅   | Ws parse text frame sets ready                                            |
|  34 | `test_ws_parse_payload_stored_correctly`               |   ✅   | Ws parse payload stored correctly                                         |
|  35 | `test_ws_parse_binary_frame_sets_ready`                |   ✅   | Ws parse binary frame sets ready                                          |
|  36 | `test_ws_parse_zero_length_unmasked_frame`             |   ✅   | Unmasked zero-length frame: 0x81 0x00.                                    |
|  37 | `test_ws_parse_zero_length_masked_frame`               |   ✅   | Masked zero-length text frame: FIN                                        | TEXT, MASK                                                 | 0, 4-byte mask key. |
|  38 | `test_ws_reject_unmasked_data_frame`                   |   ✅   | FIN                                                                       | TEXT, unmasked, length 3 - RFC 6455 §5.1 requires masking. |
|  39 | `test_ws_reject_reserved_opcode`                       |   ✅   | Opcode 0x3 is reserved (RFC 6455 §5.2) - must fail the connection.        |
|  40 | `test_ws_reject_fragmented_control_frame`              |   ✅   | PING with FIN=0 - control frames MUST NOT be fragmented (RFC 6455 §5.5).  |
|  41 | `test_ws_reject_oversized_control_frame`               |   ✅   | PING (masked) with payload length 126 - control frames MUST be <= 125     |
|  42 | `test_ws_parse_16bit_length_frame`                     |   ✅   | Build a 130-byte payload (> 125, requires 16-bit length field)            |
|  43 | `test_ws_parse_rsv1_set_closes_protocol`               |   ✅   | FIN=1, RSV1=0x40, TEXT: byte0 = 0x80                                      | 0x40                                                       | 0x01 = 0xC1         |
|  44 | `test_ws_parse_rsv2_set_closes_protocol`               |   ✅   | FIN=1, RSV2=0x20, TEXT: byte0 = 0x80                                      | 0x20                                                       | 0x01 = 0xA1         |
|  45 | `test_ws_parse_rsv3_set_closes_protocol`               |   ✅   | FIN=1, RSV3=0x10, TEXT: byte0 = 0x80                                      | 0x10                                                       | 0x01 = 0x91         |
|  46 | `test_ws_parse_64bit_length_closes_too_big`            |   ✅   | FIN=1, TEXT, MASK=1, len7=127 (64-bit length), then 8 length bytes        |
|  47 | `test_ws_parse_oversized_16bit_length_closes_too_big`  |   ✅   | Ws parse oversized 16bit length closes too big                            |
|  48 | `test_ws_fragment_start_waits_for_continuation`        |   ✅   | FIN=0, TEXT, "Hi" - start of a fragmented message; not deliverable yet.   |
|  49 | `test_ws_fragmented_message_reassembled`               |   ✅   | Ws fragmented message reassembled                                         |
|  50 | `test_ws_control_frame_interleaved_in_fragments`       |   ✅   | A PING arrives between the two data fragments; it must be handled without |
|  51 | `test_ws_fragment_accumulation_overflow_rejected`      |   ✅   | Ws fragment accumulation overflow rejected                                |
|  52 | `test_ws_continuation_without_start_rejected`          |   ✅   | CONTINUATION with no message in progress (RFC 6455 §5.4) → 1002.          |
|  53 | `test_ws_new_data_frame_during_fragmentation_rejected` |   ✅   | A second TEXT (new message) before finishing the first is illegal.        |
|  54 | `test_ws_parse_ping_auto_pong_resets_frame`            |   ✅   | FIN=1, PING=0x09: byte0 = 0x89                                            |
|  55 | `test_ws_parse_pong_silently_ignored`                  |   ✅   | FIN=1, PONG=0x0A: byte0 = 0x8A                                            |
|  56 | `test_ws_parse_close_marks_ws_closed`                  |   ✅   | FIN=1, CLOSE=0x08: byte0 = 0x88                                           |
|  57 | `test_ws_parse_stops_at_frame_ready`                   |   ✅   | Push two complete frames -- parser should stop after the first            |
|  58 | `test_ws_reset_frame_clears_fields`                    |   ✅   | Ws reset frame clears fields                                              |
|  59 | `test_ws_parse_mask_applied_correctly`                 |   ✅   | Frame with mask key {0x37, 0xFA, 0x21, 0x3D}, payload 'H' XOR 0x37 = 0x7F |
|  60 | `test_ws_text_invalid_utf8_rejected`                   |   ✅   | Ws text invalid utf8 rejected                                             |
|  61 | `test_ws_text_valid_utf8_accepted`                     |   ✅   | Ws text valid utf8 accepted                                               |
|  62 | `test_ws_binary_arbitrary_bytes_accepted`              |   ✅   | Ws binary arbitrary bytes accepted                                        |
|  63 | `test_ws_outbound_fragmentation`                       |   ✅   | Ws outbound fragmentation                                                 |
|  64 | `stress_ws_parse_reset_100_cycles`                     |   ✅   | Stress - Ws parse reset 100 cycles                                        |
|  65 | `stress_ws_alloc_free_pool_cycle`                      |   ✅   | Stress - Ws alloc free pool cycle                                         |
|  66 | `stress_ws_parse_incremental_byte_by_byte`             |   ✅   | Stress - Ws parse incremental byte by byte                                |
|  67 | `stress_ws_parse_max_payload`                          |   ✅   | Stress - Ws parse max payload                                             |
|  68 | `stress_ws_parse_two_consecutive_frames`               |   ✅   | First frame                                                               |
|  69 | `test_ws_send_frame_paths_and_parse_guard`             |   ✅   | Ws send frame paths and parse guard                                       |

</details>

---

## test_sse - native - ✅ 46 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit and stress tests for the Server-Sent Events connection pool (sse.h/cpp)._

|   # | Test                                                | Status | Description                                                                   |
| --: | :-------------------------------------------------- | :----: | :---------------------------------------------------------------------------- |
|   1 | `test_sse_pool_size`                                |   ✅   | Sse pool size                                                                 |
|   2 | `test_sse_ids_match_indices_after_init`             |   ✅   | Sse ids match indices after init                                              |
|   3 | `test_sse_all_inactive_after_init`                  |   ✅   | Sse all inactive after init                                                   |
|   4 | `test_sse_path_empty_after_init`                    |   ✅   | Sse path empty after init                                                     |
|   5 | `test_sse_alloc_returns_non_null`                   |   ✅   | Sse alloc returns non null                                                    |
|   6 | `test_sse_alloc_sets_active`                        |   ✅   | Sse alloc sets active                                                         |
|   7 | `test_sse_alloc_sets_slot_id`                       |   ✅   | Sse alloc sets slot id                                                        |
|   8 | `test_sse_alloc_stores_path`                        |   ✅   | Sse alloc stores path                                                         |
|   9 | `test_sse_alloc_stores_different_paths_per_slot`    |   ✅   | Sse alloc stores different paths per slot                                     |
|  10 | `test_sse_alloc_path_truncated_to_max`              |   ✅   | Build a path longer than MAX_PATH_LEN                                         |
|  11 | `test_sse_alloc_pool_full_returns_null`             |   ✅   | Sse alloc pool full returns null                                              |
|  12 | `test_sse_alloc_sse_id_is_pool_index`               |   ✅   | First free slot is 0 → dws_sse_id should be 0                                 |
|  13 | `test_sse_find_returns_correct_conn`                |   ✅   | Sse find returns correct conn                                                 |
|  14 | `test_sse_find_returns_null_when_empty`             |   ✅   | Sse find returns null when empty                                              |
|  15 | `test_sse_find_returns_null_for_different_slot`     |   ✅   | Sse find returns null for different slot                                      |
|  16 | `test_sse_find_after_both_slots_allocated`          |   ✅   | Sse find after both slots allocated                                           |
|  17 | `test_sse_find_checks_slot_id_not_sse_id`           |   ✅   | dws_sse_pool[0] → slot 3; dws_sse_find(3) must return it, not dws_sse_find(0) |
|  18 | `test_sse_free_deactivates_slot`                    |   ✅   | Sse free deactivates slot                                                     |
|  19 | `test_sse_free_restores_sse_id`                     |   ✅   | Sse free restores sse id                                                      |
|  20 | `test_sse_free_makes_slot_findable_as_null`         |   ✅   | Sse free makes slot findable as null                                          |
|  21 | `test_sse_free_clears_path`                         |   ✅   | Sse free clears path                                                          |
|  22 | `test_sse_free_nop_on_unallocated`                  |   ✅   | Sse free nop on unallocated                                                   |
|  23 | `test_sse_alloc_after_free_succeeds`                |   ✅   | Sse alloc after free succeeds                                                 |
|  24 | `test_sse_free_only_frees_matching_slot`            |   ✅   | Sse free only frees matching slot                                             |
|  25 | `test_sse_write_null_data_returns_false`            |   ✅   | Sse write null data returns false                                             |
|  26 | `test_sse_write_returns_false_when_conn_not_active` |   ✅   | Sse write returns false when conn not active                                  |
|  27 | `test_sse_write_returns_false_when_pcb_null`        |   ✅   | Sse write returns false when pcb null                                         |
|  28 | `test_sse_write_data_only_returns_true`             |   ✅   | Sse write data only returns true                                              |
|  29 | `test_sse_write_with_event_returns_true`            |   ✅   | Sse write with event returns true                                             |
|  30 | `test_sse_write_with_id_returns_true`               |   ✅   | Sse write with id returns true                                                |
|  31 | `test_sse_write_with_all_fields_returns_true`       |   ✅   | Sse write with all fields returns true                                        |
|  32 | `test_sse_write_does_not_affect_other_slots`        |   ✅   | Write to slot 0 -- slot 1 state must be unchanged                             |
|  33 | `test_http_conn_open_releases_stale_sse_binding`    |   ✅   | Http conn open releases stale sse binding                                     |
|  34 | `test_http_conn_open_leaves_other_slot_sse_binding` |   ✅   | Http conn open leaves other slot sse binding                                  |
|  35 | `test_sse_format_data_only`                         |   ✅   | Sse format data only                                                          |
|  36 | `test_sse_format_event_and_data`                    |   ✅   | Sse format event and data                                                     |
|  37 | `test_sse_format_id_and_data`                       |   ✅   | Sse format id and data                                                        |
|  38 | `test_sse_format_all_fields_ordering`               |   ✅   | Field order per WHATWG: event, then id, then data (blank line terminates).    |
|  39 | `test_sse_format_null_data_returns_zero`            |   ✅   | Sse format null data returns zero                                             |
|  40 | `test_sse_format_overflow_returns_zero`             |   ✅   | A record that cannot fit must report 0, never a partial (truncated) frame.    |
|  41 | `test_sse_format_zero_size_returns_zero`            |   ✅   | Sse format zero size returns zero                                             |
|  42 | `stress_sse_alloc_free_100_cycles`                  |   ✅   | Stress - Sse alloc free 100 cycles                                            |
|  43 | `stress_sse_alloc_free_both_slots_alternating`      |   ✅   | Stress - Sse alloc free both slots alternating                                |
|  44 | `stress_sse_write_100_calls`                        |   ✅   | Stress - Sse write 100 calls                                                  |
|  45 | `stress_sse_find_with_full_pool`                    |   ✅   | Stress - Sse find with full pool                                              |
|  46 | `stress_sse_write_slot_isolation`                   |   ✅   | Stress - Sse write slot isolation                                             |

</details>

---

## test_observability - native_observability - ✅ 17 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Transport observability (DWS_ENABLE_OBSERVABILITY): the dws_conn_on_event_

|   # | Test                                                          | Status | Description                                                             |
| --: | :------------------------------------------------------------ | :----: | :---------------------------------------------------------------------- |
|   1 | `test_transition_fires_hook_with_args`                        |   ✅   | Transition fires hook with args                                         |
|   2 | `test_each_reason_bumps_its_counter`                          |   ✅   | Each reason bumps its counter                                           |
|   3 | `test_closing_gauge_is_derived_from_pool`                     |   ✅   | Closing gauge is derived from pool                                      |
|   4 | `test_reset_clears_cumulative_not_derived_gauge`              |   ✅   | Reset clears cumulative not derived gauge                               |
|   5 | `test_no_hook_after_unregister`                               |   ✅   | No hook after unregister                                                |
|   6 | `test_recv_fin_counts_remote_close`                           |   ✅   | Recv fin counts remote close                                            |
|   7 | `test_err_cb_counts_error_close`                              |   ✅   | Err cb counts error close                                               |
|   8 | `test_timeout_sweep_counts_timeout`                           |   ✅   | Timeout sweep counts timeout                                            |
|   9 | `test_local_close_counts_local`                               |   ✅   | dws_conn_close(slot) reads the slot's pcb, frees the slot, and counts a |
|  10 | `test_abort_slot_counts_abort_and_frees`                      |   ✅   | Abort slot counts abort and frees                                       |
|  11 | `test_abort_slot_noop_on_free_slot`                           |   ✅   | Abort slot noop on free slot                                            |
|  12 | `test_backpressure_counts_when_ring_full`                     |   ✅   | Backpressure counts when ring full                                      |
|  13 | `test_begin_close_dwells_then_drains_on_ack`                  |   ✅   | Begin close dwells then drains on ack                                   |
|  14 | `test_begin_close_finalizes_immediately_when_already_drained` |   ✅   | Begin close finalizes immediately when already drained                  |
|  15 | `test_begin_close_noop_if_not_active`                         |   ✅   | Begin close noop if not active                                          |
|  16 | `test_closing_timeout_reaps_stuck_slot`                       |   ✅   | Closing timeout reaps stuck slot                                        |
|  17 | `test_recv_during_closing_is_drained_not_processed`           |   ✅   | Recv during closing is drained not processed                            |

</details>

---

## test_accept_gate - native_accept_gate - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the accept-time connection gates (network_drivers/transport/listener):_

|   # | Test                                     | Status | Description                       |
| --: | :--------------------------------------- | :----: | :-------------------------------- |
|   1 | `test_accept_throttle_window`            |   ✅   | Accept throttle window            |
|   2 | `test_accept_throttle_rollover`          |   ✅   | Accept throttle rollover          |
|   3 | `test_per_ip_independent_budgets`        |   ✅   | Per ip independent budgets        |
|   4 | `test_per_ip_v6_distinct_buckets`        |   ✅   | Per ip v6 distinct buckets        |
|   5 | `test_per_ip_window_rollover`            |   ✅   | Per ip window rollover            |
|   6 | `test_per_ip_unspecified_defers`         |   ✅   | Per ip unspecified defers         |
|   7 | `test_per_ip_eviction_bounded`           |   ✅   | Per ip eviction bounded           |
|   8 | `test_ip_allowlist_empty_allows_all`     |   ✅   | Ip allowlist empty allows all     |
|   9 | `test_ip_allowlist_cidr`                 |   ✅   | Ip allowlist cidr                 |
|  10 | `test_ip_allowlist_cidr_string`          |   ✅   | Ip allowlist cidr string          |
|  11 | `test_ip_allowlist_family_isolation`     |   ✅   | Ip allowlist family isolation     |
|  12 | `test_ip_allowlist_host_and_zero_prefix` |   ✅   | Ip allowlist host and zero prefix |
|  13 | `test_ip_allowlist_rejects_bad_and_full` |   ✅   | Ip allowlist rejects bad and full |

</details>

---

## test_http_ota - native_ota - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Tests the parser's streaming-body hook (DWS_ENABLE_OTA): a body larger than_

|   # | Test                                    | Status | Description                      |
| --: | :-------------------------------------- | :----: | :------------------------------- |
|   1 | `test_large_body_streams_to_completion` |   ✅   | Large body streams to completion |
|   2 | `test_no_hooks_large_body_is_413`       |   ✅   | No hooks large body is 413       |
|   3 | `test_nonmatching_path_not_streamed`    |   ✅   | Nonmatching path not streamed    |

</details>

---

## test_provisioning - native_prov - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for dws_prov_form_field(): the x-www-form-urlencoded field_

|   # | Test                           | Status | Description                                                                                      |
| --: | :----------------------------- | :----: | :----------------------------------------------------------------------------------------------- |
|   1 | `test_plain_fields`            |   ✅   | Plain fields                                                                                     |
|   2 | `test_url_decoding`            |   ✅   | Url decoding                                                                                     |
|   3 | `test_missing_field`           |   ✅   | Missing field                                                                                    |
|   4 | `test_no_substring_match`      |   ✅   | No substring match                                                                               |
|   5 | `test_capacity_bound`          |   ✅   | Capacity bound                                                                                   |
|   6 | `test_form_field_null_guards`  |   ✅   | Any null argument (or zero cap) fails closed and leaves a writable out empty.                    |
|   7 | `test_host_provisioning_stubs` |   ✅   | On host there is no NVS/WiFi: load reports no stored creds and clears the buffers; clear no-ops. |

</details>

---

## test_ssh_channel - native_ssh - ✅ 40 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH connection-protocol (channel) tests - RFC 4254, including multiplexing_

|   # | Test                                                | Status | Description                                                            |
| --: | :-------------------------------------------------- | :----: | :--------------------------------------------------------------------- |
|   1 | `test_chan_slot_and_msgtype_guards`                 |   ✅   | Chan slot and msgtype guards                                           |
|   2 | `test_chan_malformed_payloads`                      |   ✅   | Chan malformed payloads                                                |
|   3 | `test_chan_open_cap_guards`                         |   ✅   | Chan open cap guards                                                   |
|   4 | `test_chan_forward_and_channel_guards`              |   ✅   | While a slot is free: null address (262) and a too-small buffer (273). |
|   5 | `test_chan_global_request_reply_caps`               |   ✅   | Unknown request name, want_reply, no room for the 1-byte reply (246).  |
|   6 | `test_open_session_confirms`                        |   ✅   | Open session confirms                                                  |
|   7 | `test_open_unknown_type_fails`                      |   ✅   | Open unknown type fails                                                |
|   8 | `test_direct_tcpip_no_cb_prohibited`                |   ✅   | Forwarding is opt-in: with no open callback installed it is refused.   |
|   9 | `test_direct_tcpip_accept_confirms`                 |   ✅   | Direct tcpip accept confirms                                           |
|  10 | `test_direct_tcpip_refused_connect_failed`          |   ✅   | Direct tcpip refused connect failed                                    |
|  11 | `test_forward_data_routes_to_forward_cb`            |   ✅   | Forward data routes to forward cb                                      |
|  12 | `test_shell_request_success_with_reply`             |   ✅   | Shell request success with reply                                       |
|  13 | `test_unknown_request_failure`                      |   ✅   | Unknown request failure                                                |
|  14 | `test_request_no_reply_produces_nothing`            |   ✅   | Request no reply produces nothing                                      |
|  15 | `test_inbound_data_invokes_callback`                |   ✅   | Inbound data invokes callback                                          |
|  16 | `test_inbound_data_window_replenish`                |   ✅   | Inbound data window replenish                                          |
|  17 | `test_inbound_data_exceeding_window_rejected`       |   ✅   | Inbound data exceeding window rejected                                 |
|  18 | `test_outbound_data_frames_and_decrements_window`   |   ✅   | Outbound data frames and decrements window                             |
|  19 | `test_outbound_data_exceeding_peer_window_rejected` |   ✅   | Outbound data exceeding peer window rejected                           |
|  20 | `test_window_adjust_grows_peer_window`              |   ✅   | Window adjust grows peer window                                        |
|  21 | `test_build_close_emits_eof_and_close`              |   ✅   | Build close emits eof and close                                        |
|  22 | `test_inbound_close_routes_to_channel`              |   ✅   | Inbound close routes to channel                                        |
|  23 | `test_multiplex_two_channels_route_independently`   |   ✅   | Multiplex two channels route independently                             |
|  24 | `test_pool_full_open_fails`                         |   ✅   | Pool full open fails                                                   |
|  25 | `test_data_to_unknown_channel_rejected`             |   ✅   | Data to unknown channel rejected                                       |
|  26 | `test_rforward_no_cb_refused`                       |   ✅   | Rforward no cb refused                                                 |
|  27 | `test_rforward_accept_specific_port`                |   ✅   | Rforward accept specific port                                          |
|  28 | `test_rforward_port0_echoes_allocated`              |   ✅   | Rforward port0 echoes allocated                                        |
|  29 | `test_rforward_no_reply_silent`                     |   ✅   | Rforward no reply silent                                               |
|  30 | `test_rforward_cancel`                              |   ✅   | Rforward cancel                                                        |
|  31 | `test_global_unknown_request`                       |   ✅   | Global unknown request                                                 |
|  32 | `test_global_malformed`                             |   ✅   | Global malformed                                                       |
|  33 | `test_forwarded_open_builds_channel`                |   ✅   | Forwarded open builds channel                                          |
|  34 | `test_forwarded_confirm_opens_channel`              |   ✅   | Forwarded confirm opens channel                                        |
|  35 | `test_forwarded_failure_frees_channel`              |   ✅   | Forwarded failure frees channel                                        |
|  36 | `test_forwarded_confirm_unknown_rejected`           |   ✅   | Forwarded confirm unknown rejected                                     |
|  37 | `test_forwarded_inbound_data_routes_to_forward_cb`  |   ✅   | Forwarded inbound data routes to forward cb                            |
|  38 | `test_sftp_subsystem_routes`                        |   ✅   | Sftp subsystem routes                                                  |
|  39 | `test_unknown_subsystem_refused`                    |   ✅   | Unknown subsystem refused                                              |
|  40 | `test_scp_exec_routes`                              |   ✅   | Scp exec routes                                                        |

</details>

---

## test_ssh_auth - native_ssh - ✅ 21 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH user-authentication tests (RFC 4252): service request/accept, request_

|   # | Test                                           | Status | Description                                       |
| --: | :--------------------------------------------- | :----: | :------------------------------------------------ |
|   1 | `test_service_request_errors`                  |   ✅   | Service request errors                            |
|   2 | `test_build_response_guards`                   |   ✅   | Build response guards                             |
|   3 | `test_parse_request_truncations`               |   ✅   | Parse request truncations                         |
|   4 | `test_pubkey_blob_parse_failures`              |   ✅   | Pubkey blob parse failures                        |
|   5 | `test_pubkey_oversized_signed_prefix`          |   ✅   | Pubkey oversized signed prefix                    |
|   6 | `test_handle_request_index_and_parse_guards`   |   ✅   | Handle request index and parse guards             |
|   7 | `test_service_request_accept`                  |   ✅   | Service request accept                            |
|   8 | `test_service_request_rejects_unknown`         |   ✅   | Service request rejects unknown                   |
|   9 | `test_parse_password_request`                  |   ✅   | Parse password request                            |
|  10 | `test_parse_none_request`                      |   ✅   | Parse none request                                |
|  11 | `test_handle_request_success`                  |   ✅   | Handle request success                            |
|  12 | `test_handle_request_wrong_password_fails`     |   ✅   | Handle request wrong password fails               |
|  13 | `test_handle_none_request_fails_without_auth`  |   ✅   | Handle none request fails without auth            |
|  14 | `test_handle_request_no_callback_fails`        |   ✅   | No callback installed → all credentials rejected. |
|  15 | `test_pubkey_probe_returns_pk_ok`              |   ✅   | Pubkey probe returns pk ok                        |
|  16 | `test_pubkey_valid_signature_succeeds`         |   ✅   | Pubkey valid signature succeeds                   |
|  17 | `test_pubkey_rsa_sha512_signature_succeeds`    |   ✅   | Pubkey rsa sha512 signature succeeds              |
|  18 | `test_pubkey_ecdsa_signature_succeeds`         |   ✅   | Pubkey ecdsa signature succeeds                   |
|  19 | `test_pubkey_ed25519_valid_signature_succeeds` |   ✅   | Pubkey ed25519 valid signature succeeds           |
|  20 | `test_pubkey_tampered_signature_fails`         |   ✅   | Pubkey tampered signature fails                   |
|  21 | `test_pubkey_unauthorized_key_fails`           |   ✅   | Pubkey unauthorized key fails                     |

</details>

---

## test_ssh_crypto - native_ssh - ✅ 58 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH crypto layer test suite._

|   # | Test                                         | Status | Description                                                                                     |
| --: | :------------------------------------------- | :----: | :---------------------------------------------------------------------------------------------- |
|   1 | `test_ghash_table_matches_bitwise`           |   ✅   | Ghash table matches bitwise                                                                     |
|   2 | `test_sha256_empty`                          |   ✅   | SHA256("") = e3b0c44298fc1c149afb...                                                            |
|   3 | `test_sha256_abc`                            |   ✅   | SHA256("abc") = ba7816bf8f01cfea414140de5dae2ec73b00361bbef0469...                              |
|   4 | `test_sha256_448bit`                         |   ✅   | SHA256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")                              |
|   5 | `test_sha256_streaming`                      |   ✅   | Same as test_sha256_abc but using the streaming API.                                            |
|   6 | `test_hmac_sha256_tc1`                       |   ✅   | RFC 4231 Test Case 1                                                                            |
|   7 | `test_hmac_sha256_tc2`                       |   ✅   | RFC 4231 Test Case 2                                                                            |
|   8 | `test_hmac_sha256_tc3`                       |   ✅   | RFC 4231 Test Case 3                                                                            |
|   9 | `test_hmac_sha256_streaming`                 |   ✅   | Same as tc1 but via streaming API.                                                              |
|  10 | `test_hmac_sha256_tc6_large_key`             |   ✅   | Hmac sha256 tc6 large key                                                                       |
|  11 | `test_hmac_sha512_tc1`                       |   ✅   | RFC 4231 Test Case 1: Key = 0x0b x20, Data = "Hi There".                                        |
|  12 | `test_hmac_sha512_tc2`                       |   ✅   | RFC 4231 Test Case 2: Key = "Jefe", Data = "what do ya want for nothing?".                      |
|  13 | `test_hmac_sha512_streaming`                 |   ✅   | Same as tc1 but via the streaming API (also exercises the 128-byte block boundary).             |
|  14 | `test_hmac_sha512_tc6_large_key`             |   ✅   | Hmac sha512 tc6 large key                                                                       |
|  15 | `test_aes256ctr_encrypt`                     |   ✅   | NIST SP 800-38A, Section F.5.5                                                                  |
|  16 | `test_aes256ctr_decrypt`                     |   ✅   | AES-256-CTR decrypt is identical to encrypt.                                                    |
|  17 | `test_aes256ctr_multi_block`                 |   ✅   | NIST F.5.5 blocks 1-4 (64 bytes).                                                               |
|  18 | `test_aes256ctr_wipe`                        |   ✅   | After wipe, the context should be all zeros.                                                    |
|  19 | `test_bn_roundtrip`                          |   ✅   | Round-trip: bytes → SshBigNum → bytes.                                                          |
|  20 | `test_bn_cmp_equal`                          |   ✅   | Bn cmp equal                                                                                    |
|  21 | `test_bn_cmp_less`                           |   ✅   | Bn cmp less                                                                                     |
|  22 | `test_bn_cmp_greater`                        |   ✅   | Bn cmp greater                                                                                  |
|  23 | `test_bn_is_zero`                            |   ✅   | Bn is zero                                                                                      |
|  24 | `test_bn_dh_validate_rejects_zero`           |   ✅   | Bn dh validate rejects zero                                                                     |
|  25 | `test_bn_dh_validate_rejects_one`            |   ✅   | Bn dh validate rejects one                                                                      |
|  26 | `test_bn_dh_validate_accepts_two`            |   ✅   | Bn dh validate accepts two                                                                      |
|  27 | `test_expmod_exp1`                           |   ✅   | Expmod exp1                                                                                     |
|  28 | `test_expmod_exp2`                           |   ✅   | Expmod exp2                                                                                     |
|  29 | `test_expmod_exp3`                           |   ✅   | Expmod exp3                                                                                     |
|  30 | `test_expmod_commutative`                    |   ✅   | Expmod commutative                                                                              |
|  31 | `test_rsa_pkcs1_pad_structure`               |   ✅   | With d=1, sign(msg) = m^1 mod n = m (the padded message itself).                                |
|  32 | `test_rsa_sign_verify_roundtrip`             |   ✅   | Install the real keypair into the native sign fixture.                                          |
|  33 | `test_rsa_encode_pubkey`                     |   ✅   | Rsa encode pubkey                                                                               |
|  34 | `test_rsa_verify_and_encode_guards`          |   ✅   | Rsa verify and encode guards                                                                    |
|  35 | `test_rsa_verify_valid_signature`            |   ✅   | Rsa verify valid signature                                                                      |
|  36 | `test_rsa_verify_rejects_tampered_signature` |   ✅   | Rsa verify rejects tampered signature                                                           |
|  37 | `test_rsa_verify_rejects_wrong_message`      |   ✅   | Rsa verify rejects wrong message                                                                |
|  38 | `test_rsa_sha512_kat_sign_verify`            |   ✅   | Rsa sha512 kat sign verify                                                                      |
|  39 | `test_pkt_send_recv_unencrypted`             |   ✅   | Pkt send recv unencrypted                                                                       |
|  40 | `test_pkt_padding_alignment`                 |   ✅   | Packet length + padding must be multiple of 16.                                                 |
|  41 | `test_pkt_seq_increments`                    |   ✅   | Pkt seq increments                                                                              |
|  42 | `test_pkt_disconnect_zeroes_state`           |   ✅   | Pkt disconnect zeroes state                                                                     |
|  43 | `test_pkt_encrypted_roundtrip`               |   ✅   | Pkt encrypted roundtrip                                                                         |
|  44 | `test_pkt_chacha20poly1305_roundtrip`        |   ✅   | Install a chacha20-poly1305 session with the same key both directions, so ssh_pkt_send()        |
|  45 | `test_pkt_aes256gcm_roundtrip`               |   ✅   | Install an aes256-gcm@openssh.com session with the same key/IV both directions, so ssh_pkt_send |
|  46 | `test_pkt_aes_etm_sha256_roundtrip`          |   ✅   | Pkt aes etm sha256 roundtrip                                                                    |
|  47 | `test_pkt_aes_etm_sha512_roundtrip`          |   ✅   | Pkt aes etm sha512 roundtrip                                                                    |
|  48 | `test_pkt_encrypted_fragmented`              |   ✅   | Pkt encrypted fragmented                                                                        |
|  49 | `test_pkt_encrypted_two_packets`             |   ✅   | Pkt encrypted two packets                                                                       |
|  50 | `test_pkt_chacha_padding_and_incomplete`     |   ✅   | Pkt chacha padding and incomplete                                                               |
|  51 | `test_pkt_etm_padding_and_incomplete`        |   ✅   | Pkt etm padding and incomplete                                                                  |
|  52 | `test_pkt_chacha_forged_rejects`             |   ✅   | Pkt chacha forged rejects                                                                       |
|  53 | `test_pkt_etm_bad_length`                    |   ✅   | Pkt etm bad length                                                                              |
|  54 | `test_pkt_etm_forged_rejects`                |   ✅   | Pkt etm forged rejects                                                                          |
|  55 | `test_pkt_scratch_exhausted`                 |   ✅   | Pkt scratch exhausted                                                                           |
|  56 | `test_pkt_eam_forged_rejects`                |   ✅   | Pkt eam forged rejects                                                                          |
|  57 | `test_ssh_kdf_canonical_mpint_k`             |   ✅   | Ssh kdf canonical mpint k                                                                       |
|  58 | `test_ssh_kdf_extension_chain`               |   ✅   | Ssh kdf extension chain                                                                         |

</details>

---

## test_ssh_transport - native_ssh - ✅ 47 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH transport handshake tests (RFC 4253): identification-string exchange and_

|   # | Test                                                       | Status | Description                                                                               |
| --: | :--------------------------------------------------------- | :----: | :---------------------------------------------------------------------------------------- |
|   1 | `test_transport_index_guards`                              |   ✅   | Transport index guards                                                                    |
|   2 | `test_banner_and_build_caps`                               |   ✅   | Banner and build caps                                                                     |
|   3 | `test_kexinit_parse_field_and_trunc`                       |   ✅   | Kexinit parse field and trunc                                                             |
|   4 | `test_kexdh_parse_and_handle_errors`                       |   ✅   | Kexdh parse and handle errors                                                             |
|   5 | `test_server_banner_format`                                |   ✅   | Server banner format                                                                      |
|   6 | `test_recv_banner_complete`                                |   ✅   | Recv banner complete                                                                      |
|   7 | `test_recv_banner_bare_lf`                                 |   ✅   | Recv banner bare lf                                                                       |
|   8 | `test_recv_banner_split_across_reads`                      |   ✅   | Recv banner split across reads                                                            |
|   9 | `test_recv_banner_skips_preamble_lines`                    |   ✅   | RFC 4253 §4.2 allows lines before the SSH identification string.                          |
|  10 | `test_kexinit_build_starts_with_msg_and_stores_is`         |   ✅   | Kexinit build starts with msg and stores is                                               |
|  11 | `test_kexinit_parse_accepts_supported`                     |   ✅   | Kexinit parse accepts supported                                                           |
|  12 | `test_kexinit_parse_accepts_when_ours_listed_among_others` |   ✅   | Kexinit parse accepts when ours listed among others                                       |
|  13 | `test_kexinit_parse_rejects_missing_kex`                   |   ✅   | Only a KEX method we do not implement (nistp521) -> no mutual KEX -> reject. (nistp256 IS |
|  14 | `test_kexinit_parse_rejects_hostkey_we_lack`               |   ✅   | Kexinit parse rejects hostkey we lack                                                     |
|  15 | `test_kexinit_parse_steers_to_curve_ed25519`               |   ✅   | Kexinit parse steers to curve ed25519                                                     |
|  16 | `test_kexinit_parse_rejects_missing_cipher`                |   ✅   | Only ciphers we do not implement -> no mutual cipher -> reject.                           |
|  17 | `test_kexinit_parse_selects_chacha20poly1305`              |   ✅   | Kexinit parse selects chacha20poly1305                                                    |
|  18 | `test_kexinit_parse_selects_aes256gcm`                     |   ✅   | Kexinit parse selects aes256gcm                                                           |
|  19 | `test_kexinit_parse_selects_rsa_sha512`                    |   ✅   | Both offered -> rsa-sha2-512 wins (server preference).                                    |
|  20 | `test_kexinit_parse_selects_ecdsa`                         |   ✅   | Kexinit parse selects ecdsa                                                               |
|  21 | `test_kexinit_parse_selects_ecdh_nistp256`                 |   ✅   | Kexinit parse selects ecdh nistp256                                                       |
|  22 | `test_kexinit_parse_selects_etm_mac`                       |   ✅   | Kexinit parse selects etm mac                                                             |
|  23 | `test_kexinit_parse_rejects_truncated`                     |   ✅   | Kexinit parse rejects truncated                                                           |
|  24 | `test_exchange_hash_matches_independent_assembly`          |   ✅   | Populate the session fields the hash reads.                                               |
|  25 | `test_exchange_hash_changes_with_input`                    |   ✅   | Exchange hash changes with input                                                          |
|  26 | `test_kexdh_parse_init_extracts_e_with_padding`            |   ✅   | Kexdh parse init extracts e with padding                                                  |
|  27 | `test_kexdh_parse_init_extracts_small_e`                   |   ✅   | Kexdh parse init extracts small e                                                         |
|  28 | `test_kexdh_parse_init_rejects_wrong_type`                 |   ✅   | Kexdh parse init rejects wrong type                                                       |
|  29 | `test_kexdh_parse_init_rejects_oversized_e`                |   ✅   | mpint with 300 magnitude bytes → exceeds 2048 bits.                                       |
|  30 | `test_kexdh_build_reply_structure`                         |   ✅   | Kexdh build reply structure                                                               |
|  31 | `test_kexdh_handle_produces_reply_and_installs_keys`       |   ✅   | Kexdh handle produces reply and installs keys                                             |
|  32 | `test_kexdh_handle_rejects_invalid_e`                      |   ✅   | Kexdh handle rejects invalid e                                                            |
|  33 | `test_kexdh_handle_curve25519_ed25519_end_to_end`          |   ✅   | Fixed baseline host keys for deterministic regression, plus one fresh throwaway           |
|  34 | `test_kexdh_handle_curve25519_rejects_low_order`           |   ✅   | Kexdh handle curve25519 rejects low order                                                 |
|  35 | `test_kexdh_handle_ecdh_nistp256_end_to_end`               |   ✅   | Kexdh handle ecdh nistp256 end to end                                                     |
|  36 | `test_kexdh_handle_ecdh_nistp256_rejects_bad_point`        |   ✅   | Kexdh handle ecdh nistp256 rejects bad point                                              |
|  37 | `test_kexdh_handle_rsa_sha512_signature`                   |   ✅   | Kexdh handle rsa sha512 signature                                                         |
|  38 | `test_kexdh_handle_ecdsa_end_to_end`                       |   ✅   | Kexdh handle ecdsa end to end                                                             |
|  39 | `test_derive_keys_session_id_affects_output`               |   ✅   | Derive keys session id affects output                                                     |
|  40 | `test_rekey_needed_threshold`                              |   ✅   | Rekey needed threshold                                                                    |
|  41 | `test_rekey_due_volume_and_time`                           |   ✅   | Neither budget spent.                                                                     |
|  42 | `test_begin_rekey_preserves_session_and_auth`              |   ✅   | Begin rekey preserves session and auth                                                    |
|  43 | `test_kdf_edge_paths_and_slot_guards`                      |   ✅   | Kdf edge paths and slot guards                                                            |
|  44 | `test_kexinit_parse_truncation_points`                     |   ✅   | Kexinit parse truncation points                                                           |
|  45 | `test_ssh_transport_more_guards`                           |   ✅   | Ssh transport more guards                                                                 |
|  46 | `test_dh_derive_keys_gcm_installs`                         |   ✅   | Dh derive keys gcm installs                                                               |
|  47 | `test_kdf_string_k_hybrid`                                 |   ✅   | Kdf string k hybrid                                                                       |

</details>

---

## test_ssh_server - native_ssh - ✅ 27 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_End-to-end SSH server dispatcher test: drives a full handshake_

|   # | Test                                                 | Status | Description                                                           |
| --: | :--------------------------------------------------- | :----: | :-------------------------------------------------------------------- |
|   1 | `test_ssh_dispatch_bad_slot`                         |   ✅   | Ssh dispatch bad slot                                                 |
|   2 | `test_ssh_kexinit_parse_fail`                        |   ✅   | Ssh kexinit parse fail                                                |
|   3 | `test_ssh_kexdh_guards`                              |   ✅   | Ssh kexdh guards                                                      |
|   4 | `test_ssh_service_request_fail`                      |   ✅   | Ssh service request fail                                              |
|   5 | `test_ssh_userauth_guards`                           |   ✅   | Ssh userauth guards                                                   |
|   6 | `test_ssh_postauth_authed_guard`                     |   ✅   | Ssh postauth authed guard                                             |
|   7 | `test_ssh_postauth_handler_fails`                    |   ✅   | Ssh postauth handler fails                                            |
|   8 | `test_ssh_open_confirm_failure_authed`               |   ✅   | Ssh open confirm failure authed                                       |
|   9 | `test_ssh_global_request_reply`                      |   ✅   | Ssh global request reply                                              |
|  10 | `test_ssh_window_adjust_and_eof`                     |   ✅   | Ssh window adjust and eof                                             |
|  11 | `test_ssh_pkt_index_and_cap_guards`                  |   ✅   | Ssh pkt index and cap guards                                          |
|  12 | `test_ssh_pkt_recv_unencrypted_errors`               |   ✅   | Ssh pkt recv unencrypted errors                                       |
|  13 | `test_ssh_pkt_seq_overflow_guards`                   |   ✅   | Ssh pkt seq overflow guards                                           |
|  14 | `test_ssh_pkt_encrypted_roundtrip_and_mac_fail`      |   ✅   | Ssh pkt encrypted roundtrip and mac fail                              |
|  15 | `test_full_handshake_to_channel_data`                |   ✅   | Banner exchange already done out-of-band; seed V_C and enter KEXINIT. |
|  16 | `test_extinfo_build_advertises_server_sig_algs`      |   ✅   | Extinfo build advertises server sig algs                              |
|  17 | `test_extinfo_not_sent_without_ext_info_c`           |   ✅   | Extinfo not sent without ext info c                                   |
|  18 | `test_inbound_ext_info_ignored`                      |   ✅   | Inbound ext info ignored                                              |
|  19 | `test_large_client_kexinit_accepted`                 |   ✅   | Large client kexinit accepted                                         |
|  20 | `test_channel_open_before_auth_rejected`             |   ✅   | Channel open before auth rejected                                     |
|  21 | `test_service_request_before_newkeys_rejected`       |   ✅   | Service request before newkeys rejected                               |
|  22 | `test_disconnect_closes`                             |   ✅   | Disconnect closes                                                     |
|  23 | `test_ignore_is_noop`                                |   ✅   | Ignore is noop                                                        |
|  24 | `test_auth_bruteforce_disconnect`                    |   ✅   | The first SSH_MAX_AUTH_ATTEMPTS-1 failures keep the connection open.  |
|  25 | `test_auth_success_after_failures`                   |   ✅   | Auth success after failures                                           |
|  26 | `test_unimplemented_reply_for_unknown_message`       |   ✅   | Unimplemented reply for unknown message                               |
|  27 | `test_inbound_close_emits_eof_then_close_separately` |   ✅   | Open a channel so the close path has something to close (peer id 21). |

</details>

---

## test_ssh_pqc - native_ssh_pqc - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_End-to-end test of the mlkem768x25519-sha256 SSH hybrid key exchange (draft-ietf-sshm-mlkem-hybrid-_

|   # | Test                            | Status | Description              |
| --: | :------------------------------ | :----: | :----------------------- |
|   1 | `test_decaps_ref_matches_kat`   |   ✅   | Decaps ref matches kat   |
|   2 | `test_hybrid_negotiated`        |   ✅   | Hybrid negotiated        |
|   3 | `test_hybrid_absent_falls_back` |   ✅   | Hybrid absent falls back |
|   4 | `test_hybrid_kex_end_to_end`    |   ✅   | Hybrid kex end to end    |

</details>

---

## test_ssh_hardening - native_ssh_hardened - ✅ 2 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Built with DWS_SSH_ALLOW_PASSWORD=0: verifies password authentication is_

|   # | Test                                               | Status | Description                                                            |
| --: | :------------------------------------------------- | :----: | :--------------------------------------------------------------------- |
|   1 | `test_password_refused_even_with_correct_callback` |   ✅   | Even a callback that accepts everything must not authenticate, because |
|   2 | `test_failure_advertises_publickey_only`           |   ✅   | Failure advertises publickey only                                      |

</details>

---

## test_ssh_conn - native_ssh_conn - ✅ 16 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH transport-glue test: drives a ConnProto::PROTO_SSH connection through the real_

|   # | Test                                            | Status | Description                              |
| --: | :---------------------------------------------- | :----: | :--------------------------------------- |
|   1 | `test_conn_outbound_arena_exhausted`            |   ✅   | Conn outbound arena exhausted            |
|   2 | `test_conn_outbound_pkt_send_fails`             |   ✅   | Conn outbound pkt send fails             |
|   3 | `test_poll_rekey_emit_fails`                    |   ✅   | Poll rekey emit fails                    |
|   4 | `test_accept_sends_server_banner`               |   ✅   | Accept sends server banner               |
|   5 | `test_banner_then_kexinit_advances_and_replies` |   ✅   | Banner then kexinit advances and replies |
|   6 | `test_poll_triggers_server_rekey`               |   ✅   | Poll triggers server rekey               |
|   7 | `test_proto_handler_accessor`                   |   ✅   | Proto handler accessor                   |
|   8 | `test_proto_handler_wires_emit`                 |   ✅   | Proto handler wires emit                 |
|   9 | `test_send_entrypoints_reject`                  |   ✅   | Send entrypoints reject                  |
|  10 | `test_poll_rx_banner_guards`                    |   ✅   | Poll rx banner guards                    |
|  11 | `test_conn_send_close_open_channel`             |   ✅   | Conn send close open channel             |
|  12 | `test_send_channel_reject_paths`                |   ✅   | Send channel reject paths                |
|  13 | `test_accept_no_ssh_capacity`                   |   ✅   | Accept no ssh capacity                   |
|  14 | `test_poll_ignores_inactive_conn`               |   ✅   | Poll ignores inactive conn               |
|  15 | `test_rx_disconnect_tears_down`                 |   ✅   | Rx disconnect tears down                 |
|  16 | `test_rx_overlong_banner_closes`                |   ✅   | Rx overlong banner closes                |

</details>

---

## test_ssh_sftp - native_ssh_sftp - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/sftp: the SFTP protocol v3 wire codec. Covers the reader/writer round-trips, the_

|   # | Test                                   | Status | Description                                                                                             |
| --: | :------------------------------------- | :----: | :------------------------------------------------------------------------------------------------------ |
|   1 | `test_fs_path_resolve`                 |   ✅   | Fs path resolve                                                                                         |
|   2 | `test_rw_roundtrip`                    |   ✅   | Rw roundtrip                                                                                            |
|   3 | `test_reader_bounds`                   |   ✅   | Reader bounds                                                                                           |
|   4 | `test_attrs_roundtrip`                 |   ✅   | Attrs roundtrip                                                                                         |
|   5 | `test_attrs_skips_uidgid_and_extended` |   ✅   | Manually craft an ATTRS with UIDGID + PERMISSIONS + one EXTENDED pair, and confirm perms are recovered. |
|   6 | `test_framing`                         |   ✅   | Framing                                                                                                 |
|   7 | `test_parse_open_request`              |   ✅   | Parse open request                                                                                      |
|   8 | `test_build_version`                   |   ✅   | Build version                                                                                           |
|   9 | `test_build_status`                    |   ✅   | Build status                                                                                            |
|  10 | `test_build_handle_and_data`           |   ✅   | Build handle and data                                                                                   |
|  11 | `test_build_name1_realpath`            |   ✅   | Build name1 realpath                                                                                    |
|  12 | `test_name_multi_entry`                |   ✅   | Name multi entry                                                                                        |
|  13 | `test_longname_format`                 |   ✅   | Longname format                                                                                         |
|  14 | `test_builder_overflow`                |   ✅   | Builder overflow                                                                                        |

</details>

---

## test_scp - native_scp - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/scp: the SCP (RCP) wire codec. Covers parsing an `scp -t/-f <path>` exec command_

|   # | Test                               | Status | Description                                 |
| --: | :--------------------------------- | :----: | :------------------------------------------ |
|   1 | `test_parse_cmd_sink`              |   ✅   | Parse cmd sink                              |
|   2 | `test_parse_cmd_source_with_flags` |   ✅   | Parse cmd source with flags                 |
|   3 | `test_parse_cmd_invalid`           |   ✅   | no -t/-f role                               |
|   4 | `test_parse_cline`                 |   ✅   | Parse cline                                 |
|   5 | `test_parse_cline_malformed`       |   ✅   | a directory record (D) is not a file record |
|   6 | `test_build_cline_roundtrip`       |   ✅   | Build cline roundtrip                       |

</details>

---

## test_middleware - native_app - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the middleware chain (use()) and the built-in rate limiter_

|   # | Test                                          | Status | Description                                                                |
| --: | :-------------------------------------------- | :----: | :------------------------------------------------------------------------- |
|   1 | `test_middleware_runs_then_handler`           |   ✅   | Middleware runs then handler                                               |
|   2 | `test_middleware_runs_for_unmatched_route`    |   ✅   | No route registered -> 404, but the middleware still observes the request. |
|   3 | `test_middleware_can_inject_response_header`  |   ✅   | Middleware can inject response header                                      |
|   4 | `test_middleware_halt_short_circuits_handler` |   ✅   | Middleware halt short circuits handler                                     |
|   5 | `test_middleware_runs_in_registration_order`  |   ✅   | Middleware runs in registration order                                      |
|   6 | `test_use_respects_capacity_cap`              |   ✅   | Register more than MAX_MIDDLEWARE; extras are dropped, none crash.         |
|   7 | `test_rate_limit_allows_then_rejects`         |   ✅   | Rate limit allows then rejects                                             |
|   8 | `test_rate_limit_window_resets`               |   ✅   | Rate limit window resets                                                   |
|   9 | `test_rate_limit_disabled_by_default`         |   ✅   | Rate limit disabled by default                                             |

</details>

---

## test_application - native_app - ✅ 72 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit, stress, and race-condition tests for Layer 7 (Application)._

|   # | Test                                                  | Status | Description                                                                                      |
| --: | :---------------------------------------------------- | :----: | :----------------------------------------------------------------------------------------------- |
|   1 | `test_response_trailer_truncation_clamps`             |   ✅   | (a) The status line alone overflows the header buffer -> hlen >= cap -> clamp.                   |
|   2 | `test_restart_and_stop`                               |   ✅   | Before any listener, restart() forwards the no-listeners error (no stop()/begin()).              |
|   3 | `test_route_registration_variants_table_full`         |   ✅   | Route registration variants table full                                                           |
|   4 | `test_send_family_slot_and_conn_gone_guards`          |   ✅   | Send family slot and conn gone guards                                                            |
|   5 | `test_send_binary_body_with_nul`                      |   ✅   | Send binary body with nul                                                                        |
|   6 | `test_redirect_response_and_code_normalization`       |   ✅   | Redirect response and code normalization                                                         |
|   7 | `test_request_error_paths_te_method_ws`               |   ✅   | Wrong method to a GET-only route -> 405 with an Allow header.                                    |
|   8 | `test_ws_sse_upgrade_failure_paths`                   |   ✅   | (a) A Sec-WebSocket-Key that does not base64-decode to 16 bytes -> ws_accept_key rejects -> 400. |
|   9 | `test_sse_upgrade_pool_exhausted`                     |   ✅   | Sse upgrade pool exhausted                                                                       |
|  10 | `test_handler_reads_body`                             |   ✅   | Handler reads body                                                                               |
|  11 | `test_handler_reads_query_param`                      |   ✅   | Handler reads query param                                                                        |
|  12 | `test_handler_reads_header`                           |   ✅   | Handler reads header                                                                             |
|  13 | `test_wildcard_before_exact_wildcard_wins`            |   ✅   | Wildcard before exact wildcard wins                                                              |
|  14 | `test_fn_on_registers_and_dispatches`                 |   ✅   | Fn on registers and dispatches                                                                   |
|  15 | `test_fn_on_path_copied_null_terminated`              |   ✅   | A path of exactly MAX_PATH_LEN-1 chars must not overflow the route buffer.                       |
|  16 | `test_fn_on_table_full_extra_routes_dropped`          |   ✅   | Fill the table; on() beyond MAX_ROUTES must silently drop                                        |
|  17 | `test_fn_on_same_path_different_methods_are_distinct` |   ✅   | Fn on same path different methods are distinct                                                   |
|  18 | `test_fn_on_not_found_called_when_no_match`           |   ✅   | Fn on not found called when no match                                                             |
|  19 | `test_fn_on_not_found_not_called_when_match_exists`   |   ✅   | Fn on not found not called when match exists                                                     |
|  20 | `test_fn_set_cors_options_preflight_clears_slot`      |   ✅   | Fn set cors options preflight clears slot                                                        |
|  21 | `test_fn_set_cors_empty_string_disables`              |   ✅   | Fn set cors empty string disables                                                                |
|  22 | `test_wrong_method_does_not_match`                    |   ✅   | Wrong method does not match                                                                      |
|  23 | `test_wrong_path_does_not_match`                      |   ✅   | Wrong path does not match                                                                        |
|  24 | `test_all_http_methods_dispatched`                    |   ✅   | All http methods dispatched                                                                      |
|  25 | `test_root_path_matches_exactly`                      |   ✅   | Root path matches exactly                                                                        |
|  26 | `test_root_path_does_not_match_subpath`               |   ✅   | Root path does not match subpath                                                                 |
|  27 | `test_wildcard_matches_any_suffix`                    |   ✅   | Wildcard matches any suffix                                                                      |
|  28 | `test_wildcard_does_not_match_unrelated_prefix`       |   ✅   | Wildcard does not match unrelated prefix                                                         |
|  29 | `test_exact_route_wins_when_registered_first`         |   ✅   | Exact route wins when registered first                                                           |
|  30 | `test_slot_not_stuck_in_complete_after_handle`        |   ✅   | Slot not stuck in complete after handle                                                          |
|  31 | `test_parse_error_slot_auto_reset`                    |   ✅   | Parse error slot auto reset                                                                      |
|  32 | `stress_last_route_dispatched_in_full_table`          |   ✅   | Stress - Last route dispatched in full table                                                     |
|  33 | `stress_sequential_requests_no_state_leak`            |   ✅   | Stress - Sequential requests no state leak                                                       |
|  34 | `stress_all_slots_dispatched_simultaneously`          |   ✅   | Stress - All slots dispatched simultaneously                                                     |
|  35 | `stress_wildcard_matches_many_paths`                  |   ✅   | Stress - Wildcard matches many paths                                                             |
|  36 | `stress_handle_with_no_complete_slots_is_nop`         |   ✅   | All slots in ParseState::PARSE_METHOD (setUp resets them) - nothing to dispatch                  |
|  37 | `race_slot_complete_between_handle_calls`             |   ✅   | Race - Slot complete between handle calls                                                        |
|  38 | `race_conn_freed_after_parse_complete`                |   ✅   | Race - Conn freed after parse complete                                                           |
|  39 | `race_double_handle_no_double_dispatch`               |   ✅   | Race - Double handle no double dispatch                                                          |
|  40 | `race_error_and_valid_slot_in_same_handle`            |   ✅   | Slot 0: inject a parse error                                                                     |
|  41 | `race_callback_manually_resets_slot`                  |   ✅   | Race - Callback manually resets slot                                                             |
|  42 | `test_uri_too_long_auto_resets_slot`                  |   ✅   | Overflow the path buffer - handle() should send 414 and free the slot                            |
|  43 | `test_transfer_encoding_chunked_is_501`               |   ✅   | A request advertising Transfer-Encoding must be rejected with 501                                |
|  44 | `test_transfer_encoding_identity_is_501`              |   ✅   | Even "identity" is rejected - we advertise no TE support at all                                  |
|  45 | `test_redirect_emits_location_and_status`             |   ✅   | Redirect emits location and status                                                               |
|  46 | `test_redirect_invalid_code_defaults_to_302`          |   ✅   | Redirect invalid code defaults to 302                                                            |
|  47 | `test_mime_type_detection`                            |   ✅   | Mime type detection                                                                              |
|  48 | `test_serve_static_file_and_mime`                     |   ✅   | Serve static file and mime                                                                       |
|  49 | `test_serve_static_wildcard_and_route_full`           |   ✅   | Serve static wildcard and route full                                                             |
|  50 | `test_response_header_cookie_guards`                  |   ✅   | Response header cookie guards                                                                    |
|  51 | `test_serve_static_index_fallback`                    |   ✅   | Serve static index fallback                                                                      |
|  52 | `test_serve_static_gzip_when_accepted`                |   ✅   | Serve static gzip when accepted                                                                  |
|  53 | `test_serve_static_no_gzip_when_not_accepted`         |   ✅   | Serve static no gzip when not accepted                                                           |
|  54 | `test_serve_static_traversal_not_leaked`              |   ✅   | Serve static traversal not leaked                                                                |
|  55 | `test_serve_static_missing_is_404`                    |   ✅   | Serve static missing is 404                                                                      |
|  56 | `test_serve_static_etag_conditional_get`              |   ✅   | First GET: 200 with an ETag header.                                                              |
|  57 | `test_serve_static_inm_star_list_weak`                |   ✅   | First GET to capture the strong ETag (with quotes).                                              |
|  58 | `test_serve_static_last_modified_conditional_get`     |   ✅   | (1) plain GET: 200 carries the Last-Modified header.                                             |
|  59 | `test_serve_static_ims_field_comparisons`             |   ✅   | Serve static ims field comparisons                                                               |
|  60 | `test_serve_static_unrepresentable_mtime`             |   ✅   | (a) plain GET: 200 with no Last-Modified line (http_rfc1123 bailed).                             |
|  61 | `test_serve_static_if_modified_since_malformed`       |   ✅   | Serve static if modified since malformed                                                         |
|  62 | `test_serve_static_cache_control`                     |   ✅   | Serve static cache control                                                                       |
|  63 | `test_request_log_hook_fires`                         |   ✅   | Request log hook fires                                                                           |
|  64 | `test_stats_endpoint_emits_json`                      |   ✅   | Stats endpoint emits json                                                                        |
|  65 | `test_status_text_reason_phrases`                     |   ✅   | Status text reason phrases                                                                       |
|  66 | `test_allow_header_lists_methods`                     |   ✅   | Allow header lists methods                                                                       |
|  67 | `test_listen_and_begin`                               |   ✅   | begin() before any listen() -> no-listeners error, no side effects.                              |
|  68 | `test_begin_port_convenience`                         |   ✅   | Begin port convenience                                                                           |
|  69 | `test_ws_send_api`                                    |   ✅   | Ws send api                                                                                      |
|  70 | `test_sse_broadcast_after_upgrade_matches_path`       |   ✅   | Sse broadcast after upgrade matches path                                                         |
|  71 | `test_sse_send_api`                                   |   ✅   | Sse send api                                                                                     |
|  72 | `test_metrics_emits_prometheus`                       |   ✅   | Metrics emits prometheus                                                                         |

</details>

---

## test_digest_vectors - native_app - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Independent-oracle regression test for the Digest-auth math (RFC 7616,_

|   # | Test                            | Status | Description              |
| --: | :------------------------------ | :----: | :----------------------- |
|   1 | `test_sha256_fips_kats`         |   ✅   | Sha256 fips kats         |
|   2 | `test_ha1_matches_openssl`      |   ✅   | Ha1 matches openssl      |
|   3 | `test_ha2_matches_openssl`      |   ✅   | Ha2 matches openssl      |
|   4 | `test_response_matches_openssl` |   ✅   | Response matches openssl |

</details>

---

## test_dispatch - native_app - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Dispatch-level RFC 7231 compliance:_

|   # | Test                                        | Status | Description                                                                 |
| --: | :------------------------------------------ | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_method_mismatch_returns_405`          |   ✅   | Method mismatch returns 405                                                 |
|   2 | `test_405_includes_allow_header`            |   ✅   | 405 includes allow header                                                   |
|   3 | `test_405_allow_lists_all_methods_for_path` |   ✅   | 405 allow lists all methods for path                                        |
|   4 | `test_unknown_path_still_404_not_405`       |   ✅   | Unknown path still 404 not 405                                              |
|   5 | `test_unknown_method_returns_501`           |   ✅   | Unknown method returns 501                                                  |
|   6 | `test_unknown_method_not_treated_as_get`    |   ✅   | A bogus method must NOT run the GET handler (security: no method spoofing). |
|   7 | `test_head_runs_get_handler_without_body`   |   ✅   | Head runs get handler without body                                          |
|   8 | `test_get_route_advertises_head_in_allow`   |   ✅   | Get route advertises head in allow                                          |
|   9 | `test_head_on_post_only_route_405`          |   ✅   | Head on post only route 405                                                 |
|  10 | `test_http_parse_skips_ws_upgraded_slot`    |   ✅   | Http parse skips ws upgraded slot                                           |
|  11 | `test_correct_method_still_dispatches`      |   ✅   | Correct method still dispatches                                             |

</details>

---

## test_web_terminal - native_app - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the WebSocket web-serial terminal (DWS_ENABLE_WEB_TERMINAL):_

|   # | Test                                        | Status | Description                                                  |
| --: | :------------------------------------------ | :----: | :----------------------------------------------------------- |
|   1 | `test_serves_terminal_page`                 |   ✅   | Serves terminal page                                         |
|   2 | `test_ws_upgrade_tracks_client`             |   ✅   | Ws upgrade tracks client                                     |
|   3 | `test_ws_upgrade_requires_connection_token` |   ✅   | Ws upgrade requires connection token                         |
|   4 | `test_ws_upgrade_rejects_bad_key_length`    |   ✅   | Ws upgrade rejects bad key length                            |
|   5 | `test_command_delivered_to_callback`        |   ✅   | Command delivered to callback                                |
|   6 | `test_broadcast_reaches_client`             |   ✅   | Broadcast reaches client                                     |
|   7 | `test_printf_broadcast`                     |   ✅   | Printf broadcast                                             |
|   8 | `test_no_broadcast_without_clients`         |   ✅   | No handshake -> no terminal clients -> print writes nothing. |
|   9 | `test_close_clears_client`                  |   ✅   | Close clears client                                          |

</details>

---

## test_response_headers - native_app - ✅ 12 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for custom response headers and cookies:_

|   # | Test                                       | Status | Description                                                                                  |
| --: | :----------------------------------------- | :----: | :------------------------------------------------------------------------------------------- |
|   1 | `test_ntp_host_seam_accessors`             |   ✅   | Host build: begin() is a no-op returning false; synced()/epoch() reflect the injected epoch. |
|   2 | `test_date_header_emitted_when_time_set`   |   ✅   | Date header emitted when time set                                                            |
|   3 | `test_date_header_omitted_when_clockless`  |   ✅   | Date header omitted when clockless                                                           |
|   4 | `test_single_custom_header_present`        |   ✅   | Single custom header present                                                                 |
|   5 | `test_multiple_custom_headers_present`     |   ✅   | Multiple custom headers present                                                              |
|   6 | `test_set_cookie_basic`                    |   ✅   | Set cookie basic                                                                             |
|   7 | `test_set_cookie_with_attrs`               |   ✅   | Set cookie with attrs                                                                        |
|   8 | `test_custom_header_on_send_empty`         |   ✅   | Custom header on send empty                                                                  |
|   9 | `test_custom_header_on_redirect`           |   ✅   | Custom header on redirect                                                                    |
|  10 | `test_headers_do_not_leak_across_requests` |   ✅   | First request queues X-Custom on slot 0.                                                     |
|  11 | `test_clear_response_headers`              |   ✅   | Clear response headers                                                                       |
|  12 | `test_oversized_header_dropped_whole`      |   ✅   | Oversized header dropped whole                                                               |

</details>

---

## test_defer - native_app - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Phase 3a: the thread-safe app->worker deferred-callback path. On host there is_

|   # | Test                                | Status | Description                                                    |
| --: | :---------------------------------- | :----: | :------------------------------------------------------------- |
|   1 | `test_defer_runs_inline_on_host`    |   ✅   | Defer runs inline on host                                      |
|   2 | `test_server_defer_routes_by_owner` |   ✅   | Server defer routes by owner                                   |
|   3 | `test_defer_null_fn_fails`          |   ✅   | A null callback fails closed on every build (host and target). |

</details>

---

## test_template - native_app - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for send_template() {{name}} placeholder substitution._

|   # | Test                                       | Status | Description                         |
| --: | :----------------------------------------- | :----: | :---------------------------------- |
|   1 | `test_basic_substitution`                  |   ✅   | Basic substitution                  |
|   2 | `test_multiple_placeholders`               |   ✅   | Multiple placeholders               |
|   3 | `test_unknown_placeholder_is_empty`        |   ✅   | Unknown placeholder is empty        |
|   4 | `test_unterminated_placeholder_is_literal` |   ✅   | Unterminated placeholder is literal |
|   5 | `test_null_resolver_empties_all`           |   ✅   | Null resolver empties all           |
|   6 | `test_head_suppresses_body_keeps_length`   |   ✅   | Head suppresses body keeps length   |

</details>

---

## test_regex - native_app - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for bounded regex routes (DWS::on_regex())._

|   # | Test                                            | Status | Description                                                                 |
| --: | :---------------------------------------------- | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_numeric_class_plus`                       |   ✅   | Numeric class plus                                                          |
|   2 | `test_dot_star_matches_rest`                    |   ✅   | Dot star matches rest                                                       |
|   3 | `test_escaped_dot_extension`                    |   ✅   | Escaped dot extension                                                       |
|   4 | `test_optional_quantifier`                      |   ✅   | Optional quantifier                                                         |
|   5 | `test_range_class_only`                         |   ✅   | Range class only                                                            |
|   6 | `test_negated_class`                            |   ✅   | Negated class                                                               |
|   7 | `test_anchored_full_match`                      |   ✅   | Anchored full match                                                         |
|   8 | `test_method_still_enforced`                    |   ✅   | Path matches but method differs -> 405, handler not called.                 |
|   9 | `test_pathological_pattern_terminates_no_match` |   ✅   | Catastrophic-looking pattern with no possible match: must return (not hang) |
|  10 | `test_escape_class_digit`                       |   ✅   | Escape class digit                                                          |
|  11 | `test_escape_class_word`                        |   ✅   | Escape class word                                                           |
|  12 | `test_escape_class_space`                       |   ✅   | Escape class space                                                          |
|  13 | `test_class_escaped_members`                    |   ✅   | Class escaped members                                                       |

</details>

---

## test_iface - native_app - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for per-route STA/AP interface filters (DWS::on(..., DWSIface))._

|   # | Test                                          | Status | Description                                                               |
| --: | :-------------------------------------------- | :----: | :------------------------------------------------------------------------ |
|   1 | `test_ap_only_matches_on_ap`                  |   ✅   | Ap only matches on ap                                                     |
|   2 | `test_ap_only_hidden_on_sta`                  |   ✅   | Ap only hidden on sta                                                     |
|   3 | `test_sta_only_matches_on_sta`                |   ✅   | Sta only matches on sta                                                   |
|   4 | `test_sta_only_hidden_on_ap`                  |   ✅   | Sta only hidden on ap                                                     |
|   5 | `test_unfiltered_route_matches_any_interface` |   ✅   | Unfiltered route matches any interface                                    |
|   6 | `test_same_path_two_interfaces_picks_correct` |   ✅   | Same path bound to different interfaces; the request's interface decides. |
|   7 | `test_set_ap_ip_updates_global`               |   ✅   | Set ap ip updates global                                                  |

</details>

---

## test_file_serving - native_app - ✅ 12 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for serve_file()._

|   # | Test                                           | Status | Description                                                                  |
| --: | :--------------------------------------------- | :----: | :--------------------------------------------------------------------------- |
|   1 | `test_missing_file_returns_404`                |   ✅   | Missing file returns 404                                                     |
|   2 | `test_existing_file_returns_200`               |   ✅   | Existing file returns 200                                                    |
|   3 | `test_response_includes_content_type_html`     |   ✅   | Response includes content type html                                          |
|   4 | `test_response_includes_content_type_js`       |   ✅   | Response includes content type js                                            |
|   5 | `test_content_length_matches_file_size`        |   ✅   | Content length matches file size                                             |
|   6 | `test_file_body_is_sent`                       |   ✅   | File body is sent                                                            |
|   7 | `test_empty_file_returns_200_with_zero_length` |   ✅   | Empty file returns 200 with zero length                                      |
|   8 | `test_large_file_body_fully_sent`              |   ✅   | A body far larger than one send-buffer window: the cross-loop file pump must |
|   9 | `test_serve_file_does_not_affect_other_routes` |   ✅   | Serve file does not affect other routes                                      |
|  10 | `test_multiple_content_types`                  |   ✅   | Multiple content types                                                       |
|  11 | `stress_serve_file_50_requests`                |   ✅   | Stress - Serve file 50 requests                                              |
|  12 | `stress_alternate_missing_and_found`           |   ✅   | Stress - Alternate missing and found                                         |

</details>

---

## test_path_params - native_app - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for `:name` path parameters and http_get_param()._

|   # | Test                                    | Status | Description                      |
| --: | :-------------------------------------- | :----: | :------------------------------- |
|   1 | `test_single_param_captured`            |   ✅   | Single param captured            |
|   2 | `test_multiple_params_captured`         |   ✅   | Multiple params captured         |
|   3 | `test_missing_param_returns_null`       |   ✅   | Missing param returns null       |
|   4 | `test_literal_segment_mismatch_404`     |   ✅   | Literal segment mismatch 404     |
|   5 | `test_extra_segment_does_not_match`     |   ✅   | Extra segment does not match     |
|   6 | `test_empty_param_value_does_not_match` |   ✅   | Empty param value does not match |
|   7 | `test_exact_route_still_matches`        |   ✅   | Exact route still matches        |
|   8 | `test_param_route_wrong_method_405`     |   ✅   | Param route wrong method 405     |

</details>

---

## test_digest_auth - native_app - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for HTTP Digest authentication (RFC 7616, SHA-256, qop=auth)._

|   # | Test                                          | Status | Description                                                                     |
| --: | :-------------------------------------------- | :----: | :------------------------------------------------------------------------------ |
|   1 | `test_challenge_is_digest_sha256`             |   ✅   | Challenge is digest sha256                                                      |
|   2 | `test_valid_digest_authenticates`             |   ✅   | Valid digest authenticates                                                      |
|   3 | `test_wrong_password_rejected`                |   ✅   | Wrong password rejected                                                         |
|   4 | `test_bad_nonce_rejected`                     |   ✅   | Bad nonce rejected                                                              |
|   5 | `test_wrong_username_rejected`                |   ✅   | Wrong username rejected                                                         |
|   6 | `test_wrong_qop_rejected`                     |   ✅   | Wrong qop rejected                                                              |
|   7 | `test_missing_response_field_rejected`        |   ✅   | Missing response field rejected                                                 |
|   8 | `test_basic_scheme_on_digest_route_rejected`  |   ✅   | A Basic Authorization header on a Digest-protected route must not authenticate. |
|   9 | `test_uri_mismatch_rejected`                  |   ✅   | Uri mismatch rejected                                                           |
|  10 | `test_nonce_is_stateless_timestamped`         |   ✅   | Nonce is stateless timestamped                                                  |
|  11 | `test_stale_nonce_triggers_transparent_retry` |   ✅   | Stale nonce triggers transparent retry                                          |

</details>

---

## test_json - native_app - ✅ 28 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the zero-heap JSON helper: JsonWriter (serialization) and the_

|   # | Test                                                    | Status | Description                                                          |
| --: | :------------------------------------------------------ | :----: | :------------------------------------------------------------------- |
|   1 | `test_reader_non_object_and_bad_member`                 |   ✅   | Reader non object and bad member                                     |
|   2 | `test_reader_int_rejects_string_and_nondigits`          |   ✅   | Reader int rejects string and nondigits                              |
|   3 | `test_reader_unicode_escape_invalid_and_wide`           |   ✅   | Reader unicode escape invalid and wide                               |
|   4 | `test_writer_simple_object`                             |   ✅   | Writer simple object                                                 |
|   5 | `test_writer_nested_and_array`                          |   ✅   | Writer nested and array                                              |
|   6 | `test_writer_value_types`                               |   ✅   | Writer value types                                                   |
|   7 | `test_writer_escapes_strings`                           |   ✅   | Writer escapes strings                                               |
|   8 | `test_writer_control_char_unicode_escape`               |   ✅   | Writer control char unicode escape                                   |
|   9 | `test_writer_overflow_sets_not_ok_and_stays_terminated` |   ✅   | Writer overflow sets not ok and stays terminated                     |
|  10 | `test_writer_depth_overflow_sets_not_ok`                |   ✅   | Writer depth overflow sets not ok                                    |
|  11 | `test_reader_get_string`                                |   ✅   | Reader get string                                                    |
|  12 | `test_reader_get_int`                                   |   ✅   | Reader get int                                                       |
|  13 | `test_reader_get_bool`                                  |   ✅   | Reader get bool                                                      |
|  14 | `test_reader_only_matches_top_level_key`                |   ✅   | "x" exists both nested and at top level; the top-level one must win. |
|  15 | `test_reader_missing_key`                               |   ✅   | Reader missing key                                                   |
|  16 | `test_reader_type_mismatch`                             |   ✅   | "name" is a string, not an int or bool.                              |
|  17 | `test_reader_unescapes_value`                           |   ✅   | Reader unescapes value                                               |
|  18 | `test_reader_unicode_escape_to_byte`                    |   ✅   | Reader unicode escape to byte                                        |
|  19 | `test_reader_truncates_to_capacity`                     |   ✅   | Reader truncates to capacity                                         |
|  20 | `test_reader_negative_int`                              |   ✅   | Reader negative int                                                  |
|  21 | `test_writer_null_and_remaining_escapes`                |   ✅   | Writer null and remaining escapes                                    |
|  22 | `test_reader_null_guards`                               |   ✅   | Reader null guards                                                   |
|  23 | `test_reader_all_escapes`                               |   ✅   | Reader all escapes                                                   |
|  24 | `test_reader_unicode_hex_case`                          |   ✅   | Reader unicode hex case                                              |
|  25 | `test_reader_unicode_utf8_multibyte`                    |   ✅   | U+20AC EURO SIGN -> 3-byte UTF-8 E2 82 AC.                           |
|  26 | `test_reader_unicode_surrogate_edges`                   |   ✅   | Reader unicode surrogate edges                                       |
|  27 | `test_reader_false_bool`                                |   ✅   | Reader false bool                                                    |
|  28 | `test_reader_malformed`                                 |   ✅   | Reader malformed                                                     |

</details>

---

## test_auth - native_app - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for HTTP Basic Authentication (per-route)._

|   # | Test                                                   | Status | Description                                                           |
| --: | :----------------------------------------------------- | :----: | :-------------------------------------------------------------------- |
|   1 | `test_unprotected_route_fires_handler`                 |   ✅   | Unprotected route fires handler                                       |
|   2 | `test_protected_route_no_header_returns_401`           |   ✅   | Protected route no header returns 401                                 |
|   3 | `test_protected_route_wrong_password_returns_401`      |   ✅   | base64("user:wrong") = "dXNlcjp3cm9uZw=="                             |
|   4 | `test_protected_route_wrong_username_returns_401`      |   ✅   | base64("admin:pass") = "YWRtaW46cGFzcw=="                             |
|   5 | `test_protected_route_valid_credentials_fires_handler` |   ✅   | base64("user:pass") = "dXNlcjpwYXNz"                                  |
|   6 | `test_401_includes_www_authenticate_header`            |   ✅   | 401 includes www authenticate header                                  |
|   7 | `test_non_basic_scheme_returns_401`                    |   ✅   | Non basic scheme returns 401                                          |
|   8 | `test_credentials_without_colon_returns_401`           |   ✅   | base64("nocolon") = "bm9jb2xvbg=="                                    |
|   9 | `test_protected_and_unprotected_routes_coexist`        |   ✅   | Hit public route -- handler fires                                     |
|  10 | `test_auth_route_returns_404_for_wrong_path`           |   ✅   | Auth route returns 404 for wrong path                                 |
|  11 | `test_auth_checked_per_method`                         |   ✅   | Route only handles POST; a GET to that path is 405 Method Not Allowed |
|  12 | `stress_auth_50_valid_requests`                        |   ✅   | base64("u:p") = "dTpw"                                                |
|  13 | `stress_auth_50_invalid_requests`                      |   ✅   | Stress - Auth 50 invalid requests                                     |

</details>

---

## test_multipart - native_app - ✅ 25 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for multipart/form-data parser (multipart.cpp)._

|   # | Test                                             | Status | Description                                                           |
| --: | :----------------------------------------------- | :----: | :-------------------------------------------------------------------- |
|   1 | `test_no_content_type_returns_false`             |   ✅   | No content type returns false                                         |
|   2 | `test_no_boundary_in_content_type_returns_false` |   ✅   | No boundary in content type returns false                             |
|   3 | `test_body_missing_delimiter_returns_false`      |   ✅   | Body missing delimiter returns false                                  |
|   4 | `test_single_text_field_parsed`                  |   ✅   | Single text field parsed                                              |
|   5 | `test_two_text_fields_parsed`                    |   ✅   | Two text fields parsed                                                |
|   6 | `test_three_text_fields_parsed`                  |   ✅   | Three text fields parsed                                              |
|   7 | `test_file_upload_part`                          |   ✅   | File upload part                                                      |
|   8 | `test_file_upload_with_text_field`               |   ✅   | File upload with text field                                           |
|   9 | `test_get_field_found`                           |   ✅   | Get field found                                                       |
|  10 | `test_get_field_not_found_returns_null`          |   ✅   | Get field not found returns null                                      |
|  11 | `test_get_field_multiple_fields`                 |   ✅   | Get field multiple fields                                             |
|  12 | `test_data_len_is_correct`                       |   ✅   | Data len is correct                                                   |
|  13 | `test_max_parts_captured`                        |   ✅   | Build exactly MAX_MULTIPART_PARTS + 1 parts; only MAX_MULTIPART_PARTS |
|  14 | `test_empty_field_value`                         |   ✅   | Empty field value                                                     |
|  15 | `test_part_without_filename_has_null_filename`   |   ✅   | Part without filename has null filename                               |
|  16 | `test_part_without_content_type_has_null_type`   |   ✅   | Part without content type has null type                               |
|  17 | `test_long_boundary_string`                      |   ✅   | MAX_VAL_LEN=48 limits the stored Content-Type value.                  |
|  18 | `stress_parse_100_requests`                      |   ✅   | Stress - Parse 100 requests                                           |
|  19 | `stress_get_field_100_lookups`                   |   ✅   | Stress - Get field 100 lookups                                        |
|  20 | `test_binary_part_not_truncated`                 |   ✅   | Binary part not truncated                                             |
|  21 | `test_quoted_boundary`                           |   ✅   | Quoted boundary                                                       |
|  22 | `test_empty_boundary_returns_false`              |   ✅   | Empty boundary returns false                                          |
|  23 | `test_malformed_disposition_values`              |   ✅   | unquoted name= value                                                  |
|  24 | `test_body_shorter_than_delimiter`               |   ✅   | Body shorter than delimiter                                           |
|  25 | `test_truncated_part_fails_closed`               |   ✅   | Truncated part fails closed                                           |

</details>

---

## test_chunked - native_app - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for send_chunked() / ChunkedResponse streaming responses._

|   # | Test                                              | Status | Description                                |
| --: | :------------------------------------------------ | :----: | :----------------------------------------- |
|   1 | `test_chunked_source_overreport_clamped`          |   ✅   | Chunked source overreport clamped          |
|   2 | `test_chunked_backpressure_resumes_across_polls`  |   ✅   | Chunked backpressure resumes across polls  |
|   3 | `test_headers_announce_chunked_no_content_length` |   ✅   | Headers announce chunked no content length |
|   4 | `test_single_chunk_framing`                       |   ✅   | Single chunk framing                       |
|   5 | `test_multiple_chunks_in_order`                   |   ✅   | Multiple chunks in order                   |
|   6 | `test_printf_chunk`                               |   ✅   | Printf chunk                               |
|   7 | `test_single_piece_then_terminator`               |   ✅   | Single piece then terminator               |
|   8 | `test_empty_body_is_just_terminator`              |   ✅   | Empty body is just terminator              |
|   9 | `test_large_chunked_body_not_truncated`           |   ✅   | Large chunked body not truncated           |
|  10 | `test_head_sends_headers_only`                    |   ✅   | Head sends headers only                    |
|  11 | `test_custom_header_injected_into_chunked`        |   ✅   | Custom header injected into chunked        |
|  12 | `test_log_hook_reports_total_body_length`         |   ✅   | Log hook reports total body length         |
|  13 | `test_http10_falls_back_to_close_delimited`       |   ✅   | Http10 falls back to close delimited       |
|  14 | `test_http10_large_body_not_truncated`            |   ✅   | Http10 large body not truncated            |

</details>

---

## test_form_params - native_app - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for http_get_form(): application/x-www-form-urlencoded body_

|   # | Test                                   | Status | Description                     |
| --: | :------------------------------------- | :----: | :------------------------------ |
|   1 | `test_form_fields_parsed`              |   ✅   | Form fields parsed              |
|   2 | `test_form_missing_key_returns_false`  |   ✅   | Form missing key returns false  |
|   3 | `test_form_empty_value`                |   ✅   | Form empty value                |
|   4 | `test_form_wrong_content_type_ignored` |   ✅   | Form wrong content type ignored |
|   5 | `test_form_value_truncated_to_buffer`  |   ✅   | Form value truncated to buffer  |

</details>

---

## test_webdav_handler - native_webdav_handler - ✅ 29 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for the WebDAV request handler's recursive filesystem operations_

|   # | Test                                       | Status | Description                                                                        |
| --: | :----------------------------------------- | :----: | :--------------------------------------------------------------------------------- |
|   1 | `test_webdav_get_put_dest_edges`           |   ✅   | Webdav get put dest edges                                                          |
|   2 | `test_webdav_copy_dest_path_too_long_414`  |   ✅   | 240-char fs root: a short source ("/s") still joins under 256, but root + any      |
|   3 | `test_webdav_recursive_open_failure`       |   ✅   | DELETE: the resource exists but its open() fails -> dav_rm_recursive bails -> 403. |
|   4 | `test_webdav_source_path_too_long_414`     |   ✅   | Webdav source path too long 414                                                    |
|   5 | `test_webdav_dav_wildcard_and_route_full`  |   ✅   | (a) A wildcard-terminated prefix is stored as-is; a request under it still routes. |
|   6 | `test_webdav_error_paths`                  |   ✅   | Webdav error paths                                                                 |
|   7 | `test_webdav_deep_tree_rejected`           |   ✅   | Webdav deep tree rejected                                                          |
|   8 | `test_webdav_propfind_limit_and_proppatch` |   ✅   | Webdav propfind limit and proppatch                                                |
|   9 | `test_webdav_copy_fs_table_full`           |   ✅   | Webdav copy fs table full                                                          |
|  10 | `test_copy_collection_recursive`           |   ✅   | Copy collection recursive                                                          |
|  11 | `test_copy_collection_depth0_shallow`      |   ✅   | Copy collection depth0 shallow                                                     |
|  12 | `test_copy_overwrite_semantics`            |   ✅   | Copy overwrite semantics                                                           |
|  13 | `test_move_collection_recursive`           |   ✅   | Move collection recursive                                                          |
|  14 | `test_delete_collection_recursive`         |   ✅   | Delete collection recursive                                                        |
|  15 | `test_propfind_depth0_collection_only`     |   ✅   | Propfind depth0 collection only                                                    |
|  16 | `test_propfind_depth1_lists_members`       |   ✅   | Propfind depth1 lists members                                                      |
|  17 | `test_mkcol_create_and_conflict`           |   ✅   | Mkcol create and conflict                                                          |
|  18 | `test_delete_single_file`                  |   ✅   | Delete single file                                                                 |
|  19 | `test_options_advertises_dav`              |   ✅   | Options advertises dav                                                             |
|  20 | `test_get_file_through_mount`              |   ✅   | Get file through mount                                                             |
|  21 | `test_put_stream_create`                   |   ✅   | Put stream create                                                                  |
|  22 | `test_put_stream_overwrite`                |   ✅   | Put stream overwrite                                                               |
|  23 | `test_put_empty_buffered`                  |   ✅   | Put empty buffered                                                                 |
|  24 | `test_put_stream_write_fails_507`          |   ✅   | Put stream write fails 507                                                         |
|  25 | `test_put_stream_open_fails_409`           |   ✅   | Put stream open fails 409                                                          |
|  26 | `test_put_stream_traversal_403`            |   ✅   | Put stream traversal 403                                                           |
|  27 | `test_put_stream_begin_declines`           |   ✅   | Non-PUT with a body: begin sees method != PUT and declines.                        |
|  28 | `test_put_stream_abort`                    |   ✅   | Headers + a partial body: Content-Length promises 10, only 4 arrive.               |
|  29 | `test_lock_unlock_advisory`                |   ✅   | Lock unlock advisory                                                               |

</details>

---

## test_diag - native_diag - ✅ 2 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Exercises the runtime build-flag reporter (server.diag() / DWS_ENABLE_DIAG):_

|   # | Test                               | Status | Description                 |
| --: | :--------------------------------- | :----: | :-------------------------- |
|   1 | `test_diag_serves_build_info_json` |   ✅   | Diag serves build info json |
|   2 | `test_diag_json_braces_balanced`   |   ✅   | Diag json braces balanced   |

</details>

---

## test_snmp_ber - native_snmp - ✅ 21 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SNMP ASN.1 BER codec. Encodings are checked against_

|   # | Test                                                     | Status | Description                                                                     |
| --: | :------------------------------------------------------- | :----: | :------------------------------------------------------------------------------ |
|   1 | `test_integer_vectors`                                   |   ✅   | Integer vectors                                                                 |
|   2 | `test_oid_vector`                                        |   ✅   | 1.3.6.1 -> 06 03 2B 06 01                                                       |
|   3 | `test_octet_string_and_null`                             |   ✅   | Octet string and null                                                           |
|   4 | `test_counter32_keeps_unsigned`                          |   ✅   | 0x80000000 has the top bit set -> a leading 0x00 must be added.                 |
|   5 | `test_sequence_roundtrip`                                |   ✅   | Sequence roundtrip                                                              |
|   6 | `test_oid_roundtrip`                                     |   ✅   | Oid roundtrip                                                                   |
|   7 | `test_large_arc_roundtrip`                               |   ✅   | An arc > 127 exercises multi-byte base-128 encoding (e.g. enterprise 8072).     |
|   8 | `test_oid_large_first_subidentifier_roundtrip`           |   ✅   | Oid large first subidentifier roundtrip                                         |
|   9 | `test_encoder_overflow_sets_not_ok`                      |   ✅   | Encoder overflow sets not ok                                                    |
|  10 | `test_decoder_truncated_length_fails`                    |   ✅   | Claims 10 bytes of content but only 2 are present.                              |
|  11 | `test_decoder_longform_length_count_past_buffer_fails`   |   ✅   | Decoder longform length count past buffer fails                                 |
|  12 | `test_decoder_longform_length_too_wide_fails`            |   ✅   | Decoder longform length too wide fails                                          |
|  13 | `test_decoder_longform_length_content_past_buffer_fails` |   ✅   | 0x82 0x01 0x00 = long form, length 256; only a few content bytes follow.        |
|  14 | `test_decoder_longform_length_max_uint32_fails`          |   ✅   | Decoder longform length max uint32 fails                                        |
|  15 | `test_decoder_indefinite_length_fails`                   |   ✅   | Decoder indefinite length fails                                                 |
|  16 | `test_decoder_oversized_integer_fails`                   |   ✅   | Decoder oversized integer fails                                                 |
|  17 | `test_enc_len_long_form`                                 |   ✅   | A value >= 128 octets forces the long-form definite length (0x81 <len>).        |
|  18 | `test_put_oid_guards`                                    |   ✅   | Put oid guards                                                                  |
|  19 | `test_seq_end_overflow`                                  |   ✅   | A content region larger than the 16-bit back-patched length field fails closed. |
|  20 | `test_read_oid_rejects`                                  |   ✅   | dws_ber_read_oid on a non-OID TLV.                                              |
|  21 | `test_ber_skip`                                          |   ✅   | Ber skip                                                                        |

</details>

---

## test_snmp_agent - native_snmp - ✅ 28 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SNMP v1/v2c agent core (dws_snmp_agent_process). Each test_

|   # | Test                                        | Status | Description                                                                       |
| --: | :------------------------------------------ | :----: | :-------------------------------------------------------------------------------- |
|   1 | `test_registration_and_rw_edges`            |   ✅   | Registration and rw edges                                                         |
|   2 | `test_ipaddress_value_encodes`              |   ✅   | Ipaddress value encodes                                                           |
|   3 | `test_set_wrong_type_and_unknown`           |   ✅   | Set wrong type and unknown                                                        |
|   4 | `test_getbulk_variants`                     |   ✅   | non-repeaters = 1, max-repetitions = 2, one varbind at the system prefix.         |
|   5 | `test_dispatch_value_types_and_malformed`   |   ✅   | uint-typed and OID-typed varbind values decode without error.                     |
|   6 | `test_get_string_v2c`                       |   ✅   | Get string v2c                                                                    |
|   7 | `test_get_unknown_v2c_exception`            |   ✅   | Get unknown v2c exception                                                         |
|   8 | `test_get_bad_instance_v2c_nosuchinstance`  |   ✅   | Get bad instance v2c nosuchinstance                                               |
|   9 | `test_get_unknown_v1_error`                 |   ✅   | Get unknown v1 error                                                              |
|  10 | `test_getnext_walks_to_first`               |   ✅   | Getnext walks to first                                                            |
|  11 | `test_getnext_past_end_endofmibview`        |   ✅   | Getnext past end endofmibview                                                     |
|  12 | `test_set_without_rw_community_denied`      |   ✅   | Set without rw community denied                                                   |
|  13 | `test_set_with_rw_community_invokes_setter` |   ✅   | Set with rw community invokes setter                                              |
|  14 | `test_set_readonly_not_writable`            |   ✅   | Set readonly not writable                                                         |
|  15 | `test_getbulk_returns_multiple`             |   ✅   | non-repeaters=0, max-repetitions=3, one repeater starting at the system prefix.   |
|  16 | `test_dynamic_counter_value`                |   ✅   | Dynamic counter value                                                             |
|  17 | `test_uptime_is_timeticks`                  |   ✅   | Uptime is timeticks                                                               |
|  18 | `test_unknown_community_no_response`        |   ✅   | Unknown community no response                                                     |
|  19 | `test_v3_message_dropped`                   |   ✅   | V3 message dropped                                                                |
|  20 | `test_getbulk_repeaters_and_end`            |   ✅   | Pure repeaters (non_rep=0, max_rep=3) walk successive OIDs from the sys prefix.   |
|  21 | `test_getbulk_nonrep_clamp_and_v1_reject`   |   ✅   | non_rep (5) exceeds the single varbind -> clamped to the varbind count.           |
|  22 | `test_response_too_big_reencodes`           |   ✅   | Response too big reencodes                                                        |
|  23 | `test_version_and_community_guards`         |   ✅   | v3 with the USM layer not built here -> 0.                                        |
|  24 | `test_dispatch_malformed_pdu`               |   ✅   | A PDU whose header parses but whose request-id integer is truncated fails closed. |
|  25 | `test_udp_handler_via_inject`               |   ✅   | Udp handler via inject                                                            |
|  26 | `test_malformed_message_guards`             |   ✅   | Malformed message guards                                                          |
|  27 | `test_snmp_dispatch_varbind_guards`         |   ✅   | Snmp dispatch varbind guards                                                      |
|  28 | `test_snmp_oid_cmp_request_longer`          |   ✅   | Snmp oid cmp request longer                                                       |

</details>

---

## test_snmp_v3 - native_snmp_v3 - ✅ 22 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SNMPv3 USM layer. The test acts as a full SNMP manager:_

|   # | Test                                            | Status | Description                                                                     |
| --: | :---------------------------------------------- | :----: | :------------------------------------------------------------------------------ |
|   1 | `test_v3_response_scopedpdu_overflow`           |   ✅   | V3 response scopedpdu overflow                                                  |
|   2 | `test_v3_field_tag_corruption`                  |   ✅   | V3 field tag corruption                                                         |
|   3 | `test_v3_scoped_parse_rejections`               |   ✅   | V3 scoped parse rejections                                                      |
|   4 | `test_v3_discovery_malformed_scoped`            |   ✅   | V3 discovery malformed scoped                                                   |
|   5 | `test_v3_auth_edge_rejections`                  |   ✅   | V3 auth edge rejections                                                         |
|   6 | `test_v3_message_structure_rejections`          |   ✅   | V3 message structure rejections                                                 |
|   7 | `test_v3_init_and_boots_accessors`              |   ✅   | V3 init and boots accessors                                                     |
|   8 | `test_v3_discovery_variants`                    |   ✅   | V3 discovery variants                                                           |
|   9 | `test_v3_priv_not_configured`                   |   ✅   | V3 priv not configured                                                          |
|  10 | `test_v3_notify_paths`                          |   ✅   | V3 notify paths                                                                 |
|  11 | `test_v3_notify_overflow_guards`                |   ✅   | V3 notify overflow guards                                                       |
|  12 | `test_localize_key_sha256_vector`               |   ✅   | password "maplesyrup", engineID 80 00 C0 DE 05 01 02 03 04 (the agent default). |
|  13 | `test_localize_key_empty_password`              |   ✅   | Localize key empty password                                                     |
|  14 | `test_aes128_fips197_vector`                    |   ✅   | FIPS-197 C.1. CFB with IV = plaintext and zero input yields E_key(plaintext).   |
|  15 | `test_aes_cfb_roundtrip_partial_block`          |   ✅   | Aes cfb roundtrip partial block                                                 |
|  16 | `test_discovery_reports_engine_id`              |   ✅   | Discovery reports engine id                                                     |
|  17 | `test_authnopriv_get`                           |   ✅   | Authnopriv get                                                                  |
|  18 | `test_authpriv_get`                             |   ✅   | Authpriv get                                                                    |
|  19 | `test_wrong_auth_password_reports_wrong_digest` |   ✅   | Wrong auth password reports wrong digest                                        |
|  20 | `test_unknown_user_reports`                     |   ✅   | Unknown user reports                                                            |
|  21 | `test_not_in_time_window_reports`               |   ✅   | Not in time window reports                                                      |
|  22 | `test_inform_v3_builds_informrequest`           |   ✅   | Inform v3 builds informrequest                                                  |

</details>

---

## test_telnet - native_telnet - ✅ 15 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Telnet server test: drives a ConnProto::PROTO_TELNET connection through the real_

|   # | Test                                  | Status | Description                    |
| --: | :------------------------------------ | :----: | :----------------------------- |
|   1 | `test_accept_negotiates_echo_and_sga` |   ✅   | Accept negotiates echo and sga |
|   2 | `test_line_echoed_and_dispatched`     |   ✅   | Line echoed and dispatched     |
|   3 | `test_backspace_first_line`           |   ✅   | Backspace first line           |
|   4 | `test_iac_will_gets_dont`             |   ✅   | Iac will gets dont             |
|   5 | `test_iac_do_unsupported_gets_wont`   |   ✅   | Iac do unsupported gets wont   |
|   6 | `test_iac_do_echo_is_silent`          |   ✅   | Iac do echo is silent          |
|   7 | `test_iac_stripped_from_data`         |   ✅   | Iac stripped from data         |
|   8 | `test_print_broadcast`                |   ✅   | Print broadcast                |
|   9 | `test_unknown_slot_is_noop`           |   ✅   | Unknown slot is noop           |
|  10 | `test_cr_and_control_ignored`         |   ✅   | Cr and control ignored         |
|  11 | `test_iac_escaped_literal`            |   ✅   | Iac escaped literal            |
|  12 | `test_subnegotiation_consumed`        |   ✅   | Subnegotiation consumed        |
|  13 | `test_accept_no_capacity`             |   ✅   | Accept no capacity             |
|  14 | `test_output_escaping_and_printf`     |   ✅   | Output escaping and printf     |
|  15 | `test_inactive_conn_sends_nothing`    |   ✅   | Inactive conn sends nothing    |

</details>

---

## test_coap - native_coap - ✅ 44 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CoAP server core (dws_coap_server_process). Each test encodes a_

|   # | Test                                       | Status | Description                                                                         |
| --: | :----------------------------------------- | :----: | :---------------------------------------------------------------------------------- |
|   1 | `test_response_option_capacity_stop`       |   ✅   | Response option capacity stop                                                       |
|   2 | `test_coap_udp_handler_basic`              |   ✅   | Coap udp handler basic                                                              |
|   3 | `test_add_resource_limits`                 |   ✅   | Add resource limits                                                                 |
|   4 | `test_short_and_truncated_token`           |   ✅   | Short and truncated token                                                           |
|   5 | `test_malformed_options_bad_request`       |   ✅   | Malformed options bad request                                                       |
|   6 | `test_extended_delta_and_length_ignored`   |   ✅   | Extended delta and length ignored                                                   |
|   7 | `test_oversized_path_and_query`            |   ✅   | Oversized path and query                                                            |
|   8 | `test_block_option_too_wide`               |   ✅   | Block option too wide                                                               |
|   9 | `test_block1_reserved_szx`                 |   ✅   | Block1 reserved szx                                                                 |
|  10 | `test_block1_continue_no_space`            |   ✅   | Block1 continue no space                                                            |
|  11 | `test_response_payload_clamped`            |   ✅   | Response payload clamped                                                            |
|  12 | `test_response_buffer_too_small`           |   ✅   | Response buffer too small                                                           |
|  13 | `test_well_known_core_truncates`           |   ✅   | Well known core truncates                                                           |
|  14 | `test_observe_large_seq_encoding`          |   ✅   | Observe large seq encoding                                                          |
|  15 | `test_block2_explicit_paging`              |   ✅   | Block2 explicit paging                                                              |
|  16 | `test_block2_auto_when_large`              |   ✅   | Block2 auto when large                                                              |
|  17 | `test_block2_szx_clamped`                  |   ✅   | Block2 szx clamped                                                                  |
|  18 | `test_block2_absent_for_small`             |   ✅   | Block2 absent for small                                                             |
|  19 | `test_block2_out_of_range`                 |   ✅   | Block2 out of range                                                                 |
|  20 | `test_block2_reserved_szx`                 |   ✅   | Block2 reserved szx                                                                 |
|  21 | `test_block1_upload_two_blocks`            |   ✅   | Block1 upload two blocks                                                            |
|  22 | `test_block1_out_of_order`                 |   ✅   | Block1 out of order                                                                 |
|  23 | `test_block1_too_large`                    |   ✅   | Block1 too large                                                                    |
|  24 | `test_observe_option_in_response`          |   ✅   | Observe option in response                                                          |
|  25 | `test_response_option_overflows_buffer`    |   ✅   | resp holds the 4-byte header + 2-byte token (=6) but not the Content-Format option. |
|  26 | `test_no_observe_option_when_seq_negative` |   ✅   | No observe option when seq negative                                                 |
|  27 | `test_get_content`                         |   ✅   | Get content                                                                         |
|  28 | `test_not_found`                           |   ✅   | Not found                                                                           |
|  29 | `test_method_not_allowed`                  |   ✅   | Method not allowed                                                                  |
|  30 | `test_non_request_type`                    |   ✅   | Non request type                                                                    |
|  31 | `test_put_with_payload`                    |   ✅   | Put with payload                                                                    |
|  32 | `test_multi_segment_path`                  |   ✅   | Multi segment path                                                                  |
|  33 | `test_uri_query`                           |   ✅   | Uri query                                                                           |
|  34 | `test_empty_con_ping_rst`                  |   ✅   | Empty con ping rst                                                                  |
|  35 | `test_bad_version_rst`                     |   ✅   | Bad version rst                                                                     |
|  36 | `test_delete`                              |   ✅   | Delete                                                                              |
|  37 | `test_token_8_bytes`                       |   ✅   | Token 8 bytes                                                                       |
|  38 | `test_extended_option_length`              |   ✅   | Extended option length                                                              |
|  39 | `test_ack_ignored`                         |   ✅   | Ack ignored                                                                         |
|  40 | `test_root_path`                           |   ✅   | Root path                                                                           |
|  41 | `test_unknown_method_not_allowed`          |   ✅   | Code 0.05 (FETCH) is a valid class-0 code we don't implement. RFC 7252 5.8:         |
|  42 | `test_unknown_critical_option_bad_option`  |   ✅   | Hand-build: ver1/CON/TKL0, GET, MID, Uri-Path "temp", then Accept(17) - a           |
|  43 | `test_well_known_core_discovery`           |   ✅   | Well known core discovery                                                           |
|  44 | `test_well_known_core_rejects_post`        |   ✅   | Well known core rejects post                                                        |

</details>

---

## test_coap - native_coap_observe - ✅ 46 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CoAP server core (dws_coap_server_process). Each test encodes a_

|   # | Test                                       | Status | Description                                                                         |
| --: | :----------------------------------------- | :----: | :---------------------------------------------------------------------------------- |
|   1 | `test_response_option_capacity_stop`       |   ✅   | Response option capacity stop                                                       |
|   2 | `test_coap_udp_handler_basic`              |   ✅   | Coap udp handler basic                                                              |
|   3 | `test_coap_observe_over_udp`               |   ✅   | Coap observe over udp                                                               |
|   4 | `test_coap_observe_registry_full`          |   ✅   | Coap observe registry full                                                          |
|   5 | `test_add_resource_limits`                 |   ✅   | Add resource limits                                                                 |
|   6 | `test_short_and_truncated_token`           |   ✅   | Short and truncated token                                                           |
|   7 | `test_malformed_options_bad_request`       |   ✅   | Malformed options bad request                                                       |
|   8 | `test_extended_delta_and_length_ignored`   |   ✅   | Extended delta and length ignored                                                   |
|   9 | `test_oversized_path_and_query`            |   ✅   | Oversized path and query                                                            |
|  10 | `test_block_option_too_wide`               |   ✅   | Block option too wide                                                               |
|  11 | `test_block1_reserved_szx`                 |   ✅   | Block1 reserved szx                                                                 |
|  12 | `test_block1_continue_no_space`            |   ✅   | Block1 continue no space                                                            |
|  13 | `test_response_payload_clamped`            |   ✅   | Response payload clamped                                                            |
|  14 | `test_response_buffer_too_small`           |   ✅   | Response buffer too small                                                           |
|  15 | `test_well_known_core_truncates`           |   ✅   | Well known core truncates                                                           |
|  16 | `test_observe_large_seq_encoding`          |   ✅   | Observe large seq encoding                                                          |
|  17 | `test_block2_explicit_paging`              |   ✅   | Block2 explicit paging                                                              |
|  18 | `test_block2_auto_when_large`              |   ✅   | Block2 auto when large                                                              |
|  19 | `test_block2_szx_clamped`                  |   ✅   | Block2 szx clamped                                                                  |
|  20 | `test_block2_absent_for_small`             |   ✅   | Block2 absent for small                                                             |
|  21 | `test_block2_out_of_range`                 |   ✅   | Block2 out of range                                                                 |
|  22 | `test_block2_reserved_szx`                 |   ✅   | Block2 reserved szx                                                                 |
|  23 | `test_block1_upload_two_blocks`            |   ✅   | Block1 upload two blocks                                                            |
|  24 | `test_block1_out_of_order`                 |   ✅   | Block1 out of order                                                                 |
|  25 | `test_block1_too_large`                    |   ✅   | Block1 too large                                                                    |
|  26 | `test_observe_option_in_response`          |   ✅   | Observe option in response                                                          |
|  27 | `test_response_option_overflows_buffer`    |   ✅   | resp holds the 4-byte header + 2-byte token (=6) but not the Content-Format option. |
|  28 | `test_no_observe_option_when_seq_negative` |   ✅   | No observe option when seq negative                                                 |
|  29 | `test_get_content`                         |   ✅   | Get content                                                                         |
|  30 | `test_not_found`                           |   ✅   | Not found                                                                           |
|  31 | `test_method_not_allowed`                  |   ✅   | Method not allowed                                                                  |
|  32 | `test_non_request_type`                    |   ✅   | Non request type                                                                    |
|  33 | `test_put_with_payload`                    |   ✅   | Put with payload                                                                    |
|  34 | `test_multi_segment_path`                  |   ✅   | Multi segment path                                                                  |
|  35 | `test_uri_query`                           |   ✅   | Uri query                                                                           |
|  36 | `test_empty_con_ping_rst`                  |   ✅   | Empty con ping rst                                                                  |
|  37 | `test_bad_version_rst`                     |   ✅   | Bad version rst                                                                     |
|  38 | `test_delete`                              |   ✅   | Delete                                                                              |
|  39 | `test_token_8_bytes`                       |   ✅   | Token 8 bytes                                                                       |
|  40 | `test_extended_option_length`              |   ✅   | Extended option length                                                              |
|  41 | `test_ack_ignored`                         |   ✅   | Ack ignored                                                                         |
|  42 | `test_root_path`                           |   ✅   | Root path                                                                           |
|  43 | `test_unknown_method_not_allowed`          |   ✅   | Code 0.05 (FETCH) is a valid class-0 code we don't implement. RFC 7252 5.8:         |
|  44 | `test_unknown_critical_option_bad_option`  |   ✅   | Hand-build: ver1/CON/TKL0, GET, MID, Uri-Path "temp", then Accept(17) - a           |
|  45 | `test_well_known_core_discovery`           |   ✅   | Well known core discovery                                                           |
|  46 | `test_well_known_core_rejects_post`        |   ✅   | Well known core rejects post                                                        |

</details>

---

## test_webdav - native_webdav - ✅ 25 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the WebDAV server core (services/webdav): method classification,_

|   # | Test                                      | Status | Description                                                                  |
| --: | :---------------------------------------- | :----: | :--------------------------------------------------------------------------- |
|   1 | `test_method_classification`              |   ✅   | Method classification                                                        |
|   2 | `test_webdav_builder_guards`              |   ✅   | Webdav builder guards                                                        |
|   3 | `test_depth_parsing`                      |   ✅   | Depth parsing                                                                |
|   4 | `test_xml_escape`                         |   ✅   | Xml escape                                                                   |
|   5 | `test_xml_escape_truncates_safely`        |   ✅   | Xml escape truncates safely                                                  |
|   6 | `test_dest_absolute_uri`                  |   ✅   | Dest absolute uri                                                            |
|   7 | `test_dest_percent_decoded`               |   ✅   | Dest percent decoded                                                         |
|   8 | `test_dest_abs_path`                      |   ✅   | Dest abs path                                                                |
|   9 | `test_dest_rejects_malformed`             |   ✅   | Dest rejects malformed                                                       |
|  10 | `test_multistatus_file_and_collection`    |   ✅   | Multistatus file and collection                                              |
|  11 | `test_multistatus_escapes_href`           |   ✅   | Multistatus escapes href                                                     |
|  12 | `test_multistatus_entry_stops_when_full`  |   ✅   | Multistatus entry stops when full                                            |
|  13 | `test_proppatch_windows_timestamp`        |   ✅   | The PROPPATCH macOS Finder / Windows Explorer send after a PUT.              |
|  14 | `test_proppatch_multiple_and_self_closed` |   ✅   | Proppatch multiple and self closed                                           |
|  15 | `test_proppatch_remove_block`             |   ✅   | Proppatch remove block                                                       |
|  16 | `test_proppatch_escapes_href`             |   ✅   | Proppatch escapes href                                                       |
|  17 | `test_proppatch_empty_body_is_valid`      |   ✅   | Proppatch empty body is valid                                                |
|  18 | `test_proppatch_rejects_injection`        |   ✅   | A property tag carrying a stray '<' must not be echoed (no XML injection).   |
|  19 | `test_proppatch_fuzz_bounded`             |   ✅   | Throw random and partial-XML bytes at the scanner: it must always stay in    |
|  20 | `test_proppatch_stops_when_full`          |   ✅   | Proppatch stops when full                                                    |
|  21 | `test_method_all_including_head`          |   ✅   | Method all including head                                                    |
|  22 | `test_depth_and_dest_path_guards`         |   ✅   | Depth and dest path guards                                                   |
|  23 | `test_ms_entry_content_type_overflow`     |   ✅   | Ms entry content type overflow                                               |
|  24 | `test_ms_entry_mtime_and_tiny_buf`        |   ✅   | Ms entry mtime and tiny buf                                                  |
|  25 | `test_proppatch_ms_echo`                  |   ✅   | A self-closed property with trailing whitespace exercises the open-tag trim. |

</details>

---

## test_modbus - native_modbus - ✅ 23 passed

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
|  23 | `test_server_init_bounds_and_handler`       |   ✅   | Server init bounds and handler                       |

</details>

---

## test_cloudevents - native_cloudevents - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CloudEvents v1.0 envelope (services/cloudevents): the_

|   # | Test                                   | Status | Description                     |
| --: | :------------------------------------- | :----: | :------------------------------ |
|   1 | `test_build_minimal`                   |   ✅   | Build minimal                   |
|   2 | `test_build_requires_id_source_type`   |   ✅   | Build requires id source type   |
|   3 | `test_build_with_json_data`            |   ✅   | Build with json data            |
|   4 | `test_build_with_string_data`          |   ✅   | Build with string data          |
|   5 | `test_build_overflow_fails_closed`     |   ✅   | Build overflow fails closed     |
|   6 | `test_from_headers_binary_mode`        |   ✅   | From headers binary mode        |
|   7 | `test_from_headers_missing_required`   |   ✅   | From headers missing required   |
|   8 | `test_guards_and_datacontenttype_only` |   ✅   | Guards and datacontenttype only |

</details>

---

## test_redis_resp - native_redis - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Redis RESP2 codec (services/redis_resp): the command encoder_

|   # | Test                                       | Status | Description                         |
| --: | :----------------------------------------- | :----: | :---------------------------------- |
|   1 | `test_encode_command`                      |   ✅   | Encode command                      |
|   2 | `test_encode_binary_safe`                  |   ✅   | Encode binary safe                  |
|   3 | `test_encode_overflow_fails_closed`        |   ✅   | Encode overflow fails closed        |
|   4 | `test_parse_simple_and_error`              |   ✅   | Parse simple and error              |
|   5 | `test_parse_integer`                       |   ✅   | Parse integer                       |
|   6 | `test_parse_bulk_and_nil`                  |   ✅   | Parse bulk and nil                  |
|   7 | `test_parse_array_cursor`                  |   ✅   | Parse array cursor                  |
|   8 | `test_parse_incomplete_and_malformed`      |   ✅   | Parse incomplete and malformed      |
|   9 | `test_encode_guard_subconditions`          |   ✅   | Encode guard subconditions          |
|  10 | `test_parse_guard_subconditions_and_edges` |   ✅   | Parse guard subconditions and edges |
|  11 | `test_parse_resp3_null_bool`               |   ✅   | Parse resp3 null bool               |
|  12 | `test_parse_resp3_double`                  |   ✅   | Parse resp3 double                  |
|  13 | `test_parse_resp3_bignum_bulkerr_verbatim` |   ✅   | Parse resp3 bignum bulkerr verbatim |
|  14 | `test_parse_resp3_map_set_push`            |   ✅   | Parse resp3 map set push            |

</details>

---

## test_sqlite - native_sqlite - ✅ 23 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/sqlite: the SQLite3 on-disk file-format parsers. The page-1 vector below is the_

|   # | Test                                             | Status | Description                                                                                           |
| --: | :----------------------------------------------- | :----: | :---------------------------------------------------------------------------------------------------- |
|   1 | `test_db_header_real_file`                       |   ✅   | Db header real file                                                                                   |
|   2 | `test_db_header_rejects_bad_magic`               |   ✅   | Db header rejects bad magic                                                                           |
|   3 | `test_btree_header_real_page1`                   |   ✅   | Page 1's b-tree header follows the 100-byte database header.                                          |
|   4 | `test_btree_header_rejects_bad_type`             |   ✅   | Btree header rejects bad type                                                                         |
|   5 | `test_first_cell_varints`                        |   ✅   | The single cell pointer lives right after the 8-byte leaf header (offset 108), big-endian u16.        |
|   6 | `test_varint_spec_vectors`                       |   ✅   | Varint spec vectors                                                                                   |
|   7 | `test_serial_type_sizes`                         |   ✅   | Serial type sizes                                                                                     |
|   8 | `test_read_schema_row`                           |   ✅   | Read schema row                                                                                       |
|   9 | `test_column_int_signextend`                     |   ✅   | Column int signextend                                                                                 |
|  10 | `test_leaf_cell_overflow_detection`              |   ✅   | Leaf cell overflow detection                                                                          |
|  11 | `test_table_cursor_multipage`                    |   ✅   | The table's root page (page 2) is an interior table page, so this exercises the descent stack.        |
|  12 | `test_overflow_read_payload`                     |   ✅   | Overflow read payload                                                                                 |
|  13 | `test_read_payload_nonoverflow`                  |   ✅   | Read payload nonoverflow                                                                              |
|  14 | `test_read_payload_bad_overflow_pointer`         |   ✅   | Read payload bad overflow pointer                                                                     |
|  15 | `test_overflow_read_payload_bounds`              |   ✅   | Overflow read payload bounds                                                                          |
|  16 | `test_overflow_cursor`                           |   ✅   | Overflow cursor                                                                                       |
|  17 | `test_varint_encode_roundtrip`                   |   ✅   | Varint encode roundtrip                                                                               |
|  18 | `test_encode_record_roundtrip`                   |   ✅   | A row of (INT, TEXT, FLOAT, NULL, INT=0) round-trips through the record reader.                       |
|  19 | `test_build_table_db_roundtrip`                  |   ✅   | Build a real 2-page DB, then read it back with our own reader.                                        |
|  20 | `test_encode_record_int_widths`                  |   ✅   | Every integer serial type: the value round-trips and the encoder picks the minimal type.              |
|  21 | `test_encode_record_blob`                        |   ✅   | A BLOB column (serial type 12 + 2n) round-trips its raw bytes, including embedded NULs.               |
|  22 | `test_build_table_db_page_overflow_fails_closed` |   ✅   | Many rows that each fit but collectively exceed one leaf page must fail closed (distinct from the     |
|  23 | `test_build_table_db_fails_closed`               |   ✅   | A single row larger than one leaf page can hold must fail closed (bounded writer, no overflow pages). |

</details>

---

## test_stomp - native_stomp - ✅ 14 passed

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

## test_mqtt_sn - native_mqtt_sn - ✅ 13 passed

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

## test_flow_export - native_flow_export - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the flow-export codec (services/flow_export): NetFlow v5 fixed records,_

|   # | Test                                | Status | Description                           |
| --: | :---------------------------------- | :----: | :------------------------------------ |
|   1 | `test_v5_header_bytes`              |   ✅   | V5 header bytes                       |
|   2 | `test_v5_record_bytes`              |   ✅   | V5 record bytes                       |
|   3 | `test_v5_overflow_fails_closed`     |   ✅   | V5 overflow fails closed              |
|   4 | `test_ipfix_message_bytes`          |   ✅   | Ipfix message bytes                   |
|   5 | `test_v9_count_and_padding`         |   ✅   | V9 count and padding                  |
|   6 | `test_finish_overflow_fails_closed` |   ✅   | Finish overflow fails closed          |
|   7 | `test_v5_write_overflow`            |   ✅   | V5 write overflow                     |
|   8 | `test_flow_guards_and_overflows`    |   ✅   | begin null-arg guards + finish(null). |

</details>

---

## test_protobuf - native_protobuf - ✅ 13 passed

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

## test_preempt_queue - native_preempt_queue - ✅ 12 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the preempting work queue (services/preempt_queue) host core: the_

|   # | Test                                       | Status | Description                                                                 |
| --: | :----------------------------------------- | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_start_validates_and_runs`            |   ✅   | Start validates and runs                                                    |
|   2 | `test_fifo_order`                          |   ✅   | Fifo order                                                                  |
|   3 | `test_urgent_goes_to_front`                |   ✅   | Urgent goes to front                                                        |
|   4 | `test_fail_closed_when_full`               |   ✅   | The test env sizes DWS_PQ_DEPTH = 4.                                        |
|   5 | `test_high_water_tracks_peak`              |   ✅   | High water tracks peak                                                      |
|   6 | `test_from_isr_enqueues`                   |   ✅   | From isr enqueues                                                           |
|   7 | `test_drain_empties_and_reuses`            |   ✅   | Drain empties and reuses                                                    |
|   8 | `test_internal_lanes_outrank_user`         |   ✅   | DMA highest, then forward, then device, all above the user lane.            |
|   9 | `test_lanes_are_isolated`                  |   ✅   | The USER lane is already started by setUp; start the internal DMA lane too. |
|  10 | `test_lane_start_stop_running_independent` |   ✅   | Lane start stop running independent                                         |
|  11 | `test_lane_high_water_is_per_lane`         |   ✅   | Lane high water is per lane                                                 |
|  12 | `test_lane_api_urgent_and_drain`           |   ✅   | Lane api urgent and drain                                                   |

</details>

---

## test_dma - native_dma - ✅ 12 passed

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
|  12 | `test_channel_guard_subconditions`     |   ✅   | Channel guard subconditions     |

</details>

---

## test_forward - native_forward - ✅ 26 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the interface forwarding plane (services/forward): default-deny, an_

|   # | Test                                          | Status | Description                            |
| --: | :-------------------------------------------- | :----: | :------------------------------------- |
|   1 | `test_default_deny`                           |   ✅   | Default deny                           |
|   2 | `test_allow_forwards`                         |   ✅   | Allow forwards                         |
|   3 | `test_no_self_forward`                        |   ✅   | No self forward                        |
|   4 | `test_deny_wins_over_allow`                   |   ✅   | Deny wins over allow                   |
|   5 | `test_multi_destination_fanout`               |   ✅   | Multi destination fanout               |
|   6 | `test_rate_cap_drops_then_reopens`            |   ✅   | Rate cap drops then reopens            |
|   7 | `test_send_failure_counted`                   |   ✅   | Send failure counted                   |
|   8 | `test_add_if_validation_and_table_full`       |   ✅   | Add if validation and table full       |
|   9 | `test_add_rule_table_full`                    |   ✅   | Add rule table full                    |
|  10 | `test_unregistered_destination_is_inert`      |   ✅   | Unregistered destination is inert      |
|  11 | `test_acl_deny_by_byte_pattern`               |   ✅   | Acl deny by byte pattern               |
|  12 | `test_acl_allowlist_default_deny`             |   ✅   | Acl allowlist default deny             |
|  13 | `test_acl_first_match_wins`                   |   ✅   | Acl first match wins                   |
|  14 | `test_acl_src_any_content_wildcard`           |   ✅   | Acl src any content wildcard           |
|  15 | `test_acl_short_frame_skips_entry`            |   ✅   | Acl short frame skips entry            |
|  16 | `test_acl_add_validation_and_table_full`      |   ✅   | Acl add validation and table full      |
|  17 | `test_route_selects_egress_and_falls_through` |   ✅   | Route selects egress and falls through |
|  18 | `test_route_never_reflects_to_source`         |   ✅   | Route never reflects to source         |
|  19 | `test_route_unregistered_egress_fail_closed`  |   ✅   | Route unregistered egress fail closed  |
|  20 | `test_route_rate_cap`                         |   ✅   | Route rate cap                         |
|  21 | `test_route_default_any_content`              |   ✅   | Route default any content              |
|  22 | `test_route_first_match_wins`                 |   ✅   | Route first match wins                 |
|  23 | `test_route_add_validation_and_table_full`    |   ✅   | Route add validation and table full    |
|  24 | `test_inspect_pass_and_drop`                  |   ✅   | Inspect pass and drop                  |
|  25 | `test_inspect_runs_after_acl`                 |   ✅   | Inspect runs after acl                 |
|  26 | `test_inspect_cleared_by_null`                |   ✅   | Inspect cleared by null                |

</details>

---

## test_gateway - native_gateway - ✅ 12 passed

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
|  12 | `test_topic_zero_and_overflow_steps`        |   ✅   | Topic zero and overflow steps        |

</details>

---

## test_lora - native_lora - ✅ 14 passed

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
|  14 | `test_frame_parse_build_guards`                |   ✅   | Frame parse build guards                |

</details>

---

## test_nrf24 - native_nrf24 - ✅ 11 passed

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
|  11 | `test_data_rate_variants`             |   ✅   | Data rate variants             |

</details>

---

## test_enocean - native_enocean - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the EnOcean ESP3 codec (services/enocean): the CRC-8 (poly 0x07) against_

|   # | Test                                   | Status | Description                                                                     |
| --: | :------------------------------------- | :----: | :------------------------------------------------------------------------------ |
|   1 | `test_crc8_known_answers`              |   ✅   | Crc8 known answers                                                              |
|   2 | `test_build_then_parse_round_trip`     |   ✅   | Build then parse round trip                                                     |
|   3 | `test_parse_rejects_bad_sync`          |   ✅   | Parse rejects bad sync                                                          |
|   4 | `test_parse_rejects_bad_header_crc`    |   ✅   | Parse rejects bad header crc                                                    |
|   5 | `test_parse_rejects_bad_data_crc`      |   ✅   | Parse rejects bad data crc                                                      |
|   6 | `test_parse_needs_more_bytes`          |   ✅   | Parse needs more bytes                                                          |
|   7 | `test_parse_rejects_over_length`       |   ✅   | A header claiming data_len 100 (> DWS_ENOCEAN_MAX_DATA = 16) is rejected early. |
|   8 | `test_parse_resynchronises_after_junk` |   ✅   | Parse resynchronises after junk                                                 |
|   9 | `test_build_bounds`                    |   ✅   | Build bounds                                                                    |
|  10 | `test_esp3_parse_null_guard`           |   ✅   | Esp3 parse null guard                                                           |

</details>

---

## test_pn532 - native_pn532 - ✅ 11 passed

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
|   8 | `test_parse_rejects_over_length`             |   ✅   | frame_len 20 (> DWS_PN532_MAX_DATA + 1 = 9) is rejected early.           |
|   9 | `test_ack_frame`                             |   ✅   | Ack frame                                                                |
|  10 | `test_build_bounds`                          |   ✅   | Build bounds                                                             |
|  11 | `test_frame_parse_and_ack_guards`            |   ✅   | Frame parse and ack guards                                               |

</details>

---

## test_sigfox - native_sigfox - ✅ 7 passed

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

## test_zwave - native_zwave - ✅ 9 passed

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
|   7 | `test_parse_rejects_over_length`   |   ✅   | frame_len 80 (> DWS_ZWAVE_MAX_DATA + 3 = 19) is rejected early.                      |
|   8 | `test_control_bytes`               |   ✅   | Control bytes                                                                        |
|   9 | `test_build_bounds`                |   ✅   | Build bounds                                                                         |

</details>

---

## test_zigbee - native_zigbee - ✅ 10 passed

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
|  10 | `test_encode_decode_guards`                |   ✅   | Encode decode guards                                                                 |

</details>

---

## test_thread - native_thread - ✅ 15 passed

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
|  14 | `test_spinel_guards`                         |   ✅   | Spinel guards                                                                             |
|  15 | `test_thread_more_guards`                    |   ✅   | pack/unpack null-pointer guards.                                                          |

</details>

---

## test_wamp - native_wamp - ✅ 15 passed

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
|  13 | `test_builder_null_guards`               |   ✅   | Builder null guards               |
|  14 | `test_emit_uint_zero_and_no_args`        |   ✅   | Emit uint zero and no args        |
|  15 | `test_parser_error_paths`                |   ✅   | Parser error paths                |

</details>

---

## test_sunspec - native_sunspec - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SunSpec Modbus codec (services/sunspec): the map writer, the_

|   # | Test                                 | Status | Description                   |
| --: | :----------------------------------- | :----: | :---------------------------- |
|   1 | `test_build_and_walk`                |   ✅   | Build and walk                |
|   2 | `test_two_models`                    |   ✅   | Two models                    |
|   3 | `test_string_point`                  |   ✅   | String point                  |
|   4 | `test_marker_and_truncation`         |   ✅   | Marker and truncation         |
|   5 | `test_writer_overflow_fails_closed`  |   ✅   | Writer overflow fails closed  |
|   6 | `test_reader_guards_and_i32`         |   ✅   | Reader guards and i32         |
|   7 | `test_writer_error_and_string_paths` |   ✅   | Writer error and string paths |

</details>

---

## test_c37118 - native_c37118 - ✅ 6 passed

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

## test_dnp3 - native_dnp3 - ✅ 8 passed

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

## test_grpcweb - native_grpcweb - ✅ 9 passed

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
|   8 | `test_frame_and_trailer_guards`    |   ✅   | Frame and trailer guards    |
|   9 | `test_trailer_status_parse_paths`  |   ✅   | Trailer status parse paths  |

</details>

---

## test_lwm2m_tlv - native_lwm2m_tlv - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the OMA LwM2M TLV codec (services/lwm2m): the writer (raw + typed value_

|   # | Test                            | Status | Description                                                                      |
| --: | :------------------------------ | :----: | :------------------------------------------------------------------------------- |
|   1 | `test_write_int_1byte`          |   ✅   | Write int 1byte                                                                  |
|   2 | `test_write_int_2byte`          |   ✅   | Write int 2byte                                                                  |
|   3 | `test_write_string_8bit_length` |   ✅   | Write string 8bit length                                                         |
|   4 | `test_write_16bit_id`           |   ✅   | Write 16bit id                                                                   |
|   5 | `test_round_trip_and_value_int` |   ✅   | Round trip and value int                                                         |
|   6 | `test_object_instance_nested`   |   ✅   | Object instance nested                                                           |
|   7 | `test_write_16bit_length`       |   ✅   | Write 16bit length                                                               |
|   8 | `test_read_24bit_length`        |   ✅   | Read 24bit length                                                                |
|   9 | `test_value_int_4_and_8_byte`   |   ✅   | Value int 4 and 8 byte                                                           |
|  10 | `test_zero_length_value`        |   ✅   | Zero length value                                                                |
|  11 | `test_overflow_and_malformed`   |   ✅   | Overflow and malformed                                                           |
|  12 | `test_write_error_paths`        |   ✅   | Write error paths                                                                |
|  13 | `test_write_float_roundtrip`    |   ✅   | Write float roundtrip                                                            |
|  14 | `test_read_id16_and_truncation` |   ✅   | 16-bit-id resource: type 0xE1 (id16 flag + inline len 1), id 0x0405, value 0x07. |

</details>

---

## test_fins - native_fins - ✅ 6 passed

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

## test_hostlink - native_hostlink - ✅ 8 passed

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
|   8 | `test_guards_and_hex`              |   ✅   | build guards                                                                      |

</details>

---

## test_scpi - native_scpi - ✅ 24 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SCPI / IEEE 488.2 instrument-control codec (services/scpi): the command_

|   # | Test                             | Status | Description                                             |
| --: | :------------------------------- | :----: | :------------------------------------------------------ |
|   1 | `test_common_commands`           |   ✅   | Common commands                                         |
|   2 | `test_build_no_args`             |   ✅   | Build no args                                           |
|   3 | `test_build_one_arg`             |   ✅   | Build one arg                                           |
|   4 | `test_build_multi_arg`           |   ✅   | Build multi arg                                         |
|   5 | `test_build_overflow_and_guards` |   ✅   | header alone longer than the buffer                     |
|   6 | `test_fmt_real`                  |   ✅   | Fmt real                                                |
|   7 | `test_parse_number`              |   ✅   | Parse number                                            |
|   8 | `test_parse_number_rejects`      |   ✅   | Parse number rejects                                    |
|   9 | `test_parse_bool`                |   ✅   | Parse bool                                              |
|  10 | `test_parse_string`              |   ✅   | Parse string                                            |
|  11 | `test_parse_block_definite`      |   ✅   | Parse block definite                                    |
|  12 | `test_parse_block_indefinite`    |   ✅   | Parse block indefinite                                  |
|  13 | `test_parse_block_rejects`       |   ✅   | truncated definite block (says 4 bytes, only 3 present) |
|  14 | `test_status_error_queue_fifo`   |   ✅   | Status error queue fifo                                 |
|  15 | `test_status_esr_class_bits`     |   ✅   | Status esr class bits                                   |
|  16 | `test_status_stb_and_mss`        |   ✅   | Status stb and mss                                      |
|  17 | `test_status_cls`                |   ✅   | Status cls                                              |
|  18 | `test_status_queue_overflow`     |   ✅   | Status queue overflow                                   |
|  19 | `test_std_error_lookup`          |   ✅   | Std error lookup                                        |
|  20 | `test_match_short_long_form`     |   ✅   | Match short long form                                   |
|  21 | `test_match_query_suffix`        |   ✅   | Match query suffix                                      |
|  22 | `test_match_numeric_suffix`      |   ✅   | Match numeric suffix                                    |
|  23 | `test_match_common_and_root`     |   ✅   | Match common and root                                   |
|  24 | `test_match_negatives`           |   ✅   | Match negatives                                         |

</details>

---

## test_hislip - native_hislip - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the HiSLIP (IVI-6.1) message codec (services/hislip): the fixed 16-byte header_

|   # | Test                           | Status | Description                                                    |
| --: | :----------------------------- | :----: | :------------------------------------------------------------- |
|   1 | `test_header_roundtrip`        |   ✅   | Header roundtrip                                               |
|   2 | `test_header_rejects`          |   ✅   | Header rejects                                                 |
|   3 | `test_message_type_codes`      |   ✅   | Message type codes                                             |
|   4 | `test_build_initialize_vector` |   ✅   | Build initialize vector                                        |
|   5 | `test_parse_initialize`        |   ✅   | Parse initialize                                               |
|   6 | `test_initialize_response`     |   ✅   | Initialize response                                            |
|   7 | `test_async_initialize`        |   ✅   | Async initialize                                               |
|   8 | `test_build_dataend_vector`    |   ✅   | Build dataend vector                                           |
|   9 | `test_data_roundtrip`          |   ✅   | Data roundtrip                                                 |
|  10 | `test_message_id_increment`    |   ✅   | Message id increment                                           |
|  11 | `test_build_overflow`          |   ✅   | a 6-byte payload needs 22 bytes; a 20-byte buffer fails closed |

</details>

---

## test_vxi11 - native_vxi11 - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the VXI-11 codec over ONC RPC / XDR (services/vxi11): the record-marking header,_

|   # | Test                       | Status | Description                                                                                     |
| --: | :------------------------- | :----: | :---------------------------------------------------------------------------------------------- |
|   1 | `test_record_mark`         |   ✅   | Record mark                                                                                     |
|   2 | `test_create_link_vector`  |   ✅   | Create link vector                                                                              |
|   3 | `test_create_link_reply`   |   ✅   | Create link reply                                                                               |
|   4 | `test_getport`             |   ✅   | Getport                                                                                         |
|   5 | `test_device_write`        |   ✅   | header(40) + record-mark(4) + lid,io,lock,flags (16) + opaque(len 4 + 6 data + 2 pad = 12) = 72 |
|   6 | `test_device_read`         |   ✅   | Device read                                                                                     |
|   7 | `test_readstb_and_destroy` |   ✅   | Readstb and destroy                                                                             |
|   8 | `test_reply_rejects`       |   ✅   | MSG_DENIED (reply_stat = 1)                                                                     |
|   9 | `test_error_str`           |   ✅   | Error str                                                                                       |
|  10 | `test_build_overflow`      |   ✅   | Build overflow                                                                                  |

</details>

---

## test_gpib - native_gpib - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the GPIB-over-LAN (Prologix-style) command codec (services/gpib): the ++ command_

|   # | Test                       | Status | Description                                                                               |
| --: | :------------------------- | :----: | :---------------------------------------------------------------------------------------- |
|   1 | `test_command_generic`     |   ✅   | Command generic                                                                           |
|   2 | `test_addr`                |   ✅   | Addr                                                                                      |
|   3 | `test_read`                |   ✅   | Read                                                                                      |
|   4 | `test_spoll_and_eos`       |   ✅   | Spoll and eos                                                                             |
|   5 | `test_build_data_escaping` |   ✅   | Manual §8.1: 00 01 02 13 03 10 04 27 05 43 06 -> escape CR/LF/ESC/'+' with a leading ESC. |
|   6 | `test_build_data_plain`    |   ✅   | a plain SCPI command has no special bytes -> passthrough + newline                        |
|   7 | `test_is_command`          |   ✅   | Is command                                                                                |
|   8 | `test_parse_decimal`       |   ✅   | Parse decimal                                                                             |
|   9 | `test_parse_addr`          |   ✅   | Parse addr                                                                                |
|  10 | `test_parse_version`       |   ✅   | Parse version                                                                             |

</details>

---

## test_haas_mdc - native_haas_mdc - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Haas Machine Data Collection (MDC) Q-command codec (services/haas_mdc): the ?Q_

|   # | Test                          | Status | Description                                                               |
| --: | :---------------------------- | :----: | :------------------------------------------------------------------------ |
|   1 | `test_build_q`                |   ✅   | Build q                                                                   |
|   2 | `test_build_var`              |   ✅   | Build var                                                                 |
|   3 | `test_parse_simple_and_value` |   ✅   | Q100 -> serial number                                                     |
|   4 | `test_parse_status_idle`      |   ✅   | Parse status idle                                                         |
|   5 | `test_parse_status_busy`      |   ✅   | Parse status busy                                                         |
|   6 | `test_parse_macro`            |   ✅   | documented 6-decimal form                                                 |
|   7 | `test_error_and_no_frame`     |   ✅   | Error and no frame                                                        |
|   8 | `test_leading_prompt`         |   ✅   | previous response's trailing '>' prompt precedes this frame in the stream |
|   9 | `test_field_access`           |   ✅   | Field access                                                              |
|  10 | `test_dprnt`                  |   ✅   | a pushed DPRNT line: raw text + CRLF, no STX/ETB                          |

</details>

---

## test_lsv2 - native_lsv2 - ✅ 12 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Heidenhain LSV/2 telegram codec (services/lsv2): the framer (4-byte big-endian_

|   # | Test                      | Status | Description                                                          |
| --: | :------------------------ | :----: | :------------------------------------------------------------------- |
|   1 | `test_build_no_payload`   |   ✅   | R_ST with no payload -> exactly 8 bytes: 00 00 00 00 'R' '_' 'S' 'T' |
|   2 | `test_build_with_payload` |   ✅   | Build with payload                                                   |
|   3 | `test_build_run_info`     |   ✅   | Build run info                                                       |
|   4 | `test_build_login`        |   ✅   | login "INSPECT", no password -> payload "INSPECT\0" (8 bytes)        |
|   5 | `test_build_logout`       |   ✅   | no login -> log out of everything -> empty payload, 8 bytes          |
|   6 | `test_build_filename`     |   ✅   | R_FL "PGM.H" -> payload "PGM.H\0" (6 bytes)                          |
|   7 | `test_parse_ok`           |   ✅   | Parse ok                                                             |
|   8 | `test_parse_error`        |   ✅   | T_ER with a 2-byte error-class + error-code payload                  |
|   9 | `test_parse_data_reply`   |   ✅   | S_RI run-info reply carrying 3 payload bytes                         |
|  10 | `test_parse_incomplete`   |   ✅   | fewer than 8 header bytes -> false, and out is cleared               |
|  11 | `test_parse_stream_multi` |   ✅   | two telegrams back-to-back: T_OK then S_RI(2 bytes)                  |
|  12 | `test_roundtrip`          |   ✅   | build then parse: run-info request survives a frame/parse round trip |

</details>

---

## test_ikev2 - native_ikev2 - ✅ 16 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the IKEv2 (RFC 7296) message + payload codec (services/ikev2): the 28-octet header, the_

|   # | Test                      | Status | Description                                                                               |
| --: | :------------------------ | :----: | :---------------------------------------------------------------------------------------- |
|   1 | `test_hdr_build`          |   ✅   | Hdr build                                                                                 |
|   2 | `test_hdr_parse`          |   ✅   | Hdr parse                                                                                 |
|   3 | `test_hdr_set_length`     |   ✅   | Hdr set length                                                                            |
|   4 | `test_ke`                 |   ✅   | Ke                                                                                        |
|   5 | `test_nonce`              |   ✅   | Nonce                                                                                     |
|   6 | `test_notify`             |   ✅   | Notify                                                                                    |
|   7 | `test_delete`             |   ✅   | Delete                                                                                    |
|   8 | `test_sa_build_no_keylen` |   ✅   | Sa build no keylen                                                                        |
|   9 | `test_sa_build_keylen`    |   ✅   | Sa build keylen                                                                           |
|  10 | `test_sa_parse`           |   ✅   | parse the SA body (proposal area, after the 4-byte generic header) from the keylen vector |
|  11 | `test_id_auth`            |   ✅   | Id auth                                                                                   |
|  12 | `test_ts`                 |   ✅   | generic(4) + num/res(4) + selector(8 + 2*4) = 24                                          |
|  13 | `test_sk_frame`           |   ✅   | Sk frame                                                                                  |
|  14 | `test_full_build`         |   ✅   | Full build                                                                                |
|  15 | `test_full_chain_walk`    |   ✅   | Full chain walk                                                                           |
|  16 | `test_parse_malformed`    |   ✅   | a payload claiming length 3 (< 4) is rejected                                             |

</details>

---

## test_senml - native_senml - ✅ 9 passed

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

## test_df1 - native_df1 - ✅ 10 passed

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
|  10 | `test_parse_edges_and_guards`      |   ✅   | build guards                         |

</details>

---

## test_cotp - native_cotp - ✅ 7 passed

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
|   7 | `test_guards_and_types`   |   ✅   | Guards and types                             |

</details>

---

## test_s7comm - native_s7comm - ✅ 9 passed

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
|   9 | `test_null_and_short_guards`       |   ✅   | Null and short guards       |

</details>

---

## test_melsec - native_melsec - ✅ 7 passed

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
|   7 | `test_parse_guards`                |   ✅   | Parse guards                |

</details>

---

## test_ads - native_ads - ✅ 17 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Beckhoff ADS / AMS codec (services/ads): the request builders and the_

|   # | Test                                      | Status | Description                        |
| --: | :---------------------------------------- | :----: | :--------------------------------- |
|   1 | `test_build_read_bytes`                   |   ✅   | Build read bytes                   |
|   2 | `test_parse_read_response`                |   ✅   | Parse read response                |
|   3 | `test_build_write`                        |   ✅   | Build write                        |
|   4 | `test_build_read_write_symbol`            |   ✅   | Build read write symbol            |
|   5 | `test_read_state_roundtrip`               |   ✅   | Read state roundtrip               |
|   6 | `test_parse_device_info`                  |   ✅   | Parse device info                  |
|   7 | `test_write_control_and_result`           |   ✅   | Write control and result           |
|   8 | `test_add_notification`                   |   ✅   | Add notification                   |
|   9 | `test_parse_notification_stream`          |   ✅   | Parse notification stream          |
|  10 | `test_build_overflow_fails_closed`        |   ✅   | Build overflow fails closed        |
|  11 | `test_parse_guards`                       |   ✅   | Parse guards                       |
|  12 | `test_build_read_device_info_and_del`     |   ✅   | Build read device info and del     |
|  13 | `test_build_null_and_small_buffer_guards` |   ✅   | Build null and small buffer guards |
|  14 | `test_build_write_control_variants`       |   ✅   | Build write control variants       |
|  15 | `test_parse_ams_header_more_guards`       |   ✅   | Parse ams header more guards       |
|  16 | `test_parse_payload_guards`               |   ✅   | Parse payload guards               |
|  17 | `test_parse_notification_guards`          |   ✅   | Parse notification guards          |

</details>

---

## test_focas - native_focas - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the FANUC FOCAS Ethernet codec (services/focas): the request builders and the_

|   # | Test                               | Status | Description                 |
| --: | :--------------------------------- | :----: | :-------------------------- |
|   1 | `test_build_open`                  |   ✅   | Build open                  |
|   2 | `test_build_close`                 |   ✅   | Build close                 |
|   3 | `test_build_sysinfo`               |   ✅   | Build sysinfo               |
|   4 | `test_build_read_position`         |   ✅   | Build read position         |
|   5 | `test_build_read_param`            |   ✅   | Build read param            |
|   6 | `test_build_request_extra`         |   ✅   | Build request extra         |
|   7 | `test_parse_sysinfo_response`      |   ✅   | Parse sysinfo response      |
|   8 | `test_parse_alarm_and_status`      |   ✅   | Parse alarm and status      |
|   9 | `test_decode8_value`               |   ✅   | 123.456 mm = 123456 / 10^3. |
|  10 | `test_build_overflow_fails_closed` |   ✅   | Build overflow fails closed |
|  11 | `test_parse_guards`                |   ✅   | Parse guards                |

</details>

---

## test_pqc_sha3 - native_pqc - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Known-answer tests for the Keccak/SHA-3/SHAKE primitive (network_drivers/presentation/pqc/sha3),_

|   # | Test                           | Status | Description             |
| --: | :----------------------------- | :----: | :---------------------- |
|   1 | `test_sha3_256`                |   ✅   | Sha3 256                |
|   2 | `test_sha3_512`                |   ✅   | Sha3 512                |
|   3 | `test_shake_empty`             |   ✅   | Shake empty             |
|   4 | `test_shake_stream_continuity` |   ✅   | Shake stream continuity |

</details>

---

## test_pqc_mlkem - native_pqc - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Known-answer test for ML-KEM-768 Encaps (network_drivers/presentation/pqc/mlkem), the post-quantum_

|   # | Test                                 | Status | Description                   |
| --: | :----------------------------------- | :----: | :---------------------------- |
|   1 | `test_mlkem768_encaps_kat`           |   ✅   | Mlkem768 encaps kat           |
|   2 | `test_mlkem768_encaps_varies_with_m` |   ✅   | Mlkem768 encaps varies with m |
|   3 | `test_mlkem768_rejects_malformed_ek` |   ✅   | Mlkem768 rejects malformed ek |

</details>

---

## test_iface_bridge - native_iface_bridge - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the interface-bridge pure core (services/iface_bridge): the address:port -> bus rule_

|   # | Test                               | Status | Description                              |
| --: | :--------------------------------- | :----: | :--------------------------------------- |
|   1 | `test_map_and_find`                |   ✅   | Map and find                             |
|   2 | `test_any_interface_and_dedup`     |   ✅   | Any interface and dedup                  |
|   3 | `test_bad_address_rejected`        |   ✅   | Bad address rejected                     |
|   4 | `test_table_full`                  |   ✅   | Table full                               |
|   5 | `test_txn_roundtrip`               |   ✅   | Txn roundtrip                            |
|   6 | `test_txn_partial_and_readonly`    |   ✅   | Partial header (< 4 bytes) -> need more. |
|   7 | `test_build_overflow_fails_closed` |   ✅   | Build overflow fails closed              |

</details>

---

## test_rtcm3 - native_rtcm3 - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the RTCM 3.x pure codec (services/gnss/rtcm3): CRC-24Q, MSB-first bit I/O, the transport_

|   # | Test                                | Status | Description                                                                                           |
| --: | :---------------------------------- | :----: | :---------------------------------------------------------------------------------------------------- |
|   1 | `test_build_1005_matches_pyrtcm`    |   ✅   | Build 1005 matches pyrtcm                                                                             |
|   2 | `test_build_1006_matches_pyrtcm`    |   ✅   | Build 1006 matches pyrtcm                                                                             |
|   3 | `test_parse_frame_and_1005`         |   ✅   | Parse frame and 1005                                                                                  |
|   4 | `test_parse_frame_and_1006`         |   ✅   | Parse frame and 1006                                                                                  |
|   5 | `test_crc24q_matches_frame`         |   ✅   | The 3 trailing CRC bytes are CRC-24Q over the preamble + header + payload (all but the last 3 bytes). |
|   6 | `test_crc_detects_corruption`       |   ✅   | Crc detects corruption                                                                                |
|   7 | `test_partial_frame_needs_more`     |   ✅   | Partial frame needs more                                                                              |
|   8 | `test_sync_finds_preamble`          |   ✅   | Sync finds preamble                                                                                   |
|   9 | `test_bit_io_roundtrip`             |   ✅   | Bit io roundtrip                                                                                      |
|  10 | `test_writer_overflow_fails_closed` |   ✅   | Writer overflow fails closed                                                                          |
|  11 | `test_frame_build_roundtrip`        |   ✅   | Frame build roundtrip                                                                                 |

</details>

---

## test_gnss_survey - native_gnss_survey - ✅ 22 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the GNSS survey-in core (services/gnss/dws_gnss_survey): the WGS84 geodetic->ECEF transform,_

|   # | Test                                       | Status | Description                                          |
| --: | :----------------------------------------- | :----: | :--------------------------------------------------- |
|   1 | `test_geodetic_to_ecef_matches_pyproj`     |   ✅   | Geodetic to ecef matches pyproj                      |
|   2 | `test_ecef_to_geodetic_roundtrip`          |   ✅   | Ecef to geodetic roundtrip                           |
|   3 | `test_m_to_01mm_rounds_half_away`          |   ✅   | M to 01mm rounds half away                           |
|   4 | `test_survey_single_fix_matches_reference` |   ✅   | Survey single fix matches reference                  |
|   5 | `test_survey_averages_out_scatter`         |   ✅   | Survey averages out scatter                          |
|   6 | `test_survey_empty_has_no_mean`            |   ✅   | Survey empty has no mean                             |
|   7 | `test_gga_to_geodetic`                     |   ✅   | Gga to geodetic                                      |
|   8 | `test_gga_no_fix_rejected`                 |   ✅   | Fix quality field (index 6) = 0 -> no fix -> reject. |
|   9 | `test_survey_add_gga_folds_fix`            |   ✅   | Survey add gga folds fix                             |
|  10 | `test_ecef_to_geodetic_north_pole`         |   ✅   | Ecef to geodetic north pole                          |
|  11 | `test_ecef_to_geodetic_south_pole`         |   ✅   | Ecef to geodetic south pole                          |
|  12 | `test_gga_empty_lat_rejected`              |   ✅   | Gga empty lat rejected                               |
|  13 | `test_gga_nonnumeric_lat_rejected`         |   ✅   | Gga nonnumeric lat rejected                          |
|  14 | `test_gga_empty_lon_rejected`              |   ✅   | Gga empty lon rejected                               |
|  15 | `test_gga_empty_quality_rejected`          |   ✅   | Gga empty quality rejected                           |
|  16 | `test_gga_empty_altitude_rejected`         |   ✅   | Gga empty altitude rejected                          |
|  17 | `test_gga_too_few_fields_rejected`         |   ✅   | Gga too few fields rejected                          |
|  18 | `test_gga_southern_eastern_hemisphere`     |   ✅   | Gga southern eastern hemisphere                      |
|  19 | `test_gga_lowercase_hemispheres`           |   ✅   | Gga lowercase hemispheres                            |
|  20 | `test_gga_geoid_absent_defaults_zero`      |   ✅   | Gga geoid absent defaults zero                       |
|  21 | `test_gga_bad_args_and_types_rejected`     |   ✅   | Gga bad args and types rejected                      |
|  22 | `test_survey_add_gga_rejects_bad_fix`      |   ✅   | Survey add gga rejects bad fix                       |

</details>

---

## test_ntrip_caster - native_ntrip_caster - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the NTRIP caster protocol codec (services/gnss/dws_ntrip_caster): rover request parsing_

|   # | Test                                              | Status | Description                                                         |
| --: | :------------------------------------------------ | :----: | :------------------------------------------------------------------ |
|   1 | `test_parse_v1_stream_request`                    |   ✅   | Parse v1 stream request                                             |
|   2 | `test_parse_v2_request_detects_version`           |   ✅   | Parse v2 request detects version                                    |
|   3 | `test_parse_sourcetable_request`                  |   ✅   | Parse sourcetable request                                           |
|   4 | `test_parse_extracts_basic_auth`                  |   ✅   | The parser spans the base64 token verbatim (it does not decode it). |
|   5 | `test_parse_incomplete_needs_more`                |   ✅   | Parse incomplete needs more                                         |
|   6 | `test_parse_rejects_non_get`                      |   ✅   | Parse rejects non get                                               |
|   7 | `test_stream_response_v1_v2`                      |   ✅   | Stream response v1 v2                                               |
|   8 | `test_str_record_format`                          |   ✅   | Str record format                                                   |
|   9 | `test_str_record_defaults_and_negative_small_lon` |   ✅   | Str record defaults and negative small lon                          |
|  10 | `test_sourcetable_has_records_and_correct_length` |   ✅   | Sourcetable has records and correct length                          |
|  11 | `test_sourcetable_v2_content_type`                |   ✅   | Sourcetable v2 content type                                         |
|  12 | `test_error_response`                             |   ✅   | Error response                                                      |
|  13 | `test_unauthorized_response`                      |   ✅   | Unauthorized response                                               |
|  14 | `test_response_overflow_fails_closed`             |   ✅   | Response overflow fails closed                                      |

</details>

---

## test_bacnet - native_bacnet - ✅ 9 passed

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

## test_enip - native_enip - ✅ 7 passed

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
|   7 | `test_build_and_parse_guards`      |   ✅   | Build and parse guards      |

</details>

---

## test_amqp - native_amqp - ✅ 8 passed

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
|   8 | `test_build_and_parse_guards`      |   ✅   | Build and parse guards               |

</details>

---

## test_cip - native_cip - ✅ 9 passed

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

## test_nats - native_nats - ✅ 14 passed

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

## test_proxy_protocol - native_proxy_protocol - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the HAProxy PROXY protocol codec (services/proxy_protocol): the v1 (text)_

|   # | Test                                      | Status | Description                                                                           |
| --: | :---------------------------------------- | :----: | :------------------------------------------------------------------------------------ |
|   1 | `test_v1_build`                           |   ✅   | V1 build                                                                              |
|   2 | `test_v1_round_trip`                      |   ✅   | V1 round trip                                                                         |
|   3 | `test_v2_build_bytes`                     |   ✅   | V2 build bytes                                                                        |
|   4 | `test_v2_round_trip`                      |   ✅   | V2 round trip                                                                         |
|   5 | `test_v1_unknown`                         |   ✅   | V1 unknown                                                                            |
|   6 | `test_not_a_proxy_header`                 |   ✅   | Not a proxy header                                                                    |
|   7 | `test_incomplete`                         |   ✅   | v1 prefix but no CRLF yet.                                                            |
|   8 | `test_build_overflow_fails_closed`        |   ✅   | Build overflow fails closed                                                           |
|   9 | `test_v1_malformed_addresses_fail_closed` |   ✅   | Each line is CRLF-terminated so it reaches parse_ipv4 / parse_u16 (a header without a |
|  10 | `test_parse_and_build_guards`             |   ✅   | proxy_parse null-argument guards + proxy_v1_build null buffer.                        |

</details>

---

## test_sparkplug - native_sparkplug - ✅ 7 passed

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

## test_modbus_master - native_modbus_master - ✅ 12 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Modbus master codec (services/modbus/dws_modbus_master): request_

|   # | Test                               | Status | Description                                                             |
| --: | :--------------------------------- | :----: | :---------------------------------------------------------------------- |
|   1 | `test_build_read_bytes`            |   ✅   | Build read bytes                                                        |
|   2 | `test_build_rejects_bad_args`      |   ✅   | Build rejects bad args                                                  |
|   3 | `test_round_trip_holding_regs`     |   ✅   | Round trip holding regs                                                 |
|   4 | `test_round_trip_exception`        |   ✅   | Read a wildly out-of-range address: the slave returns an exception ADU. |
|   5 | `test_parse_short_frame_fails`     |   ✅   | Parse short frame fails                                                 |
|   6 | `test_build_null_out_and_input_fc` |   ✅   | Build null out and input fc                                             |
|   7 | `test_parse_null_adu`              |   ✅   | Parse null adu                                                          |
|   8 | `test_parse_bad_protocol_id`       |   ✅   | Parse bad protocol id                                                   |
|   9 | `test_parse_unexpected_function`   |   ✅   | Parse unexpected function                                               |
|  10 | `test_parse_exception_null_out`    |   ✅   | Parse exception null out                                                |
|  11 | `test_parse_bad_byte_count`        |   ✅   | Parse bad byte count                                                    |
|  12 | `test_parse_max_regs_and_null_out` |   ✅   | A 4-register response (byte count 8), len = 9 + 8 = 17.                 |

</details>

---

## test_ota_rollback - native_ota_rollback - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the OTA rollback decision (services/ota_rollback). The esp_ota_

|   # | Test                                      | Status | Description                                                                      |
| --: | :---------------------------------------- | :----: | :------------------------------------------------------------------------------- |
|   1 | `test_not_pending_waits`                  |   ✅   | A normally-booted (valid/undefined) image never rolls back.                      |
|   2 | `test_pending_self_test_ok_commits`       |   ✅   | Pending self test ok commits                                                     |
|   3 | `test_pending_within_window_waits`        |   ✅   | Pending within window waits                                                      |
|   4 | `test_pending_window_elapsed_rolls_back`  |   ✅   | Pending window elapsed rolls back                                                |
|   5 | `test_self_test_ok_beats_window`          |   ✅   | A passing self-test commits even past the window.                                |
|   6 | `test_host_platform_hooks_are_safe_noops` |   ✅   | On a host build there are no OTA partitions: img_state reports UNDEFINED and the |

</details>

---

## test_totp - native_totp - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for TOTP (services/totp): the RFC 6238 Appendix B test vectors_

|   # | Test                                      | Status | Description                                           |
| --: | :---------------------------------------- | :----: | :---------------------------------------------------- |
|   1 | `test_rfc6238_vectors`                    |   ✅   | RFC 6238 Appendix B (SHA-1, T0=0, step=30, digits=8). |
|   2 | `test_verify_window`                      |   ✅   | Verify window                                         |
|   3 | `test_base32_decode`                      |   ✅   | Base32 decode                                         |
|   4 | `test_base32_rejects_invalid`             |   ✅   | Base32 rejects invalid                                |
|   5 | `test_long_key_default_period_and_base32` |   ✅   | Long key default period and base32                    |

</details>

---

## test_webhook - native_webhook - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the webhook builders (services/webhook): IFTTT URL + payload_

|   # | Test                                        | Status | Description                                                                                          |
| --: | :------------------------------------------ | :----: | :--------------------------------------------------------------------------------------------------- |
|   1 | `test_ifttt_url`                            |   ✅   | Ifttt url                                                                                            |
|   2 | `test_payload_three_values`                 |   ✅   | Payload three values                                                                                 |
|   3 | `test_payload_omits_nulls`                  |   ✅   | Payload omits nulls                                                                                  |
|   4 | `test_payload_escapes_json`                 |   ✅   | Payload escapes json                                                                                 |
|   5 | `test_overflow_fails_closed`                |   ✅   | Overflow fails closed                                                                                |
|   6 | `test_ifttt_trigger_and_post_stub`          |   ✅   | Host build (no HTTP client): webhook_post is a -1 stub; ifttt_trigger builds url+payload then posts. |
|   7 | `test_builder_arg_guards`                   |   ✅   | Builder arg guards                                                                                   |
|   8 | `test_payload_escape_overflow_fails_closed` |   ✅   | "{\"value1\":\"" is 11 chars; a 10-char plain value overruns mid-escape-loop.                        |
|   9 | `test_trigger_build_failures`               |   ✅   | Trigger build failures                                                                               |

</details>

---

## test_radio_power - native_radio_power - ✅ 2 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the radio-power mode names (services/radio_power). Applying the_

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_ps_names`              |   ✅   | Ps names              |
|   2 | `test_apply_is_noop_on_host` |   ✅   | Apply is noop on host |

</details>

---

## test_dns_resolver - native_dns_resolver - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the DNS answer classifier / verifier (services/dns_resolver)._

|   # | Test                             | Status | Description               |
| --: | :------------------------------- | :----: | :------------------------ |
|   1 | `test_classify`                  |   ✅   | Classify                  |
|   2 | `test_verify_rejects_suspicious` |   ✅   | Verify rejects suspicious |
|   3 | `test_verify_accepts_plausible`  |   ✅   | Verify accepts plausible  |
|   4 | `test_resolve_is_noop_on_host`   |   ✅   | Resolve is noop on host   |
|   5 | `test_resolve_verified_paths`    |   ✅   | resolve fails -> false.   |

</details>

---

## test_audit_log - native_audit_log - ✅ 16 passed

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

## test_oidc - native_oidc - ✅ 19 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the OIDC RS256 ID-token verifier (services/oidc). Vectors are_

|   # | Test                                 | Status | Description                                                               |
| --: | :----------------------------------- | :----: | :------------------------------------------------------------------------ |
|   1 | `test_oidc_parse_edge_guards`        |   ✅   | Oidc parse edge guards                                                    |
|   2 | `test_oidc_signed_claim_guards`      |   ✅   | Oidc signed claim guards                                                  |
|   3 | `test_jwks_malformed_keys`           |   ✅   | Jwks malformed keys                                                       |
|   4 | `test_token_kid_guards`              |   ✅   | Token kid guards                                                          |
|   5 | `test_jwks_find_guards`              |   ✅   | Jwks find guards                                                          |
|   6 | `test_verify_guards_and_malformed`   |   ✅   | Verify guards and malformed                                               |
|   7 | `test_token_kid`                     |   ✅   | Token kid                                                                 |
|   8 | `test_jwks_find`                     |   ✅   | Jwks find                                                                 |
|   9 | `test_jwks_find_missing_kid_fails`   |   ✅   | Jwks find missing kid fails                                               |
|  10 | `test_verify_valid_token_and_claims` |   ✅   | Verify valid token and claims                                             |
|  11 | `test_verify_aud_array`              |   ✅   | Verify aud array                                                          |
|  12 | `test_reject_expired`                |   ✅   | Reject expired                                                            |
|  13 | `test_reject_wrong_issuer`           |   ✅   | Reject wrong issuer                                                       |
|  14 | `test_reject_wrong_audience`         |   ✅   | Reject wrong audience                                                     |
|  15 | `test_reject_non_rs256_header`       |   ✅   | Reject non rs256 header                                                   |
|  16 | `test_reject_tampered_payload`       |   ✅   | Reject tampered payload                                                   |
|  17 | `test_reject_tampered_signature`     |   ✅   | Reject tampered signature                                                 |
|  18 | `test_reject_unknown_key`            |   ✅   | JWKS whose only key has a different kid than the token's.                 |
|  19 | `test_reject_malformed`              |   ✅   | No kid extractable -> the sole JWKS key is selected, then the token shape |

</details>

---

## test_vfs - native_vfs - ✅ 12 passed

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
|  12 | `test_ram_guard_subconditions`            |   ✅   | Ram guard subconditions            |

</details>

---

## test_graphql - native_graphql - ✅ 32 passed

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

## test_espnow - native_espnow - ✅ 8 passed

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
|   8 | `test_peer_guard_and_host_stubs`                |   ✅   | Peer guard and host stubs                |

</details>

---

## test_oauth2 - native_oauth2 - ✅ 9 passed

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
|   9 | `test_oauth2_build_parse_guards`                       |   ✅   | Oauth2 build parse guards                                             |

</details>

---

## test_opcua - native_opcua - ✅ 47 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for OPC UA (services/opcua): the Binary built-in type codec (incl._

|   # | Test                                             | Status | Description                                                                                          |
| --: | :----------------------------------------------- | :----: | :--------------------------------------------------------------------------------------------------- |
|   1 | `test_parse_read_optional_fields`                |   ✅   | Parse read optional fields                                                                           |
|   2 | `test_parse_rejections`                          |   ✅   | Parse rejections                                                                                     |
|   3 | `test_build_guards_and_overflow`                 |   ✅   | Build guards and overflow                                                                            |
|   4 | `test_setters_and_endpoint_url`                  |   ✅   | Setters and endpoint url                                                                             |
|   5 | `test_variant_scalar_types`                      |   ✅   | Variant scalar types                                                                                 |
|   6 | `test_variant_errors`                            |   ✅   | Variant errors                                                                                       |
|   7 | `test_datavalue_all_masks`                       |   ✅   | Datavalue all masks                                                                                  |
|   8 | `test_nodeid_encodings`                          |   ✅   | Nodeid encodings                                                                                     |
|   9 | `test_reader_underruns`                          |   ✅   | Reader underruns                                                                                     |
|  10 | `test_codec_roundtrip`                           |   ✅   | Codec roundtrip                                                                                      |
|  11 | `test_string_null_roundtrip`                     |   ✅   | String null roundtrip                                                                                |
|  12 | `test_reader_underrun_latches`                   |   ✅   | Reader underrun latches                                                                              |
|  13 | `test_writer_overflow_fails_closed`              |   ✅   | Writer overflow fails closed                                                                         |
|  14 | `test_parse_header`                              |   ✅   | Parse header                                                                                         |
|  15 | `test_parse_hello`                               |   ✅   | Parse hello                                                                                          |
|  16 | `test_parse_hello_rejects_short`                 |   ✅   | Parse hello rejects short                                                                            |
|  17 | `test_build_ack_negotiates`                      |   ✅   | Build ack negotiates                                                                                 |
|  18 | `test_nodeid_roundtrip`                          |   ✅   | Nodeid roundtrip                                                                                     |
|  19 | `test_filetime_from_unix`                        |   ✅   | Filetime from unix                                                                                   |
|  20 | `test_parse_open`                                |   ✅   | Parse open                                                                                           |
|  21 | `test_parse_open_rejects_wrong_type`             |   ✅   | Corrupt the message type so it is no longer "OPN".                                                   |
|  22 | `test_build_open_response`                       |   ✅   | Build open response                                                                                  |
|  23 | `test_parse_msg`                                 |   ✅   | Parse msg                                                                                            |
|  24 | `test_parse_msg_rejects_non_msg`                 |   ✅   | Parse msg rejects non msg                                                                            |
|  25 | `test_build_create_session_response`             |   ✅   | Build create session response                                                                        |
|  26 | `test_build_activate_session_response`           |   ✅   | Build activate session response                                                                      |
|  27 | `test_datavalue_good_int32`                      |   ✅   | Datavalue good int32                                                                                 |
|  28 | `test_datavalue_bad_status`                      |   ✅   | Datavalue bad status                                                                                 |
|  29 | `test_parse_read`                                |   ✅   | Parse read                                                                                           |
|  30 | `test_build_read_response`                       |   ✅   | Build read response                                                                                  |
|  31 | `test_parse_browse`                              |   ✅   | Parse browse                                                                                         |
|  32 | `test_build_browse_response`                     |   ✅   | Build browse response                                                                                |
|  33 | `test_build_browse_response_unknown`             |   ✅   | Build browse response unknown                                                                        |
|  34 | `test_build_close_session_response`              |   ✅   | Build close session response                                                                         |
|  35 | `test_build_get_endpoints`                       |   ✅   | Build get endpoints                                                                                  |
|  36 | `test_build_service_fault`                       |   ✅   | Build service fault                                                                                  |
|  37 | `test_datavalue_roundtrip`                       |   ✅   | Datavalue roundtrip                                                                                  |
|  38 | `test_parse_and_build_write`                     |   ✅   | Build a WriteRequest writing one Int32 to ns=1;i=10 (value-only DataValue).                          |
|  39 | `test_rx_and_proto_handler_host_stubs`           |   ✅   | Rx and proto handler host stubs                                                                      |
|  40 | `test_parse_open_with_cert_and_nonce`            |   ✅   | An OPEN carrying non-empty SenderCertificate + ReceiverCertificateThumbprint + ClientNonce           |
|  41 | `test_parse_read_truncated_item_rejected`        |   ✅   | A NodesToRead count larger than the items actually present makes the per-item NodeId read            |
|  42 | `test_parse_browse_truncated_item_rejected`      |   ✅   | Parse browse truncated item rejected                                                                 |
|  43 | `test_parse_write_truncated_item_and_indexrange` |   ✅   | Count claims two items but only one is present -> the second NodeId read underruns -> reject.        |
|  44 | `test_parse_open_wrong_body_typeid`              |   ✅   | Body TypeId is OPEN_REQ (446 -> FourByte bytes 01 00 BE 01); corrupt the id so it no longer matches. |
|  45 | `test_parse_write_malformed_datavalue_rejected`  |   ✅   | The item's DataValue is INT32 0x11223344; corrupt its Variant type byte to an unsupported value.     |
|  46 | `test_parse_request_header_truncated_addhdr`     |   ✅   | Parse request header truncated addhdr                                                                |
|  47 | `test_parse_open_truncated_frames`               |   ✅   | Parse open truncated frames                                                                          |

</details>

---

## test_opcua_client - native_opcua_client - ✅ 20 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Round-trip tests for the OPC UA client (services/dws_opcua_client): the client builds_

|   # | Test                                          | Status | Description                                                                               |
| --: | :-------------------------------------------- | :----: | :---------------------------------------------------------------------------------------- |
|   1 | `test_browse_display_name_locale`             |   ✅   | Browse display name locale                                                                |
|   2 | `test_on_read_all_variant_types`              |   ✅   | On read all variant types                                                                 |
|   3 | `test_client_parsers_reject_fault`            |   ✅   | Client parsers reject fault                                                               |
|   4 | `test_client_parsers_reject_malformed`        |   ✅   | Client parsers reject malformed                                                           |
|   5 | `test_hello_ack_roundtrip`                    |   ✅   | Hello ack roundtrip                                                                       |
|   6 | `test_open_roundtrip`                         |   ✅   | Open roundtrip                                                                            |
|   7 | `test_session_roundtrip`                      |   ✅   | Session roundtrip                                                                         |
|   8 | `test_get_endpoints_roundtrip`                |   ✅   | Get endpoints roundtrip                                                                   |
|   9 | `test_service_fault_rejected_by_parsers`      |   ✅   | An unknown service draws a ServiceFault; a typed parser must reject it (wrong TypeId).    |
|  10 | `test_read_roundtrip`                         |   ✅   | Read roundtrip                                                                            |
|  11 | `test_browse_roundtrip`                       |   ✅   | Browse roundtrip                                                                          |
|  12 | `test_write_roundtrip`                        |   ✅   | Write roundtrip                                                                           |
|  13 | `test_close_session_roundtrip`                |   ✅   | Close session roundtrip                                                                   |
|  14 | `test_close_channel_is_clo`                   |   ✅   | Close channel is clo                                                                      |
|  15 | `test_seq_and_request_id_increment`           |   ✅   | Seq and request id increment                                                              |
|  16 | `test_builder_overflow_guard`                 |   ✅   | A capacity too small for even the frame header overflows the writer; cw_patch returns 0.  |
|  17 | `test_on_read_unknown_variant_rejected`       |   ✅   | A server sending a DataValue whose Variant type byte is unsupported must be rejected, not |
|  18 | `test_response_parsers_reject_negative_count` |   ✅   | Response parsers reject negative count                                                    |
|  19 | `test_on_open_guards`                         |   ✅   | On open guards                                                                            |
|  20 | `test_response_header_string_table_skip`      |   ✅   | A ResponseHeader carrying a non-empty StringTable makes cr_skip_string_array iterate; the |

</details>

---

## test_umati - native_umati - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for the umati (OPC UA for Machine Tools, OPC 40501-1) MachineTool model:_

|   # | Test                                           | Status | Description                             |
| --: | :--------------------------------------------- | :----: | :-------------------------------------- |
|   1 | `test_browse_objects_folder_has_machinetool`   |   ✅   | Browse objects folder has machinetool   |
|   2 | `test_browse_machinetool_components`           |   ✅   | Browse machinetool components           |
|   3 | `test_browse_identification_variables`         |   ✅   | Browse identification variables         |
|   4 | `test_browse_monitoring_and_children`          |   ✅   | Browse monitoring and children          |
|   5 | `test_browse_leaf_and_unknown_return_negative` |   ✅   | Browse leaf and unknown return negative |
|   6 | `test_read_identification`                     |   ✅   | Read identification                     |
|   7 | `test_read_monitoring_values`                  |   ✅   | Read monitoring values                  |
|   8 | `test_read_production_and_notification`        |   ✅   | Read production and notification        |
|   9 | `test_read_null_string_served_as_empty`        |   ✅   | Read null string served as empty        |
|  10 | `test_read_rejects_unknown_ns_attr_and_node`   |   ✅   | Read rejects unknown ns attr and node   |
|  11 | `test_read_before_bind_is_a_clean_miss`        |   ✅   | Read before bind is a clean miss        |

</details>

---

## test_keepalive - native_keepalive - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_HTTP/1.1 keep-alive (DWS_ENABLE_KEEPALIVE). Each test drives one or more_

|   # | Test                                     | Status | Description                                                                |
| --: | :--------------------------------------- | :----: | :------------------------------------------------------------------------- |
|   1 | `test_conn_token_ws_and_bare_keepalive`  |   ✅   | Conn token ws and bare keepalive                                           |
|   2 | `test_http11_default_keeps_alive`        |   ✅   | Http11 default keeps alive                                                 |
|   3 | `test_http11_explicit_close`             |   ✅   | Http11 explicit close                                                      |
|   4 | `test_http10_default_closes`             |   ✅   | Http10 default closes                                                      |
|   5 | `test_http10_explicit_keepalive`         |   ✅   | Http10 explicit keepalive                                                  |
|   6 | `test_connection_token_list_close`       |   ✅   | "close" appearing in a token list must still be honored.                   |
|   7 | `test_two_sequential_requests_same_slot` |   ✅   | Two sequential requests same slot                                          |
|   8 | `test_pipelined_requests`                |   ✅   | Two requests delivered in one shot: the proactive drain in handle() must   |
|   9 | `test_404_still_keeps_alive`             |   ✅   | A well-formed request to an unknown path is a normal response, not an      |
|  10 | `test_max_requests_cap_closes`           |   ✅   | DWS_KEEPALIVE_MAX_REQUESTS=3: the 3rd response closes the connection.      |
|  11 | `test_fresh_connection_resets_count`     |   ✅   | Run a slot up to the cap, then re-open it (new connection) and confirm the |

</details>

---

## test_range - native_range - ✅ 20 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_HTTP Range requests / 206 Partial Content (DWS_ENABLE_RANGE). Each test_

|   # | Test                                               | Status | Description                                 |
| --: | :------------------------------------------------- | :----: | :------------------------------------------ |
|   1 | `test_file_send_backpressure_resumes_across_polls` |   ✅   | File send backpressure resumes across polls |
|   2 | `test_file_send_write_fails_then_retries`          |   ✅   | File send write fails then retries          |
|   3 | `test_file_send_short_read_stops`                  |   ✅   | File send short read stops                  |
|   4 | `test_range_trailing_garbage_ignored`              |   ✅   | Range trailing garbage ignored              |
|   5 | `test_range_start_after_end_unsatisfiable`         |   ✅   | Range start after end unsatisfiable         |
|   6 | `test_range_suffix_on_empty_file`                  |   ✅   | Range suffix on empty file                  |
|   7 | `test_serve_file_connection_gone`                  |   ✅   | Serve file connection gone                  |
|   8 | `test_no_range_full_200`                           |   ✅   | No range full 200                           |
|   9 | `test_range_prefix`                                |   ✅   | Range prefix                                |
|  10 | `test_range_open_ended`                            |   ✅   | Range open ended                            |
|  11 | `test_range_suffix`                                |   ✅   | Range suffix                                |
|  12 | `test_range_single_byte`                           |   ✅   | Range single byte                           |
|  13 | `test_range_clamped_to_eof`                        |   ✅   | Range clamped to eof                        |
|  14 | `test_range_unsatisfiable_416`                     |   ✅   | Range unsatisfiable 416                     |
|  15 | `test_malformed_range_ignored`                     |   ✅   | Malformed range ignored                     |
|  16 | `test_range_overflow_start_unsatisfiable`          |   ✅   | Range overflow start unsatisfiable          |
|  17 | `test_range_overflow_end_clamps`                   |   ✅   | Range overflow end clamps                   |
|  18 | `test_range_suffix_zero_unsatisfiable`             |   ✅   | Range suffix zero unsatisfiable             |
|  19 | `test_multirange_falls_back_to_200`                |   ✅   | Multirange falls back to 200                |
|  20 | `test_head_with_range_no_body`                     |   ✅   | Head with range no body                     |

</details>

---

## test_syslog - native_syslog - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the RFC 5424 syslog client (dws_syslog_format formatter + dws_syslog_init /_

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
|   9 | `test_format_null_and_pri_clamp`    |   ✅   | Guard clauses return 0.      |
|  10 | `test_init_truncates_long_fields`   |   ✅   | Init truncates long fields   |

</details>

---

## test_smb_client - native_smb - ✅ 58 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SMB2 client dialogue engine (services/smb/smb_client): smb_open drives the_

|   # | Test                                      | Status | Description                        |
| --: | :---------------------------------------- | :----: | :--------------------------------- |
|   1 | `test_open_close_success`                 |   ✅   | Open close success                 |
|   2 | `test_auth_failure`                       |   ✅   | Auth failure                       |
|   3 | `test_bad_share`                          |   ✅   | Bad share                          |
|   4 | `test_create_not_found`                   |   ✅   | Create not found                   |
|   5 | `test_io_error`                           |   ✅   | Io error                           |
|   6 | `test_arg_validation`                     |   ✅   | Arg validation                     |
|   7 | `test_read_file`                          |   ✅   | Read file                          |
|   8 | `test_read_past_eof`                      |   ✅   | Read past eof                      |
|   9 | `test_write_file`                         |   ✅   | Write file                         |
|  10 | `test_write_then_read_roundtrip`          |   ✅   | Write then read roundtrip          |
|  11 | `test_negotiate_malformed`                |   ✅   | Negotiate malformed                |
|  12 | `test_negotiate_dropped`                  |   ✅   | Negotiate dropped                  |
|  13 | `test_session1_bad_header`                |   ✅   | Session1 bad header                |
|  14 | `test_session1_wrong_status`              |   ✅   | Session1 wrong status              |
|  15 | `test_session1_bad_body`                  |   ✅   | Session1 bad body                  |
|  16 | `test_session1_no_secbuf`                 |   ✅   | Session1 no secbuf                 |
|  17 | `test_session1_bad_spnego`                |   ✅   | Session1 bad spnego                |
|  18 | `test_session1_bad_ntlmssp`               |   ✅   | Session1 bad ntlmssp               |
|  19 | `test_session2_dropped`                   |   ✅   | Session2 dropped                   |
|  20 | `test_session2_bad_header`                |   ✅   | Session2 bad header                |
|  21 | `test_tree_dropped`                       |   ✅   | Tree dropped                       |
|  22 | `test_tree_bad_body`                      |   ✅   | Tree bad body                      |
|  23 | `test_create_dropped`                     |   ✅   | Create dropped                     |
|  24 | `test_create_bad_body`                    |   ✅   | Create bad body                    |
|  25 | `test_long_share_overflow`                |   ✅   | Long share overflow                |
|  26 | `test_long_path_overflow`                 |   ✅   | Long path overflow                 |
|  27 | `test_long_user_overflow`                 |   ✅   | Long user overflow                 |
|  28 | `test_challenge_ti_ntlmv2_overflow`       |   ✅   | Challenge ti ntlmv2 overflow       |
|  29 | `test_challenge_ti_authenticate_overflow` |   ✅   | Challenge ti authenticate overflow |
|  30 | `test_challenge_ti_spnego_overflow`       |   ✅   | Challenge ti spnego overflow       |
|  31 | `test_av_eol_only`                        |   ✅   | Av eol only                        |
|  32 | `test_av_skip_then_find`                  |   ✅   | Av skip then find                  |
|  33 | `test_av_truncated_timestamp`             |   ✅   | Av truncated timestamp             |
|  34 | `test_read_arg`                           |   ✅   | Read arg                           |
|  35 | `test_read_send_io`                       |   ✅   | Read send io                       |
|  36 | `test_read_recv_io`                       |   ✅   | Read recv io                       |
|  37 | `test_read_bad_header`                    |   ✅   | Read bad header                    |
|  38 | `test_read_status_error`                  |   ✅   | Read status error                  |
|  39 | `test_read_bad_body`                      |   ✅   | Read bad body                      |
|  40 | `test_read_data_too_long`                 |   ✅   | Read data too long                 |
|  41 | `test_read_zero_data`                     |   ✅   | Read zero data                     |
|  42 | `test_write_arg`                          |   ✅   | Write arg                          |
|  43 | `test_write_send_io`                      |   ✅   | Write send io                      |
|  44 | `test_write_recv_io`                      |   ✅   | Write recv io                      |
|  45 | `test_write_recv_overflow`                |   ✅   | Write recv overflow                |
|  46 | `test_write_bad_header`                   |   ✅   | Write bad header                   |
|  47 | `test_write_status_error`                 |   ✅   | Write status error                 |
|  48 | `test_write_bad_body`                     |   ✅   | Write bad body                     |
|  49 | `test_write_zero_count`                   |   ✅   | Write zero count                   |
|  50 | `test_write_count_too_big`                |   ✅   | Write count too big                |
|  51 | `test_close_arg`                          |   ✅   | Close arg                          |
|  52 | `test_close_send_io`                      |   ✅   | Close send io                      |
|  53 | `test_close_recv_overflow`                |   ✅   | Close recv overflow                |
|  54 | `test_close_recv_zero_len`                |   ✅   | Close recv zero len                |
|  55 | `test_close_recv_trunc_body`              |   ✅   | Close recv trunc body              |
|  56 | `test_close_bad_header`                   |   ✅   | Close bad header                   |
|  57 | `test_close_status_error`                 |   ✅   | Close status error                 |
|  58 | `test_close_bad_body`                     |   ✅   | Close bad body                     |

</details>

---

## test_smb_crypto - native_smb - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_KAT tests for the NTLM digests (services/smb/smb_md): MD5 (RFC 1321 App A.5),_

|   # | Test                            | Status | Description              |
| --: | :------------------------------ | :----: | :----------------------- |
|   1 | `test_md5_vectors`              |   ✅   | Md5 vectors              |
|   2 | `test_md4_vectors`              |   ✅   | Md4 vectors              |
|   3 | `test_hmac_md5_vectors`         |   ✅   | Hmac md5 vectors         |
|   4 | `test_streaming_equals_oneshot` |   ✅   | Streaming equals oneshot |
|   5 | `test_nt_hash`                  |   ✅   | Nt hash                  |

</details>

---

## test_spnego - native_smb - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SPNEGO GSS-API DER wrapping (services/smb/spnego): the InitialContextToken_

|   # | Test                             | Status | Description               |
| --: | :------------------------------- | :----: | :------------------------ |
|   1 | `test_wrap_negotiate_bytes`      |   ✅   | Wrap negotiate bytes      |
|   2 | `test_authenticate_roundtrip`    |   ✅   | Authenticate roundtrip    |
|   3 | `test_parse_server_response`     |   ✅   | Parse server response     |
|   4 | `test_parse_rejects`             |   ✅   | Parse rejects             |
|   5 | `test_wrap_len_2byte`            |   ✅   | Wrap len 2byte            |
|   6 | `test_wrap_len_3byte`            |   ✅   | Wrap len 3byte            |
|   7 | `test_wrap_negotiate_guards`     |   ✅   | Wrap negotiate guards     |
|   8 | `test_wrap_authenticate_guards`  |   ✅   | Wrap authenticate guards  |
|   9 | `test_parse_null_args`           |   ✅   | Parse null args           |
|  10 | `test_parse_truncated_header`    |   ✅   | Parse truncated header    |
|  11 | `test_parse_bad_longform_len`    |   ✅   | Parse bad longform len    |
|  12 | `test_parse_inner_not_seq`       |   ✅   | Parse inner not seq       |
|  13 | `test_parse_field_malformed`     |   ✅   | Parse field malformed     |
|  14 | `test_parse_resptoken_not_octet` |   ✅   | Parse resptoken not octet |

</details>

---

## test_ntlm - native_smb - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_KAT test for the NTLMv2 response (services/smb/ntlm) against the MS-NLMP section 4.2_

|   # | Test                           | Status | Description             |
| --: | :----------------------------- | :----: | :---------------------- |
|   1 | `test_ntowfv2`                 |   ✅   | Ntowfv2                 |
|   2 | `test_ntlmv2_response`         |   ✅   | Ntlmv2 response         |
|   3 | `test_fail_closed`             |   ✅   | Fail closed             |
|   4 | `test_ntowfv2_user_overflow`   |   ✅   | Ntowfv2 user overflow   |
|   5 | `test_ntowfv2_domain_overflow` |   ✅   | Ntowfv2 domain overflow |
|   6 | `test_ntowfv2_upper_high_char` |   ✅   | Ntowfv2 upper high char |
|   7 | `test_v2_response_null_out`    |   ✅   | V2 response null out    |
|   8 | `test_v2_response_null_skey`   |   ✅   | V2 response null skey   |

</details>

---

## test_ntlmssp - native_smb - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the NTLMSSP message codec (services/smb/ntlmssp, MS-NLMP 2.2.1): the_

|   # | Test                           | Status | Description             |
| --: | :----------------------------- | :----: | :---------------------- |
|   1 | `test_build_negotiate`         |   ✅   | Build negotiate         |
|   2 | `test_parse_challenge`         |   ✅   | Parse challenge         |
|   3 | `test_parse_challenge_rejects` |   ✅   | Parse challenge rejects |
|   4 | `test_build_authenticate`      |   ✅   | Build authenticate      |
|   5 | `test_end_to_end`              |   ✅   | End to end              |

</details>

---

## test_smb2 - native_smb - ✅ 19 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SMB2 client wire codec (services/smb, MS-SMB2): the Direct-TCP transport_

|   # | Test                                    | Status | Description                      |
| --: | :-------------------------------------- | :----: | :------------------------------- |
|   1 | `test_transport_frame`                  |   ✅   | Transport frame                  |
|   2 | `test_build_and_parse_header`           |   ✅   | Build and parse header           |
|   3 | `test_parse_header_rejects`             |   ✅   | Parse header rejects             |
|   4 | `test_build_negotiate`                  |   ✅   | Build negotiate                  |
|   5 | `test_parse_negotiate_response`         |   ✅   | Parse negotiate response         |
|   6 | `test_parse_negotiate_response_rejects` |   ✅   | Parse negotiate response rejects |
|   7 | `test_build_session_setup`              |   ✅   | Build session setup              |
|   8 | `test_parse_session_setup_response`     |   ✅   | Parse session setup response     |
|   9 | `test_session_setup_rejects`            |   ✅   | Session setup rejects            |
|  10 | `test_session_setup_spnego_flow`        |   ✅   | Session setup spnego flow        |
|  11 | `test_build_tree_connect`               |   ✅   | Build tree connect               |
|  12 | `test_parse_tree_connect_response`      |   ✅   | Parse tree connect response      |
|  13 | `test_build_create`                     |   ✅   | Build create                     |
|  14 | `test_parse_create_response`            |   ✅   | Parse create response            |
|  15 | `test_close_roundtrip`                  |   ✅   | Close roundtrip                  |
|  16 | `test_build_read`                       |   ✅   | Build read                       |
|  17 | `test_parse_read_response`              |   ✅   | Parse read response              |
|  18 | `test_build_write`                      |   ✅   | Build write                      |
|  19 | `test_parse_write_response`             |   ✅   | Parse write response             |

</details>

---

## test_smtp - native_smtp - ✅ 22 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SMTP client dialogue engine (services/smtp/smtp_run). A scripted_

|   # | Test                                    | Status | Description                                                                                        |
| --: | :-------------------------------------- | :----: | :------------------------------------------------------------------------------------------------- |
|   1 | `test_happy_path_no_auth`               |   ✅   | Happy path no auth                                                                                 |
|   2 | `test_auth_login`                       |   ✅   | Auth login                                                                                         |
|   3 | `test_auth_rejected`                    |   ✅   | Auth rejected                                                                                      |
|   4 | `test_greeting_not_ready`               |   ✅   | Greeting not ready                                                                                 |
|   5 | `test_rcpt_rejected`                    |   ✅   | Rcpt rejected                                                                                      |
|   6 | `test_data_refused`                     |   ✅   | Data refused                                                                                       |
|   7 | `test_dot_stuffing`                     |   ✅   | Dot stuffing                                                                                       |
|   8 | `test_multiline_reply_and_lf_body`      |   ✅   | Multiline reply and lf body                                                                        |
|   9 | `test_partial_reads_dribble`            |   ✅   | Partial reads dribble                                                                              |
|  10 | `test_missing_required_arg`             |   ✅   | Missing required arg                                                                               |
|  11 | `test_io_error_when_server_hangs`       |   ✅   | Io error when server hangs                                                                         |
|  12 | `test_reply_buffer_overflow`            |   ✅   | Reply buffer overflow                                                                              |
|  13 | `test_command_send_fails`               |   ✅   | Command send fails                                                                                 |
|  14 | `test_body_send_fails`                  |   ✅   | Body send fails                                                                                    |
|  15 | `test_auth_secret_too_long`             |   ✅   | Auth secret too long                                                                               |
|  16 | `test_io_error_at_each_step`            |   ✅   | greeting ok, then hang before: EHLO / MAIL(no auth) / AUTH(user) / pass-leg / RCPT / DATA / final. |
|  17 | `test_protocol_error_at_each_step`      |   ✅   | Protocol error at each step                                                                        |
|  18 | `test_command_line_overflows`           |   ✅   | Command line overflows                                                                             |
|  19 | `test_message_header_overflow`          |   ✅   | Message header overflow                                                                            |
|  20 | `test_cr_in_body_dropped`               |   ✅   | Cr in body dropped                                                                                 |
|  21 | `test_build_message_boundary_overflows` |   ✅   | Build message boundary overflows                                                                   |
|  22 | `test_host_smtp_send_stub`              |   ✅   | Host smtp send stub                                                                                |

</details>

---

## test_ntp_server - native_ntp_server - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the NTP server response codec (services/dws_ntp_server_build_response): a pure_

|   # | Test                              | Status | Description                |
| --: | :-------------------------------- | :----: | :------------------------- |
|   1 | `test_happy_path_fields`          |   ✅   | Happy path fields          |
|   2 | `test_origin_is_client_transmit`  |   ✅   | Origin is client transmit  |
|   3 | `test_version_echo`               |   ✅   | Version echo               |
|   4 | `test_poll_echo_and_default`      |   ✅   | Poll echo and default      |
|   5 | `test_stratum_passthrough`        |   ✅   | Stratum passthrough        |
|   6 | `test_big_endian_encoding`        |   ✅   | Big endian encoding        |
|   7 | `test_length_guards`              |   ✅   | Length guards              |
|   8 | `test_root_dispersion_advertised` |   ✅   | Root dispersion advertised |

</details>

---

## test_dns_server - native_dns_server - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the authoritative DNS server (services/dns_server): the pure response_

|   # | Test                                     | Status | Description                       |
| --: | :--------------------------------------- | :----: | :-------------------------------- |
|   1 | `test_a_record_answer`                   |   ✅   | A record answer                   |
|   2 | `test_nxdomain`                          |   ✅   | Nxdomain                          |
|   3 | `test_non_a_query_no_error`              |   ✅   | Non a query no error              |
|   4 | `test_multilabel_name_reaches_resolver`  |   ✅   | Multilabel name reaches resolver  |
|   5 | `test_malformed_guards`                  |   ✅   | Malformed guards                  |
|   6 | `test_table_add_lookup_case_insensitive` |   ✅   | Table add lookup case insensitive |
|   7 | `test_end_to_end_with_table`             |   ✅   | End to end with table             |
|   8 | `test_dns_opcode_notimp`                 |   ✅   | Dns opcode notimp                 |
|   9 | `test_dns_truncated_questions`           |   ✅   | Dns truncated questions           |
|  10 | `test_dns_oversized_name`                |   ✅   | Dns oversized name                |
|  11 | `test_dns_question_exceeds_out_cap`      |   ✅   | Dns question exceeds out cap      |
|  12 | `test_dns_add_and_lookup_guards`         |   ✅   | Dns add and lookup guards         |
|  13 | `test_dns_begin_host_stub`               |   ✅   | Dns begin host stub               |

</details>

---

## test_rtc - native_rtc - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the DS1307/DS3231 RTC conversions (services/rtc): the BCD time registers_

|   # | Test                            | Status | Description                                                                                |
| --: | :------------------------------ | :----: | :----------------------------------------------------------------------------------------- |
|   1 | `test_known_epoch_2000`         |   ✅   | Known epoch 2000                                                                           |
|   2 | `test_decode_datetime`          |   ✅   | Decode datetime                                                                            |
|   3 | `test_12hour_mode_equivalence`  |   ✅   | 14:00 as 24-hour (0x14) and as 12-hour PM 2 (0x40                                          | 0x20 | 0x02) must be the same time. |
|   4 | `test_12hour_midnight_and_noon` |   ✅   | 12hour midnight and noon                                                                   |
|   5 | `test_roundtrip_over_range`     |   ✅   | Roundtrip over range                                                                       |
|   6 | `test_leap_day`                 |   ✅   | Leap day                                                                                   |
|   7 | `test_masks_ch_and_century`     |   ✅   | The DS1307 clock-halt bit (sec bit7) and the DS3231 century bit (month bit7) must be       |
|   8 | `test_invalid_guards`           |   ✅   | Invalid guards                                                                             |
|   9 | `test_host_i2c_stubs`           |   ✅   | Host build: no I2C bus. begin() reports ready, reads yield 0, set fails, time source is 0. |

</details>

---

## test_relay - native_relay - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the TCP relay / DNAT byte pump (services/relay): bidirectional transfer, the_

|   # | Test                           | Status | Description                                                             |
| --: | :----------------------------- | :----: | :---------------------------------------------------------------------- |
|   1 | `test_bidirectional`           |   ✅   | Bidirectional                                                           |
|   2 | `test_backpressure`            |   ✅   | Backpressure                                                            |
|   3 | `test_half_close_shutdown`     |   ✅   | Half close shutdown                                                     |
|   4 | `test_send_error`              |   ✅   | Send error                                                              |
|   5 | `test_one_way_idle_then_close` |   ✅   | origin never sends; client sends then closes -> relay completes cleanly |
|   6 | `test_note_eof_out_of_band`    |   ✅   | Note eof out of band                                                    |

</details>

---

## test_ld2410 - native_ld2410 - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the LD2410 mmWave radar codec (services/ld2410): decoding a basic and an_

|   # | Test                               | Status | Description                                                    |
| --: | :--------------------------------- | :----: | :------------------------------------------------------------- |
|   1 | `test_parse_basic`                 |   ✅   | Parse basic                                                    |
|   2 | `test_parse_engineering`           |   ✅   | Parse engineering                                              |
|   3 | `test_reject_malformed`            |   ✅   | bad header                                                     |
|   4 | `test_stream_resync_and_split`     |   ✅   | Stream resync and split                                        |
|   5 | `test_stream_absurd_length_drops`  |   ✅   | Stream absurd length drops                                     |
|   6 | `test_helpers`                     |   ✅   | Helpers                                                        |
|   7 | `test_command_encoders`            |   ✅   | Command encoders                                               |
|   8 | `test_host_stubs_and_parse_guards` |   ✅   | Host build: the UART bind functions fail closed / return null. |

</details>

---

## test_sen0192 - native_sen0192 - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SEN0192 microwave motion sensor's pure presence state machine_

|   # | Test                                     | Status | Description                       |
| --: | :--------------------------------------- | :----: | :-------------------------------- |
|   1 | `test_asserts_on_active_and_counts_edge` |   ✅   | Asserts on active and counts edge |
|   2 | `test_holds_then_clears_after_window`    |   ✅   | Holds then clears after window    |
|   3 | `test_reasserts_as_new_event`            |   ✅   | Reasserts as new event            |
|   4 | `test_active_low_polarity`               |   ✅   | Active low polarity               |
|   5 | `test_active_age`                        |   ✅   | Active age                        |

</details>

---

## test_mpr121 - native_mpr121 - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the MPR121 capacitive-touch codec (services/mpr121): decoding the touch-status_

|   # | Test                               | Status | Description                                                                              |
| --: | :--------------------------------- | :----: | :--------------------------------------------------------------------------------------- |
|   1 | `test_touched_decode`              |   ✅   | low byte -> electrodes 0..7; here electrodes 0 and 2.                                    |
|   2 | `test_prox_and_overcurrent_masked` |   ✅   | Proximity (status bit 12 = high-byte bit 4) and OVCF (bit 15 = high-byte bit 7) must not |
|   3 | `test_word10`                      |   ✅   | Word10                                                                                   |
|   4 | `test_build_init_bytes`            |   ✅   | Build init bytes                                                                         |
|   5 | `test_build_init_guards`           |   ✅   | one electrode: 26 fixed + 4 threshold + 8 tail = 38 bytes; ECR enables 1 electrode.      |
|   6 | `test_host_i2c_stubs`              |   ✅   | Host build: no I2C bus. begin() fails, register reads return 0.                          |

</details>

---

## test_sht3x - native_sht3x - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Sensirion SHT3x codec (services/sht3x): the CRC-8 against the datasheet_

|   # | Test                         | Status | Description                                                   |
| --: | :--------------------------- | :----: | :------------------------------------------------------------ |
|   1 | `test_crc8_datasheet_vector` |   ✅   | Crc8 datasheet vector                                         |
|   2 | `test_conversion`            |   ✅   | Endpoints of the linear map are exact.                        |
|   3 | `test_parse_valid`           |   ✅   | Parse valid                                                   |
|   4 | `test_parse_bad_crc`         |   ✅   | Parse bad crc                                                 |
|   5 | `test_parse_null_out`        |   ✅   | Parse null out                                                |
|   6 | `test_host_i2c_stubs`        |   ✅   | Host build: no I2C. begin() fails and read() reports failure. |

</details>

---

## test_pca9685 - native_pca9685 - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the PCA9685 PWM/servo codec (services/pca9685): the PRESCALE computation from a_

|   # | Test                                | Status | Description                                                           |
| --: | :---------------------------------- | :----: | :-------------------------------------------------------------------- |
|   1 | `test_prescale`                     |   ✅   | Prescale                                                              |
|   2 | `test_channel_reg`                  |   ✅   | Channel reg                                                           |
|   3 | `test_us_to_count`                  |   ✅   | Us to count                                                           |
|   4 | `test_set_pwm_bytes`                |   ✅   | channel 0, on=0, off=307 (0x133) -> reg 0x06, off_l 0x33, off_h 0x01. |
|   5 | `test_prescale_zero_and_host_stubs` |   ✅   | Zero frequency takes the max-prescale early return.                   |

</details>

---

## test_ads1115 - native_ads1115 - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the ADS1115 ADC codec (services/ads1115): building the 16-bit config word for a_

|   # | Test                              | Status | Description                                                                                    |
| --: | :-------------------------------- | :----: | :--------------------------------------------------------------------------------------------- |
|   1 | `test_config_word`                |   ✅   | ch0, +/-4.096V, 128 SPS: OS                                                                    | MUX_AIN0 | PGA1 | MODE_SINGLE | DR128 | COMP_DISABLE. |
|   2 | `test_config_fallbacks`           |   ✅   | Out-of-range channel/gain/dr fall back to ch0 / +/-2.048V / 128 SPS = 0xC583.                  |
|   3 | `test_raw_to_uv`                  |   ✅   | gain 1 (+/-4.096 V) -> 125 uV/LSB.                                                             |
|   4 | `test_raw_to_uv_gain_clamp`       |   ✅   | An out-of-range gain code clamps to GAIN_2 (its FSR), so the conversion never indexes past the |
|   5 | `test_host_i2c_stubs_fail_closed` |   ✅   | On a host build there is no I2C: begin and both reads fail closed (false).                     |

</details>

---

## test_ina219 - native_ina219 - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the INA219 current/power codec (services/ina219): decoding the bus-voltage_

|   # | Test                              | Status | Description                                                                                   |
| --: | :-------------------------------- | :----: | :-------------------------------------------------------------------------------------------- |
|   1 | `test_bus_mv`                     |   ✅   | 3300 mV -> value 825 (0x339) in bits [15:3] -> register 825<<3 = 0x19C8.                      |
|   2 | `test_shunt_uv`                   |   ✅   | Shunt uv                                                                                      |
|   3 | `test_calibration`                |   ✅   | Calibration                                                                                   |
|   4 | `test_current_and_power`          |   ✅   | current = raw * current_LSB (uA); power = raw * 20 * current_LSB (uW).                        |
|   5 | `test_host_i2c_stubs_fail_closed` |   ✅   | On a host build there is no I2C: begin and every read fail closed (return false), so a caller |

</details>

---

## test_hpack - native_hpack - ✅ 15 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the HPACK codec (network_drivers/presentation/http2/hpack) against the RFC 7541_

|   # | Test                           | Status | Description                                                         |
| --: | :----------------------------- | :----: | :------------------------------------------------------------------ |
|   1 | `test_hpack_prim_edge_guards`  |   ✅   | Hpack prim edge guards                                              |
|   2 | `test_hpack_more_errors`       |   ✅   | Hpack more errors                                                   |
|   3 | `test_dyn_size_update`         |   ✅   | Dyn size update                                                     |
|   4 | `test_oversize_entry_clears`   |   ✅   | Oversize entry clears                                               |
|   5 | `test_dynamic_name_and_index`  |   ✅   | Dynamic name and index                                              |
|   6 | `test_hpack_decode_errors`     |   ✅   | Hpack decode errors                                                 |
|   7 | `test_hpack_buffer_bounds`     |   ✅   | Hpack buffer bounds                                                 |
|   8 | `test_hpack_encode_paths`      |   ✅   | dws_hpack_dyn_init clamps a too-large max to the table storage.     |
|   9 | `test_int_coding`              |   ✅   | C.1.1: 10, prefix 5 -> 0x0a                                         |
|  10 | `test_huffman`                 |   ✅   | Huffman                                                             |
|  11 | `test_decode_c31_and_index`    |   ✅   | RFC 7541 C.3.1: GET / with :authority www.example.com (no Huffman). |
|  12 | `test_dynamic_eviction`        |   ✅   | Dynamic eviction                                                    |
|  13 | `test_encode_static`           |   ✅   | Encode static                                                       |
|  14 | `test_encode_decode_roundtrip` |   ✅   | Encode decode roundtrip                                             |
|  15 | `test_reject_malformed`        |   ✅   | Reject malformed                                                    |

</details>

---

## test_h2_frame - native_h2frame - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the HTTP/2 frame layer (network_drivers/presentation/http2/dws_h2_frame, RFC 9113):_

|   # | Test                                     | Status | Description                                                       |
| --: | :--------------------------------------- | :----: | :---------------------------------------------------------------- |
|   1 | `test_header_roundtrip`                  |   ✅   | Header roundtrip                                                  |
|   2 | `test_settings_build_parse`              |   ✅   | Settings build parse                                              |
|   3 | `test_settings_validation`               |   ✅   | Settings validation                                               |
|   4 | `test_control_frames`                    |   ✅   | SETTINGS ACK: length 0, type 4, flags ACK, stream 0               |
|   5 | `test_headers_and_data`                  |   ✅   | HEADERS stream 1, one HPACK byte, end_stream -> flags END_HEADERS | END_STREAM = 0x05 |
|   6 | `test_preface`                           |   ✅   | Preface                                                           |
|   7 | `test_settings_all_ids_and_build_guards` |   ✅   | Settings all ids and build guards                                 |

</details>

---

## test_h2_conn - native_h2conn - ✅ 22 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the HTTP/2 connection engine (network_drivers/presentation/http2/dws_h2_conn,_

|   # | Test                                   | Status | Description                     |
| --: | :------------------------------------- | :----: | :------------------------------ |
|   1 | `test_init_and_request`                |   ✅   | Init and request                |
|   2 | `test_respond_roundtrip`               |   ✅   | Respond roundtrip               |
|   3 | `test_ping_and_split_recv`             |   ✅   | Ping and split recv             |
|   4 | `test_bad_preface`                     |   ✅   | Bad preface                     |
|   5 | `test_h2_headers_padded_priority`      |   ✅   | H2 headers padded priority      |
|   6 | `test_h2_headers_pad_overflow`         |   ✅   | H2 headers pad overflow         |
|   7 | `test_h2_stream_id_must_increase`      |   ✅   | H2 stream id must increase      |
|   8 | `test_h2_headers_bad_stream_id`        |   ✅   | H2 headers bad stream id        |
|   9 | `test_h2_stream_table_full_rst`        |   ✅   | H2 stream table full rst        |
|  10 | `test_h2_continuation`                 |   ✅   | H2 continuation                 |
|  11 | `test_h2_continuation_guards`          |   ✅   | H2 continuation guards          |
|  12 | `test_h2_data`                         |   ✅   | H2 data                         |
|  13 | `test_h2_window_update`                |   ✅   | H2 window update                |
|  14 | `test_h2_rst_priority_push`            |   ✅   | H2 rst priority push            |
|  15 | `test_h2_goaway_then_ignore`           |   ✅   | H2 goaway then ignore           |
|  16 | `test_h2_settings_ack_and_bad`         |   ✅   | H2 settings ack and bad         |
|  17 | `test_h2_ping_bad`                     |   ✅   | H2 ping bad                     |
|  18 | `test_h2_frame_too_big`                |   ✅   | H2 frame too big                |
|  19 | `test_h2_respond_paths_and_goaway`     |   ✅   | H2 respond paths and goaway     |
|  20 | `test_h2_more_guards`                  |   ✅   | H2 more guards                  |
|  21 | `test_h2_continuation_more`            |   ✅   | H2 continuation more            |
|  22 | `test_h2_respond_content_type_too_big` |   ✅   | H2 respond content type too big |

</details>

---

## test_quic_varint - native_quic_varint - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the QUIC variable-length integer codec (network_drivers/presentation/http3/_

|   # | Test                         | Status | Description                                                              |
| --: | :--------------------------- | :----: | :----------------------------------------------------------------------- |
|   1 | `test_rfc_examples`          |   ✅   | RFC 9000 Appendix A.1                                                    |
|   2 | `test_non_minimal_decode`    |   ✅   | The RFC's two-byte encoding of 37 must decode to 37 (consuming 2 bytes). |
|   3 | `test_boundaries_and_guards` |   ✅   | Length boundaries.                                                       |

</details>

---

## test_h3_frame - native_h3frame - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the HTTP/3 framing layer (network_drivers/presentation/http3/dws_h3_frame, RFC 9114_

|   # | Test                         | Status | Description                                                     |
| --: | :--------------------------- | :----: | :-------------------------------------------------------------- |
|   1 | `test_header_roundtrip`      |   ✅   | SETTINGS(4), length 0 -> two 1-byte varints.                    |
|   2 | `test_build_data_and_goaway` |   ✅   | Build data and goaway                                           |
|   3 | `test_settings_roundtrip`    |   ✅   | header (type 0x04 + length 0x08) + payload: 01 5000 06 80100000 |
|   4 | `test_reserved`              |   ✅   | Reserved                                                        |
|   5 | `test_build_headers`         |   ✅   | Build headers                                                   |
|   6 | `test_builder_overflow`      |   ✅   | Builder overflow                                                |
|   7 | `test_parse_errors`          |   ✅   | Parse errors                                                    |

</details>

---

## test_jwt - native_jwt - ✅ 22 passed

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
|  17 | `test_time_no_clock_skips_claims` |   ✅   | Time no clock skips claims            |
|  18 | `test_time_exp_enforced`          |   ✅   | Time exp enforced                     |
|  19 | `test_time_nbf_enforced`          |   ✅   | Time nbf enforced                     |
|  20 | `test_time_no_claims_valid`       |   ✅   | Time no claims valid                  |
|  21 | `test_bearer_valid_at`            |   ✅   | Bearer valid at                       |
|  22 | `test_bearer_header_guards`       |   ✅   | Bearer header guards                  |

</details>

---

## test_upload - native_upload - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Streaming file upload (DWS_ENABLE_UPLOAD): a POST body is streamed straight_

|   # | Test                                   | Status | Description                     |
| --: | :------------------------------------- | :----: | :------------------------------ |
|   1 | `test_upload_streams_body_to_file`     |   ✅   | Upload streams body to file     |
|   2 | `test_small_body_single_chunk`         |   ✅   | Small body single chunk         |
|   3 | `test_empty_body_not_streamed`         |   ✅   | Empty body not streamed         |
|   4 | `test_non_post_body_rejected_by_begin` |   ✅   | Non post body rejected by begin |
|   5 | `test_wrong_path_rejected_by_begin`    |   ✅   | Wrong path rejected by begin    |
|   6 | `test_open_failure_replies_500`        |   ✅   | Open failure replies 500        |
|   7 | `test_null_dest_replies_500`           |   ✅   | Null dest replies 500           |
|   8 | `test_write_failure_replies_500`       |   ✅   | Write failure replies 500       |

</details>

---

## test_http_client - native_http_client - ✅ 15 passed

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

## test_compliance - native_compliance - ✅ 15 passed

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

## test_mqtt - native_mqtt - ✅ 22 passed

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

## test_ws_client - native_ws_client - ✅ 17 passed

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
|   9 | `test_build_handshake_subprotocol`  |   ✅   | Build handshake subprotocol           |
|  10 | `test_check_response_ok`            |   ✅   | Check response ok                     |
|  11 | `test_check_response_bad_accept`    |   ✅   | Check response bad accept             |
|  12 | `test_check_response_not_101`       |   ✅   | Check response not 101                |
|  13 | `test_build_frame_masked`           |   ✅   | Build frame masked                    |
|  14 | `test_build_frame_extended_len`     |   ✅   | Build frame extended len              |
|  15 | `test_parse_frame_server_text`      |   ✅   | Server (unmasked) text frame "hello". |
|  16 | `test_parse_frame_incomplete`       |   ✅   | Parse frame incomplete                |
|  17 | `test_parse_frame_extended_len`     |   ✅   | Parse frame extended len              |

</details>

---

## test_scratch - native_scratch - ✅ 15 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the shared per-dispatch scratch arena (session/scratch): bump_

|   # | Test                                                    | Status | Description                                                           |
| --: | :------------------------------------------------------ | :----: | :-------------------------------------------------------------------- |
|   1 | `test_zero_align_uses_default`                          |   ✅   | Zero align uses default                                               |
|   2 | `test_alloc_returns_nonnull_and_advances_used`          |   ✅   | Alloc returns nonnull and advances used                               |
|   3 | `test_sequential_allocs_are_distinct_and_ordered`       |   ✅   | Sequential allocs are distinct and ordered                            |
|   4 | `test_reset_frees_all_and_reuses_base`                  |   ✅   | Reset frees all and reuses base                                       |
|   5 | `test_alignment_is_honored`                             |   ✅   | Alignment is honored                                                  |
|   6 | `test_exhaustion_returns_null_without_corrupting_arena` |   ✅   | Exhaustion returns null without corrupting arena                      |
|   7 | `test_alloc_larger_than_capacity_returns_null`          |   ✅   | Alloc larger than capacity returns null                               |
|   8 | `test_alignment_padding_cannot_overflow_arena`          |   ✅   | Fill to one byte below capacity, then a large-alignment request whose |
|   9 | `test_high_water_bounds`                                |   ✅   | High water bounds                                                     |
|  10 | `test_zero_size_alloc_returns_nonnull_when_space`       |   ✅   | Zero size alloc returns nonnull when space                            |
|  11 | `test_mark_release_reclaims`                            |   ✅   | Mark release reclaims                                                 |
|  12 | `test_release_allows_reuse_of_same_region`              |   ✅   | Release allows reuse of same region                                   |
|  13 | `test_scratch_scope_releases_on_scope_exit`             |   ✅   | Scratch scope releases on scope exit                                  |
|  14 | `test_nested_scopes_reclaim_lifo`                       |   ✅   | Nested scopes reclaim lifo                                            |
|  15 | `test_sequential_scopes_do_not_accumulate`              |   ✅   | Mirrors ssh_pkt_recv's multi-packet loop: each iteration borrows then |

</details>

---

## test_snmp_trap - native_snmp_trap - ✅ 7 passed

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

## test_inflate - native_inflate - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the RFC 1951 INFLATE core (network_drivers/presentation/inflate)._

|   # | Test                                      | Status | Description                                                            |
| --: | :---------------------------------------- | :----: | :--------------------------------------------------------------------- |
|   1 | `test_malformed_deflate_blocks`           |   ✅   | Malformed deflate blocks                                               |
|   2 | `test_fixed_huffman`                      |   ✅   | Fixed huffman                                                          |
|   3 | `test_back_references`                    |   ✅   | Back references                                                        |
|   4 | `test_stored_block`                       |   ✅   | Stored block                                                           |
|   5 | `test_dynamic_huffman`                    |   ✅   | Dynamic huffman                                                        |
|   6 | `test_empty_message`                      |   ✅   | Empty message                                                          |
|   7 | `test_permessage_deflate_marker`          |   ✅   | Permessage deflate marker                                              |
|   8 | `test_permessage_deflate_back_references` |   ✅   | Permessage deflate back references                                     |
|   9 | `test_output_overflow_fails_closed`       |   ✅   | Output overflow fails closed                                           |
|  10 | `test_scratch_too_small_fails_closed`     |   ✅   | Scratch too small fails closed                                         |
|  11 | `test_truncated_input_is_malformed`       |   ✅   | Half of the fixed-Huffman stream: decode runs out of input mid-symbol. |
|  12 | `test_reserved_block_type_is_malformed`   |   ✅   | Reserved block type is malformed                                       |
|  13 | `test_corrupt_stored_nlen_is_malformed`   |   ✅   | Corrupt stored nlen is malformed                                       |
|  14 | `test_inflate_error_paths`                |   ✅   | OVERFLOW: a valid stream decompressed into a buffer that is too small. |

</details>

---

## test_deflate - native_deflate - ✅ 10 passed

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

## test_ssh_zlib - native_ssh_zlib - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SSH server-to-client streaming compressor_

|   # | Test                                          | Status | Description                            |
| --: | :-------------------------------------------- | :----: | :------------------------------------- |
|   1 | `test_session_roundtrip_and_context_takeover` |   ✅   | Session roundtrip and context takeover |
|   2 | `test_empty_payloads`                         |   ✅   | Empty payloads                         |
|   3 | `test_all_byte_values`                        |   ✅   | All byte values                        |
|   4 | `test_window_slide_long_session`              |   ✅   | Window slide long session              |
|   5 | `test_max_input_payload`                      |   ✅   | Max input payload                      |
|   6 | `test_fuzz_stream_roundtrip`                  |   ✅   | Fuzz stream roundtrip                  |
|   7 | `test_fuzz_low_entropy_stream`                |   ✅   | Fuzz low entropy stream                |
|   8 | `test_oversize_input_rejected`                |   ✅   | Oversize input rejected                |
|   9 | `test_output_overflow_fails_closed`           |   ✅   | Output overflow fails closed           |

</details>

---

## test_ssh_comp - native_ssh_comp - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Integration test for SSH server-to-client compression WIRING (network_drivers/presentation/ssh):_

|   # | Test                                     | Status | Description                                                                                     |
| --: | :--------------------------------------- | :----: | :---------------------------------------------------------------------------------------------- |
|   1 | `test_delayed_activation`                |   ✅   | Delayed activation                                                                              |
|   2 | `test_immediate_activation`              |   ✅   | Immediate activation                                                                            |
|   3 | `test_none_never_activates`              |   ✅   | None never activates                                                                            |
|   4 | `test_packet_layer_stream_roundtrip`     |   ✅   | Packet layer stream roundtrip                                                                   |
|   5 | `test_packet_layer_window_slide`         |   ✅   | Packet layer window slide                                                                       |
|   6 | `test_packet_compress_scratch_exhausted` |   ✅   | Packet compress scratch exhausted                                                               |
|   7 | `test_comp_slot_guards`                  |   ✅   | Comp slot guards                                                                                |
|   8 | `test_comp_activation_idempotent`        |   ✅   | zlib: NEWKEYS starts it; a second NEWKEYS is a no-op (s2c_active already true), and USERAUTH is |

</details>

---

## test_websocket - native_ws_deflate - ✅ 74 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit and stress tests for SHA-1, Base64, and the WebSocket frame parser._

|   # | Test                                                   | Status | Description                                                               |
| --: | :----------------------------------------------------- | :----: | :------------------------------------------------------------------------ |
|   1 | `test_sha1_empty_string`                               |   ✅   | Sha1 empty string                                                         |
|   2 | `test_sha1_abc`                                        |   ✅   | Sha1 abc                                                                  |
|   3 | `test_sha1_rfc6455_handshake_key`                      |   ✅   | Client sends: Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==                 |
|   4 | `test_sha1_different_inputs_different_digests`         |   ✅   | Sha1 different inputs different digests                                   |
|   5 | `test_base64_encode_one_byte`                          |   ✅   | Base64 encode one byte                                                    |
|   6 | `test_base64_encode_two_bytes`                         |   ✅   | Base64 encode two bytes                                                   |
|   7 | `test_base64_encode_three_bytes`                       |   ✅   | Base64 encode three bytes                                                 |
|   8 | `test_base64_encode_ws_accept_key`                     |   ✅   | Base64 encode ws accept key                                               |
|   9 | `test_base64_decode_one_byte`                          |   ✅   | Base64 decode one byte                                                    |
|  10 | `test_base64_decode_two_bytes`                         |   ✅   | Base64 decode two bytes                                                   |
|  11 | `test_base64_decode_three_bytes`                       |   ✅   | Base64 decode three bytes                                                 |
|  12 | `test_base64_decode_ws_accept_key`                     |   ✅   | Base64 decode ws accept key                                               |
|  13 | `test_base64_decode_rejects_misplaced_padding`         |   ✅   | Base64 decode rejects misplaced padding                                   |
|  14 | `test_base64_decode_respects_capacity`                 |   ✅   | "TWFu" decodes to 3 bytes ("Man"); a 2-byte buffer is too small.          |
|  15 | `test_base64_round_trip`                               |   ✅   | Base64 round trip                                                         |
|  16 | `test_ws_pool_size`                                    |   ✅   | Ws pool size                                                              |
|  17 | `test_ws_ids_match_indices_after_init`                 |   ✅   | Ws ids match indices after init                                           |
|  18 | `test_ws_all_inactive_after_init`                      |   ✅   | Ws all inactive after init                                                |
|  19 | `test_ws_alloc_returns_non_null`                       |   ✅   | Ws alloc returns non null                                                 |
|  20 | `test_ws_alloc_sets_active`                            |   ✅   | Ws alloc sets active                                                      |
|  21 | `test_ws_alloc_sets_slot_id`                           |   ✅   | Ws alloc sets slot id                                                     |
|  22 | `test_ws_alloc_sets_parse_state_header1`               |   ✅   | Ws alloc sets parse state header1                                         |
|  23 | `test_ws_alloc_pool_full_returns_null`                 |   ✅   | Ws alloc pool full returns null                                           |
|  24 | `test_ws_find_returns_correct_conn`                    |   ✅   | Ws find returns correct conn                                              |
|  25 | `test_ws_find_returns_null_when_empty`                 |   ✅   | Ws find returns null when empty                                           |
|  26 | `test_ws_find_returns_null_for_different_slot`         |   ✅   | Ws find returns null for different slot                                   |
|  27 | `test_ws_find_after_both_slots_allocated`              |   ✅   | Ws find after both slots allocated                                        |
|  28 | `test_ws_free_deactivates_slot`                        |   ✅   | Ws free deactivates slot                                                  |
|  29 | `test_ws_free_restores_ws_id`                          |   ✅   | Ws free restores ws id                                                    |
|  30 | `test_ws_free_makes_slot_findable_as_null`             |   ✅   | Ws free makes slot findable as null                                       |
|  31 | `test_ws_free_nop_on_unallocated`                      |   ✅   | Ws free nop on unallocated                                                |
|  32 | `test_ws_alloc_after_free_succeeds`                    |   ✅   | Ws alloc after free succeeds                                              |
|  33 | `test_ws_parse_text_frame_sets_ready`                  |   ✅   | Ws parse text frame sets ready                                            |
|  34 | `test_ws_parse_payload_stored_correctly`               |   ✅   | Ws parse payload stored correctly                                         |
|  35 | `test_ws_parse_binary_frame_sets_ready`                |   ✅   | Ws parse binary frame sets ready                                          |
|  36 | `test_ws_parse_zero_length_unmasked_frame`             |   ✅   | Unmasked zero-length frame: 0x81 0x00.                                    |
|  37 | `test_ws_parse_zero_length_masked_frame`               |   ✅   | Masked zero-length text frame: FIN                                        | TEXT, MASK                                                 | 0, 4-byte mask key. |
|  38 | `test_ws_reject_unmasked_data_frame`                   |   ✅   | FIN                                                                       | TEXT, unmasked, length 3 - RFC 6455 §5.1 requires masking. |
|  39 | `test_ws_reject_reserved_opcode`                       |   ✅   | Opcode 0x3 is reserved (RFC 6455 §5.2) - must fail the connection.        |
|  40 | `test_ws_reject_fragmented_control_frame`              |   ✅   | PING with FIN=0 - control frames MUST NOT be fragmented (RFC 6455 §5.5).  |
|  41 | `test_ws_reject_oversized_control_frame`               |   ✅   | PING (masked) with payload length 126 - control frames MUST be <= 125     |
|  42 | `test_ws_parse_16bit_length_frame`                     |   ✅   | Build a 130-byte payload (> 125, requires 16-bit length field)            |
|  43 | `test_ws_parse_rsv1_set_closes_protocol`               |   ✅   | FIN=1, RSV1=0x40, TEXT: byte0 = 0x80                                      | 0x40                                                       | 0x01 = 0xC1         |
|  44 | `test_ws_parse_rsv2_set_closes_protocol`               |   ✅   | FIN=1, RSV2=0x20, TEXT: byte0 = 0x80                                      | 0x20                                                       | 0x01 = 0xA1         |
|  45 | `test_ws_parse_rsv3_set_closes_protocol`               |   ✅   | FIN=1, RSV3=0x10, TEXT: byte0 = 0x80                                      | 0x10                                                       | 0x01 = 0x91         |
|  46 | `test_ws_parse_64bit_length_closes_too_big`            |   ✅   | FIN=1, TEXT, MASK=1, len7=127 (64-bit length), then 8 length bytes        |
|  47 | `test_ws_parse_oversized_16bit_length_closes_too_big`  |   ✅   | Ws parse oversized 16bit length closes too big                            |
|  48 | `test_ws_fragment_start_waits_for_continuation`        |   ✅   | FIN=0, TEXT, "Hi" - start of a fragmented message; not deliverable yet.   |
|  49 | `test_ws_fragmented_message_reassembled`               |   ✅   | Ws fragmented message reassembled                                         |
|  50 | `test_ws_control_frame_interleaved_in_fragments`       |   ✅   | A PING arrives between the two data fragments; it must be handled without |
|  51 | `test_ws_fragment_accumulation_overflow_rejected`      |   ✅   | Ws fragment accumulation overflow rejected                                |
|  52 | `test_ws_continuation_without_start_rejected`          |   ✅   | CONTINUATION with no message in progress (RFC 6455 §5.4) → 1002.          |
|  53 | `test_ws_new_data_frame_during_fragmentation_rejected` |   ✅   | A second TEXT (new message) before finishing the first is illegal.        |
|  54 | `test_ws_parse_ping_auto_pong_resets_frame`            |   ✅   | FIN=1, PING=0x09: byte0 = 0x89                                            |
|  55 | `test_ws_parse_pong_silently_ignored`                  |   ✅   | FIN=1, PONG=0x0A: byte0 = 0x8A                                            |
|  56 | `test_ws_parse_close_marks_ws_closed`                  |   ✅   | FIN=1, CLOSE=0x08: byte0 = 0x88                                           |
|  57 | `test_ws_parse_stops_at_frame_ready`                   |   ✅   | Push two complete frames -- parser should stop after the first            |
|  58 | `test_ws_reset_frame_clears_fields`                    |   ✅   | Ws reset frame clears fields                                              |
|  59 | `test_ws_parse_mask_applied_correctly`                 |   ✅   | Frame with mask key {0x37, 0xFA, 0x21, 0x3D}, payload 'H' XOR 0x37 = 0x7F |
|  60 | `test_ws_text_invalid_utf8_rejected`                   |   ✅   | Ws text invalid utf8 rejected                                             |
|  61 | `test_ws_text_valid_utf8_accepted`                     |   ✅   | Ws text valid utf8 accepted                                               |
|  62 | `test_ws_binary_arbitrary_bytes_accepted`              |   ✅   | Ws binary arbitrary bytes accepted                                        |
|  63 | `test_ws_permessage_deflate_inbound`                   |   ✅   | "Hello, World!" as permessage-deflate (SYNC_FLUSH, marker stripped) - the |
|  64 | `test_ws_rsv1_without_negotiation_closes`              |   ✅   | Ws rsv1 without negotiation closes                                        |
|  65 | `test_ws_permessage_deflate_outbound`                  |   ✅   | Ws permessage deflate outbound                                            |
|  66 | `test_ws_deflate_inflate_error_closes`                 |   ✅   | Ws deflate inflate error closes                                           |
|  67 | `test_ws_outbound_incompressible_not_flagged`          |   ✅   | Ws outbound incompressible not flagged                                    |
|  68 | `test_ws_outbound_fragmentation`                       |   ✅   | Ws outbound fragmentation                                                 |
|  69 | `stress_ws_parse_reset_100_cycles`                     |   ✅   | Stress - Ws parse reset 100 cycles                                        |
|  70 | `stress_ws_alloc_free_pool_cycle`                      |   ✅   | Stress - Ws alloc free pool cycle                                         |
|  71 | `stress_ws_parse_incremental_byte_by_byte`             |   ✅   | Stress - Ws parse incremental byte by byte                                |
|  72 | `stress_ws_parse_max_payload`                          |   ✅   | Stress - Ws parse max payload                                             |
|  73 | `stress_ws_parse_two_consecutive_frames`               |   ✅   | First frame                                                               |
|  74 | `test_ws_send_frame_paths_and_parse_guard`             |   ✅   | Ws send frame paths and parse guard                                       |

</details>

---

## test_time_source - native_time_source - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the multi-source time fallback matrix (services/time_source):_

|   # | Test                                       | Status | Description                                                                               |
| --: | :----------------------------------------- | :----: | :---------------------------------------------------------------------------------------- |
|   1 | `test_single_source`                       |   ✅   | Single source                                                                             |
|   2 | `test_priority_order_lowest_value_wins`    |   ✅   | Priority order lowest value wins                                                          |
|   3 | `test_falls_back_when_primary_unavailable` |   ✅   | Falls back when primary unavailable                                                       |
|   4 | `test_all_unavailable_returns_zero`        |   ✅   | All unavailable returns zero                                                              |
|   5 | `test_first_valid_short_circuits`          |   ✅   | First valid short circuits                                                                |
|   6 | `test_fallback_queries_in_priority_order`  |   ✅   | Fallback queries in priority order                                                        |
|   7 | `test_table_full_rejects`                  |   ✅   | Table full rejects                                                                        |
|   8 | `test_null_fn_rejected`                    |   ✅   | Null fn rejected                                                                          |
|   9 | `test_reset_clears_sources`                |   ✅   | Reset clears sources                                                                      |
|  10 | `test_http_date_from_active_source`        |   ✅   | The HTTP Date header draws from the registry: no valid source -> nothing; a source with a |

</details>

---

## test_config_store - native_config_store - ✅ 15 passed

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
|  15 | `test_setter_getter_null_guards`              |   ✅   | Setter getter null guards              |

</details>

---

## test_device_id - native_device_id - ✅ 4 passed

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

## test_auth_lockout - native_auth_lockout - ✅ 12 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the per-peer brute-force auth lockout (services/auth_lockout)._

|   # | Test                                               | Status | Description                                 |
| --: | :------------------------------------------------- | :----: | :------------------------------------------ |
|   1 | `test_below_threshold_not_locked`                  |   ✅   | Below threshold not locked                  |
|   2 | `test_locks_at_threshold`                          |   ✅   | Locks at threshold                          |
|   3 | `test_exponential_backoff`                         |   ✅   | Exponential backoff                         |
|   4 | `test_caps_at_max`                                 |   ✅   | Caps at max                                 |
|   5 | `test_expires_after_window`                        |   ✅   | Expires after window                        |
|   6 | `test_success_clears`                              |   ✅   | Success clears                              |
|   7 | `test_isolates_addresses`                          |   ✅   | Isolates addresses                          |
|   8 | `test_v6_distinct_from_v4_and_each_other`          |   ✅   | V6 distinct from v4 and each other          |
|   9 | `test_zero_ip_never_locked`                        |   ✅   | Zero ip never locked                        |
|  10 | `test_table_full_tracks_new_address`               |   ✅   | Table full tracks new address               |
|  11 | `test_active_lockout_survives_eviction`            |   ✅   | Active lockout survives eviction            |
|  12 | `test_succeed_unspecified_and_table_full_eviction` |   ✅   | Succeed unspecified and table full eviction |

</details>

---

## test_csrf - native_csrf - ✅ 10 passed

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
|  10 | `test_reset_and_verify_guards`       |   ✅   | Reset and verify guards       |

</details>

---

## test_telemetry - native_telemetry - ✅ 8 passed

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

## test_dashboard - native_dashboard - ✅ 15 passed

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

## test_net_egress - native_net_egress - ✅ 6 passed

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

## test_partition_monitor - native_partition - ✅ 6 passed

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
|   6 | `test_partition_kind_data_subtypes`   |   ✅   | Partition kind data subtypes   |

</details>

---

## test_cbor - native_cbor - ✅ 21 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CBOR encoder (network_drivers/presentation/cbor). Expected_

|   # | Test                                | Status | Description                  |
| --: | :---------------------------------- | :----: | :--------------------------- |
|   1 | `test_cbor_decode_more_types`       |   ✅   | Cbor decode more types       |
|   2 | `test_cbor_head_reserved_and_trunc` |   ✅   | Cbor head reserved and trunc |
|   3 | `test_cbor_read_empty`              |   ✅   | Cbor read empty              |
|   4 | `test_uint`                         |   ✅   | Uint                         |
|   5 | `test_peek_each_type`               |   ✅   | Peek each type               |
|   6 | `test_uint_8byte`                   |   ✅   | Uint 8byte                   |
|   7 | `test_read_double_encoded_float`    |   ✅   | Read double encoded float    |
|   8 | `test_read_map_type_mismatch`       |   ✅   | Read map type mismatch       |
|   9 | `test_int`                          |   ✅   | Int                          |
|  10 | `test_text`                         |   ✅   | Text                         |
|  11 | `test_bytes`                        |   ✅   | Bytes                        |
|  12 | `test_simple`                       |   ✅   | Simple                       |
|  13 | `test_float`                        |   ✅   | Float                        |
|  14 | `test_array_and_map`                |   ✅   | Array and map                |
|  15 | `test_overflow_fails_closed`        |   ✅   | Overflow fails closed        |
|  16 | `test_decode_uint`                  |   ✅   | Decode uint                  |
|  17 | `test_decode_int`                   |   ✅   | Decode int                   |
|  18 | `test_decode_float_roundtrip`       |   ✅   | Decode float roundtrip       |
|  19 | `test_decode_roundtrip_map`         |   ✅   | Decode roundtrip map         |
|  20 | `test_decode_truncated`             |   ✅   | Decode truncated             |
|  21 | `test_decode_type_mismatch`         |   ✅   | Decode type mismatch         |

</details>

---

## test_msgpack - native_msgpack - ✅ 23 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the MessagePack encoder and decoder_

|   # | Test                            | Status | Description                                                                 |
| --: | :------------------------------ | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_encode_wide32`            |   ✅   | Encode wide32                                                               |
|   2 | `test_peek_wide_types`          |   ✅   | Peek wide types                                                             |
|   3 | `test_read_int_all_widths`      |   ✅   | Read int all widths                                                         |
|   4 | `test_read_on_empty_reader`     |   ✅   | Read on empty reader                                                        |
|   5 | `test_read_wrong_type_byte`     |   ✅   | Read wrong type byte                                                        |
|   6 | `test_read_truncated_widths`    |   ✅   | Read truncated widths                                                       |
|   7 | `test_uint`                     |   ✅   | Uint                                                                        |
|   8 | `test_wide_roundtrip`           |   ✅   | Wide roundtrip                                                              |
|   9 | `test_decode_wide_fails_closed` |   ✅   | str16 header claims 300 bytes, body absent                                  |
|  10 | `test_int`                      |   ✅   | Int                                                                         |
|  11 | `test_str`                      |   ✅   | Str                                                                         |
|  12 | `test_bytes`                    |   ✅   | Bytes                                                                       |
|  13 | `test_simple`                   |   ✅   | Simple                                                                      |
|  14 | `test_float`                    |   ✅   | Float                                                                       |
|  15 | `test_array_and_map`            |   ✅   | Array and map                                                               |
|  16 | `test_overflow_fails_closed`    |   ✅   | Overflow fails closed                                                       |
|  17 | `test_decode_uint`              |   ✅   | positive fixint, uint8, uint16, uint32, uint64                              |
|  18 | `test_decode_int`               |   ✅   | negative fixint (-1, -32), int8 (-128), int16 (-32768), int32 (-2147483648) |
|  19 | `test_decode_str_and_bytes`     |   ✅   | Decode str and bytes                                                        |
|  20 | `test_decode_simple_and_float`  |   ✅   | Decode simple and float                                                     |
|  21 | `test_decode_array_and_map`     |   ✅   | Decode array and map                                                        |
|  22 | `test_decode_roundtrip`         |   ✅   | Encode a small document, then decode it back and check each field.          |
|  23 | `test_decode_fails_closed`      |   ✅   | truncated uint16 (header says read 2 more bytes, only 1 present)            |

</details>

---

## test_gpio_map - native_gpio_map - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the GPIO pin-mapper core (services/gpio_map): the direction_

|   # | Test                                  | Status | Description                                                            |
| --: | :------------------------------------ | :----: | :--------------------------------------------------------------------- |
|   1 | `test_dir_name`                       |   ✅   | Dir name                                                               |
|   2 | `test_json`                           |   ✅   | Json                                                                   |
|   3 | `test_json_empty`                     |   ✅   | Json empty                                                             |
|   4 | `test_json_small_buffer_fails_closed` |   ✅   | Json small buffer fails closed                                         |
|   5 | `test_parse_set`                      |   ✅   | Parse set                                                              |
|   6 | `test_parse_set_rejects_partial`      |   ✅   | Parse set rejects partial                                              |
|   7 | `test_parse_set_no_prefix_match`      |   ✅   | "spin=2" must not satisfy the "pin" field (field-boundary check).      |
|   8 | `test_is_output`                      |   ✅   | Is output                                                              |
|   9 | `test_host_gpio_stubs`                |   ✅   | Host build: the GPIO bind functions are no-ops (no digitalRead/Write). |

</details>

---

## test_udp_telemetry - native_udp_telemetry - ✅ 8 passed

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
|   8 | `test_host_stubs_and_line_overflow` |   ✅   | Host stubs and line overflow |

</details>

---

## test_statsd - native_statsd - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the StatsD client (services/statsd): the pure line formatter_

|   # | Test                                 | Status | Description                                                                            |
| --: | :----------------------------------- | :----: | :------------------------------------------------------------------------------------- |
|   1 | `test_format_types`                  |   ✅   | Format types                                                                           |
|   2 | `test_format_sample_rate`            |   ✅   | Format sample rate                                                                     |
|   3 | `test_format_tags_and_both`          |   ✅   | Format tags and both                                                                   |
|   4 | `test_format_guards`                 |   ✅   | Format guards                                                                          |
|   5 | `test_emit_counter_and_negative`     |   ✅   | Emit counter and negative                                                              |
|   6 | `test_emit_gauge_and_delta`          |   ✅   | Emit gauge and delta                                                                   |
|   7 | `test_emit_timing_set_sampled`       |   ✅   | Emit timing set sampled                                                                |
|   8 | `test_emit_global_tags`              |   ✅   | Emit global tags                                                                       |
|   9 | `test_emit_noop_until_begin`         |   ✅   | Emit noop until begin                                                                  |
|  10 | `test_rate_clamp_and_stage_overflow` |   ✅   | A rate rounding below one thousandth clamps up to 1; a rate near 1 clamps down to 999. |

</details>

---

## test_guardrails - native_guardrails - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the guardrails core (services/guardrails): the threshold_

|   # | Test                                  | Status | Description                                                                                    |
| --: | :------------------------------------ | :----: | :--------------------------------------------------------------------------------------------- |
|   1 | `test_eval_all_clear`                 |   ✅   | Eval all clear                                                                                 |
|   2 | `test_eval_heap_breach`               |   ✅   | Eval heap breach                                                                               |
|   3 | `test_eval_frag_and_stack`            |   ✅   | Eval frag and stack                                                                            |
|   4 | `test_eval_all_breached`              |   ✅   | Eval all breached                                                                              |
|   5 | `test_json`                           |   ✅   | Json                                                                                           |
|   6 | `test_json_small_buffer_fails_closed` |   ✅   | Json small buffer fails closed                                                                 |
|   7 | `test_eval_null_health_is_clear`      |   ✅   | A null health snapshot reports no breach (nothing to evaluate).                                |
|   8 | `test_json_guards_fail_closed`        |   ✅   | Null out or zero cap -> 0 (nothing written).                                                   |
|   9 | `test_host_sampler_stubs`             |   ✅   | On host there are no live counters: sample() zeroes the snapshot (and no-ops on null), begin() |

</details>

---

## test_failsafe - native_failsafe - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/failsafe: the software watchdog / deadlock detector. Uses the explicit_

|   # | Test                                         | Status | Description                                                                                   |
| --: | :------------------------------------------- | :----: | :-------------------------------------------------------------------------------------------- |
|   1 | `test_overdue_predicate`                     |   ✅   | Overdue predicate                                                                             |
|   2 | `test_register_and_not_overdue_when_fresh`   |   ✅   | Register and not overdue when fresh                                                           |
|   3 | `test_breach_fires_once_then_clears_on_feed` |   ✅   | b has a huge deadline so it never trips during this test - a stays the only overdue lifeline. |
|   4 | `test_registry_full`                         |   ✅   | Registry full                                                                                 |
|   5 | `test_feed_bad_id`                           |   ✅   | Feed bad id                                                                                   |
|   6 | `test_json`                                  |   ✅   | Json                                                                                          |
|   7 | `test_millis_wrappers_and_json`              |   ✅   | Millis wrappers and json                                                                      |

</details>

---

## test_sleep_sched - native_sleep_sched - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/sleep_sched: the dynamic sleep-cycle decision core. Pure, synthetic clock._

|   # | Test                            | Status | Description                                                                            |
| --: | :------------------------------ | :----: | :------------------------------------------------------------------------------------- |
|   1 | `test_awake_when_recent`        |   ✅   | idle 999 < 1000 -> stay awake.                                                         |
|   2 | `test_min_window_at_threshold`  |   ✅   | idle exactly 1000: past threshold, 0 doublings -> the floor.                           |
|   3 | `test_ramp_doubles`             |   ✅   | idle 1500: one ramp period (500) past threshold -> 100<<1 = 200.                       |
|   4 | `test_clamps_to_ceiling`        |   ✅   | idle 10000: many periods, clamped to max_ms = 2000 (not 100<<18).                      |
|   5 | `test_no_ramp_jumps_to_ceiling` |   ✅   | No ramp jumps to ceiling                                                               |
|   6 | `test_degenerate_max_below_min` |   ✅   | Degenerate max below min                                                               |
|   7 | `test_wrap_safe`                |   ✅   | last_active just before the millis() rollover, now just after: real idle 1284 >= 1000. |
|   8 | `test_null_cfg`                 |   ✅   | Null cfg                                                                               |

</details>

---

## test_wearlevel - native_wearlevel - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/wearlevel: the flash wear-leveling slot selector._

|   # | Test                                     | Status | Description                                                                        |
| --: | :--------------------------------------- | :----: | :--------------------------------------------------------------------------------- |
|   1 | `test_pick_least_worn_ties_lowest_index` |   ✅   | Pick least worn ties lowest index                                                  |
|   2 | `test_pick_edge`                         |   ✅   | Pick edge                                                                          |
|   3 | `test_pick_plus_mark_levels_the_region`  |   ✅   | Repeated pick+mark must keep every slot within 1 of the others (round-robin wear). |
|   4 | `test_mark_saturates_and_bounds`         |   ✅   | Mark saturates and bounds                                                          |
|   5 | `test_spread`                            |   ✅   | Spread                                                                             |

</details>

---

## test_netadapt - native_netadapt - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/netadapt: TCP window sizing by free RAM + DHCP->static fallback._

|   # | Test                                   | Status | Description                                                              |
| --: | :------------------------------------- | :----: | :----------------------------------------------------------------------- |
|   1 | `test_window_floor_when_low_heap`      |   ✅   | heap at or below the reserve -> the floor.                               |
|   2 | `test_window_scales_with_heap`         |   ✅   | (free - reserve)/4, clamped. free=40000, reserve=8000 -> 32000/4 = 8000. |
|   3 | `test_window_clamps_to_ceiling`        |   ✅   | Huge heap -> clamped to max_win.                                         |
|   4 | `test_window_degenerate_max_below_min` |   ✅   | Window degenerate max below min                                          |
|   5 | `test_dhcp_fallback_on_timeout`        |   ✅   | Dhcp fallback on timeout                                                 |
|   6 | `test_dhcp_fallback_on_attempts`       |   ✅   | Dhcp fallback on attempts                                                |

</details>

---

## test_dshot - native_dshot - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/dshot: the DShot ESC throttle frame codec (hand-computed vectors)._

|   # | Test                                    | Status | Description                                                                          |
| --: | :-------------------------------------- | :----: | :----------------------------------------------------------------------------------- |
|   1 | `test_encode_known_vector`              |   ✅   | Encode known vector                                                                  |
|   2 | `test_encode_telemetry_bit`             |   ✅   | value 1046, telemetry set: v12 = 0x82D, nibbles 8^2^D = 7, frame = 0x82D7.           |
|   3 | `test_encode_bidirectional_inverts_crc` |   ✅   | Same value, bidirectional: crc = ~6 & 0xF = 9, frame = 0x82C9.                       |
|   4 | `test_value_masked_to_11_bits`          |   ✅   | 0xF000                                                                               | 1046: the high bits are dropped to the 11-bit field -> same as 1046. |
|   5 | `test_decode_roundtrip_and_crc`         |   ✅   | Decode roundtrip and crc                                                             |
|   6 | `test_bit_timing`                       |   ✅   | 600 kbit: period 1667 ns; "1" ~3/4, "0" ~3/8.                                        |
|   7 | `test_esc_pwm_mapping`                  |   ✅   | OneShot125: 125..250 us.                                                             |
|   8 | `test_bit_ns_all_rates`                 |   ✅   | Each supported line rate maps to a non-zero bit period; an unknown rate is rejected. |

</details>

---

## test_hart - native_hart - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/hart: the HART command frame + HART-IP header codec._

|   # | Test                                    | Status | Description                                                               |
| --: | :-------------------------------------- | :----: | :------------------------------------------------------------------------ |
|   1 | `test_checksum`                         |   ✅   | XOR longitudinal parity.                                                  |
|   2 | `test_build_command0_short`             |   ✅   | Command 0 (read unique id), STX, primary-master short address 0, no data. |
|   3 | `test_build_with_data`                  |   ✅   | [02 80 01 02 AB CD ck], ck = 02^80^01^02^AB^CD = 0xE7.                    |
|   4 | `test_build_long_address`               |   ✅   | Build long address                                                        |
|   5 | `test_parse_roundtrip_and_bad_checksum` |   ✅   | Parse roundtrip and bad checksum                                          |
|   6 | `test_hartip_header`                    |   ✅   | Hartip header                                                             |
|   7 | `test_build_and_parse_guards`           |   ✅   | Build and parse guards                                                    |

</details>

---

## test_nts - native_nts - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/nts: the NTS-KE record + NTS NTP extension-field wire codec (RFC 8915)._

|   # | Test                           | Status | Description                                                                      |
| --: | :----------------------------- | :----: | :------------------------------------------------------------------------------- |
|   1 | `test_ke_record`               |   ✅   | Ke record                                                                        |
|   2 | `test_ke_request`              |   ✅   | Next-Protocol(NTPv4) + AEAD(AES-SIV-CMAC-256=15) + End-of-Message, all critical. |
|   3 | `test_ke_parse`                |   ✅   | Ke parse                                                                         |
|   4 | `test_extension_field_padding` |   ✅   | 32-byte unique id: 4 + 32 = 36, already a multiple of 4.                         |
|   5 | `test_ef_wrappers_and_guards`  |   ✅   | Ef wrappers and guards                                                           |

</details>

---

## test_dds - native_dds - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/dds: the RTPS message + submessage framing codec._

|   # | Test                         | Status | Description                                                       |
| --: | :--------------------------- | :----: | :---------------------------------------------------------------- |
|   1 | `test_header`                |   ✅   | Header                                                            |
|   2 | `test_submessage_endianness` |   ✅   | Little-endian (E flag set): octetsToNextHeader = 0x0008 -> 08 00. |
|   3 | `test_parse_message`         |   ✅   | Parse message                                                     |
|   4 | `test_parse_rejects`         |   ✅   | Parse rejects                                                     |
|   5 | `test_rtps_build_guards`     |   ✅   | Rtps build guards                                                 |

</details>

---

## test_xmpp - native_xmpp - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/xmpp: the XMPP stanza builder + minimal reader._

|   # | Test                                         | Status | Description                                                                                       |
| --: | :------------------------------------------- | :----: | :------------------------------------------------------------------------------------------------ |
|   1 | `test_escape`                                |   ✅   | Escape                                                                                            |
|   2 | `test_message`                               |   ✅   | Message                                                                                           |
|   3 | `test_presence`                              |   ✅   | Presence                                                                                          |
|   4 | `test_iq`                                    |   ✅   | Iq                                                                                                |
|   5 | `test_stanza_name`                           |   ✅   | Stanza name                                                                                       |
|   6 | `test_attr`                                  |   ✅   | Attr                                                                                              |
|   7 | `test_escape_all_entities_and_overflow`      |   ✅   | Every escapable character plus a normal one exercises each switch case in put_escaped.            |
|   8 | `test_builders_overflow_fail_closed`         |   ✅   | Builders overflow fail closed                                                                     |
|   9 | `test_builders_omit_optional_and_null_attrs` |   ✅   | body/child null skip the optional block; null attr values skip put_attr (its `!value` true side). |
|  10 | `test_stanza_name_edges`                     |   ✅   | Each terminator: '>', '/', space, tab, newline.                                                   |
|  11 | `test_attr_edges`                            |   ✅   | Single-quoted value + the leading-space substring guard (must not match 'to' inside 'xto').       |

</details>

---

## test_rawl2 - native_rawl2 - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/rawl2: the Ethernet II / 802.1Q frame codec + the FCS._

|   # | Test                          | Status | Description                                                        |
| --: | :---------------------------- | :----: | :----------------------------------------------------------------- |
|   1 | `test_build_ethernet_ii`      |   ✅   | Build ethernet ii                                                  |
|   2 | `test_build_vlan`             |   ✅   | pcp 3, dei 0, vid 100 -> TCI 0x6064; PROFINET ethertype.           |
|   3 | `test_parse`                  |   ✅   | Parse                                                              |
|   4 | `test_fcs_check_vector`       |   ✅   | The canonical CRC-32 check value: CRC of "123456789" = 0xCBF43926. |
|   5 | `test_eth_build_parse_guards` |   ✅   | Eth build parse guards                                             |

</details>

---

## test_spa_router - native_spa_router - ✅ 2 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/spa_router: the single-page-app routing decision._

|   # | Test                 | Status | Description   |
| --: | :------------------- | :----: | :------------ |
|   1 | `test_has_extension` |   ✅   | Has extension |
|   2 | `test_route`         |   ✅   | Route         |

</details>

---

## test_goose - native_goose - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/goose: the IEC 61850 GOOSE BER PDU + Ethernet frame codec._

|   # | Test                            | Status | Description                                                              |
| --: | :------------------------------ | :----: | :----------------------------------------------------------------------- |
|   1 | `test_pdu_structure`            |   ✅   | Content is 42 octets (see goose.cpp field sizes); PDU = 61 2A <42> = 44. |
|   2 | `test_integer_leading_zero`     |   ✅   | Integer leading zero                                                     |
|   3 | `test_frame`                    |   ✅   | Frame                                                                    |
|   4 | `test_goose_error_and_longform` |   ✅   | Goose error and longform                                                 |

</details>

---

## test_mtconnect - native_mtconnect - ✅ 12 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/mtconnect: the MTConnectStreams + MTConnectError document builders._

|   # | Test                                 | Status | Description                   |
| --: | :----------------------------------- | :----: | :---------------------------- |
|   1 | `test_streams_document`              |   ✅   | Streams document              |
|   2 | `test_streams_escapes_value`         |   ✅   | Streams escapes value         |
|   3 | `test_error_document`                |   ✅   | Error document                |
|   4 | `test_overflow_returns_zero`         |   ✅   | Overflow returns zero         |
|   5 | `test_escape_gt_quote_and_overflow`  |   ✅   | Escape gt quote and overflow  |
|   6 | `test_devices_probe_document`        |   ✅   | Devices probe document        |
|   7 | `test_devices_escape_and_overflow`   |   ✅   | Devices escape and overflow   |
|   8 | `test_assets_document`               |   ✅   | Assets document               |
|   9 | `test_assets_escape_and_overflow`    |   ✅   | Assets escape and overflow    |
|  10 | `test_sample_buffer_and_query`       |   ✅   | Sample buffer and query       |
|  11 | `test_sample_buffer_eviction`        |   ✅   | Sample buffer eviction        |
|  12 | `test_sample_query_future_and_empty` |   ✅   | Sample query future and empty |

</details>

---

## test_wal - native_wal - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/wal: record framing + CRC32 + crash-recovery replay (the atomicity core)._

|   # | Test                                                | Status | Description                                                                    |
| --: | :-------------------------------------------------- | :----: | :----------------------------------------------------------------------------- |
|   1 | `test_crc32_known_vector`                           |   ✅   | The canonical CRC-32/ISO-HDLC check value for "123456789".                     |
|   2 | `test_encode_replay_roundtrip`                      |   ✅   | Encode replay roundtrip                                                        |
|   3 | `test_replay_recovers_to_last_good_on_corrupt_tail` |   ✅   | Corrupt a payload byte of the third record -> its CRC now fails.               |
|   4 | `test_replay_stops_on_truncated_tail`               |   ✅   | Simulate a power loss mid-write of record 2: only part of it made it to media. |
|   5 | `test_encode_capacity_and_empty_payload`            |   ✅   | Exactly fits a 3-byte payload.                                                 |
|   6 | `test_replay_empty_and_garbage`                     |   ✅   | Replay empty and garbage                                                       |

</details>

---

## test_wal_store - native_wal - ✅ 29 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/wal dws_wal_store: A/B superblock + checkpoint + mount/recover over a RAM device._

|   # | Test                                               | Status | Description                                 |
| --: | :------------------------------------------------- | :----: | :------------------------------------------ |
|   1 | `test_format_then_mount_empty`                     |   ✅   | Format then mount empty                     |
|   2 | `test_mount_unformatted_fails`                     |   ✅   | Mount unformatted fails                     |
|   3 | `test_append_without_checkpoint_recovers_via_tail` |   ✅   | Append without checkpoint recovers via tail |
|   4 | `test_checkpoint_commits_then_tail`                |   ✅   | Checkpoint commits then tail                |
|   5 | `test_torn_tail_recovers_to_last_good`             |   ✅   | Torn tail recovers to last good             |
|   6 | `test_ab_superblock_fallback`                      |   ✅   | Ab superblock fallback                      |
|   7 | `test_append_full_fails_closed`                    |   ✅   | Append full fails closed                    |
|   8 | `test_format_and_mount_too_small`                  |   ✅   | Format and mount too small                  |
|   9 | `test_format_write_b_unwired_fails`                |   ✅   | Format write b unwired fails                |
|  10 | `test_format_write_super_a_fails`                  |   ✅   | Format write super a fails                  |
|  11 | `test_null_sync_still_commits`                     |   ✅   | Null sync still commits                     |
|  12 | `test_mount_read_unwired_fails`                    |   ✅   | Mount read unwired fails                    |
|  13 | `test_mount_super_crc_mismatch`                    |   ✅   | Mount super crc mismatch                    |
|  14 | `test_mount_head_past_capacity_rejected`           |   ✅   | Mount head past capacity rejected           |
|  15 | `test_replay_truncated_len_stops`                  |   ✅   | Replay truncated len stops                  |
|  16 | `test_replay_header_read_fails`                    |   ✅   | Replay header read fails                    |
|  17 | `test_replay_payload_read_fails`                   |   ✅   | Replay payload read fails                   |
|  18 | `test_append_header_write_fails`                   |   ✅   | Append header write fails                   |
|  19 | `test_append_payload_write_fails`                  |   ✅   | Append payload write fails                  |
|  20 | `test_checkpoint_super_write_fails`                |   ✅   | Checkpoint super write fails                |
|  21 | `test_checkpoint_second_sync_fails`                |   ✅   | Checkpoint second sync fails                |
|  22 | `test_scan_reads_records`                          |   ✅   | Scan reads records                          |
|  23 | `test_scan_null_callback_counts`                   |   ✅   | Scan null callback counts                   |
|  24 | `test_scan_scratch_too_small`                      |   ✅   | Scan scratch too small                      |
|  25 | `test_scan_header_read_fails`                      |   ✅   | Scan header read fails                      |
|  26 | `test_scan_full_read_fails`                        |   ✅   | Scan full read fails                        |
|  27 | `test_scan_bad_magic_stops`                        |   ✅   | Scan bad magic stops                        |
|  28 | `test_scan_crc_mismatch_stops`                     |   ✅   | Scan crc mismatch stops                     |
|  29 | `test_pread_in_and_out_of_range`                   |   ✅   | Pread in and out of range                   |

</details>

---

## test_j2735 - native_j2735 - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/j2735: the ASN.1 UPER primitive codec + the BSMcore block._

|   # | Test                               | Status | Description                                                                                      |
| --: | :--------------------------------- | :----: | :----------------------------------------------------------------------------------------------- |
|   1 | `test_cint_bits`                   |   ✅   | Cint bits                                                                                        |
|   2 | `test_bit_writer_pattern`          |   ✅   | Write 0b101 (3 bits) then 0b11 (2 bits): stream 10111 000 -> 0xB8.                               |
|   3 | `test_writer_null_and_zero`        |   ✅   | A null buffer (or zero cap) leaves the writer not-ok and must not dereference it.                |
|   4 | `test_cint_roundtrip`              |   ✅   | Cint roundtrip                                                                                   |
|   5 | `test_bsm_core_roundtrip`          |   ✅   | Bsm core roundtrip                                                                               |
|   6 | `test_bsm_core_bit_length`         |   ✅   | msgCnt 7 + id 32 + secMark 16 + lat 31 + long 32 + elev 16 + speed 13 + heading 15 = 162 bits    |
|   7 | `test_spat_roundtrip`              |   ✅   | Spat roundtrip                                                                                   |
|   8 | `test_spat_decode_too_many`        |   ✅   | Only room for 1 but 2 encoded -> false.                                                          |
|   9 | `test_map_roundtrip`               |   ✅   | Map roundtrip                                                                                    |
|  10 | `test_uper_overflow_and_bsm_guard` |   ✅   | Uper overflow and bsm guard                                                                      |
|  11 | `test_j2735_guards_and_truncation` |   ✅   | dws_uper_put_cint / dws_uper_get_cint with a single-value (zero-bit) range: nothing on the wire. |

</details>

---

## test_nema_ts2 - native_nema_ts2 - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/nema_ts2: the TS 2 SDLC frame codec + CRC-16/X-25._

|   # | Test                                   | Status | Description                                                     |
| --: | :------------------------------------- | :----: | :-------------------------------------------------------------- |
|   1 | `test_crc_check_vector`                |   ✅   | CRC-16/X-25 canonical check value: CRC of "123456789" = 0x906E. |
|   2 | `test_build_and_parse`                 |   ✅   | Build and parse                                                 |
|   3 | `test_no_data_frame`                   |   ✅   | No data frame                                                   |
|   4 | `test_parse_rejects_bad_crc_and_short` |   ✅   | Parse rejects bad crc and short                                 |

</details>

---

## test_snp - native_snp - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/snp: the GE Fanuc SNP serial frame codec._

|   # | Test                    | Status | Description                                            |
| --: | :---------------------- | :----: | :----------------------------------------------------- |
|   1 | `test_bcc`              |   ✅   | sum = 0x01+0x03+0x10+0x20+0x30 = 0x64.                 |
|   2 | `test_build_and_parse`  |   ✅   | [01][03][10 20 30][BCC] ; BCC = 01+03+10+20+30 = 0x64. |
|   3 | `test_empty_data`       |   ✅   | Empty data                                             |
|   4 | `test_parse_rejects`    |   ✅   | Parse rejects                                          |
|   5 | `test_snp_build_guards` |   ✅   | Snp build guards                                       |

</details>

---

## test_directnet - native_directnet - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/directnet: the DirectNET serial frame codec._

|   # | Test                        | Status | Description                                                               |
| --: | :-------------------------- | :----: | :------------------------------------------------------------------------ |
|   1 | `test_lrc`                  |   ✅   | Lrc                                                                       |
|   2 | `test_header_frame`         |   ✅   | SOH(1) + slave(2) + type(1) + addr(4) + blocks(2) + ETB(1) + LRC(1) = 12. |
|   3 | `test_data_frame_roundtrip` |   ✅   | STX + ABCD + ETX + LRC = 7.                                               |
|   4 | `test_data_parse_rejects`   |   ✅   | Data parse rejects                                                        |
|   5 | `test_guards`               |   ✅   | Guards                                                                    |

</details>

---

## test_sep2 - native_sep2 - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/sep2: the IEEE 2030.5 resource document builders._

|   # | Test                                 | Status | Description                   |
| --: | :----------------------------------- | :----: | :---------------------------- |
|   1 | `test_device_capability`             |   ✅   | Device capability             |
|   2 | `test_end_device`                    |   ✅   | End device                    |
|   3 | `test_der_control_negative_setpoint` |   ✅   | Der control negative setpoint |
|   4 | `test_xml_escape_in_href`            |   ✅   | Xml escape in href            |
|   5 | `test_overflow`                      |   ✅   | Overflow                      |

</details>

---

## test_profinet - native_profinet - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/profinet: the PROFINET DCP frame codec._

|   # | Test                          | Status | Description                                                                                    |
| --: | :---------------------------- | :----: | :--------------------------------------------------------------------------------------------- |
|   1 | `test_header_roundtrip`       |   ✅   | Header roundtrip                                                                               |
|   2 | `test_block_even_padding`     |   ✅   | NameOfStation "plc" is 3 bytes (odd) -> padded to an even total, filler not counted in length. |
|   3 | `test_walk_blocks`            |   ✅   | Walk blocks                                                                                    |
|   4 | `test_walk_rejects_truncated` |   ✅   | blockLength claims 10 but only 2 value bytes present.                                          |
|   5 | `test_pn_guards`              |   ✅   | Pn guards                                                                                      |

</details>

---

## test_ntcip - native_ntcip - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/ntcip: the NTCIP object OID definitions + the OID builder._

|   # | Test                                | Status | Description                                       |
| --: | :---------------------------------- | :----: | :------------------------------------------------ |
|   1 | `test_roots_under_nema`             |   ✅   | Every NTCIP object is under 1.3.6.1.4.1.1206.4.2. |
|   2 | `test_oid_builder_scalar_and_index` |   ✅   | A scalar takes .0.                                |
|   3 | `test_oid_builder_overflow`         |   ✅   | Oid builder overflow                              |

</details>

---

## test_openadr - native_openadr - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/openadr: the OpenADR 3.0 event / report JSON builders._

|   # | Test                               | Status | Description                 |
| --: | :--------------------------------- | :----: | :-------------------------- |
|   1 | `test_event`                       |   ✅   | Event                       |
|   2 | `test_report_negative_value`       |   ✅   | Report negative value       |
|   3 | `test_json_escape`                 |   ✅   | Json escape                 |
|   4 | `test_overflow`                    |   ✅   | Overflow                    |
|   5 | `test_openadr_escape_and_overflow` |   ✅   | Openadr escape and overflow |

</details>

---

## test_mms - native_mms - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/mms: the IEC 61850 MMS Read PDU codec._

|   # | Test                                       | Status | Description                                                                           |
| --: | :----------------------------------------- | :----: | :------------------------------------------------------------------------------------ |
|   1 | `test_read_request_structure`              |   ✅   | Read request structure                                                                |
|   2 | `test_read_request_parse`                  |   ✅   | Read request parse                                                                    |
|   3 | `test_read_response_roundtrip`             |   ✅   | A caller-encoded Data value: boolean-ish [3] BOOLEAN true -> 83 01 FF (context Data). |
|   4 | `test_parse_rejects_bad_tag`               |   ✅   | Parse rejects bad tag                                                                 |
|   5 | `test_invoke_id_zero_and_msb`              |   ✅   | id 0 -> int_content emits {0x00}; round-trips back to 0.                              |
|   6 | `test_read_request_bad_args`               |   ✅   | Read request bad args                                                                 |
|   7 | `test_read_request_long_name_long_form`    |   ✅   | Read request long name long form                                                      |
|   8 | `test_read_response_bad_args_and_overflow` |   ✅   | data_len set but data null -> reject.                                                 |
|   9 | `test_parse_null_and_short`                |   ✅   | Parse null and short                                                                  |
|  10 | `test_parse_malformed`                     |   ✅   | Outer length in long form but the count byte is malformed (nb == 0).                  |
|  11 | `test_parse_no_service`                    |   ✅   | Parse no service                                                                      |

</details>

---

## test_cclink - native_cclink - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/cclink: the CC-Link cyclic frame codec + process-image accessors._

|   # | Test                             | Status | Description               |
| --: | :------------------------------- | :----: | :------------------------ |
|   1 | `test_sum`                       |   ✅   | Sum                       |
|   2 | `test_build_and_parse`           |   ✅   | Build and parse           |
|   3 | `test_bit_accessors`             |   ✅   | Bit accessors             |
|   4 | `test_parse_rejects`             |   ✅   | Parse rejects             |
|   5 | `test_build_and_accessor_guards` |   ✅   | Build and accessor guards |

</details>

---

## test_powerlink - native_powerlink - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/powerlink: the Ethernet POWERLINK basic frame codec._

|   # | Test                       | Status | Description                                  |
| --: | :------------------------- | :----: | :------------------------------------------- |
|   1 | `test_soc`                 |   ✅   | Soc                                          |
|   2 | `test_preq_pres_roundtrip` |   ✅   | PReq: MN (240) -> CN 5, carrying output PDO. |
|   3 | `test_parse_rejects`       |   ✅   | Parse rejects                                |
|   4 | `test_epl_build_guards`    |   ✅   | Epl build guards                             |

</details>

---

## test_sercos - native_sercos - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/sercos: the SERCOS III telegram + IDN codec._

|   # | Test                           | Status | Description                                              |
| --: | :----------------------------- | :----: | :------------------------------------------------------- |
|   1 | `test_idn_roundtrip`           |   ✅   | S-0-0100 (velocity loop): S-parameter, set 0, block 100. |
|   2 | `test_telegram_roundtrip`      |   ✅   | Telegram roundtrip                                       |
|   3 | `test_at_telegram_and_rejects` |   ✅   | At telegram and rejects                                  |
|   4 | `test_sercos_build_guards`     |   ✅   | Sercos build guards                                      |

</details>

---

## test_profibus - native_profibus - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/profibus: the PROFIBUS-DP FDL telegram codec._

|   # | Test                                       | Status | Description                                                             |
| --: | :----------------------------------------- | :----: | :---------------------------------------------------------------------- |
|   1 | `test_fcs`                                 |   ✅   | Fcs                                                                     |
|   2 | `test_sd1`                                 |   ✅   | SD1 DA SA FC FCS ED : 10 03 02 49 4E 16                                 |
|   3 | `test_sd2_roundtrip`                       |   ✅   | le = 3 + 3 = 6; total = 4 + 6 + 2 = 12.                                 |
|   4 | `test_parse_rejects`                       |   ✅   | Parse rejects                                                           |
|   5 | `test_build_and_parse_guard_subconditions` |   ✅   | Build guards: null out and a capacity below the frame size fail closed. |

</details>

---

## test_lonworks - native_lonworks - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/lonworks: the LonTalk NV PDU + SNVT scalar codec._

|   # | Test                                 | Status | Description                                                             |
| --: | :----------------------------------- | :----: | :---------------------------------------------------------------------- |
|   1 | `test_nv_pdu_roundtrip`              |   ✅   | selector 0x1234 is 14-bit -> stored 0x12 0x34.                          |
|   2 | `test_nv_selector_masked_to_14_bits` |   ✅   | The top two bits of the selector byte are not part of the 14-bit value. |
|   3 | `test_snvt_temp`                     |   ✅   | Snvt temp                                                               |
|   4 | `test_snvt_switch`                   |   ✅   | Snvt switch                                                             |
|   5 | `test_snvt_clamps_and_guards`        |   ✅   | Snvt clamps and guards                                                  |

</details>

---

## test_mbplus - native_mbplus - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/mbplus: the Modbus Plus HDLC token-bus frame codec._

|   # | Test                              | Status | Description                                           |
| --: | :-------------------------------- | :----: | :---------------------------------------------------- |
|   1 | `test_crc_check_vector`           |   ✅   | CRC-16/X-25 check value: CRC of "123456789" = 0x906E. |
|   2 | `test_build_and_parse`            |   ✅   | 7E 05 00 10 03 00 CRClo CRChi 7E = 9 bytes.           |
|   3 | `test_token_frame_no_payload`     |   ✅   | Token frame no payload                                |
|   4 | `test_next_token_ring`            |   ✅   | Next token ring                                       |
|   5 | `test_parse_rejects`              |   ✅   | Parse rejects                                         |
|   6 | `test_build_parse_and_token_wrap` |   ✅   | Build parse and token wrap                            |

</details>

---

## test_interbus - native_interbus - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/interbus: the summation-frame codec._

|   # | Test                      | Status | Description                                                  |
| --: | :------------------------ | :----: | :----------------------------------------------------------- |
|   1 | `test_fcs_check_vector`   |   ✅   | CRC-16/CCITT-FALSE check value: CRC of "123456789" = 0x29B1. |
|   2 | `test_build_and_parse`    |   ✅   | Three device slices: 0x1111, 0x2222, 0x3333.                 |
|   3 | `test_empty_frame`        |   ✅   | Empty frame                                                  |
|   4 | `test_parse_rejects`      |   ✅   | Corrupt FCS.                                                 |
|   5 | `test_build_parse_guards` |   ✅   | Build parse guards                                           |

</details>

---

## test_iccp - native_iccp - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/iccp: the ICCP / TASE.2 Data_Value codec._

|   # | Test                           | Status | Description                                      |
| --: | :----------------------------- | :----: | :----------------------------------------------- |
|   1 | `test_state_q_no_time`         |   ✅   | A2 { 85 01 <sq> } ; sq = (ON=2)<<6               | valid(0) = 0x80. -> A2 03 85 01 80 |
|   2 | `test_state_q_with_time`       |   ✅   | State q with time                                |
|   3 | `test_real_q`                  |   ✅   | Real q                                           |
|   4 | `test_real_q_negative`         |   ✅   | -1 -> minimal two's complement INTEGER 02 01 FF. |
|   5 | `test_state_and_real_q_guards` |   ✅   | State and real q guards                          |

</details>

---

## test_wave - native_wave - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/wave: the IEEE 1609 WSMP + 1609.2 envelope + PSID codec._

|   # | Test                            | Status | Description              |
| --: | :------------------------------ | :----: | :----------------------- |
|   1 | `test_psid_p_encoding`          |   ✅   | 1-octet: 0x20 -> 20.     |
|   2 | `test_psid_four_octet_and_caps` |   ✅   | Psid four octet and caps |
|   3 | `test_psid_decode_guards`       |   ✅   | Psid decode guards       |
|   4 | `test_wsmp_build_guards`        |   ✅   | Wsmp build guards        |
|   5 | `test_wsmp_parse_more_guards`   |   ✅   | Wsmp parse more guards   |
|   6 | `test_1609dot2_wrap_guards`     |   ✅   | 1609dot2 wrap guards     |
|   7 | `test_wsmp_roundtrip`           |   ✅   | Wsmp roundtrip           |
|   8 | `test_1609dot2_wrap`            |   ✅   | 1609dot2 wrap            |
|   9 | `test_wsmp_parse_rejects`       |   ✅   | Wsmp parse rejects       |

</details>

---

## test_utmc - native_utmc - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/utmc: the UTMC common-database request/response codec._

|   # | Test                        | Status | Description          |
| --: | :-------------------------- | :----: | :------------------- |
|   1 | `test_request`              |   ✅   | Request              |
|   2 | `test_response`             |   ✅   | Response             |
|   3 | `test_response_escapes`     |   ✅   | Response escapes     |
|   4 | `test_parse_request`        |   ✅   | Parse request        |
|   5 | `test_overflow`             |   ✅   | Overflow             |
|   6 | `test_parse_request_guards` |   ✅   | Parse request guards |

</details>

---

## test_ocit - native_ocit - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/ocit: the OCIT-Outstations message codec._

|   # | Test                       | Status | Description         |
| --: | :------------------------- | :----: | :------------------ |
|   1 | `test_build_and_parse`     |   ✅   | Build and parse     |
|   2 | `test_set_u16_helper`      |   ✅   | Set u16 helper      |
|   3 | `test_get_no_value`        |   ✅   | Get no value        |
|   4 | `test_parse_rejects_short` |   ✅   | Parse rejects short |

</details>

---

## test_atc - native_atc - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/atc: the ATC field-I/O interop snapshot._

|   # | Test                             | Status | Description               |
| --: | :------------------------------- | :----: | :------------------------ |
|   1 | `test_snapshot_json`             |   ✅   | Snapshot json             |
|   2 | `test_set_output`                |   ✅   | Set an output.            |
|   3 | `test_get`                       |   ✅   | Get                       |
|   4 | `test_empty_and_overflow`        |   ✅   | Empty and overflow        |
|   5 | `test_json_escapes_and_overflow` |   ✅   | Json escapes and overflow |

</details>

---

## test_southbound - native_southbound - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/southbound: the driver registry + name-dispatched read/write facade._

|   # | Test                             | Status | Description                                                   |
| --: | :------------------------------- | :----: | :------------------------------------------------------------ |
|   1 | `test_register_and_find`         |   ✅   | Register and find                                             |
|   2 | `test_read_write_dispatch`       |   ✅   | Read write dispatch                                           |
|   3 | `test_block_atomic`              |   ✅   | Block atomic                                                  |
|   4 | `test_unsupported_capability`    |   ✅   | A driver that only implements single-point read.              |
|   5 | `test_registry_full`             |   ✅   | Fill the registry with distinct-named drivers, then overflow. |
|   6 | `test_dispatch_not_found_guards` |   ✅   | Dispatch not found guards                                     |

</details>

---

## test_exc_decoder - native_exc_decoder - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/exc_decoder: parsing a real ESP32 Guru Meditation panic dump._

|   # | Test                                            | Status | Description                                                                                   |
| --: | :---------------------------------------------- | :----: | :-------------------------------------------------------------------------------------------- |
|   1 | `test_exc_edge_guards`                          |   ✅   | Exc edge guards                                                                               |
|   2 | `test_parse_full`                               |   ✅   | Parse full                                                                                    |
|   3 | `test_json`                                     |   ✅   | Json                                                                                          |
|   4 | `test_backtrace_only_and_corrupted`             |   ✅   | No register dump: PC must fall back to the first backtrace frame. Trailing corruption marker. |
|   5 | `test_garbage_returns_false`                    |   ✅   | Garbage returns false                                                                         |
|   6 | `test_json_omits_core_when_absent_and_overflow` |   ✅   | Json omits core when absent and overflow                                                      |
|   7 | `test_upper_hex_and_json_overflow`              |   ✅   | Uppercase hex addresses exercise the A-F branch of the nibble parser.                         |

</details>

---

## test_http_delivery - native_http_delivery - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/http_delivery: RFC 5861 stale-while-revalidate, RFC 7233 byte ranges,_

|   # | Test                                 | Status | Description                                                      |
| --: | :----------------------------------- | :----: | :--------------------------------------------------------------- |
|   1 | `test_range_and_builder_edge_guards` |   ✅   | Oversized start (>10 digits) -> read_u32 overflow guard rejects. |
|   2 | `test_swr_decision`                  |   ✅   | max-age=60, swr=30.                                              |
|   3 | `test_cache_control`                 |   ✅   | Cache control                                                    |
|   4 | `test_range_forms`                   |   ✅   | X-Y                                                              |
|   5 | `test_range_rejects`                 |   ✅   | Range rejects                                                    |
|   6 | `test_content_range`                 |   ✅   | Content range                                                    |
|   7 | `test_sw_manifest`                   |   ✅   | Sw manifest                                                      |
|   8 | `test_delivery_guards_and_escape`    |   ✅   | Delivery guards and escape                                       |

</details>

---

## test_hw_health - native_hw_health - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/hw_health: rail droop, SPI CRC backoff, GPIO short, cap leakage._

|   # | Test                                        | Status | Description                                       |
| --: | :------------------------------------------ | :----: | :------------------------------------------------ |
|   1 | `test_hwhealth_null_guards_and_init_clamps` |   ✅   | Hwhealth null guards and init clamps              |
|   2 | `test_rail_monitor`                         |   ✅   | Rail monitor                                      |
|   3 | `test_spi_backoff`                          |   ✅   | Spi backoff                                       |
|   4 | `test_spi_backoff_clamps`                   |   ✅   | Spi backoff clamps                                |
|   5 | `test_gpio_short`                           |   ✅   | Gpio short                                        |
|   6 | `test_cap_leak`                             |   ✅   | Expected 100ms decay, 10% tolerance -> [90, 110]. |
|   7 | `test_rail_ok_spi_clamps_probes`            |   ✅   | Rail ok spi clamps probes                         |

</details>

---

## test_mdns_adaptive - native_mdns_adaptive - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/mdns_adaptive: RF-aware backoff, TTL refresher, auto-sleep beacon._

|   # | Test                               | Status | Description                 |
| --: | :--------------------------------- | :----: | :-------------------------- |
|   1 | `test_refresh_interval`            |   ✅   | Refresh interval            |
|   2 | `test_backoff_and_recover`         |   ✅   | Backoff and recover         |
|   3 | `test_due`                         |   ✅   | Due                         |
|   4 | `test_presleep`                    |   ✅   | Presleep                    |
|   5 | `test_refresh_interval_and_beacon` |   ✅   | Refresh interval and beacon |

</details>

---

## test_sockpool - native_sockpool - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/sockpool: the LRU connection-slot recycling pool._

|   # | Test                            | Status | Description                         |
| --: | :------------------------------ | :----: | :---------------------------------- |
|   1 | `test_acquire_free`             |   ✅   | Acquire free                        |
|   2 | `test_lru_recycle`              |   ✅   | Fill: id 100@t10, 101@t20, 102@t30. |
|   3 | `test_touch_changes_lru`        |   ✅   | Touch changes lru                   |
|   4 | `test_release_reopens_free`     |   ✅   | Release reopens free                |
|   5 | `test_empty_pool_fails`         |   ✅   | Empty pool fails                    |
|   6 | `test_null_guard_subconditions` |   ✅   | Null guard subconditions            |

</details>

---

## test_psram_pool - native_psram_pool - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/psram_pool: DRAM/PSRAM placement policy + DMA ping-pong bookkeeping._

|   # | Test                             | Status | Description                                                         |
| --: | :------------------------------- | :----: | :------------------------------------------------------------------ |
|   1 | `test_place_large_prefers_psram` |   ✅   | 64KB asset, threshold 4KB, plenty of both heaps, 32KB DRAM reserve. |
|   2 | `test_place_small_prefers_dram`  |   ✅   | 512B hot buffer, threshold 4KB -> DRAM.                             |
|   3 | `test_place_dma_forces_dram`     |   ✅   | DMA-required buffer must be DRAM even if large.                     |
|   4 | `test_place_edges`               |   ✅   | Place edges                                                         |
|   5 | `test_pingpong`                  |   ✅   | Pingpong                                                            |

</details>

---

## test_happy_eyeballs - native_happy_eyeballs - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/happy_eyeballs: RFC 6724 ordering + RFC 8305 family interleave + attempt gate._

|   # | Test                               | Status | Description                                                                                    |
| --: | :--------------------------------- | :----: | :--------------------------------------------------------------------------------------------- |
|   1 | `test_pref_order`                  |   ✅   | Global outranks link-local outranks loopback; within global, native v6 outranks v4.            |
|   2 | `test_order_and_interleave`        |   ✅   | Two global v6 + one global v4, given v4-first: sort puts v6 ahead, interleave alternates.      |
|   3 | `test_order_single_family`         |   ✅   | All v4: interleave is a no-op, order stays preference-sorted (global before private).          |
|   4 | `test_attempt_due`                 |   ✅   | Attempt due                                                                                    |
|   5 | `test_pref_scopes_and_order_edges` |   ✅   | Exercise the multicast + unspecified scope arms of dws_he_pref (values are dws_ip-classified). |

</details>

---

## test_wifi_sniffer - native_wifi_sniffer - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/wifi_sniffer: 802.11 header decode, traffic tally, roaming decision._

|   # | Test                                   | Status | Description                                                          |
| --: | :------------------------------------- | :----: | :------------------------------------------------------------------- |
|   1 | `test_parse_data`                      |   ✅   | Parse data                                                           |
|   2 | `test_parse_beacon`                    |   ✅   | Parse beacon                                                         |
|   3 | `test_parse_ctrl_short`                |   ✅   | Parse ctrl short                                                     |
|   4 | `test_stats`                           |   ✅   | Stats                                                                |
|   5 | `test_roam`                            |   ✅   | Current -80 dBm, candidate -70 dBm, 8 dB hysteresis: 10 > 8 -> roam. |
|   6 | `test_stats_add_null_and_default_type` |   ✅   | Stats add null and default type                                      |

</details>

---

## test_link_manager - native_link_manager - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/link_manager: egress selection, graceful escalation, failover._

|   # | Test                           | Status | Description                                                               |
| --: | :----------------------------- | :----: | :------------------------------------------------------------------------ |
|   1 | `test_init_none_up`            |   ✅   | Init none up                                                              |
|   2 | `test_escalation_and_failover` |   ✅   | WiFi STA comes up first -> it becomes active.                             |
|   3 | `test_tie_break_lower_index`   |   ✅   | Two interfaces at equal priority: the lower index wins.                   |
|   4 | `test_out_of_range_no_change`  |   ✅   | Out of range no change                                                    |
|   5 | `test_select_null_guards`      |   ✅   | Select null guards                                                        |
|   6 | `test_init_and_active_null`    |   ✅   | Init and active null                                                      |
|   7 | `test_set_guard_paths`         |   ✅   | Null manager: reports -1 for both previous and new active, returns false. |

</details>

---

## test_cc1101 - native_cc1101 - ✅ 18 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CC1101 driver (services/cc1101) against a mock chip emulating the SPI header_

|   # | Test                                   | Status | Description                                             |
| --: | :------------------------------------- | :----: | :------------------------------------------------------ |
|   1 | `test_init_configures_and_detects`     |   ✅   | Init configures and detects                             |
|   2 | `test_init_fails_when_absent`          |   ✅   | Init fails when absent                                  |
|   3 | `test_send_writes_fifo_and_strobes_tx` |   ✅   | Send writes fifo and strobes tx                         |
|   4 | `test_send_rejects_bad_len`            |   ✅   | Send rejects bad len                                    |
|   5 | `test_tx_done`                         |   ✅   | Tx done                                                 |
|   6 | `test_set_rx`                          |   ✅   | Set rx                                                  |
|   7 | `test_recv_reads_packet_and_rssi`      |   ✅   | FIFO: [len=3][A][B][C][rssi_raw][lqi]; RXBYTES = 6.     |
|   8 | `test_recv_empty`                      |   ✅   | Recv empty                                              |
|   9 | `test_recv_truncates`                  |   ✅   | Recv truncates                                          |
|  10 | `test_rssi_decode`                     |   ✅   | TI formula: raw>=128 -> (raw-256)/2-74 ; else raw/2-74. |
|  11 | `test_send_guard_subconditions`        |   ✅   | Send guard subconditions                                |
|  12 | `test_init_null_args`                  |   ✅   | Init null args                                          |
|  13 | `test_init_no_regs`                    |   ✅   | Init no regs                                            |
|  14 | `test_tx_done_null_args`               |   ✅   | Tx done null args                                       |
|  15 | `test_set_rx_null_args`                |   ✅   | Set rx null args                                        |
|  16 | `test_recv_null_args`                  |   ✅   | Recv null args                                          |
|  17 | `test_recv_bad_length`                 |   ✅   | Zero length byte with bytes waiting.                    |
|  18 | `test_send_null_spi`                   |   ✅   | Send null spi                                           |

</details>

---

## test_fdc2214 - native_fdc2214 - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/fdc2214: the capacitance-to-digital codec (data combine, error flags,_

|   # | Test                          | Status | Description                                                                       |
| --: | :---------------------------- | :----: | :-------------------------------------------------------------------------------- |
|   1 | `test_data_combine`           |   ✅   | MSB register: error flags 0x3 in top nibble, data MSB 0xABC; LSB register 0x1234. |
|   2 | `test_freq_scale`             |   ✅   | data = 2^27 (half scale), fref = 40 MHz -> f_sensor = 20 MHz.                     |
|   3 | `test_build_config`           |   ✅   | Build config                                                                      |
|   4 | `test_build_config_too_small` |   ✅   | Build config too small                                                            |

</details>

---

## test_ldc1614 - native_ldc1614 - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/ldc1614: the inductance-to-digital codec (data combine, error flags,_

|   # | Test                          | Status | Description            |
| --: | :---------------------------- | :----: | :--------------------- |
|   1 | `test_data_combine`           |   ✅   | Data combine           |
|   2 | `test_freq_scale`             |   ✅   | Freq scale             |
|   3 | `test_build_config`           |   ✅   | Build config           |
|   4 | `test_build_config_too_small` |   ✅   | Build config too small |

</details>

---

## test_vl53l0x - native_vl53l0x - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/vl53l0x: the ToF ranging codec (range combine, data-ready, range status)._

|   # | Test                | Status | Description                                                                |
| --: | :------------------ | :----: | :------------------------------------------------------------------------- |
|   1 | `test_range_mm`     |   ✅   | Range mm                                                                   |
|   2 | `test_data_ready`   |   ✅   | Data ready                                                                 |
|   3 | `test_range_status` |   ✅   | DeviceRangeStatus = 11 (valid) in bits 6:3 -> register value 11<<3 = 0x58. |

</details>

---

## test_radio_sniff - native_radio_sniff - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/radio_sniff: the int->float32 RSSI encode and the 802.15.4 TAP pcap record._

|   # | Test                       | Status | Description                           |
| --: | :------------------------- | :----: | :------------------------------------ |
|   1 | `test_i2f32`               |   ✅   | I2f32                                 |
|   2 | `test_global_header`       |   ✅   | Global header                         |
|   3 | `test_tap_record`          |   ✅   | record(16) + tap(20) + frame(5) = 41. |
|   4 | `test_tap_record_overflow` |   ✅   | Tap record overflow                   |

</details>

---

## test_ble_gatt - native_ble_gatt - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/ble_gatt: the ATT PDU codec + GATT characteristic JSON._

|   # | Test                             | Status | Description                 |
| --: | :------------------------------- | :----: | :-------------------------- |
|   1 | `test_build_pdus`                |   ✅   | Read Request handle 0x0025. |
|   2 | `test_read_rsp_and_build_guards` |   ✅   | Read rsp and build guards   |
|   3 | `test_parse_guards_and_opcodes`  |   ✅   | Parse guards and opcodes    |
|   4 | `test_char_json_guards`          |   ✅   | Char json guards            |
|   5 | `test_build_overflow`            |   ✅   | Build overflow              |
|   6 | `test_parse`                     |   ✅   | Write Request with value.   |
|   7 | `test_char_json`                 |   ✅   | Char json                   |

</details>

---

## test_tls_policy - native_tls_policy - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/tls_policy: version negotiation, cipher selection, AEAD classification._

|   # | Test                     | Status | Description                                                                                  |
| --: | :----------------------- | :----: | :------------------------------------------------------------------------------------------- |
|   1 | `test_negotiate_version` |   ✅   | Server supports 1.2..1.3.                                                                    |
|   2 | `test_version_name`      |   ✅   | Version name                                                                                 |
|   3 | `test_select_cipher`     |   ✅   | Server prefers ECDHE_RSA_AES_128_GCM then CHACHA20; client offers CHACHA20 + a legacy suite. |
|   4 | `test_is_aead`           |   ✅   | Is aead                                                                                      |

</details>

---

## test_wisun - native_wisun - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/wisun: the CoAP client request builder (RFC 7252) + the FAN node registry._

|   # | Test                                           | Status | Description                                                                                   |
| --: | :--------------------------------------------- | :----: | :-------------------------------------------------------------------------------------------- |
|   1 | `test_build_coap_get`                          |   ✅   | CON GET "sensors/temp", msg id 0x1234, no token.                                              |
|   2 | `test_build_coap_put_with_token_and_payload`   |   ✅   | Header: 0x52 (ver=01, type NON=01, tkl=0010), code 0x03 (PUT), mid 0x00 0x05.                 |
|   3 | `test_build_coap_long_segment_extended_length` |   ✅   | A 13-char path segment forces the extended-length nibble (0xD).                               |
|   4 | `test_build_coap_rejects_bad_args`             |   ✅   | Build coap rejects bad args                                                                   |
|   5 | `test_node_registry`                           |   ✅   | Node registry                                                                                 |
|   6 | `test_registry_full_and_misses`                |   ✅   | Registry full and misses                                                                      |
|   7 | `test_coap_length_ext`                         |   ✅   | A Uri-Path segment >= 269 bytes drives the 2-byte length-extension encoding.                  |
|   8 | `test_coap_overflow_and_emit_fail`             |   ✅   | Header fits (cap == 4) but no room for even the first option header -> emit fails -> build 0. |
|   9 | `test_coap_arg_guards`                         |   ✅   | Coap arg guards                                                                               |
|  10 | `test_wisun_null_guards`                       |   ✅   | Wisun null guards                                                                             |

</details>

---

## test_logbuf - native_logbuf - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the rotating log ring (services/logbuf): append order, the_

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_append_and_order`      |   ✅   | Append and order      |
|   2 | `test_dump`                  |   ✅   | Dump                  |
|   3 | `test_rotation_drops_oldest` |   ✅   | Rotation drops oldest |
|   4 | `test_trap_threshold`        |   ✅   | Trap threshold        |
|   5 | `test_dump_guards`           |   ✅   | Dump guards           |

</details>

---

## test_config_io - native_config_io - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for schema-driven config export/restore (services/config_io) over_

|   # | Test                                | Status | Description                  |
| --: | :---------------------------------- | :----: | :--------------------------- |
|   1 | `test_export_format`                |   ✅   | Export format                |
|   2 | `test_round_trip`                   |   ✅   | Round trip                   |
|   3 | `test_import_skips_unknown_keys`    |   ✅   | Import skips unknown keys    |
|   4 | `test_export_overflow_fails_closed` |   ✅   | Export overflow fails closed |
|   5 | `test_export_import_null_guards`    |   ✅   | Export import null guards    |

</details>

---

## test_workers - native_workers - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Phase 2 core-partitioning invariant (built with DWS_WORKER_COUNT=2): a worker_

|   # | Test                                           | Status | Description                                                                                |
| --: | :--------------------------------------------- | :----: | :----------------------------------------------------------------------------------------- |
|   1 | `test_worker_count_is_two`                     |   ✅   | Worker count is two                                                                        |
|   2 | `test_check_timeouts_reaps_only_owned_slots`   |   ✅   | Check timeouts reaps only owned slots                                                      |
|   3 | `test_pool_init_defaults_owner_zero`           |   ✅   | Pool init defaults owner zero                                                              |
|   4 | `test_worker_self_id_roundtrip`                |   ✅   | dws_worker_set_self binds the calling context's worker id; dws_worker_self reads it back.  |
|   5 | `test_host_worker_lifecycle_is_noops`          |   ✅   | On host there is no worker task: start/stop/wake are no-ops and running() stays false.     |
|   6 | `test_host_defer_runs_inline_and_rejects_null` |   ✅   | On host the caller and pipeline are the same thread, so dws_defer runs the callback inline |

</details>

---

## test_clock - native_clock - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the pluggable monotonic clock (services/dws_clock): the platform_

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

## test_concurrency - native_concurrency - ✅ 2 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Concurrency proof for the cross-thread slot fields (DWSAtomic state / rx_head /_

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_spsc_ring_no_race`     |   ✅   | Spsc ring no race     |
|   2 | `test_state_handoff_no_race` |   ✅   | State handoff no race |

</details>

---

## test_concurrency - native_tsan - ✅ 2 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Concurrency proof for the cross-thread slot fields (DWSAtomic state / rx_head /_

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_spsc_ring_no_race`     |   ✅   | Spsc ring no race     |
|   2 | `test_state_handoff_no_race` |   ✅   | State handoff no race |

</details>

---

## test_qpack - native_qpack - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the QPACK codec (network_drivers/presentation/http3/qpack, RFC 9204): the_

|   # | Test                                    | Status | Description                                                                               |
| --: | :-------------------------------------- | :----: | :---------------------------------------------------------------------------------------- |
|   1 | `test_appendix_b1_decode`               |   ✅   | Appendix b1 decode                                                                        |
|   2 | `test_encode_indexed`                   |   ✅   | Encode indexed                                                                            |
|   3 | `test_encode_nameref_roundtrip`         |   ✅   | Encode nameref roundtrip                                                                  |
|   4 | `test_literal_name`                     |   ✅   | Literal name                                                                              |
|   5 | `test_full_section`                     |   ✅   | Full section                                                                              |
|   6 | `test_reject_dynamic`                   |   ✅   | Reject dynamic                                                                            |
|   7 | `test_encode_edges`                     |   ✅   | Encode edges                                                                              |
|   8 | `test_decode_errors`                    |   ✅   | Decode errors                                                                             |
|   9 | `test_value_string_paths`               |   ✅   | Value marked Huffman (0x81 = H, len 1) but 0xFF is not a valid single-byte code.          |
|  10 | `test_qpack_more_encode_decode_paths`   |   ✅   | A short literal name that does not Huffman-compress takes the raw memcpy path.            |
|  11 | `test_qpack_emit_fail_and_namelen_past` |   ✅   | Literal Field Line with Name Reference + a valid value, but the emit callback rejects it. |

</details>

---

## test_quic_packet - native_quic_packet - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the QUIC packet header + packet-number codec (network_drivers/presentation/http3/_

|   # | Test                         | Status | Description                                                              |
| --: | :--------------------------- | :----: | :----------------------------------------------------------------------- |
|   1 | `test_long_header_roundtrip` |   ✅   | Long header roundtrip                                                    |
|   2 | `test_version_negotiation`   |   ✅   | Version negotiation                                                      |
|   3 | `test_short_header_parse`    |   ✅   | Short header parse                                                       |
|   4 | `test_pn_encode`             |   ✅   | RFC 9000 A.2: acked 0xabe8b3, sending 0xac5c02 -> 16-bit encoding.       |
|   5 | `test_pn_decode`             |   ✅   | RFC 9000 A.3: largest 0xa82f30ea, 16-bit truncated 0x9b32 -> 0xa82f9b32. |
|   6 | `test_reject`                |   ✅   | Destination Connection ID length 21 (> 20) must be dropped.              |
|   7 | `test_build_guards`          |   ✅   | Build guards                                                             |
|   8 | `test_short_header_guards`   |   ✅   | Short header guards                                                      |

</details>

---

## test_quic_frame - native_quic_frame - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the QUIC frame codec (network_drivers/presentation/http3/dws_quic_frame, RFC 9000_

|   # | Test                           | Status | Description                                                                                          |
| --: | :----------------------------- | :----: | :--------------------------------------------------------------------------------------------------- |
|   1 | `test_frame_edge_guards`       |   ✅   | STREAM with LEN set but the Length varint is absent -> rejected at the length read.                  |
|   2 | `test_simple_frames`           |   ✅   | Simple frames                                                                                        |
|   3 | `test_ack`                     |   ✅   | Ack                                                                                                  |
|   4 | `test_ack_multi_range`         |   ✅   | type 0x03, largest 60, delay 5, range_count 2, first_range 3, [gap 2,len 4][gap 1,len 1], ECN 1/2/0. |
|   5 | `test_crypto`                  |   ✅   | Crypto                                                                                               |
|   6 | `test_stream`                  |   ✅   | With offset + FIN.                                                                                   |
|   7 | `test_max_data_and_close`      |   ✅   | Max data and close                                                                                   |
|   8 | `test_sequence_and_truncation` |   ✅   | A packet payload: PADDING, PING, then a CRYPTO frame - parse them in order.                          |
|   9 | `test_builder_overflow`        |   ✅   | Builder overflow                                                                                     |
|  10 | `test_parse_errors`            |   ✅   | Parse errors                                                                                         |
|  11 | `test_skip_and_extra_frames`   |   ✅   | One-varint frames: type followed by a single varint.                                                 |

</details>

---

## test_quic_crypto - native_quic_crypto - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for QUIC Initial packet crypto (network_drivers/presentation/http3/dws_quic_hkdf,_

|   # | Test                                | Status | Description                  |
| --: | :---------------------------------- | :----: | :--------------------------- |
|   1 | `test_aes128_block_fips197`         |   ✅   | Aes128 block fips197         |
|   2 | `test_aes128_gcm_testcase4`         |   ✅   | Aes128 gcm testcase4         |
|   3 | `test_initial_secrets_appendix_a1`  |   ✅   | Initial secrets appendix a1  |
|   4 | `test_server_initial_a3`            |   ✅   | Server initial a3            |
|   5 | `test_client_initial_a2`            |   ✅   | Client initial a2            |
|   6 | `test_retry_integrity_a4`           |   ✅   | Retry integrity a4           |
|   7 | `test_gcm_open_rejects_short`       |   ✅   | Gcm open rejects short       |
|   8 | `test_protect_rejects_bad_pn_len`   |   ✅   | Protect rejects bad pn len   |
|   9 | `test_protect_rejects_small_cap`    |   ✅   | Protect rejects small cap    |
|  10 | `test_unprotect_rejects_short`      |   ✅   | Unprotect rejects short      |
|  11 | `test_unprotect_rejects_tampered`   |   ✅   | Unprotect rejects tampered   |
|  12 | `test_retry_tag_rejects_oversize`   |   ✅   | Retry tag rejects oversize   |
|  13 | `test_hkdf_expand_label_multiblock` |   ✅   | Hkdf expand label multiblock |

</details>

---

## test_dtls_record - native_dtls - ✅ 16 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_DTLS 1.3 record layer tests (RFC 9147 §4). The record + key derivation is pinned byte-for-byte_

|   # | Test                                     | Status | Description                       |
| --: | :--------------------------------------- | :----: | :-------------------------------- |
|   1 | `test_dtls_record_keys_derive_kat`       |   ✅   | Dtls record keys derive kat       |
|   2 | `test_dtls_ciphertext_protect_kat`       |   ✅   | Dtls ciphertext protect kat       |
|   3 | `test_dtls_ciphertext_unprotect_kat`     |   ✅   | Dtls ciphertext unprotect kat     |
|   4 | `test_dtls_ciphertext_roundtrip`         |   ✅   | Dtls ciphertext roundtrip         |
|   5 | `test_dtls_seq_reconstruction`           |   ✅   | Dtls seq reconstruction           |
|   6 | `test_dtls_ciphertext_unprotect_rejects` |   ✅   | Dtls ciphertext unprotect rejects |
|   7 | `test_dtls_cid_roundtrip`                |   ✅   | Dtls cid roundtrip                |
|   8 | `test_dtls_cid_rejects`                  |   ✅   | Dtls cid rejects                  |
|   9 | `test_dtls_plaintext_roundtrip`          |   ✅   | Dtls plaintext roundtrip          |
|  10 | `test_dtls_replay_window`                |   ✅   | Dtls replay window                |
|  11 | `test_dtls_seq_rollover_both_directions` |   ✅   | Dtls seq rollover both directions |
|  12 | `test_dtls_plaintext_bounds`             |   ✅   | total > out_cap.                  |
|  13 | `test_dtls_protect_bounds`               |   ✅   | Dtls protect bounds               |
|  14 | `test_dtls_unprotect_bounds`             |   ✅   | Dtls unprotect bounds             |
|  15 | `test_dtls_unprotect_all_zero_inner`     |   ✅   | Dtls unprotect all zero inner     |
|  16 | `test_dtls_replay_mark_below_window`     |   ✅   | Dtls replay mark below window     |

</details>

---

## test_dtls_handshake - native_dtls_hs - ✅ 15 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_DTLS 1.3 handshake framing + reliability tests (RFC 9147 §5, §7): the 12-byte handshake header,_

|   # | Test                                    | Status | Description                                                                          |
| --: | :-------------------------------------- | :----: | :----------------------------------------------------------------------------------- |
|   1 | `test_hs_header_roundtrip`              |   ✅   | Hs header roundtrip                                                                  |
|   2 | `test_hs_header_parse_rejects`          |   ✅   | Shorter than the 12-byte header.                                                     |
|   3 | `test_hs_reasm_single_fragment`         |   ✅   | Hs reasm single fragment                                                             |
|   4 | `test_hs_reasm_in_order`                |   ✅   | Hs reasm in order                                                                    |
|   5 | `test_hs_reasm_out_of_order`            |   ✅   | Hs reasm out of order                                                                |
|   6 | `test_hs_reasm_overlap_and_duplicate`   |   ✅   | Hs reasm overlap and duplicate                                                       |
|   7 | `test_hs_reasm_wrong_msg_seq_ignored`   |   ✅   | Hs reasm wrong msg seq ignored                                                       |
|   8 | `test_hs_reasm_empty_body`              |   ✅   | Hs reasm empty body                                                                  |
|   9 | `test_hs_reasm_rejects`                 |   ✅   | Hs reasm rejects                                                                     |
|  10 | `test_ack_roundtrip`                    |   ✅   | Ack roundtrip                                                                        |
|  11 | `test_ack_parse_rejects`                |   ✅   | Ack parse rejects                                                                    |
|  12 | `test_cookie_kat`                       |   ✅   | Cookie kat                                                                           |
|  13 | `test_cookie_verify_accept_and_payload` |   ✅   | max_age = 0 disables the freshness check, isolating the MAC + payload recovery.      |
|  14 | `test_cookie_verify_rejects`            |   ✅   | A different client address fails the MAC (the address is authenticated, not stored). |
|  15 | `test_cookie_freshness`                 |   ✅   | Cookie freshness                                                                     |

</details>

---

## test_dtls_tls13 - native_dtls_tls13 - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_TLS 1.3 messages the DTLS 1.3 handshake adds to dws_tls13_msg (RFC 8446 §4.1.4 / §4.4.1): the_

|   # | Test                              | Status | Description                                                                         |
| --: | :-------------------------------- | :----: | :---------------------------------------------------------------------------------- |
|   1 | `test_hrr_magic_symbol`           |   ✅   | The builder and the RFC constant agree.                                             |
|   2 | `test_hrr_build_kat`              |   ✅   | Hrr build kat                                                                       |
|   3 | `test_hrr_echoes_session_id`      |   ✅   | Hrr echoes session id                                                               |
|   4 | `test_message_hash`               |   ✅   | Message hash                                                                        |
|   5 | `test_empty_encrypted_extensions` |   ✅   | Empty encrypted extensions                                                          |
|   6 | `test_client_hello_cookie_parse`  |   ✅   | Assemble a minimal but well-formed ClientHello with exactly one extension (cookie). |

</details>

---

## test_dtls_conn - native_dtls_conn - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_DTLS 1.3 server handshake (RFC 9147 §5-6). A self-consistent proof: the test plays a minimal DTLS_

|   # | Test                                     | Status | Description                       |
| --: | :--------------------------------------- | :----: | :-------------------------------- |
|   1 | `test_full_handshake`                    |   ✅   | Full handshake                    |
|   2 | `test_cid_handshake`                     |   ✅   | Cid handshake                     |
|   3 | `test_hrr_group_renegotiation`           |   ✅   | Hrr group renegotiation           |
|   4 | `test_hrr_retry_without_cookie_rejected` |   ✅   | Hrr retry without cookie rejected |
|   5 | `test_pto_retransmit_and_recovery`       |   ✅   | Pto retransmit and recovery       |
|   6 | `test_pto_backoff_and_giveup`            |   ✅   | Pto backoff and giveup            |
|   7 | `test_pto_ack_cancels_retransmit`        |   ✅   | Pto ack cancels retransmit        |
|   8 | `test_reject_no_tls13`                   |   ✅   | Reject no tls13                   |

</details>

---

## test_coaps - native_coaps - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_CoAP over DTLS (coaps.h) end-to-end. An in-test DTLS 1.3 client completes the handshake against_

|   # | Test                                 | Status | Description                   |
| --: | :----------------------------------- | :----: | :---------------------------- |
|   1 | `test_coap_over_dtls`                |   ✅   | Coap over dtls                |
|   2 | `test_coap_over_dtls_replay_dropped` |   ✅   | Coap over dtls replay dropped |
|   3 | `test_coaps_no_coap_response`        |   ✅   | Coaps no coap response        |
|   4 | `test_coaps_non_app_record`          |   ✅   | Coaps non app record          |
|   5 | `test_coaps_wrong_epoch_record`      |   ✅   | Coaps wrong epoch record      |
|   6 | `test_coaps_forwards_handshake`      |   ✅   | Coaps forwards handshake      |

</details>

---

## test_coaps_server - native_coaps_server - ✅ 15 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_CoAP-over-DTLS server front-end (coaps_server.h): the per-peer DtlsConn pool + ingest/poll seam._

|   # | Test                                 | Status | Description                   |
| --: | :----------------------------------- | :----: | :---------------------------- |
|   1 | `test_server_single_peer`            |   ✅   | Server single peer            |
|   2 | `test_two_peers_routing`             |   ✅   | Two peers routing             |
|   3 | `test_idle_reap`                     |   ✅   | Idle reap                     |
|   4 | `test_pto_retransmit_driven_by_poll` |   ✅   | Pto retransmit driven by poll |
|   5 | `test_cid_address_migration`         |   ✅   | Cid address migration         |
|   6 | `test_begin_rejects_invalid_cfg`     |   ✅   | Begin rejects invalid cfg     |
|   7 | `test_poll_when_stopped`             |   ✅   | Poll when stopped             |
|   8 | `test_ingest_rejects_bad_len`        |   ✅   | Ingest rejects bad len        |
|   9 | `test_ingest_ring_full`              |   ✅   | Ingest ring full              |
|  10 | `test_ingest_addr_copy_edges`        |   ✅   | Ingest addr copy edges        |
|  11 | `test_malformed_peer_addr`           |   ✅   | Malformed peer addr           |
|  12 | `test_fatal_handshake_frees_slot`    |   ✅   | Fatal handshake frees slot    |
|  13 | `test_pool_full_rejects_new_peer`    |   ✅   | Pool full rejects new peer    |
|  14 | `test_pto_ceiling_frees_slot`        |   ✅   | Pto ceiling frees slot        |
|  15 | `test_unknown_cid_dropped`           |   ✅   | Unknown cid dropped           |

</details>

---

## test_tls13_kdf - native_tls13_kdf - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the TLS 1.3 key schedule (network_drivers/presentation/http3/dws_tls13_kdf; RFC 8446_

|   # | Test                            | Status | Description               |
| --: | :------------------------------ | :----: | :------------------------ |
|   1 | `test_early_secret`             |   ✅   | Early secret              |
|   2 | `test_handshake_secrets`        |   ✅   | Handshake secrets         |
|   3 | `test_master_secrets`           |   ✅   | Master secrets            |
|   4 | `test_server_hs_write_keys`     |   ✅   | Server hs write keys      |
|   5 | `test_server_finished`          |   ✅   | ClientHello (196 octets). |
|   6 | `test_kdf_expand_label_wrapper` |   ✅   | Kdf expand label wrapper  |

</details>

---

## test_quic_tp - native_quic_tp - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the QUIC transport-parameters codec (network_drivers/presentation/http3/dws_quic_tp;_

|   # | Test                        | Status | Description                                                                                  |
| --: | :-------------------------- | :----: | :------------------------------------------------------------------------------------------- |
|   1 | `test_defaults`             |   ✅   | Defaults                                                                                     |
|   2 | `test_roundtrip`            |   ✅   | Roundtrip                                                                                    |
|   3 | `test_parse_bytes`          |   ✅   | Parse bytes                                                                                  |
|   4 | `test_skip_unknown`         |   ✅   | id 0x1a (unknown), len 3, value 01 02 03; then 04 01 20 (initial_max_data = 0x20 = 32).      |
|   5 | `test_reject_duplicate`     |   ✅   | initial_max_data twice.                                                                      |
|   6 | `test_reject_oversized_cid` |   ✅   | original_destination_connection_id with a 21-byte value (max is 20).                         |
|   7 | `test_reject_bad_values`    |   ✅   | active_connection_id_limit = 1 (must be >= 2).                                               |
|   8 | `test_quic_tp_more_paths`   |   ✅   | Encode overflow: a CID param's ID varint, length varint, and value each fail at a tight cap. |

</details>

---

## test_tls13_msg - native_tls13_msg - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the TLS 1.3 handshake messages (network_drivers/presentation/http3/dws_tls13_msg;_

|   # | Test                                           | Status | Description                                                                                     |
| --: | :--------------------------------------------- | :----: | :---------------------------------------------------------------------------------------------- |
|   1 | `test_tls13_extension_and_truncation_coverage` |   ✅   | Body ends right after cipher_suites -> r_u8(compression_methods length) truncates.              |
|   2 | `test_tls13_malformed_extensions`              |   ✅   | Tls13 malformed extensions                                                                      |
|   3 | `test_tls13_parse_guards`                      |   ✅   | Tls13 parse guards                                                                              |
|   4 | `test_tls13_builder_cap_guards`                |   ✅   | Tls13 builder cap guards                                                                        |
|   5 | `test_parse_client_hello`                      |   ✅   | Parse client hello                                                                              |
|   6 | `test_build_server_hello`                      |   ✅   | Build server hello                                                                              |
|   7 | `test_build_certificate`                       |   ✅   | Reconstruct the DER cert from the expected message: strip the 11-byte prefix and 2-byte suffix. |
|   8 | `test_build_finished`                          |   ✅   | Build finished                                                                                  |
|   9 | `test_encrypted_extensions`                    |   ✅   | Encrypted extensions                                                                            |
|  10 | `test_cert_verify_content`                     |   ✅   | Cert verify content                                                                             |
|  11 | `test_cert_verify_sign_roundtrip`              |   ✅   | Cert verify sign roundtrip                                                                      |

</details>

---

## test_quic_tls - native_quic_tls - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the TLS 1.3 server handshake state machine (network_drivers/presentation/http3/_

|   # | Test                                          | Status | Description                                                                                  |
| --: | :-------------------------------------------- | :----: | :------------------------------------------------------------------------------------------- |
|   1 | `test_full_handshake_roundtrip`               |   ✅   | Full handshake roundtrip                                                                     |
|   2 | `test_reject_bad_client_finished`             |   ✅   | Reject bad client finished                                                                   |
|   3 | `test_reject_no_h3_alpn`                      |   ✅   | Reject no h3 alpn                                                                            |
|   4 | `test_partial_client_hello`                   |   ✅   | Partial client hello                                                                         |
|   5 | `test_reject_no_tls13`                        |   ✅   | Reject no tls13                                                                              |
|   6 | `test_reject_no_key_share`                    |   ✅   | Reject no key share                                                                          |
|   7 | `test_reject_no_x25519_group`                 |   ✅   | Reject no x25519 group                                                                       |
|   8 | `test_reject_no_ed25519`                      |   ✅   | Reject no ed25519                                                                            |
|   9 | `test_reject_no_transport_params`             |   ✅   | Reject no transport params                                                                   |
|  10 | `test_reject_bad_transport_params`            |   ✅   | Reject bad transport params                                                                  |
|  11 | `test_reject_malformed_client_hello`          |   ✅   | Reject malformed client hello                                                                |
|  12 | `test_quic_tls_more_guards`                   |   ✅   | A Finished-typed message of the wrong length -> DECODE_ERROR inside process_client_finished. |
|  13 | `test_quic_tls_cert_size_boundary_emit_fails` |   ✅   | Quic tls cert size boundary emit fails                                                       |

</details>

---

## test_quic_tls - native_quic_tls_pqc - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the TLS 1.3 server handshake state machine (network_drivers/presentation/http3/_

|   # | Test                                          | Status | Description                                                                                  |
| --: | :-------------------------------------------- | :----: | :------------------------------------------------------------------------------------------- |
|   1 | `test_full_handshake_roundtrip`               |   ✅   | Full handshake roundtrip                                                                     |
|   2 | `test_reject_bad_client_finished`             |   ✅   | Reject bad client finished                                                                   |
|   3 | `test_reject_no_h3_alpn`                      |   ✅   | Reject no h3 alpn                                                                            |
|   4 | `test_partial_client_hello`                   |   ✅   | Partial client hello                                                                         |
|   5 | `test_reject_no_tls13`                        |   ✅   | Reject no tls13                                                                              |
|   6 | `test_reject_no_key_share`                    |   ✅   | Reject no key share                                                                          |
|   7 | `test_reject_no_x25519_group`                 |   ✅   | Reject no x25519 group                                                                       |
|   8 | `test_reject_no_ed25519`                      |   ✅   | Reject no ed25519                                                                            |
|   9 | `test_reject_no_transport_params`             |   ✅   | Reject no transport params                                                                   |
|  10 | `test_reject_bad_transport_params`            |   ✅   | Reject bad transport params                                                                  |
|  11 | `test_reject_malformed_client_hello`          |   ✅   | Reject malformed client hello                                                                |
|  12 | `test_quic_tls_more_guards`                   |   ✅   | A Finished-typed message of the wrong length -> DECODE_ERROR inside process_client_finished. |
|  13 | `test_quic_tls_cert_size_boundary_emit_fails` |   ✅   | Quic tls cert size boundary emit fails                                                       |
|  14 | `test_hybrid_handshake_roundtrip`             |   ✅   | Hybrid handshake roundtrip                                                                   |

</details>

---

## test_quic_conn - native_quic_conn - ✅ 27 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the QUIC v1 server connection engine (network_drivers/presentation/http3/dws_quic_conn;_

|   # | Test                                       | Status | Description                         |
| --: | :----------------------------------------- | :----: | :---------------------------------- |
|   1 | `test_full_handshake_and_stream`           |   ✅   | Full handshake and stream           |
|   2 | `test_pto_retransmits_flight`              |   ✅   | Pto retransmits flight              |
|   3 | `test_connection_close_api`                |   ✅   | Connection close api                |
|   4 | `test_connection_close_on_malformed_frame` |   ✅   | Connection close on malformed frame |
|   5 | `test_quic_send_amplification_limited`     |   ✅   | Quic send amplification limited     |
|   6 | `test_quic_crypto_out_of_order_and_dup`    |   ✅   | Quic crypto out of order and dup    |
|   7 | `test_quic_timeout_when_closed`            |   ✅   | Quic timeout when closed            |
|   8 | `test_quic_stream_send_table_full`         |   ✅   | Quic stream send table full         |
|   9 | `test_quic_recv_connection_close`          |   ✅   | Quic recv connection close          |
|  10 | `test_quic_recv_ping_and_max_data`         |   ✅   | Quic recv ping and max data         |
|  11 | `test_quic_recv_bad_version`               |   ✅   | Quic recv bad version               |
|  12 | `test_quic_recv_unsupported_long_type`     |   ✅   | Quic recv unsupported long type     |
|  13 | `test_quic_recv_short_before_app_keys`     |   ✅   | Quic recv short before app keys     |
|  14 | `test_quic_recv_short_too_short`           |   ✅   | Quic recv short too short           |
|  15 | `test_quic_recv_unprotect_failure`         |   ✅   | Quic recv unprotect failure         |
|  16 | `test_quic_recv_truncated_long_header`     |   ✅   | Quic recv truncated long header     |
|  17 | `test_quic_recv_malformed_initial_headers` |   ✅   | Quic recv malformed initial headers |
|  18 | `test_quic_recv_handshake_done_frame`      |   ✅   | Quic recv handshake done frame      |
|  19 | `test_quic_conn_stream_frames`             |   ✅   | Quic conn stream frames             |
|  20 | `test_quic_conn_crypto_window_clamp`       |   ✅   | Quic conn crypto window clamp       |
|  21 | `test_quic_conn_crypto_error_close`        |   ✅   | Quic conn crypto error close        |
|  22 | `test_quic_conn_no_keys_build`             |   ✅   | Quic conn no keys build             |
|  23 | `test_quic_conn_pto_not_yet`               |   ✅   | Quic conn pto not yet               |
|  24 | `test_quic_conn_send_tiny_cap`             |   ✅   | Quic conn send tiny cap             |
|  25 | `test_quic_conn_stream_nothing_to_send`    |   ✅   | Quic conn stream nothing to send    |
|  26 | `test_quic_conn_short_header_tiny_cap`     |   ✅   | Quic conn short header tiny cap     |
|  27 | `test_quic_conn_close_level_fallback`      |   ✅   | Quic conn close level fallback      |

</details>

---

## test_h3_conn - native_h3_conn - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the HTTP/3 application engine (network_drivers/presentation/http3/dws_h3_conn; RFC_

|   # | Test                                  | Status | Description                    |
| --: | :------------------------------------ | :----: | :----------------------------- |
|   1 | `test_request_dispatch_and_response`  |   ✅   | Request dispatch and response  |
|   2 | `test_post_with_body`                 |   ✅   | Post with body                 |
|   3 | `test_control_stream_settings_sent`   |   ✅   | Control stream settings sent   |
|   4 | `test_client_control_stream_settings` |   ✅   | Client control stream settings |
|   5 | `test_client_uni_stream_types`        |   ✅   | Client uni stream types        |
|   6 | `test_handshake_done_idempotent`      |   ✅   | Handshake done idempotent      |
|   7 | `test_malformed_request_frame`        |   ✅   | Malformed request frame        |
|   8 | `test_respond_body_too_large`         |   ✅   | Respond body too large         |
|   9 | `test_stream_pool_full`               |   ✅   | Stream pool full               |
|  10 | `test_uni_stream_partial_type`        |   ✅   | Uni stream partial type        |
|  11 | `test_overlong_field_truncated`       |   ✅   | Overlong field truncated       |

</details>

---

## test_h3_e2e - native_h3_e2e - ✅ 1 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_End-to-end capstone for the whole HTTP/3 stack: a QUIC client (in the test) completes the TLS 1.3_

|   # | Test                        | Status | Description          |
| --: | :-------------------------- | :----: | :------------------- |
|   1 | `test_http3_get_end_to_end` |   ✅   | Http3 get end to end |

</details>

---

## test_quic_server - native_quic_server - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_HTTP/3 server-glue test: the same end-to-end flow as test_h3_e2e (a QUIC client completes the_

|   # | Test                            | Status | Description              |
| --: | :------------------------------ | :----: | :----------------------- |
|   1 | `test_quic_server_http3_get`    |   ✅   | Quic server http3 get    |
|   2 | `test_idle_connection_reaped`   |   ✅   | Idle connection reaped   |
|   3 | `test_quic_server_input_guards` |   ✅   | Quic server input guards |
|   4 | `test_quic_server_pool_full`    |   ✅   | Quic server pool full    |

</details>

---

## test_h3_server - native_h3_server - ✅ 1 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_HTTP/3 dispatch-bridge test: proves an HTTP/3 request served by a *real DWS route*. A_

|   # | Test                              | Status | Description                |
| --: | :-------------------------------- | :----: | :------------------------- |
|   1 | `test_h3_request_served_by_route` |   ✅   | H3 request served by route |

</details>

---

## test_ssh_chachapoly - native_ssh_chachapoly - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the chacha20-poly1305@openssh.com cipher and its primitives:_

|   # | Test                              | Status | Description                |
| --: | :-------------------------------- | :----: | :------------------------- |
|   1 | `test_chacha20_block_rfc8439`     |   ✅   | Chacha20 block rfc8439     |
|   2 | `test_poly1305_rfc8439`           |   ✅   | Poly1305 rfc8439           |
|   3 | `test_chachapoly_roundtrip`       |   ✅   | Chachapoly roundtrip       |
|   4 | `test_chachapoly_tamper_rejected` |   ✅   | Chachapoly tamper rejected |
|   5 | `test_chachapoly_empty_payload`   |   ✅   | Chachapoly empty payload   |

</details>

---

## test_ssh_aesgcm - native_ssh_aesgcm - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the AES-256-GCM AEAD used by aes256-gcm@openssh.com (RFC 5647):_

|   # | Test                                      | Status | Description                        |
| --: | :---------------------------------------- | :----: | :--------------------------------- |
|   1 | `test_aesgcm_nist_tc16_seal`              |   ✅   | Aesgcm nist tc16 seal              |
|   2 | `test_aesgcm_nist_tc16_open`              |   ✅   | Aesgcm nist tc16 open              |
|   3 | `test_aesgcm_invocation_counter_advances` |   ✅   | Aesgcm invocation counter advances |

</details>

---

## test_ssh_ecdsa - native_ssh_ecdsa - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_NIST P-256 native software-path tests (ecdsa-sha2-nistp256 signatures + ecdh-sha2-nistp256 KEX)._

|   # | Test                                   | Status | Description                     |
| --: | :------------------------------------- | :----: | :------------------------------ |
|   1 | `test_ecdsa_pubkey_matches_rfc6979`    |   ✅   | Ecdsa pubkey matches rfc6979    |
|   2 | `test_ecdsa_sign_deterministic_sample` |   ✅   | Ecdsa sign deterministic sample |
|   3 | `test_ecdsa_sign_deterministic_test`   |   ✅   | Ecdsa sign deterministic test   |
|   4 | `test_ecdsa_verify_valid`              |   ✅   | Ecdsa verify valid              |
|   5 | `test_ecdsa_verify_rejects_tamper`     |   ✅   | Ecdsa verify rejects tamper     |
|   6 | `test_ecdsa_roundtrip_other_key`       |   ✅   | Ecdsa roundtrip other key       |
|   7 | `test_ecdsa_random_roundtrip_stress`   |   ✅   | Ecdsa random roundtrip stress   |
|   8 | `test_ecdsa_pubkey_rejects_bad_scalar` |   ✅   | Ecdsa pubkey rejects bad scalar |
|   9 | `test_ecdh_rfc5903_shared_secret`      |   ✅   | Ecdh rfc5903 shared secret      |
|  10 | `test_ecdh_rfc5903_pubkeys`            |   ✅   | Ecdh rfc5903 pubkeys            |
|  11 | `test_ecdh_rejects_bad_point`          |   ✅   | Ecdh rejects bad point          |

</details>

---

## test_forwarded_trust - native_forwarded_trust - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the trusted-reverse-proxy forwarded-client resolver (services/forwarded_trust)._

|   # | Test                                     | Status | Description                       |
| --: | :--------------------------------------- | :----: | :-------------------------------- |
|   1 | `test_empty_table_trusts_nothing`        |   ✅   | Empty table trusts nothing        |
|   2 | `test_v4_cidr_membership`                |   ✅   | V4 cidr membership                |
|   3 | `test_v6_cidr_and_host_route`            |   ✅   | V6 cidr and host route            |
|   4 | `test_add_cidr_rejects_malformed`        |   ✅   | Add cidr rejects malformed        |
|   5 | `test_table_full`                        |   ✅   | Table full                        |
|   6 | `test_trusted_peer_honors_forwarded`     |   ✅   | Trusted peer honors forwarded     |
|   7 | `test_trusted_peer_honors_v6_forwarded`  |   ✅   | Trusted peer honors v6 forwarded  |
|   8 | `test_untrusted_peer_ignores_forwarded`  |   ✅   | Untrusted peer ignores forwarded  |
|   9 | `test_trusted_peer_bad_token_falls_back` |   ✅   | Trusted peer bad token falls back |
|  10 | `test_null_guards`                       |   ✅   | Null guards                       |

</details>

---
