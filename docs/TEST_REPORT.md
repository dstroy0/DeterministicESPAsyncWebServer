# Test Report

**Generated:** 2026-07-07 00:50:35
**Command:** `pio test` over 204 auto-discovered native envs (excludes native_pentest, native_codeql)
**Result:** ✅ 2438 passed - 595s

---

## Summary

| Suite                    | Environment             | Tests | Status |     Duration |
| :----------------------- | :---------------------- | ----: | :----: | -----------: |
| `test_canopen`           | `native_canopen`        |    17 |   ✅   | 00:00:03.768 |
| `test_det_primitives`    | `native_det_primitives` |     5 |   ✅   | 00:00:00.798 |
| `test_det_ip`            | `native_det_ip`         |    10 |   ✅   | 00:00:00.783 |
| `test_det_arena`         | `native_det_arena`      |    17 |   ✅   | 00:00:00.827 |
| `test_ssh_ed25519`       | `native_ssh_ed25519`    |    16 |   ✅   | 00:00:04.918 |
| `test_promisc`           | `native_promisc`        |     8 |   ✅   | 00:00:00.785 |
| `test_bus_capture`       | `native_bus_capture`    |     5 |   ✅   | 00:00:00.786 |
| `test_j1939`             | `native_j1939`          |     9 |   ✅   | 00:00:00.779 |
| `test_devicenet`         | `native_devicenet`      |     8 |   ✅   | 00:00:00.792 |
| `test_nmea2000`          | `native_nmea2000`       |     6 |   ✅   | 00:00:00.873 |
| `test_mbus`              | `native_mbus`           |    11 |   ✅   | 00:00:00.805 |
| `test_iec60870`          | `native_iec60870`       |     8 |   ✅   | 00:00:00.810 |
| `test_sdi12`             | `native_sdi12`          |     6 |   ✅   | 00:00:00.814 |
| `test_dmx`               | `native_dmx`            |     5 |   ✅   | 00:00:00.791 |
| `test_nmea0183`          | `native_nmea0183`       |     7 |   ✅   | 00:00:00.789 |
| `test_iolink`            | `native_iolink`         |     5 |   ✅   | 00:00:00.797 |
| `test_sse`               | `native`                |    37 |   ✅   | 00:00:01.439 |
| `test_session`           | `native`                |    19 |   ✅   | 00:00:00.710 |
| `test_presentation`      | `native`                |    63 |   ✅   | 00:00:00.762 |
| `test_transport`         | `native`                |    44 |   ✅   | 00:00:00.751 |
| `test_websocket`         | `native`                |    68 |   ✅   | 00:00:00.816 |
| `test_http_parser`       | `native`                |    93 |   ✅   | 00:00:00.746 |
| `test_observability`     | `native_observability`  |    17 |   ✅   | 00:00:00.971 |
| `test_accept_gate`       | `native_accept_gate`    |    13 |   ✅   | 00:00:01.328 |
| `test_http_ota`          | `native_ota`            |     3 |   ✅   | 00:00:00.865 |
| `test_provisioning`      | `native_prov`           |     5 |   ✅   | 00:00:00.803 |
| `test_ssh_crypto`        | `native_ssh`            |    45 |   ✅   | 00:00:04.503 |
| `test_ssh_auth`          | `native_ssh`            |    19 |   ✅   | 00:00:00.796 |
| `test_ssh_server`        | `native_ssh`            |    16 |   ✅   | 00:00:01.023 |
| `test_ssh_transport`     | `native_ssh`            |    34 |   ✅   | 00:00:01.346 |
| `test_ssh_channel`       | `native_ssh`            |    32 |   ✅   | 00:00:00.758 |
| `test_ssh_hardening`     | `native_ssh_hardened`   |     2 |   ✅   | 00:00:01.337 |
| `test_ssh_conn`          | `native_ssh_conn`       |     3 |   ✅   | 00:00:02.067 |
| `test_regex`             | `native_app`            |     9 |   ✅   | 00:00:01.664 |
| `test_template`          | `native_app`            |     6 |   ✅   | 00:00:00.788 |
| `test_path_params`       | `native_app`            |     8 |   ✅   | 00:00:00.761 |
| `test_digest_vectors`    | `native_app`            |     4 |   ✅   | 00:00:00.698 |
| `test_form_params`       | `native_app`            |     5 |   ✅   | 00:00:00.791 |
| `test_iface`             | `native_app`            |     7 |   ✅   | 00:00:00.764 |
| `test_json`              | `native_app`            |    23 |   ✅   | 00:00:00.712 |
| `test_response_headers`  | `native_app`            |    11 |   ✅   | 00:00:00.780 |
| `test_middleware`        | `native_app`            |     9 |   ✅   | 00:00:00.773 |
| `test_digest_auth`       | `native_app`            |    11 |   ✅   | 00:00:00.800 |
| `test_web_terminal`      | `native_app`            |     9 |   ✅   | 00:00:00.791 |
| `test_defer`             | `native_app`            |     3 |   ✅   | 00:00:00.738 |
| `test_multipart`         | `native_app`            |    19 |   ✅   | 00:00:00.774 |
| `test_auth`              | `native_app`            |    13 |   ✅   | 00:00:00.758 |
| `test_file_serving`      | `native_app`            |    12 |   ✅   | 00:00:00.775 |
| `test_dispatch`          | `native_app`            |    11 |   ✅   | 00:00:00.755 |
| `test_chunked`           | `native_app`            |    12 |   ✅   | 00:00:00.785 |
| `test_application`       | `native_app`            |    53 |   ✅   | 00:00:00.920 |
| `test_webdav_handler`    | `native_webdav_handler` |    12 |   ✅   | 00:00:01.723 |
| `test_diag`              | `native_diag`           |     2 |   ✅   | 00:00:01.675 |
| `test_snmp_ber`          | `native_snmp`           |    16 |   ✅   | 00:00:00.895 |
| `test_snmp_agent`        | `native_snmp`           |    19 |   ✅   | 00:00:00.678 |
| `test_snmp_v3`           | `native_snmp_v3`        |    15 |   ✅   | 00:00:02.442 |
| `test_telnet`            | `native_telnet`         |    15 |   ✅   | 00:00:01.013 |
| `test_coap`              | `native_coap`           |    41 |   ✅   | 00:00:01.119 |
| `test_coap`              | `native_coap_observe`   |    41 |   ✅   | 00:00:00.949 |
| `test_webdav`            | `native_webdav`         |    19 |   ✅   | 00:00:00.822 |
| `test_modbus`            | `native_modbus`         |    22 |   ✅   | 00:00:00.801 |
| `test_cloudevents`       | `native_cloudevents`    |     7 |   ✅   | 00:00:00.929 |
| `test_redis_resp`        | `native_redis`          |     8 |   ✅   | 00:00:00.781 |
| `test_stomp`             | `native_stomp`          |    14 |   ✅   | 00:00:00.806 |
| `test_mqtt_sn`           | `native_mqtt_sn`        |    13 |   ✅   | 00:00:00.778 |
| `test_flow_export`       | `native_flow_export`    |     6 |   ✅   | 00:00:00.777 |
| `test_protobuf`          | `native_protobuf`       |    13 |   ✅   | 00:00:00.775 |
| `test_preempt_queue`     | `native_preempt_queue`  |    11 |   ✅   | 00:00:00.854 |
| `test_dma`               | `native_dma`            |    11 |   ✅   | 00:00:00.942 |
| `test_forward`           | `native_forward`        |    16 |   ✅   | 00:00:01.032 |
| `test_gateway`           | `native_gateway`        |    11 |   ✅   | 00:00:00.943 |
| `test_lora`              | `native_lora`           |    13 |   ✅   | 00:00:00.767 |
| `test_nrf24`             | `native_nrf24`          |    10 |   ✅   | 00:00:00.767 |
| `test_enocean`           | `native_enocean`        |     9 |   ✅   | 00:00:00.790 |
| `test_pn532`             | `native_pn532`          |    10 |   ✅   | 00:00:00.774 |
| `test_sigfox`            | `native_sigfox`         |     7 |   ✅   | 00:00:00.772 |
| `test_zwave`             | `native_zwave`          |     9 |   ✅   | 00:00:00.779 |
| `test_zigbee`            | `native_zigbee`         |     9 |   ✅   | 00:00:00.782 |
| `test_thread`            | `native_thread`         |    13 |   ✅   | 00:00:00.790 |
| `test_wamp`              | `native_wamp`           |    12 |   ✅   | 00:00:00.843 |
| `test_sunspec`           | `native_sunspec`        |     5 |   ✅   | 00:00:00.778 |
| `test_c37118`            | `native_c37118`         |     6 |   ✅   | 00:00:00.759 |
| `test_dnp3`              | `native_dnp3`           |     8 |   ✅   | 00:00:00.774 |
| `test_grpcweb`           | `native_grpcweb`        |     7 |   ✅   | 00:00:00.760 |
| `test_lwm2m_tlv`         | `native_lwm2m_tlv`      |    11 |   ✅   | 00:00:00.777 |
| `test_fins`              | `native_fins`           |     6 |   ✅   | 00:00:00.774 |
| `test_hostlink`          | `native_hostlink`       |     7 |   ✅   | 00:00:00.757 |
| `test_senml`             | `native_senml`          |     9 |   ✅   | 00:00:00.863 |
| `test_df1`               | `native_df1`            |     9 |   ✅   | 00:00:00.763 |
| `test_cotp`              | `native_cotp`           |     6 |   ✅   | 00:00:00.779 |
| `test_s7comm`            | `native_s7comm`         |     8 |   ✅   | 00:00:00.768 |
| `test_melsec`            | `native_melsec`         |     6 |   ✅   | 00:00:00.766 |
| `test_bacnet`            | `native_bacnet`         |     9 |   ✅   | 00:00:00.789 |
| `test_enip`              | `native_enip`           |     6 |   ✅   | 00:00:00.770 |
| `test_amqp`              | `native_amqp`           |     7 |   ✅   | 00:00:00.751 |
| `test_cip`               | `native_cip`            |     9 |   ✅   | 00:00:00.770 |
| `test_nats`              | `native_nats`           |    14 |   ✅   | 00:00:00.767 |
| `test_proxy_protocol`    | `native_proxy_protocol` |     8 |   ✅   | 00:00:00.788 |
| `test_sparkplug`         | `native_sparkplug`      |     7 |   ✅   | 00:00:00.798 |
| `test_modbus_master`     | `native_modbus_master`  |     5 |   ✅   | 00:00:00.817 |
| `test_ota_rollback`      | `native_ota_rollback`   |     5 |   ✅   | 00:00:00.750 |
| `test_totp`              | `native_totp`           |     4 |   ✅   | 00:00:00.798 |
| `test_webhook`           | `native_webhook`        |     5 |   ✅   | 00:00:00.788 |
| `test_radio_power`       | `native_radio_power`    |     2 |   ✅   | 00:00:00.748 |
| `test_dns_resolver`      | `native_dns_resolver`   |     4 |   ✅   | 00:00:00.786 |
| `test_audit_log`         | `native_audit_log`      |    16 |   ✅   | 00:00:00.852 |
| `test_oidc`              | `native_oidc`           |    17 |   ✅   | 00:00:01.105 |
| `test_vfs`               | `native_vfs`            |    11 |   ✅   | 00:00:00.796 |
| `test_graphql`           | `native_graphql`        |    32 |   ✅   | 00:00:00.826 |
| `test_espnow`            | `native_espnow`         |     7 |   ✅   | 00:00:00.816 |
| `test_oauth2`            | `native_oauth2`         |     8 |   ✅   | 00:00:00.852 |
| `test_opcua`             | `native_opcua`          |    38 |   ✅   | 00:00:00.954 |
| `test_opcua_client`      | `native_opcua_client`   |    14 |   ✅   | 00:00:00.870 |
| `test_keepalive`         | `native_keepalive`      |    10 |   ✅   | 00:00:01.554 |
| `test_range`             | `native_range`          |    13 |   ✅   | 00:00:01.585 |
| `test_syslog`            | `native_syslog`         |    10 |   ✅   | 00:00:00.877 |
| `test_smtp`              | `native_smtp`           |    11 |   ✅   | 00:00:00.991 |
| `test_ntp_server`        | `native_ntp_server`     |     8 |   ✅   | 00:00:00.846 |
| `test_dns_server`        | `native_dns_server`     |     7 |   ✅   | 00:00:00.799 |
| `test_rtc`               | `native_rtc`            |     8 |   ✅   | 00:00:00.769 |
| `test_ld2410`            | `native_ld2410`         |     7 |   ✅   | 00:00:00.848 |
| `test_mpr121`            | `native_mpr121`         |     5 |   ✅   | 00:00:00.791 |
| `test_sht3x`             | `native_sht3x`          |     5 |   ✅   | 00:00:00.796 |
| `test_pca9685`           | `native_pca9685`        |     4 |   ✅   | 00:00:00.771 |
| `test_ads1115`           | `native_ads1115`        |     3 |   ✅   | 00:00:00.763 |
| `test_ina219`            | `native_ina219`         |     4 |   ✅   | 00:00:00.774 |
| `test_hpack`             | `native_hpack`          |     7 |   ✅   | 00:00:00.946 |
| `test_h2_frame`          | `native_h2frame`        |     6 |   ✅   | 00:00:00.780 |
| `test_h2_conn`           | `native_h2conn`         |     4 |   ✅   | 00:00:01.114 |
| `test_quic_varint`       | `native_quic_varint`    |     3 |   ✅   | 00:00:00.831 |
| `test_h3_frame`          | `native_h3frame`        |     7 |   ✅   | 00:00:00.853 |
| `test_jwt`               | `native_jwt`            |    21 |   ✅   | 00:00:00.882 |
| `test_upload`            | `native_upload`         |     3 |   ✅   | 00:00:01.601 |
| `test_http_client`       | `native_http_client`    |    15 |   ✅   | 00:00:00.839 |
| `test_compliance`        | `native_compliance`     |    15 |   ✅   | 00:00:00.823 |
| `test_mqtt`              | `native_mqtt`           |    22 |   ✅   | 00:00:00.819 |
| `test_ws_client`         | `native_ws_client`      |    16 |   ✅   | 00:00:00.906 |
| `test_scratch`           | `native_scratch`        |    14 |   ✅   | 00:00:00.870 |
| `test_snmp_trap`         | `native_snmp_trap`      |     7 |   ✅   | 00:00:00.865 |
| `test_inflate`           | `native_inflate`        |    12 |   ✅   | 00:00:00.843 |
| `test_deflate`           | `native_deflate`        |    10 |   ✅   | 00:00:00.866 |
| `test_ssh_zlib`          | `native_ssh_zlib`       |     9 |   ✅   | 00:00:00.874 |
| `test_ssh_comp`          | `native_ssh_comp`       |     5 |   ✅   | 00:00:01.421 |
| `test_websocket`         | `native_ws_deflate`     |    72 |   ✅   | 00:00:01.468 |
| `test_time_source`       | `native_time_source`    |     9 |   ✅   | 00:00:00.818 |
| `test_config_store`      | `native_config_store`   |    14 |   ✅   | 00:00:00.830 |
| `test_device_id`         | `native_device_id`      |     4 |   ✅   | 00:00:00.861 |
| `test_auth_lockout`      | `native_auth_lockout`   |    11 |   ✅   | 00:00:00.885 |
| `test_csrf`              | `native_csrf`           |     9 |   ✅   | 00:00:00.886 |
| `test_telemetry`         | `native_telemetry`      |     8 |   ✅   | 00:00:00.874 |
| `test_dashboard`         | `native_dashboard`      |    15 |   ✅   | 00:00:00.831 |
| `test_net_egress`        | `native_net_egress`     |     6 |   ✅   | 00:00:00.801 |
| `test_partition_monitor` | `native_partition`      |     5 |   ✅   | 00:00:00.818 |
| `test_cbor`              | `native_cbor`           |    18 |   ✅   | 00:00:00.859 |
| `test_msgpack`           | `native_msgpack`        |    17 |   ✅   | 00:00:00.833 |
| `test_gpio_map`          | `native_gpio_map`       |     8 |   ✅   | 00:00:00.865 |
| `test_udp_telemetry`     | `native_udp_telemetry`  |     7 |   ✅   | 00:00:00.829 |
| `test_statsd`            | `native_statsd`         |     9 |   ✅   | 00:00:00.860 |
| `test_guardrails`        | `native_guardrails`     |     6 |   ✅   | 00:00:00.820 |
| `test_failsafe`          | `native_failsafe`       |     6 |   ✅   | 00:00:00.856 |
| `test_sleep_sched`       | `native_sleep_sched`    |     8 |   ✅   | 00:00:00.821 |
| `test_wearlevel`         | `native_wearlevel`      |     5 |   ✅   | 00:00:00.796 |
| `test_netadapt`          | `native_netadapt`       |     6 |   ✅   | 00:00:00.808 |
| `test_dshot`             | `native_dshot`          |     7 |   ✅   | 00:00:00.818 |
| `test_hart`              | `native_hart`           |     6 |   ✅   | 00:00:00.817 |
| `test_nts`               | `native_nts`            |     4 |   ✅   | 00:00:00.832 |
| `test_dds`               | `native_dds`            |     4 |   ✅   | 00:00:00.814 |
| `test_xmpp`              | `native_xmpp`           |     6 |   ✅   | 00:00:00.841 |
| `test_rawl2`             | `native_rawl2`          |     4 |   ✅   | 00:00:00.811 |
| `test_spa_router`        | `native_spa_router`     |     2 |   ✅   | 00:00:00.774 |
| `test_goose`             | `native_goose`          |     3 |   ✅   | 00:00:00.790 |
| `test_mtconnect`         | `native_mtconnect`      |     4 |   ✅   | 00:00:00.838 |
| `test_j2735`             | `native_j2735`          |     9 |   ✅   | 00:00:00.819 |
| `test_nema_ts2`          | `native_nema_ts2`       |     4 |   ✅   | 00:00:00.784 |
| `test_snp`               | `native_snp`            |     4 |   ✅   | 00:00:00.794 |
| `test_directnet`         | `native_directnet`      |     4 |   ✅   | 00:00:00.790 |
| `test_sep2`              | `native_sep2`           |     5 |   ✅   | 00:00:00.807 |
| `test_profinet`          | `native_profinet`       |     4 |   ✅   | 00:00:00.804 |
| `test_ntcip`             | `native_ntcip`          |     3 |   ✅   | 00:00:00.807 |
| `test_openadr`           | `native_openadr`        |     4 |   ✅   | 00:00:00.820 |
| `test_mms`               | `native_mms`            |     4 |   ✅   | 00:00:00.821 |
| `test_cclink`            | `native_cclink`         |     4 |   ✅   | 00:00:00.813 |
| `test_powerlink`         | `native_powerlink`      |     3 |   ✅   | 00:00:00.790 |
| `test_sercos`            | `native_sercos`         |     3 |   ✅   | 00:00:00.807 |
| `test_profibus`          | `native_profibus`       |     4 |   ✅   | 00:00:00.779 |
| `test_lonworks`          | `native_lonworks`       |     4 |   ✅   | 00:00:00.767 |
| `test_mbplus`            | `native_mbplus`         |     5 |   ✅   | 00:00:00.799 |
| `test_interbus`          | `native_interbus`       |     4 |   ✅   | 00:00:00.776 |
| `test_iccp`              | `native_iccp`           |     4 |   ✅   | 00:00:00.784 |
| `test_wave`              | `native_wave`           |     4 |   ✅   | 00:00:00.785 |
| `test_utmc`              | `native_utmc`           |     5 |   ✅   | 00:00:00.774 |
| `test_ocit`              | `native_ocit`           |     4 |   ✅   | 00:00:00.809 |
| `test_atc`               | `native_atc`            |     4 |   ✅   | 00:00:00.819 |
| `test_southbound`        | `native_southbound`     |     5 |   ✅   | 00:00:00.803 |
| `test_exc_decoder`       | `native_exc_decoder`    |     5 |   ✅   | 00:00:00.822 |
| `test_http_delivery`     | `native_http_delivery`  |     6 |   ✅   | 00:00:00.833 |
| `test_hw_health`         | `native_hw_health`      |     5 |   ✅   | 00:00:00.823 |
| `test_mdns_adaptive`     | `native_mdns_adaptive`  |     4 |   ✅   | 00:00:00.776 |
| `test_sockpool`          | `native_sockpool`       |     5 |   ✅   | 00:00:00.803 |
| `test_psram_pool`        | `native_psram_pool`     |     5 |   ✅   | 00:00:00.781 |
| `test_happy_eyeballs`    | `native_happy_eyeballs` |     4 |   ✅   | 00:00:00.816 |
| `test_wifi_sniffer`      | `native_wifi_sniffer`   |     5 |   ✅   | 00:00:00.794 |
| `test_link_manager`      | `native_link_manager`   |     4 |   ✅   | 00:00:00.787 |
| `test_cc1101`            | `native_cc1101`         |    10 |   ✅   | 00:00:00.788 |
| `test_fdc2214`           | `native_fdc2214`        |     4 |   ✅   | 00:00:00.780 |
| `test_ldc1614`           | `native_ldc1614`        |     4 |   ✅   | 00:00:00.794 |
| `test_vl53l0x`           | `native_vl53l0x`        |     3 |   ✅   | 00:00:00.765 |
| `test_radio_sniff`       | `native_radio_sniff`    |     4 |   ✅   | 00:00:00.771 |
| `test_ble_gatt`          | `native_ble_gatt`       |     4 |   ✅   | 00:00:00.792 |
| `test_tls_policy`        | `native_tls_policy`     |     4 |   ✅   | 00:00:00.774 |
| `test_wisun`             | `native_wisun`          |     6 |   ✅   | 00:00:00.835 |
| `test_logbuf`            | `native_logbuf`         |     4 |   ✅   | 00:00:00.788 |
| `test_config_io`         | `native_config_io`      |     4 |   ✅   | 00:00:00.845 |
| `test_workers`           | `native_workers`        |     3 |   ✅   | 00:00:00.932 |
| `test_clock`             | `native_clock`          |     7 |   ✅   | 00:00:00.771 |
| `test_concurrency`       | `native_concurrency`    |     2 |   ✅   | 00:00:00.927 |
| `test_concurrency`       | `native_tsan`           |     2 |   ✅   | 00:00:01.483 |
| `test_qpack`             | `native_qpack`          |     9 |   ✅   | 00:00:00.912 |
| `test_quic_packet`       | `native_quic_packet`    |     8 |   ✅   | 00:00:00.793 |
| `test_quic_frame`        | `native_quic_frame`     |     8 |   ✅   | 00:00:00.813 |
| `test_quic_crypto`       | `native_quic_crypto`    |     7 |   ✅   | 00:00:00.932 |
| `test_tls13_kdf`         | `native_tls13_kdf`      |     5 |   ✅   | 00:00:00.874 |
| `test_quic_tp`           | `native_quic_tp`        |     7 |   ✅   | 00:00:00.870 |
| `test_tls13_msg`         | `native_tls13_msg`      |     7 |   ✅   | 00:00:00.972 |
| `test_quic_tls`          | `native_quic_tls`       |    11 |   ✅   | 00:00:01.263 |
| `test_quic_conn`         | `native_quic_conn`      |     4 |   ✅   | 00:00:01.360 |
| `test_h3_conn`           | `native_h3_conn`        |    11 |   ✅   | 00:00:01.297 |
| `test_h3_e2e`            | `native_h3_e2e`         |     1 |   ✅   | 00:00:01.344 |
| `test_quic_server`       | `native_quic_server`    |     2 |   ✅   | 00:00:01.410 |
| `test_h3_server`         | `native_h3_server`      |     1 |   ✅   | 00:00:02.218 |
| `test_ssh_chachapoly`    | `native_ssh_chachapoly` |     4 |   ✅   | 00:00:00.850 |

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

## test_det_ip - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the DetIp address core (network_drivers/network/det_ip): RFC 4291 text_

|   # | Test                     | Status | Description                                                                 |
| --: | :----------------------- | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_v4_round_trip`     |   ✅   | V4 round trip                                                               |
|   2 | `test_from_v6_bytes`     |   ✅   | 2001:db8::1 as raw network-order bytes -> DetIp -> canonical text.          |
|   3 | `test_is_unspecified`    |   ✅   | Is unspecified                                                              |
|   4 | `test_prefix_match`      |   ✅   | IPv4 CIDR containment (the allowlist primitive - full address, no hashing). |
|   5 | `test_v6_canonical_5952` |   ✅   | RFC 5952: lower-case, no leading zeros, longest zero run -> "::".           |
|   6 | `test_v4_mapped`         |   ✅   | V4 mapped                                                                   |
|   7 | `test_classify_v4`       |   ✅   | Classify v4                                                                 |
|   8 | `test_classify_v6`       |   ✅   | Classify v6                                                                 |
|   9 | `test_reject_malformed`  |   ✅   | Reject malformed                                                            |
|  10 | `test_equal_and_from_v4` |   ✅   | Equal and from v4                                                           |

</details>

---

## test_det_arena - ✅ 17 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the unified double-ended arena (network_drivers/session/det_arena):_

|   # | Test                                          | Status | Description                                                                        |
| --: | :-------------------------------------------- | :----: | :--------------------------------------------------------------------------------- |
|   1 | `test_persist_basic_alloc`                    |   ✅   | Persist basic alloc                                                                |
|   2 | `test_persist_zeroed`                         |   ✅   | Persist zeroed                                                                     |
|   3 | `test_persist_first_fit_reuse`                |   ✅   | Persist first fit reuse                                                            |
|   4 | `test_persist_coalesce`                       |   ✅   | Persist coalesce                                                                   |
|   5 | `test_persist_free_shrinks_boundary`          |   ✅   | Persist free shrinks boundary                                                      |
|   6 | `test_scratch_bump_and_reset`                 |   ✅   | Scratch bump and reset                                                             |
|   7 | `test_scratch_mark_release`                   |   ✅   | Scratch mark release                                                               |
|   8 | `test_persist_and_scratch_no_overlap`         |   ✅   | Persist and scratch no overlap                                                     |
|   9 | `test_boundary_collision_fail_closed`         |   ✅   | Take most of the arena from the bottom, then from the top, until they nearly meet. |
|  10 | `test_scratch_reset_frees_middle_for_persist` |   ✅   | Scratch reset frees middle for persist                                             |
|  11 | `test_alignment_various_sizes`                |   ✅   | Alignment various sizes                                                            |
|  12 | `test_scratch_alignment_16`                   |   ✅   | The real scratch callers (SSH, WS deflate) ask for 16-byte alignment.              |
|  13 | `test_zero_size_and_null_free`                |   ✅   | Zero size and null free                                                            |
|  14 | `test_set_add_limits`                         |   ✅   | Set add limits                                                                     |
|  15 | `test_set_persist_overflow_and_prefer`        |   ✅   | Set persist overflow and prefer                                                    |
|  16 | `test_set_persist_free_routes_by_address`     |   ✅   | Set persist free routes by address                                                 |
|  17 | `test_set_scratch_overflow_and_unwind`        |   ✅   | Set scratch overflow and unwind                                                    |

</details>

---

## test_ssh_ed25519 - ✅ 16 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Known-answer tests for the modern SSH crypto suite (curve25519-sha256 KEX +_

|   # | Test                                    | Status | Description                      |
| --: | :-------------------------------------- | :----: | :------------------------------- |
|   1 | `test_sha512_empty`                     |   ✅   | Sha512 empty                     |
|   2 | `test_sha512_abc`                       |   ✅   | Sha512 abc                       |
|   3 | `test_sha512_one_block_boundary`        |   ✅   | Sha512 one block boundary        |
|   4 | `test_sha512_two_block_boundary`        |   ✅   | Sha512 two block boundary        |
|   5 | `test_sha512_million_a_streaming`       |   ✅   | Sha512 million a streaming       |
|   6 | `test_sha512_streaming_matches_oneshot` |   ✅   | Sha512 streaming matches oneshot |
|   7 | `test_x25519_rfc7748_vector1`           |   ✅   | X25519 rfc7748 vector1           |
|   8 | `test_x25519_rfc7748_vector2`           |   ✅   | X25519 rfc7748 vector2           |
|   9 | `test_x25519_iterated_1`                |   ✅   | X25519 iterated 1                |
|  10 | `test_x25519_iterated_1000`             |   ✅   | X25519 iterated 1000             |
|  11 | `test_x25519_dh_agreement`              |   ✅   | X25519 dh agreement              |
|  12 | `test_ed25519_vector_empty_msg`         |   ✅   | Ed25519 vector empty msg         |
|  13 | `test_ed25519_vector_rfc8032_test2`     |   ✅   | Ed25519 vector rfc8032 test2     |
|  14 | `test_ed25519_vector_zero_seed`         |   ✅   | Ed25519 vector zero seed         |
|  15 | `test_ed25519_verify_rejects_tampering` |   ✅   | Ed25519 verify rejects tampering |
|  16 | `test_ed25519_roundtrip_long`           |   ✅   | Ed25519 roundtrip long           |

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

## test_sse - ✅ 37 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit and stress tests for the Server-Sent Events connection pool (sse.h/cpp)._

|   # | Test                                                | Status | Description                                                       |
| --: | :-------------------------------------------------- | :----: | :---------------------------------------------------------------- |
|   1 | `test_sse_pool_size`                                |   ✅   | Sse pool size                                                     |
|   2 | `test_sse_ids_match_indices_after_init`             |   ✅   | Sse ids match indices after init                                  |
|   3 | `test_sse_all_inactive_after_init`                  |   ✅   | Sse all inactive after init                                       |
|   4 | `test_sse_path_empty_after_init`                    |   ✅   | Sse path empty after init                                         |
|   5 | `test_sse_alloc_returns_non_null`                   |   ✅   | Sse alloc returns non null                                        |
|   6 | `test_sse_alloc_sets_active`                        |   ✅   | Sse alloc sets active                                             |
|   7 | `test_sse_alloc_sets_slot_id`                       |   ✅   | Sse alloc sets slot id                                            |
|   8 | `test_sse_alloc_stores_path`                        |   ✅   | Sse alloc stores path                                             |
|   9 | `test_sse_alloc_stores_different_paths_per_slot`    |   ✅   | Sse alloc stores different paths per slot                         |
|  10 | `test_sse_alloc_path_truncated_to_max`              |   ✅   | Build a path longer than MAX_PATH_LEN                             |
|  11 | `test_sse_alloc_pool_full_returns_null`             |   ✅   | Sse alloc pool full returns null                                  |
|  12 | `test_sse_alloc_sse_id_is_pool_index`               |   ✅   | First free slot is 0 → sse_id should be 0                         |
|  13 | `test_sse_find_returns_correct_conn`                |   ✅   | Sse find returns correct conn                                     |
|  14 | `test_sse_find_returns_null_when_empty`             |   ✅   | Sse find returns null when empty                                  |
|  15 | `test_sse_find_returns_null_for_different_slot`     |   ✅   | Sse find returns null for different slot                          |
|  16 | `test_sse_find_after_both_slots_allocated`          |   ✅   | Sse find after both slots allocated                               |
|  17 | `test_sse_find_checks_slot_id_not_sse_id`           |   ✅   | sse_pool[0] → slot 3; sse_find(3) must return it, not sse_find(0) |
|  18 | `test_sse_free_deactivates_slot`                    |   ✅   | Sse free deactivates slot                                         |
|  19 | `test_sse_free_restores_sse_id`                     |   ✅   | Sse free restores sse id                                          |
|  20 | `test_sse_free_makes_slot_findable_as_null`         |   ✅   | Sse free makes slot findable as null                              |
|  21 | `test_sse_free_clears_path`                         |   ✅   | Sse free clears path                                              |
|  22 | `test_sse_free_nop_on_unallocated`                  |   ✅   | Sse free nop on unallocated                                       |
|  23 | `test_sse_alloc_after_free_succeeds`                |   ✅   | Sse alloc after free succeeds                                     |
|  24 | `test_sse_free_only_frees_matching_slot`            |   ✅   | Sse free only frees matching slot                                 |
|  25 | `test_sse_write_null_data_returns_false`            |   ✅   | Sse write null data returns false                                 |
|  26 | `test_sse_write_returns_false_when_conn_not_active` |   ✅   | Sse write returns false when conn not active                      |
|  27 | `test_sse_write_returns_false_when_pcb_null`        |   ✅   | Sse write returns false when pcb null                             |
|  28 | `test_sse_write_data_only_returns_true`             |   ✅   | Sse write data only returns true                                  |
|  29 | `test_sse_write_with_event_returns_true`            |   ✅   | Sse write with event returns true                                 |
|  30 | `test_sse_write_with_id_returns_true`               |   ✅   | Sse write with id returns true                                    |
|  31 | `test_sse_write_with_all_fields_returns_true`       |   ✅   | Sse write with all fields returns true                            |
|  32 | `test_sse_write_does_not_affect_other_slots`        |   ✅   | Write to slot 0 -- slot 1 state must be unchanged                 |
|  33 | `stress_sse_alloc_free_100_cycles`                  |   ✅   | Stress - Sse alloc free 100 cycles                                |
|  34 | `stress_sse_alloc_free_both_slots_alternating`      |   ✅   | Stress - Sse alloc free both slots alternating                    |
|  35 | `stress_sse_write_100_calls`                        |   ✅   | Stress - Sse write 100 calls                                      |
|  36 | `stress_sse_find_with_full_pool`                    |   ✅   | Stress - Sse find with full pool                                  |
|  37 | `stress_sse_write_slot_isolation`                   |   ✅   | Stress - Sse write slot isolation                                 |

</details>

---

## test_session - ✅ 19 passed

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
|  15 | `test_multiple_events_drained_in_one_tick`         |   ✅   | Slot 0: dirty state → EVT_CONNECT → reset             |
|  16 | `race_external_free_between_ticks`                 |   ✅   | First tick: slot expires inside check_timeouts        |
|  17 | `race_activity_update_saves_slot_from_timeout`     |   ✅   | Race - Activity update saves slot from timeout        |
|  18 | `race_all_expire_then_idle_tick`                   |   ✅   | Race - All expire then idle tick                      |
|  19 | `race_millis_wraparound_no_spurious_timeout`       |   ✅   | last_activity close to UINT32_MAX, now just past wrap |

</details>

---

## test_presentation - ✅ 63 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit, stress, and race-condition tests for Layer 6 (Presentation)._

|   # | Test                                                 | Status | Description                                                                      |
| --: | :--------------------------------------------------- | :----: | :------------------------------------------------------------------------------- |
|   1 | `test_fn_reset_sets_parse_state_to_method`           |   ✅   | Fn reset sets parse state to method                                              |
|   2 | `test_fn_reset_sets_slot_id`                         |   ✅   | Fn reset sets slot id                                                            |
|   3 | `test_fn_reset_clears_method`                        |   ✅   | Fn reset clears method                                                           |
|   4 | `test_fn_reset_clears_path_and_idx`                  |   ✅   | Fn reset clears path and idx                                                     |
|   5 | `test_fn_reset_clears_query_raw_and_params`          |   ✅   | Fn reset clears query raw and params                                             |
|   6 | `test_fn_reset_clears_all_header_slots`              |   ✅   | Fn reset clears all header slots                                                 |
|   7 | `test_fn_reset_clears_body_fields`                   |   ✅   | Fn reset clears body fields                                                      |
|   8 | `test_fn_reset_out_of_range_is_nop`                  |   ✅   | Fn reset out of range is nop                                                     |
|   9 | `test_fn_reset_is_idempotent`                        |   ✅   | Fn reset is idempotent                                                           |
|  10 | `test_fn_get_header_null_when_no_headers`            |   ✅   | setUp already reset all slots - header_count is 0                                |
|  11 | `test_fn_get_header_finds_single_header`             |   ✅   | Fn get header finds single header                                                |
|  12 | `test_fn_get_header_finds_first_of_many`             |   ✅   | Fn get header finds first of many                                                |
|  13 | `test_fn_get_header_finds_middle_of_many`            |   ✅   | Fn get header finds middle of many                                               |
|  14 | `test_fn_get_header_finds_last_of_many`              |   ✅   | Fn get header finds last of many                                                 |
|  15 | `test_fn_get_header_case_insensitive_lowercase`      |   ✅   | Fn get header case insensitive lowercase                                         |
|  16 | `test_fn_get_header_case_insensitive_uppercase`      |   ✅   | Fn get header case insensitive uppercase                                         |
|  17 | `test_fn_get_header_returns_null_for_absent_key`     |   ✅   | Fn get header returns null for absent key                                        |
|  18 | `test_fn_get_header_does_not_bleed_across_slots`     |   ✅   | Fn get header does not bleed across slots                                        |
|  19 | `test_fn_get_query_null_when_no_params`              |   ✅   | Fn get query null when no params                                                 |
|  20 | `test_fn_get_query_finds_single_param`               |   ✅   | Fn get query finds single param                                                  |
|  21 | `test_fn_get_query_finds_first_param`                |   ✅   | Fn get query finds first param                                                   |
|  22 | `test_fn_get_query_finds_middle_param`               |   ✅   | Fn get query finds middle param                                                  |
|  23 | `test_fn_get_query_finds_last_param`                 |   ✅   | Fn get query finds last param                                                    |
|  24 | `test_fn_get_query_returns_null_for_absent_key`      |   ✅   | Fn get query returns null for absent key                                         |
|  25 | `test_fn_get_query_empty_value`                      |   ✅   | Fn get query empty value                                                         |
|  26 | `test_fn_get_query_does_not_bleed_across_slots`      |   ✅   | Fn get query does not bleed across slots                                         |
|  27 | `test_get_parses_complete`                           |   ✅   | Get parses complete                                                              |
|  28 | `test_post_body_stored`                              |   ✅   | Post body stored                                                                 |
|  29 | `test_put_parses_complete`                           |   ✅   | Put parses complete                                                              |
|  30 | `test_delete_parses_complete`                        |   ✅   | Delete parses complete                                                           |
|  31 | `test_patch_parses_complete`                         |   ✅   | Patch parses complete                                                            |
|  32 | `test_head_parses_complete`                          |   ✅   | Head parses complete                                                             |
|  33 | `test_query_single_param`                            |   ✅   | Query single param                                                               |
|  34 | `test_query_multiple_params`                         |   ✅   | Query multiple params                                                            |
|  35 | `test_body_null_terminated`                          |   ✅   | Body null terminated                                                             |
|  36 | `test_body_over_buf_size_is_413`                     |   ✅   | Content-Length > BODY_BUF_SIZE → PARSE_ENTITY_TOO_LARGE before any body is read. |
|  37 | `test_overflow_method_sets_error`                    |   ✅   | Overflow method sets error                                                       |
|  38 | `test_overflow_path_sets_414`                        |   ✅   | Overflow path sets 414                                                           |
|  39 | `test_bad_lf_after_cr_sets_error`                    |   ✅   | Null byte would terminate the C-string in push(), so use a visible non-LF byte.  |
|  40 | `test_headers_beyond_max_are_dropped`                |   ✅   | Headers beyond max are dropped                                                   |
|  41 | `test_query_params_beyond_max_are_dropped`           |   ✅   | Query params beyond max are dropped                                              |
|  42 | `test_incremental_two_pushes_completes`              |   ✅   | Incremental two pushes completes                                                 |
|  43 | `test_body_starting_with_newline_stored`             |   ✅   | Body starting with newline stored                                                |
|  44 | `test_put_body_stored`                               |   ✅   | Put body stored                                                                  |
|  45 | `test_content_length_header_stored_in_headers_array` |   ✅   | Content length header stored in headers array                                    |
|  46 | `stress_parse_reset_100_cycles`                      |   ✅   | Stress - Parse reset 100 cycles                                                  |
|  47 | `stress_all_slots_parse_simultaneously`              |   ✅   | Stress - All slots parse simultaneously                                          |
|  48 | `stress_method_at_max_7_chars_no_error`              |   ✅   | Stress - Method at max 7 chars no error                                          |
|  49 | `stress_path_at_exact_limit_no_error`                |   ✅   | Stress - Path at exact limit no error                                            |
|  50 | `stress_body_exactly_buf_size_all_stored`            |   ✅   | Stress - Body exactly buf size all stored                                        |
|  51 | `stress_exactly_max_headers_all_stored`              |   ✅   | Stress - Exactly max headers all stored                                          |
|  52 | `stress_exactly_max_query_params_all_stored`         |   ✅   | Stress - Exactly max query params all stored                                     |
|  53 | `stress_incremental_byte_by_byte_no_error`           |   ✅   | Stress - Incremental byte by byte no error                                       |
|  54 | `stress_sequential_requests_no_state_leak`           |   ✅   | Stress - Sequential requests no state leak                                       |
|  55 | `race_interleaved_producer_consumer_ring_buffer`     |   ✅   | Producer writes first 100 bytes                                                  |
|  56 | `race_ring_buffer_full_prevents_write`               |   ✅   | Race - Ring buffer full prevents write                                           |
|  57 | `race_aba_slot_reuse_fresh_timestamp`                |   ✅   | Race - Aba slot reuse fresh timestamp                                            |
|  58 | `race_double_free_is_nop`                            |   ✅   | Race - Double free is nop                                                        |
|  59 | `race_concurrent_slot_parse_isolation`               |   ✅   | Slot 0: push a full request                                                      |
|  60 | `race_reset_during_parse_header_val`                 |   ✅   | Race - Reset during parse header val                                             |
|  61 | `race_reset_during_parse_query`                      |   ✅   | Race - Reset during parse query                                                  |
|  62 | `race_reset_during_parse_body`                       |   ✅   | Race - Reset during parse body                                                   |
|  63 | `race_parse_after_complete_is_nop`                   |   ✅   | Race - Parse after complete is nop                                               |

</details>

---

## test_transport - ✅ 44 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit and stress tests for Layer 4 (Transport) - constants, pool invariants,_

|   # | Test                                             | Status | Description                                      |
| --: | :----------------------------------------------- | :----: | :----------------------------------------------- |
|   1 | `test_pool_capacity_is_four`                     |   ✅   | Pool capacity is four                            |
|   2 | `test_rx_buffer_size_is_one_kb`                  |   ✅   | Rx buffer size is one kb                         |
|   3 | `test_timeout_constant_is_5000ms`                |   ✅   | Timeout constant is 5000ms                       |
|   4 | `test_all_slots_free_after_init`                 |   ✅   | All slots free after init                        |
|   5 | `test_all_pcbs_null_after_init`                  |   ✅   | All pcbs null after init                         |
|   6 | `test_all_ring_buffers_empty_after_init`         |   ✅   | All ring buffers empty after init                |
|   7 | `test_slot_ids_match_indices`                    |   ✅   | Slot ids match indices                           |
|   8 | `test_ring_empty_when_head_equals_tail`          |   ✅   | Ring empty when head equals tail                 |
|   9 | `test_ring_wrap_at_boundary`                     |   ✅   | Ring wrap at boundary                            |
|  10 | `test_ring_full_sentinel_one_slot_reserved`      |   ✅   | Ring full sentinel one slot reserved             |
|  11 | `test_ring_can_store_size_minus_one_bytes`       |   ✅   | Ring can store size minus one bytes              |
|  12 | `test_event_types_are_distinct`                  |   ✅   | Event types are distinct                         |
|  13 | `test_timeout_does_not_fire_on_free_slot`        |   ✅   | Timeout does not fire on free slot               |
|  14 | `test_timeout_does_not_fire_before_deadline`     |   ✅   | Timeout does not fire before deadline            |
|  15 | `test_timeout_fires_at_deadline`                 |   ✅   | Timeout fires at deadline                        |
|  16 | `test_timeout_fires_only_on_stale_slots`         |   ✅   | Timeout fires only on stale slots                |
|  17 | `test_init_succeeds_on_native`                   |   ✅   | Init succeeds on native                          |
|  18 | `test_all_last_activity_ms_zero_after_init`      |   ✅   | All last activity ms zero after init             |
|  19 | `test_queue_not_null_after_init`                 |   ✅   | Queue not null after init                        |
|  20 | `stress_ring_buffer_fill_drain_integrity`        |   ✅   | Write known pattern                              |
|  21 | `stress_ring_buffer_multi_cycle_no_corruption`   |   ✅   | Stress - Ring buffer multi cycle no corruption   |
|  22 | `stress_all_slots_timeout_simultaneously`        |   ✅   | Stress - All slots timeout simultaneously        |
|  23 | `stress_timeout_arm_recover_cycle`               |   ✅   | Stress - Timeout arm recover cycle               |
|  24 | `stress_check_timeouts_high_call_rate`           |   ✅   | Stress - Check timeouts high call rate           |
|  25 | `stress_ring_buffer_byte_by_byte_fill_and_drain` |   ✅   | Stress - Ring buffer byte by byte fill and drain |
|  26 | `test_accept_throttle_blocks_over_budget`        |   ✅   | Accept throttle blocks over budget               |
|  27 | `test_accept_throttle_window_refills`            |   ✅   | Accept throttle window refills                   |
|  28 | `test_accept_throttle_handles_rollover`          |   ✅   | Accept throttle handles rollover                 |
|  29 | `test_per_ip_throttle_blocks_over_budget`        |   ✅   | Per ip throttle blocks over budget               |
|  30 | `test_per_ip_throttle_isolates_addresses`        |   ✅   | Per ip throttle isolates addresses               |
|  31 | `test_per_ip_throttle_window_refills`            |   ✅   | Per ip throttle window refills                   |
|  32 | `test_per_ip_throttle_evicts_when_full`          |   ✅   | Per ip throttle evicts when full                 |
|  33 | `test_per_ip_throttle_zero_ip_always_allowed`    |   ✅   | Per ip throttle zero ip always allowed           |
|  34 | `test_per_ip_throttle_v6_distinct`               |   ✅   | Per ip throttle v6 distinct                      |
|  35 | `test_per_ip_throttle_handles_rollover`          |   ✅   | Per ip throttle handles rollover                 |
|  36 | `test_ip_allowlist_empty_allows_all`             |   ✅   | Ip allowlist empty allows all                    |
|  37 | `test_ip_allowlist_host_match`                   |   ✅   | Ip allowlist host match                          |
|  38 | `test_ip_allowlist_cidr_match`                   |   ✅   | Ip allowlist cidr match                          |
|  39 | `test_ip_allowlist_masks_host_bits`              |   ✅   | Ip allowlist masks host bits                     |
|  40 | `test_ip_allowlist_multiple_rules`               |   ✅   | Ip allowlist multiple rules                      |
|  41 | `test_ip_allowlist_zero_prefix_matches_all`      |   ✅   | Ip allowlist zero prefix matches all             |
|  42 | `test_ip_allowlist_v6_cidr`                      |   ✅   | Ip allowlist v6 cidr                             |
|  43 | `test_ip_allowlist_rejects_bad_prefix`           |   ✅   | Ip allowlist rejects bad prefix                  |
|  44 | `test_ip_allowlist_table_full`                   |   ✅   | Ip allowlist table full                          |

</details>

---

## test_websocket - ✅ 68 passed

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

</details>

---

## test_http_parser - ✅ 93 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Comprehensive unit tests for the standalone HTTP/1.1 parser._

|   # | Test                                                     | Status | Description                                                                     |
| --: | :------------------------------------------------------- | :----: | :------------------------------------------------------------------------------ |
|   1 | `test_accessor_null_guards`                              |   ✅   | Accessor null guards                                                            |
|   2 | `test_cookie_parse_edges`                                |   ✅   | Cookie parse edges                                                              |
|   3 | `test_forwarded_ip_whitespace_and_invalid`               |   ✅   | Forwarded ip whitespace and invalid                                             |
|   4 | `test_reset_sets_parse_method_state`                     |   ✅   | Reset sets parse method state                                                   |
|   5 | `test_reset_preserves_slot_id`                           |   ✅   | Reset preserves slot id                                                         |
|   6 | `test_reset_clears_method`                               |   ✅   | Reset clears method                                                             |
|   7 | `test_reset_clears_path`                                 |   ✅   | Reset clears path                                                               |
|   8 | `test_reset_clears_header_count`                         |   ✅   | Reset clears header count                                                       |
|   9 | `test_reset_clears_body`                                 |   ✅   | Reset clears body                                                               |
|  10 | `test_reset_clears_query_count`                          |   ✅   | Reset clears query count                                                        |
|  11 | `test_feed_after_complete_does_not_change_state`         |   ✅   | Feed after complete does not change state                                       |
|  12 | `test_feed_after_error_does_not_change_state`            |   ✅   | Feed after error does not change state                                          |
|  13 | `test_feed_after_entity_too_large_does_not_change_state` |   ✅   | Feed after entity too large does not change state                               |
|  14 | `test_method_get`                                        |   ✅   | Method get                                                                      |
|  15 | `test_method_post`                                       |   ✅   | Method post                                                                     |
|  16 | `test_method_put`                                        |   ✅   | Method put                                                                      |
|  17 | `test_method_delete`                                     |   ✅   | Method delete                                                                   |
|  18 | `test_method_patch`                                      |   ✅   | Method patch                                                                    |
|  19 | `test_method_head`                                       |   ✅   | Method head                                                                     |
|  20 | `test_method_options`                                    |   ✅   | Method options                                                                  |
|  21 | `test_method_overflow_is_error`                          |   ✅   | More than 7 chars (sizeof method - 1) before a space → PARSE_ERROR              |
|  22 | `test_path_root`                                         |   ✅   | Path root                                                                       |
|  23 | `test_path_segments`                                     |   ✅   | Path segments                                                                   |
|  24 | `test_path_without_query`                                |   ✅   | Path without query                                                              |
|  25 | `test_path_overflow_is_414`                              |   ✅   | Build a path longer than MAX_PATH_LEN                                           |
|  26 | `test_single_query_param`                                |   ✅   | Single query param                                                              |
|  27 | `test_two_query_params`                                  |   ✅   | Two query params                                                                |
|  28 | `test_query_key_not_found_returns_null`                  |   ✅   | Query key not found returns null                                                |
|  29 | `test_query_empty_value`                                 |   ✅   | Query empty value                                                               |
|  30 | `test_single_header_stored`                              |   ✅   | Single header stored                                                            |
|  31 | `test_header_lookup_case_insensitive`                    |   ✅   | Header lookup case insensitive                                                  |
|  32 | `test_cookie_basic_and_positions`                        |   ✅   | Cookie basic and positions                                                      |
|  33 | `test_cookie_missing_and_no_header`                      |   ✅   | Cookie missing and no header                                                    |
|  34 | `test_cookie_exact_name_not_substring`                   |   ✅   | Cookie exact name not substring                                                 |
|  35 | `test_cookie_quoted_and_value_with_equals`               |   ✅   | Cookie quoted and value with equals                                             |
|  36 | `test_forwarded_rfc7239`                                 |   ✅   | Forwarded rfc7239                                                               |
|  37 | `test_forwarded_leftmost_client`                         |   ✅   | Both header forms list the original client leftmost.                            |
|  38 | `test_forwarded_strips_quotes_and_port`                  |   ✅   | Forwarded strips quotes and port                                                |
|  39 | `test_forwarded_ipv6_recovered_unknown_rejected`         |   ✅   | RFC 7239 §6: an IPv6 for= value is DQUOTE-wrapped + bracketed, optional :port.  |
|  40 | `test_header_leading_space_stripped`                     |   ✅   | Header leading space stripped                                                   |
|  41 | `test_content_length_header_parsed`                      |   ✅   | Content length header parsed                                                    |
|  42 | `test_content_length_in_headers_array`                   |   ✅   | Content length in headers array                                                 |
|  43 | `test_multiple_headers_stored`                           |   ✅   | Multiple headers stored                                                         |
|  44 | `test_missing_header_returns_null`                       |   ✅   | Missing header returns null                                                     |
|  45 | `test_get_no_body_completes`                             |   ✅   | Get no body completes                                                           |
|  46 | `test_post_with_body`                                    |   ✅   | Post with body                                                                  |
|  47 | `test_put_with_body`                                     |   ✅   | Put with body                                                                   |
|  48 | `test_body_starting_with_newline`                        |   ✅   | Body starting with newline                                                      |
|  49 | `test_post_content_length_zero`                          |   ✅   | Post content length zero                                                        |
|  50 | `test_body_exactly_at_buffer_limit`                      |   ✅   | Body of exactly BODY_BUF_SIZE bytes - should succeed                            |
|  51 | `test_body_null_terminated_after_complete`               |   ✅   | Body null terminated after complete                                             |
|  52 | `test_body_one_over_limit_is_413`                        |   ✅   | Content-Length == BODY_BUF_SIZE + 1 → PARSE_ENTITY_TOO_LARGE                    |
|  53 | `test_body_far_over_limit_is_413`                        |   ✅   | Body far over limit is 413                                                      |
|  54 | `test_413_no_body_bytes_fed`                             |   ✅   | Even though we detected 413, no body bytes should have been stored              |
|  55 | `test_413_header_still_stored`                           |   ✅   | Headers before the blank line must be accessible even when 413                  |
|  56 | `test_body_exactly_at_limit_is_not_413`                  |   ✅   | BODY_BUF_SIZE is the max that fits - should NOT trigger 413                     |
|  57 | `test_path_overflow_stops_feeding`                       |   ✅   | Bytes fed after URI_TOO_LONG are ignored - state must not change                |
|  58 | `test_414_path_filled_to_capacity`                       |   ✅   | Buffer fills to MAX_PATH_LEN-1 chars before overflow is detected                |
|  59 | `test_method_nul_byte_is_error`                          |   ✅   | Method nul byte is error                                                        |
|  60 | `test_method_control_char_is_error`                      |   ✅   | Method control char is error                                                    |
|  61 | `test_method_del_byte_is_error`                          |   ✅   | Method del byte is error                                                        |
|  62 | `test_method_non_tchar_symbol_is_error`                  |   ✅   | '(' is VCHAR but not tchar                                                      |
|  63 | `test_method_tchar_symbols_accepted`                     |   ✅   | '-' is a valid tchar; a custom method like "X-CMD" is valid per RFC 7230        |
|  64 | `test_path_nul_byte_is_error`                            |   ✅   | Path nul byte is error                                                          |
|  65 | `test_path_control_char_is_error`                        |   ✅   | Path control char is error                                                      |
|  66 | `test_path_del_byte_is_error`                            |   ✅   | Path del byte is error                                                          |
|  67 | `test_query_nul_byte_is_error`                           |   ✅   | Query nul byte is error                                                         |
|  68 | `test_query_control_char_is_error`                       |   ✅   | Query control char is error                                                     |
|  69 | `test_header_key_space_is_error`                         |   ✅   | Space in a field-name is not a valid tchar                                      |
|  70 | `test_header_key_nul_byte_is_error`                      |   ✅   | Header key nul byte is error                                                    |
|  71 | `test_header_key_control_char_is_error`                  |   ✅   | Header key control char is error                                                |
|  72 | `test_header_key_mid_cr_is_error`                        |   ✅   | CR in the middle of a key name must be PARSE_ERROR, not blank-line detection    |
|  73 | `test_header_key_colon_at_start_skips_header`            |   ✅   | Empty key name (colon immediately after CRLF): transition to val with empty key |
|  74 | `test_long_standard_header_key_accepted`                 |   ✅   | Regression: "Sec-WebSocket-Extensions" (24 chars) is a standard header that     |
|  75 | `test_overlong_header_key_truncated_not_error`           |   ✅   | A header name longer than MAX_KEY_LEN is capped (capacity), not rejected:       |
|  76 | `test_header_val_nul_byte_is_error`                      |   ✅   | Header val nul byte is error                                                    |
|  77 | `test_header_val_control_char_is_error`                  |   ✅   | Header val control char is error                                                |
|  78 | `test_header_val_del_byte_is_error`                      |   ✅   | Header val del byte is error                                                    |
|  79 | `test_header_val_htab_mid_value_allowed`                 |   ✅   | HTAB is valid mid-value (RFC 7230 §3.2)                                         |
|  80 | `test_header_val_leading_htab_stripped`                  |   ✅   | Leading HTAB (OWS) is stripped just like leading SP                             |
|  81 | `test_header_val_obs_text_allowed`                       |   ✅   | obs-text bytes (%x80-FF) are allowed for legacy compatibility (RFC 7230 §3.2.6) |
|  82 | `test_version_http11_recognized`                         |   ✅   | Version http11 recognized                                                       |
|  83 | `test_version_http10_recognized`                         |   ✅   | Version http10 recognized                                                       |
|  84 | `test_version_unknown_is_http_unknown`                   |   ✅   | Version unknown is http unknown                                                 |
|  85 | `test_version_reset_to_unknown`                          |   ✅   | Version reset to unknown                                                        |
|  86 | `test_bad_expect_lf_is_error`                            |   ✅   | CRLF in version line replaced by CR + X (no LF)                                 |
|  87 | `test_blank_line_non_lf_is_error`                        |   ✅   | Header block ends with CR + non-LF in the blank line                            |
|  88 | `test_slots_are_independent`                             |   ✅   | Slots are independent                                                           |
|  89 | `test_incremental_byte_by_byte`                          |   ✅   | Incremental byte by byte                                                        |
|  90 | `test_incremental_two_chunks`                            |   ✅   | Incremental two chunks                                                          |
|  91 | `stress_many_requests_same_slot`                         |   ✅   | Stress - Many requests same slot                                                |
|  92 | `stress_max_headers`                                     |   ✅   | Build a request with MAX_HEADERS header lines                                   |
|  93 | `stress_max_query_params`                                |   ✅   | Build a query string with MAX_QUERY_PARAMS parameters                           |

</details>

---

## test_observability - ✅ 17 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Transport observability (DETWS_ENABLE_OBSERVABILITY): the det_conn_on_event_

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
|   9 | `test_local_close_counts_local`                               |   ✅   | det_conn_close(slot) reads the slot's pcb, frees the slot, and counts a |
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

## test_accept_gate - ✅ 13 passed

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

## test_ssh_crypto - ✅ 45 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH crypto layer test suite._

|   # | Test                                         | Status | Description                                                                              |
| --: | :------------------------------------------- | :----: | :--------------------------------------------------------------------------------------- |
|   1 | `test_sha256_empty`                          |   ✅   | SHA256("") = e3b0c44298fc1c149afb...                                                     |
|   2 | `test_sha256_abc`                            |   ✅   | SHA256("abc") = ba7816bf8f01cfea414140de5dae2ec73b00361bbef0469...                       |
|   3 | `test_sha256_448bit`                         |   ✅   | SHA256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")                       |
|   4 | `test_sha256_streaming`                      |   ✅   | Same as test_sha256_abc but using the streaming API.                                     |
|   5 | `test_hmac_sha256_tc1`                       |   ✅   | RFC 4231 Test Case 1                                                                     |
|   6 | `test_hmac_sha256_tc2`                       |   ✅   | RFC 4231 Test Case 2                                                                     |
|   7 | `test_hmac_sha256_tc3`                       |   ✅   | RFC 4231 Test Case 3                                                                     |
|   8 | `test_hmac_sha256_streaming`                 |   ✅   | Same as tc1 but via streaming API.                                                       |
|   9 | `test_hmac_sha512_tc1`                       |   ✅   | RFC 4231 Test Case 1: Key = 0x0b x20, Data = "Hi There".                                 |
|  10 | `test_hmac_sha512_tc2`                       |   ✅   | RFC 4231 Test Case 2: Key = "Jefe", Data = "what do ya want for nothing?".               |
|  11 | `test_hmac_sha512_streaming`                 |   ✅   | Same as tc1 but via the streaming API (also exercises the 128-byte block boundary).      |
|  12 | `test_aes256ctr_encrypt`                     |   ✅   | NIST SP 800-38A, Section F.5.5                                                           |
|  13 | `test_aes256ctr_decrypt`                     |   ✅   | AES-256-CTR decrypt is identical to encrypt.                                             |
|  14 | `test_aes256ctr_multi_block`                 |   ✅   | NIST F.5.5 blocks 1-4 (64 bytes).                                                        |
|  15 | `test_aes256ctr_wipe`                        |   ✅   | After wipe, the context should be all zeros.                                             |
|  16 | `test_bn_roundtrip`                          |   ✅   | Round-trip: bytes → SshBigNum → bytes.                                                   |
|  17 | `test_bn_cmp_equal`                          |   ✅   | Bn cmp equal                                                                             |
|  18 | `test_bn_cmp_less`                           |   ✅   | Bn cmp less                                                                              |
|  19 | `test_bn_cmp_greater`                        |   ✅   | Bn cmp greater                                                                           |
|  20 | `test_bn_is_zero`                            |   ✅   | Bn is zero                                                                               |
|  21 | `test_bn_dh_validate_rejects_zero`           |   ✅   | Bn dh validate rejects zero                                                              |
|  22 | `test_bn_dh_validate_rejects_one`            |   ✅   | Bn dh validate rejects one                                                               |
|  23 | `test_bn_dh_validate_accepts_two`            |   ✅   | Bn dh validate accepts two                                                               |
|  24 | `test_expmod_exp1`                           |   ✅   | Expmod exp1                                                                              |
|  25 | `test_expmod_exp2`                           |   ✅   | Expmod exp2                                                                              |
|  26 | `test_expmod_exp3`                           |   ✅   | Expmod exp3                                                                              |
|  27 | `test_expmod_commutative`                    |   ✅   | Expmod commutative                                                                       |
|  28 | `test_rsa_pkcs1_pad_structure`               |   ✅   | With d=1, sign(msg) = m^1 mod n = m (the padded message itself).                         |
|  29 | `test_rsa_sign_verify_roundtrip`             |   ✅   | Install the real keypair into the native sign fixture.                                   |
|  30 | `test_rsa_encode_pubkey`                     |   ✅   | Rsa encode pubkey                                                                        |
|  31 | `test_rsa_verify_valid_signature`            |   ✅   | Rsa verify valid signature                                                               |
|  32 | `test_rsa_verify_rejects_tampered_signature` |   ✅   | Rsa verify rejects tampered signature                                                    |
|  33 | `test_rsa_verify_rejects_wrong_message`      |   ✅   | Rsa verify rejects wrong message                                                         |
|  34 | `test_pkt_send_recv_unencrypted`             |   ✅   | Pkt send recv unencrypted                                                                |
|  35 | `test_pkt_padding_alignment`                 |   ✅   | Packet length + padding must be multiple of 16.                                          |
|  36 | `test_pkt_seq_increments`                    |   ✅   | Pkt seq increments                                                                       |
|  37 | `test_pkt_disconnect_zeroes_state`           |   ✅   | Pkt disconnect zeroes state                                                              |
|  38 | `test_pkt_encrypted_roundtrip`               |   ✅   | Pkt encrypted roundtrip                                                                  |
|  39 | `test_pkt_chacha20poly1305_roundtrip`        |   ✅   | Install a chacha20-poly1305 session with the same key both directions, so ssh_pkt_send() |
|  40 | `test_pkt_aes_etm_sha256_roundtrip`          |   ✅   | Pkt aes etm sha256 roundtrip                                                             |
|  41 | `test_pkt_aes_etm_sha512_roundtrip`          |   ✅   | Pkt aes etm sha512 roundtrip                                                             |
|  42 | `test_pkt_encrypted_fragmented`              |   ✅   | Pkt encrypted fragmented                                                                 |
|  43 | `test_pkt_encrypted_two_packets`             |   ✅   | Pkt encrypted two packets                                                                |
|  44 | `test_ssh_kdf_canonical_mpint_k`             |   ✅   | Ssh kdf canonical mpint k                                                                |
|  45 | `test_ssh_kdf_extension_chain`               |   ✅   | Ssh kdf extension chain                                                                  |

</details>

---

## test_ssh_auth - ✅ 19 passed

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
|  17 | `test_pubkey_ed25519_valid_signature_succeeds` |   ✅   | Pubkey ed25519 valid signature succeeds           |
|  18 | `test_pubkey_tampered_signature_fails`         |   ✅   | Pubkey tampered signature fails                   |
|  19 | `test_pubkey_unauthorized_key_fails`           |   ✅   | Pubkey unauthorized key fails                     |

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

## test_ssh_transport - ✅ 34 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH transport handshake tests (RFC 4253): identification-string exchange and_

|   # | Test                                                       | Status | Description                                                                  |
| --: | :--------------------------------------------------------- | :----: | :--------------------------------------------------------------------------- |
|   1 | `test_transport_index_guards`                              |   ✅   | Transport index guards                                                       |
|   2 | `test_banner_and_build_caps`                               |   ✅   | Banner and build caps                                                        |
|   3 | `test_kexinit_parse_field_and_trunc`                       |   ✅   | Kexinit parse field and trunc                                                |
|   4 | `test_kexdh_parse_and_handle_errors`                       |   ✅   | Kexdh parse and handle errors                                                |
|   5 | `test_server_banner_format`                                |   ✅   | Server banner format                                                         |
|   6 | `test_recv_banner_complete`                                |   ✅   | Recv banner complete                                                         |
|   7 | `test_recv_banner_bare_lf`                                 |   ✅   | Recv banner bare lf                                                          |
|   8 | `test_recv_banner_split_across_reads`                      |   ✅   | Recv banner split across reads                                               |
|   9 | `test_recv_banner_skips_preamble_lines`                    |   ✅   | RFC 4253 §4.2 allows lines before the SSH identification string.             |
|  10 | `test_kexinit_build_starts_with_msg_and_stores_is`         |   ✅   | Kexinit build starts with msg and stores is                                  |
|  11 | `test_kexinit_parse_accepts_supported`                     |   ✅   | Kexinit parse accepts supported                                              |
|  12 | `test_kexinit_parse_accepts_when_ours_listed_among_others` |   ✅   | Kexinit parse accepts when ours listed among others                          |
|  13 | `test_kexinit_parse_rejects_missing_kex`                   |   ✅   | Only a KEX method we do not implement (nistp256) -> no mutual KEX -> reject. |
|  14 | `test_kexinit_parse_rejects_hostkey_we_lack`               |   ✅   | Kexinit parse rejects hostkey we lack                                        |
|  15 | `test_kexinit_parse_steers_to_curve_ed25519`               |   ✅   | Kexinit parse steers to curve ed25519                                        |
|  16 | `test_kexinit_parse_rejects_missing_cipher`                |   ✅   | Only ciphers we do not implement -> no mutual cipher -> reject.              |
|  17 | `test_kexinit_parse_selects_chacha20poly1305`              |   ✅   | Kexinit parse selects chacha20poly1305                                       |
|  18 | `test_kexinit_parse_selects_etm_mac`                       |   ✅   | Kexinit parse selects etm mac                                                |
|  19 | `test_kexinit_parse_rejects_truncated`                     |   ✅   | Kexinit parse rejects truncated                                              |
|  20 | `test_exchange_hash_matches_independent_assembly`          |   ✅   | Populate the session fields the hash reads.                                  |
|  21 | `test_exchange_hash_changes_with_input`                    |   ✅   | Exchange hash changes with input                                             |
|  22 | `test_kexdh_parse_init_extracts_e_with_padding`            |   ✅   | Kexdh parse init extracts e with padding                                     |
|  23 | `test_kexdh_parse_init_extracts_small_e`                   |   ✅   | Kexdh parse init extracts small e                                            |
|  24 | `test_kexdh_parse_init_rejects_wrong_type`                 |   ✅   | Kexdh parse init rejects wrong type                                          |
|  25 | `test_kexdh_parse_init_rejects_oversized_e`                |   ✅   | mpint with 300 magnitude bytes → exceeds 2048 bits.                          |
|  26 | `test_kexdh_build_reply_structure`                         |   ✅   | Kexdh build reply structure                                                  |
|  27 | `test_kexdh_handle_produces_reply_and_installs_keys`       |   ✅   | Kexdh handle produces reply and installs keys                                |
|  28 | `test_kexdh_handle_rejects_invalid_e`                      |   ✅   | Kexdh handle rejects invalid e                                               |
|  29 | `test_kexdh_handle_curve25519_ed25519_end_to_end`          |   ✅   | Kexdh handle curve25519 ed25519 end to end                                   |
|  30 | `test_kexdh_handle_curve25519_rejects_low_order`           |   ✅   | Kexdh handle curve25519 rejects low order                                    |
|  31 | `test_derive_keys_session_id_affects_output`               |   ✅   | Derive keys session id affects output                                        |
|  32 | `test_rekey_needed_threshold`                              |   ✅   | Rekey needed threshold                                                       |
|  33 | `test_rekey_due_volume_and_time`                           |   ✅   | Neither budget spent.                                                        |
|  34 | `test_begin_rekey_preserves_session_and_auth`              |   ✅   | Begin rekey preserves session and auth                                       |

</details>

---

## test_ssh_channel - ✅ 32 passed

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
|  21 | `test_rforward_no_cb_refused`                       |   ✅   | Rforward no cb refused                                               |
|  22 | `test_rforward_accept_specific_port`                |   ✅   | Rforward accept specific port                                        |
|  23 | `test_rforward_port0_echoes_allocated`              |   ✅   | Rforward port0 echoes allocated                                      |
|  24 | `test_rforward_no_reply_silent`                     |   ✅   | Rforward no reply silent                                             |
|  25 | `test_rforward_cancel`                              |   ✅   | Rforward cancel                                                      |
|  26 | `test_global_unknown_request`                       |   ✅   | Global unknown request                                               |
|  27 | `test_global_malformed`                             |   ✅   | Global malformed                                                     |
|  28 | `test_forwarded_open_builds_channel`                |   ✅   | Forwarded open builds channel                                        |
|  29 | `test_forwarded_confirm_opens_channel`              |   ✅   | Forwarded confirm opens channel                                      |
|  30 | `test_forwarded_failure_frees_channel`              |   ✅   | Forwarded failure frees channel                                      |
|  31 | `test_forwarded_confirm_unknown_rejected`           |   ✅   | Forwarded confirm unknown rejected                                   |
|  32 | `test_forwarded_inbound_data_routes_to_forward_cb`  |   ✅   | Forwarded inbound data routes to forward cb                          |

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

## test_ssh_conn - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH transport-glue test: drives a PROTO_SSH connection through the real_

|   # | Test                                            | Status | Description                              |
| --: | :---------------------------------------------- | :----: | :--------------------------------------- |
|   1 | `test_accept_sends_server_banner`               |   ✅   | Accept sends server banner               |
|   2 | `test_banner_then_kexinit_advances_and_replies` |   ✅   | Banner then kexinit advances and replies |
|   3 | `test_poll_triggers_server_rekey`               |   ✅   | Poll triggers server rekey               |

</details>

---

## test_regex - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for bounded regex routes (DetWebServer::on_regex())._

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

</details>

---

## test_template - ✅ 6 passed

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

## test_path_params - ✅ 8 passed

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

## test_digest_vectors - ✅ 4 passed

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

## test_form_params - ✅ 5 passed

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

## test_iface - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for per-route STA/AP interface filters (DetWebServer::on(..., DetIface))._

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

## test_json - ✅ 23 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the zero-heap JSON helper: JsonWriter (serialization) and the_

|   # | Test                                                    | Status | Description                                                          |
| --: | :------------------------------------------------------ | :----: | :------------------------------------------------------------------- |
|   1 | `test_writer_simple_object`                             |   ✅   | Writer simple object                                                 |
|   2 | `test_writer_nested_and_array`                          |   ✅   | Writer nested and array                                              |
|   3 | `test_writer_value_types`                               |   ✅   | Writer value types                                                   |
|   4 | `test_writer_escapes_strings`                           |   ✅   | Writer escapes strings                                               |
|   5 | `test_writer_control_char_unicode_escape`               |   ✅   | Writer control char unicode escape                                   |
|   6 | `test_writer_overflow_sets_not_ok_and_stays_terminated` |   ✅   | Writer overflow sets not ok and stays terminated                     |
|   7 | `test_writer_depth_overflow_sets_not_ok`                |   ✅   | Writer depth overflow sets not ok                                    |
|   8 | `test_reader_get_string`                                |   ✅   | Reader get string                                                    |
|   9 | `test_reader_get_int`                                   |   ✅   | Reader get int                                                       |
|  10 | `test_reader_get_bool`                                  |   ✅   | Reader get bool                                                      |
|  11 | `test_reader_only_matches_top_level_key`                |   ✅   | "x" exists both nested and at top level; the top-level one must win. |
|  12 | `test_reader_missing_key`                               |   ✅   | Reader missing key                                                   |
|  13 | `test_reader_type_mismatch`                             |   ✅   | "name" is a string, not an int or bool.                              |
|  14 | `test_reader_unescapes_value`                           |   ✅   | Reader unescapes value                                               |
|  15 | `test_reader_unicode_escape_to_byte`                    |   ✅   | Reader unicode escape to byte                                        |
|  16 | `test_reader_truncates_to_capacity`                     |   ✅   | Reader truncates to capacity                                         |
|  17 | `test_reader_negative_int`                              |   ✅   | Reader negative int                                                  |
|  18 | `test_writer_null_and_remaining_escapes`                |   ✅   | Writer null and remaining escapes                                    |
|  19 | `test_reader_null_guards`                               |   ✅   | Reader null guards                                                   |
|  20 | `test_reader_all_escapes`                               |   ✅   | Reader all escapes                                                   |
|  21 | `test_reader_unicode_hex_case`                          |   ✅   | Reader unicode hex case                                              |
|  22 | `test_reader_false_bool`                                |   ✅   | Reader false bool                                                    |
|  23 | `test_reader_malformed`                                 |   ✅   | Reader malformed                                                     |

</details>

---

## test_response_headers - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for custom response headers and cookies:_

|   # | Test                                       | Status | Description                              |
| --: | :----------------------------------------- | :----: | :--------------------------------------- |
|   1 | `test_date_header_emitted_when_time_set`   |   ✅   | Date header emitted when time set        |
|   2 | `test_date_header_omitted_when_clockless`  |   ✅   | Date header omitted when clockless       |
|   3 | `test_single_custom_header_present`        |   ✅   | Single custom header present             |
|   4 | `test_multiple_custom_headers_present`     |   ✅   | Multiple custom headers present          |
|   5 | `test_set_cookie_basic`                    |   ✅   | Set cookie basic                         |
|   6 | `test_set_cookie_with_attrs`               |   ✅   | Set cookie with attrs                    |
|   7 | `test_custom_header_on_send_empty`         |   ✅   | Custom header on send empty              |
|   8 | `test_custom_header_on_redirect`           |   ✅   | Custom header on redirect                |
|   9 | `test_headers_do_not_leak_across_requests` |   ✅   | First request queues X-Custom on slot 0. |
|  10 | `test_clear_response_headers`              |   ✅   | Clear response headers                   |
|  11 | `test_oversized_header_dropped_whole`      |   ✅   | Oversized header dropped whole           |

</details>

---

## test_middleware - ✅ 9 passed

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

## test_digest_auth - ✅ 11 passed

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

## test_web_terminal - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the WebSocket web-serial terminal (DETWS_ENABLE_WEB_TERMINAL):_

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

## test_defer - ✅ 3 passed

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

## test_multipart - ✅ 19 passed

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

</details>

---

## test_auth - ✅ 13 passed

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

## test_file_serving - ✅ 12 passed

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

## test_dispatch - ✅ 11 passed

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

## test_chunked - ✅ 12 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for send_chunked() / ChunkedResponse streaming responses._

|   # | Test                                              | Status | Description                                |
| --: | :------------------------------------------------ | :----: | :----------------------------------------- |
|   1 | `test_headers_announce_chunked_no_content_length` |   ✅   | Headers announce chunked no content length |
|   2 | `test_single_chunk_framing`                       |   ✅   | Single chunk framing                       |
|   3 | `test_multiple_chunks_in_order`                   |   ✅   | Multiple chunks in order                   |
|   4 | `test_printf_chunk`                               |   ✅   | Printf chunk                               |
|   5 | `test_single_piece_then_terminator`               |   ✅   | Single piece then terminator               |
|   6 | `test_empty_body_is_just_terminator`              |   ✅   | Empty body is just terminator              |
|   7 | `test_large_chunked_body_not_truncated`           |   ✅   | Large chunked body not truncated           |
|   8 | `test_head_sends_headers_only`                    |   ✅   | Head sends headers only                    |
|   9 | `test_custom_header_injected_into_chunked`        |   ✅   | Custom header injected into chunked        |
|  10 | `test_log_hook_reports_total_body_length`         |   ✅   | Log hook reports total body length         |
|  11 | `test_http10_falls_back_to_close_delimited`       |   ✅   | Http10 falls back to close delimited       |
|  12 | `test_http10_large_body_not_truncated`            |   ✅   | Http10 large body not truncated            |

</details>

---

## test_application - ✅ 53 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit, stress, and race-condition tests for Layer 7 (Application)._

|   # | Test                                                  | Status | Description                                                                |
| --: | :---------------------------------------------------- | :----: | :------------------------------------------------------------------------- |
|   1 | `test_handler_reads_body`                             |   ✅   | Handler reads body                                                         |
|   2 | `test_handler_reads_query_param`                      |   ✅   | Handler reads query param                                                  |
|   3 | `test_handler_reads_header`                           |   ✅   | Handler reads header                                                       |
|   4 | `test_wildcard_before_exact_wildcard_wins`            |   ✅   | Wildcard before exact wildcard wins                                        |
|   5 | `test_fn_on_registers_and_dispatches`                 |   ✅   | Fn on registers and dispatches                                             |
|   6 | `test_fn_on_path_copied_null_terminated`              |   ✅   | A path of exactly MAX_PATH_LEN-1 chars must not overflow the route buffer. |
|   7 | `test_fn_on_table_full_extra_routes_dropped`          |   ✅   | Fill the table; on() beyond MAX_ROUTES must silently drop                  |
|   8 | `test_fn_on_same_path_different_methods_are_distinct` |   ✅   | Fn on same path different methods are distinct                             |
|   9 | `test_fn_on_not_found_called_when_no_match`           |   ✅   | Fn on not found called when no match                                       |
|  10 | `test_fn_on_not_found_not_called_when_match_exists`   |   ✅   | Fn on not found not called when match exists                               |
|  11 | `test_fn_set_cors_options_preflight_clears_slot`      |   ✅   | Fn set cors options preflight clears slot                                  |
|  12 | `test_fn_set_cors_empty_string_disables`              |   ✅   | Fn set cors empty string disables                                          |
|  13 | `test_wrong_method_does_not_match`                    |   ✅   | Wrong method does not match                                                |
|  14 | `test_wrong_path_does_not_match`                      |   ✅   | Wrong path does not match                                                  |
|  15 | `test_all_http_methods_dispatched`                    |   ✅   | All http methods dispatched                                                |
|  16 | `test_root_path_matches_exactly`                      |   ✅   | Root path matches exactly                                                  |
|  17 | `test_root_path_does_not_match_subpath`               |   ✅   | Root path does not match subpath                                           |
|  18 | `test_wildcard_matches_any_suffix`                    |   ✅   | Wildcard matches any suffix                                                |
|  19 | `test_wildcard_does_not_match_unrelated_prefix`       |   ✅   | Wildcard does not match unrelated prefix                                   |
|  20 | `test_exact_route_wins_when_registered_first`         |   ✅   | Exact route wins when registered first                                     |
|  21 | `test_slot_not_stuck_in_complete_after_handle`        |   ✅   | Slot not stuck in complete after handle                                    |
|  22 | `test_parse_error_slot_auto_reset`                    |   ✅   | Parse error slot auto reset                                                |
|  23 | `stress_last_route_dispatched_in_full_table`          |   ✅   | Stress - Last route dispatched in full table                               |
|  24 | `stress_sequential_requests_no_state_leak`            |   ✅   | Stress - Sequential requests no state leak                                 |
|  25 | `stress_all_slots_dispatched_simultaneously`          |   ✅   | Stress - All slots dispatched simultaneously                               |
|  26 | `stress_wildcard_matches_many_paths`                  |   ✅   | Stress - Wildcard matches many paths                                       |
|  27 | `stress_handle_with_no_complete_slots_is_nop`         |   ✅   | All slots in PARSE_METHOD (setUp resets them) - nothing to dispatch        |
|  28 | `race_slot_complete_between_handle_calls`             |   ✅   | Race - Slot complete between handle calls                                  |
|  29 | `race_conn_freed_after_parse_complete`                |   ✅   | Race - Conn freed after parse complete                                     |
|  30 | `race_double_handle_no_double_dispatch`               |   ✅   | Race - Double handle no double dispatch                                    |
|  31 | `race_error_and_valid_slot_in_same_handle`            |   ✅   | Slot 0: inject a parse error                                               |
|  32 | `race_callback_manually_resets_slot`                  |   ✅   | Race - Callback manually resets slot                                       |
|  33 | `test_uri_too_long_auto_resets_slot`                  |   ✅   | Overflow the path buffer - handle() should send 414 and free the slot      |
|  34 | `test_transfer_encoding_chunked_is_501`               |   ✅   | A request advertising Transfer-Encoding must be rejected with 501          |
|  35 | `test_transfer_encoding_identity_is_501`              |   ✅   | Even "identity" is rejected - we advertise no TE support at all            |
|  36 | `test_redirect_emits_location_and_status`             |   ✅   | Redirect emits location and status                                         |
|  37 | `test_redirect_invalid_code_defaults_to_302`          |   ✅   | Redirect invalid code defaults to 302                                      |
|  38 | `test_mime_type_detection`                            |   ✅   | Mime type detection                                                        |
|  39 | `test_serve_static_file_and_mime`                     |   ✅   | Serve static file and mime                                                 |
|  40 | `test_serve_static_index_fallback`                    |   ✅   | Serve static index fallback                                                |
|  41 | `test_serve_static_gzip_when_accepted`                |   ✅   | Serve static gzip when accepted                                            |
|  42 | `test_serve_static_no_gzip_when_not_accepted`         |   ✅   | Serve static no gzip when not accepted                                     |
|  43 | `test_serve_static_traversal_not_leaked`              |   ✅   | Serve static traversal not leaked                                          |
|  44 | `test_serve_static_missing_is_404`                    |   ✅   | Serve static missing is 404                                                |
|  45 | `test_serve_static_etag_conditional_get`              |   ✅   | First GET: 200 with an ETag header.                                        |
|  46 | `test_serve_static_inm_star_list_weak`                |   ✅   | First GET to capture the strong ETag (with quotes).                        |
|  47 | `test_serve_static_last_modified_conditional_get`     |   ✅   | (1) plain GET: 200 carries the Last-Modified header.                       |
|  48 | `test_serve_static_if_modified_since_malformed`       |   ✅   | Serve static if modified since malformed                                   |
|  49 | `test_serve_static_cache_control`                     |   ✅   | Serve static cache control                                                 |
|  50 | `test_request_log_hook_fires`                         |   ✅   | Request log hook fires                                                     |
|  51 | `test_stats_endpoint_emits_json`                      |   ✅   | Stats endpoint emits json                                                  |
|  52 | `test_sse_broadcast_after_upgrade_matches_path`       |   ✅   | Sse broadcast after upgrade matches path                                   |
|  53 | `test_metrics_emits_prometheus`                       |   ✅   | Metrics emits prometheus                                                   |

</details>

---

## test_webdav_handler - ✅ 12 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for the WebDAV request handler's recursive filesystem operations_

|   # | Test                                   | Status | Description                     |
| --: | :------------------------------------- | :----: | :------------------------------ |
|   1 | `test_copy_collection_recursive`       |   ✅   | Copy collection recursive       |
|   2 | `test_copy_collection_depth0_shallow`  |   ✅   | Copy collection depth0 shallow  |
|   3 | `test_copy_overwrite_semantics`        |   ✅   | Copy overwrite semantics        |
|   4 | `test_move_collection_recursive`       |   ✅   | Move collection recursive       |
|   5 | `test_delete_collection_recursive`     |   ✅   | Delete collection recursive     |
|   6 | `test_propfind_depth0_collection_only` |   ✅   | Propfind depth0 collection only |
|   7 | `test_propfind_depth1_lists_members`   |   ✅   | Propfind depth1 lists members   |
|   8 | `test_mkcol_create_and_conflict`       |   ✅   | Mkcol create and conflict       |
|   9 | `test_delete_single_file`              |   ✅   | Delete single file              |
|  10 | `test_options_advertises_dav`          |   ✅   | Options advertises dav          |
|  11 | `test_get_file_through_mount`          |   ✅   | Get file through mount          |
|  12 | `test_lock_unlock_advisory`            |   ✅   | Lock unlock advisory            |

</details>

---

## test_diag - ✅ 2 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Exercises the runtime build-flag reporter (server.diag() / DETWS_ENABLE_DIAG):_

|   # | Test                               | Status | Description                 |
| --: | :--------------------------------- | :----: | :-------------------------- |
|   1 | `test_diag_serves_build_info_json` |   ✅   | Diag serves build info json |
|   2 | `test_diag_json_braces_balanced`   |   ✅   | Diag json braces balanced   |

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

## test_telnet - ✅ 15 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Telnet server test: drives a PROTO_TELNET connection through the real_

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

## test_keepalive - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_HTTP/1.1 keep-alive (DETWS_ENABLE_KEEPALIVE). Each test drives one or more_

|   # | Test                                     | Status | Description                                                                |
| --: | :--------------------------------------- | :----: | :------------------------------------------------------------------------- |
|   1 | `test_http11_default_keeps_alive`        |   ✅   | Http11 default keeps alive                                                 |
|   2 | `test_http11_explicit_close`             |   ✅   | Http11 explicit close                                                      |
|   3 | `test_http10_default_closes`             |   ✅   | Http10 default closes                                                      |
|   4 | `test_http10_explicit_keepalive`         |   ✅   | Http10 explicit keepalive                                                  |
|   5 | `test_connection_token_list_close`       |   ✅   | "close" appearing in a token list must still be honored.                   |
|   6 | `test_two_sequential_requests_same_slot` |   ✅   | Two sequential requests same slot                                          |
|   7 | `test_pipelined_requests`                |   ✅   | Two requests delivered in one shot: the proactive drain in handle() must   |
|   8 | `test_404_still_keeps_alive`             |   ✅   | A well-formed request to an unknown path is a normal response, not an      |
|   9 | `test_max_requests_cap_closes`           |   ✅   | DETWS_KEEPALIVE_MAX_REQUESTS=3: the 3rd response closes the connection.    |
|  10 | `test_fresh_connection_resets_count`     |   ✅   | Run a slot up to the cap, then re-open it (new connection) and confirm the |

</details>

---

## test_range - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_HTTP Range requests / 206 Partial Content (DETWS_ENABLE_RANGE). Each test_

|   # | Test                                      | Status | Description                        |
| --: | :---------------------------------------- | :----: | :--------------------------------- |
|   1 | `test_no_range_full_200`                  |   ✅   | No range full 200                  |
|   2 | `test_range_prefix`                       |   ✅   | Range prefix                       |
|   3 | `test_range_open_ended`                   |   ✅   | Range open ended                   |
|   4 | `test_range_suffix`                       |   ✅   | Range suffix                       |
|   5 | `test_range_single_byte`                  |   ✅   | Range single byte                  |
|   6 | `test_range_clamped_to_eof`               |   ✅   | Range clamped to eof               |
|   7 | `test_range_unsatisfiable_416`            |   ✅   | Range unsatisfiable 416            |
|   8 | `test_malformed_range_ignored`            |   ✅   | Malformed range ignored            |
|   9 | `test_range_overflow_start_unsatisfiable` |   ✅   | Range overflow start unsatisfiable |
|  10 | `test_range_overflow_end_clamps`          |   ✅   | Range overflow end clamps          |
|  11 | `test_range_suffix_zero_unsatisfiable`    |   ✅   | Range suffix zero unsatisfiable    |
|  12 | `test_multirange_falls_back_to_200`       |   ✅   | Multirange falls back to 200       |
|  13 | `test_head_with_range_no_body`            |   ✅   | Head with range no body            |

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

## test_smtp - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SMTP client dialogue engine (services/smtp/smtp_run). A scripted_

|   # | Test                               | Status | Description                 |
| --: | :--------------------------------- | :----: | :-------------------------- |
|   1 | `test_happy_path_no_auth`          |   ✅   | Happy path no auth          |
|   2 | `test_auth_login`                  |   ✅   | Auth login                  |
|   3 | `test_auth_rejected`               |   ✅   | Auth rejected               |
|   4 | `test_greeting_not_ready`          |   ✅   | Greeting not ready          |
|   5 | `test_rcpt_rejected`               |   ✅   | Rcpt rejected               |
|   6 | `test_data_refused`                |   ✅   | Data refused                |
|   7 | `test_dot_stuffing`                |   ✅   | Dot stuffing                |
|   8 | `test_multiline_reply_and_lf_body` |   ✅   | Multiline reply and lf body |
|   9 | `test_partial_reads_dribble`       |   ✅   | Partial reads dribble       |
|  10 | `test_missing_required_arg`        |   ✅   | Missing required arg        |
|  11 | `test_io_error_when_server_hangs`  |   ✅   | Io error when server hangs  |

</details>

---

## test_ntp_server - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the NTP server response codec (services/ntp_server_build_response): a pure_

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

## test_dns_server - ✅ 7 passed

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

</details>

---

## test_rtc - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the DS1307/DS3231 RTC conversions (services/rtc): the BCD time registers_

|   # | Test                            | Status | Description                                                                          |
| --: | :------------------------------ | :----: | :----------------------------------------------------------------------------------- |
|   1 | `test_known_epoch_2000`         |   ✅   | Known epoch 2000                                                                     |
|   2 | `test_decode_datetime`          |   ✅   | Decode datetime                                                                      |
|   3 | `test_12hour_mode_equivalence`  |   ✅   | 14:00 as 24-hour (0x14) and as 12-hour PM 2 (0x40                                    | 0x20 | 0x02) must be the same time. |
|   4 | `test_12hour_midnight_and_noon` |   ✅   | 12hour midnight and noon                                                             |
|   5 | `test_roundtrip_over_range`     |   ✅   | Roundtrip over range                                                                 |
|   6 | `test_leap_day`                 |   ✅   | Leap day                                                                             |
|   7 | `test_masks_ch_and_century`     |   ✅   | The DS1307 clock-halt bit (sec bit7) and the DS3231 century bit (month bit7) must be |
|   8 | `test_invalid_guards`           |   ✅   | Invalid guards                                                                       |

</details>

---

## test_ld2410 - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the LD2410 mmWave radar codec (services/ld2410): decoding a basic and an_

|   # | Test                              | Status | Description                |
| --: | :-------------------------------- | :----: | :------------------------- |
|   1 | `test_parse_basic`                |   ✅   | Parse basic                |
|   2 | `test_parse_engineering`          |   ✅   | Parse engineering          |
|   3 | `test_reject_malformed`           |   ✅   | bad header                 |
|   4 | `test_stream_resync_and_split`    |   ✅   | Stream resync and split    |
|   5 | `test_stream_absurd_length_drops` |   ✅   | Stream absurd length drops |
|   6 | `test_helpers`                    |   ✅   | Helpers                    |
|   7 | `test_command_encoders`           |   ✅   | Command encoders           |

</details>

---

## test_mpr121 - ✅ 5 passed

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

</details>

---

## test_sht3x - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Sensirion SHT3x codec (services/sht3x): the CRC-8 against the datasheet_

|   # | Test                         | Status | Description                            |
| --: | :--------------------------- | :----: | :------------------------------------- |
|   1 | `test_crc8_datasheet_vector` |   ✅   | Crc8 datasheet vector                  |
|   2 | `test_conversion`            |   ✅   | Endpoints of the linear map are exact. |
|   3 | `test_parse_valid`           |   ✅   | Parse valid                            |
|   4 | `test_parse_bad_crc`         |   ✅   | Parse bad crc                          |
|   5 | `test_parse_null_out`        |   ✅   | Parse null out                         |

</details>

---

## test_pca9685 - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the PCA9685 PWM/servo codec (services/pca9685): the PRESCALE computation from a_

|   # | Test                 | Status | Description                                                           |
| --: | :------------------- | :----: | :-------------------------------------------------------------------- |
|   1 | `test_prescale`      |   ✅   | Prescale                                                              |
|   2 | `test_channel_reg`   |   ✅   | Channel reg                                                           |
|   3 | `test_us_to_count`   |   ✅   | Us to count                                                           |
|   4 | `test_set_pwm_bytes` |   ✅   | channel 0, on=0, off=307 (0x133) -> reg 0x06, off_l 0x33, off_h 0x01. |

</details>

---

## test_ads1115 - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the ADS1115 ADC codec (services/ads1115): building the 16-bit config word for a_

|   # | Test                    | Status | Description                                                                   |
| --: | :---------------------- | :----: | :---------------------------------------------------------------------------- |
|   1 | `test_config_word`      |   ✅   | ch0, +/-4.096V, 128 SPS: OS                                                   | MUX_AIN0 | PGA1 | MODE_SINGLE | DR128 | COMP_DISABLE. |
|   2 | `test_config_fallbacks` |   ✅   | Out-of-range channel/gain/dr fall back to ch0 / +/-2.048V / 128 SPS = 0xC583. |
|   3 | `test_raw_to_uv`        |   ✅   | gain 1 (+/-4.096 V) -> 125 uV/LSB.                                            |

</details>

---

## test_ina219 - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the INA219 current/power codec (services/ina219): decoding the bus-voltage_

|   # | Test                     | Status | Description                                                              |
| --: | :----------------------- | :----: | :----------------------------------------------------------------------- |
|   1 | `test_bus_mv`            |   ✅   | 3300 mV -> value 825 (0x339) in bits [15:3] -> register 825<<3 = 0x19C8. |
|   2 | `test_shunt_uv`          |   ✅   | Shunt uv                                                                 |
|   3 | `test_calibration`       |   ✅   | Calibration                                                              |
|   4 | `test_current_and_power` |   ✅   | current = raw * current_LSB (uA); power = raw * 20 * current_LSB (uW).   |

</details>

---

## test_hpack - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the HPACK codec (network_drivers/presentation/http2/hpack) against the RFC 7541_

|   # | Test                           | Status | Description                                                         |
| --: | :----------------------------- | :----: | :------------------------------------------------------------------ |
|   1 | `test_int_coding`              |   ✅   | C.1.1: 10, prefix 5 -> 0x0a                                         |
|   2 | `test_huffman`                 |   ✅   | Huffman                                                             |
|   3 | `test_decode_c31_and_index`    |   ✅   | RFC 7541 C.3.1: GET / with :authority www.example.com (no Huffman). |
|   4 | `test_dynamic_eviction`        |   ✅   | Dynamic eviction                                                    |
|   5 | `test_encode_static`           |   ✅   | Encode static                                                       |
|   6 | `test_encode_decode_roundtrip` |   ✅   | Encode decode roundtrip                                             |
|   7 | `test_reject_malformed`        |   ✅   | Reject malformed                                                    |

</details>

---

## test_h2_frame - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the HTTP/2 frame layer (network_drivers/presentation/http2/h2_frame, RFC 9113):_

|   # | Test                        | Status | Description                                                       |
| --: | :-------------------------- | :----: | :---------------------------------------------------------------- |
|   1 | `test_header_roundtrip`     |   ✅   | Header roundtrip                                                  |
|   2 | `test_settings_build_parse` |   ✅   | Settings build parse                                              |
|   3 | `test_settings_validation`  |   ✅   | Settings validation                                               |
|   4 | `test_control_frames`       |   ✅   | SETTINGS ACK: length 0, type 4, flags ACK, stream 0               |
|   5 | `test_headers_and_data`     |   ✅   | HEADERS stream 1, one HPACK byte, end_stream -> flags END_HEADERS | END_STREAM = 0x05 |
|   6 | `test_preface`              |   ✅   | Preface                                                           |

</details>

---

## test_h2_conn - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the HTTP/2 connection engine (network_drivers/presentation/http2/h2_conn,_

|   # | Test                       | Status | Description         |
| --: | :------------------------- | :----: | :------------------ |
|   1 | `test_init_and_request`    |   ✅   | Init and request    |
|   2 | `test_respond_roundtrip`   |   ✅   | Respond roundtrip   |
|   3 | `test_ping_and_split_recv` |   ✅   | Ping and split recv |
|   4 | `test_bad_preface`         |   ✅   | Bad preface         |

</details>

---

## test_quic_varint - ✅ 3 passed

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

## test_h3_frame - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the HTTP/3 framing layer (network_drivers/presentation/http3/h3_frame, RFC 9114_

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

## test_jwt - ✅ 21 passed

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

</details>

---

## test_upload - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Streaming file upload (DETWS_ENABLE_UPLOAD): a POST body is streamed straight_

|   # | Test                               | Status | Description                 |
| --: | :--------------------------------- | :----: | :-------------------------- |
|   1 | `test_upload_streams_body_to_file` |   ✅   | Upload streams body to file |
|   2 | `test_small_body_single_chunk`     |   ✅   | Small body single chunk     |
|   3 | `test_empty_body_not_streamed`     |   ✅   | Empty body not streamed     |

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

## test_ssh_zlib - ✅ 9 passed

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

## test_ssh_comp - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Integration test for SSH server-to-client compression WIRING (network_drivers/presentation/ssh):_

|   # | Test                                 | Status | Description                   |
| --: | :----------------------------------- | :----: | :---------------------------- |
|   1 | `test_delayed_activation`            |   ✅   | Delayed activation            |
|   2 | `test_immediate_activation`          |   ✅   | Immediate activation          |
|   3 | `test_none_never_activates`          |   ✅   | None never activates          |
|   4 | `test_packet_layer_stream_roundtrip` |   ✅   | Packet layer stream roundtrip |
|   5 | `test_packet_layer_window_slide`     |   ✅   | Packet layer window slide     |

</details>

---

## test_websocket - ✅ 72 passed

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
|  66 | `test_ws_outbound_incompressible_not_flagged`          |   ✅   | Ws outbound incompressible not flagged                                    |
|  67 | `test_ws_outbound_fragmentation`                       |   ✅   | Ws outbound fragmentation                                                 |
|  68 | `stress_ws_parse_reset_100_cycles`                     |   ✅   | Stress - Ws parse reset 100 cycles                                        |
|  69 | `stress_ws_alloc_free_pool_cycle`                      |   ✅   | Stress - Ws alloc free pool cycle                                         |
|  70 | `stress_ws_parse_incremental_byte_by_byte`             |   ✅   | Stress - Ws parse incremental byte by byte                                |
|  71 | `stress_ws_parse_max_payload`                          |   ✅   | Stress - Ws parse max payload                                             |
|  72 | `stress_ws_parse_two_consecutive_frames`               |   ✅   | First frame                                                               |

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

## test_auth_lockout - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the per-peer brute-force auth lockout (services/auth_lockout)._

|   # | Test                                      | Status | Description                        |
| --: | :---------------------------------------- | :----: | :--------------------------------- |
|   1 | `test_below_threshold_not_locked`         |   ✅   | Below threshold not locked         |
|   2 | `test_locks_at_threshold`                 |   ✅   | Locks at threshold                 |
|   3 | `test_exponential_backoff`                |   ✅   | Exponential backoff                |
|   4 | `test_caps_at_max`                        |   ✅   | Caps at max                        |
|   5 | `test_expires_after_window`               |   ✅   | Expires after window               |
|   6 | `test_success_clears`                     |   ✅   | Success clears                     |
|   7 | `test_isolates_addresses`                 |   ✅   | Isolates addresses                 |
|   8 | `test_v6_distinct_from_v4_and_each_other` |   ✅   | V6 distinct from v4 and each other |
|   9 | `test_zero_ip_never_locked`               |   ✅   | Zero ip never locked               |
|  10 | `test_table_full_tracks_new_address`      |   ✅   | Table full tracks new address      |
|  11 | `test_active_lockout_survives_eviction`   |   ✅   | Active lockout survives eviction   |

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

## test_statsd - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the StatsD client (services/statsd): the pure line formatter_

|   # | Test                             | Status | Description               |
| --: | :------------------------------- | :----: | :------------------------ |
|   1 | `test_format_types`              |   ✅   | Format types              |
|   2 | `test_format_sample_rate`        |   ✅   | Format sample rate        |
|   3 | `test_format_tags_and_both`      |   ✅   | Format tags and both      |
|   4 | `test_format_guards`             |   ✅   | Format guards             |
|   5 | `test_emit_counter_and_negative` |   ✅   | Emit counter and negative |
|   6 | `test_emit_gauge_and_delta`      |   ✅   | Emit gauge and delta      |
|   7 | `test_emit_timing_set_sampled`   |   ✅   | Emit timing set sampled   |
|   8 | `test_emit_global_tags`          |   ✅   | Emit global tags          |
|   9 | `test_emit_noop_until_begin`     |   ✅   | Emit noop until begin     |

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

## test_failsafe - ✅ 6 passed

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

</details>

---

## test_sleep_sched - ✅ 8 passed

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

## test_wearlevel - ✅ 5 passed

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

## test_netadapt - ✅ 6 passed

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

## test_dshot - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/dshot: the DShot ESC throttle frame codec (hand-computed vectors)._

|   # | Test                                    | Status | Description                                                                |
| --: | :-------------------------------------- | :----: | :------------------------------------------------------------------------- |
|   1 | `test_encode_known_vector`              |   ✅   | Encode known vector                                                        |
|   2 | `test_encode_telemetry_bit`             |   ✅   | value 1046, telemetry set: v12 = 0x82D, nibbles 8^2^D = 7, frame = 0x82D7. |
|   3 | `test_encode_bidirectional_inverts_crc` |   ✅   | Same value, bidirectional: crc = ~6 & 0xF = 9, frame = 0x82C9.             |
|   4 | `test_value_masked_to_11_bits`          |   ✅   | 0xF000                                                                     | 1046: the high bits are dropped to the 11-bit field -> same as 1046. |
|   5 | `test_decode_roundtrip_and_crc`         |   ✅   | Decode roundtrip and crc                                                   |
|   6 | `test_bit_timing`                       |   ✅   | 600 kbit: period 1667 ns; "1" ~3/4, "0" ~3/8.                              |
|   7 | `test_esc_pwm_mapping`                  |   ✅   | OneShot125: 125..250 us.                                                   |

</details>

---

## test_hart - ✅ 6 passed

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

</details>

---

## test_nts - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/nts: the NTS-KE record + NTS NTP extension-field wire codec (RFC 8915)._

|   # | Test                           | Status | Description                                                                      |
| --: | :----------------------------- | :----: | :------------------------------------------------------------------------------- |
|   1 | `test_ke_record`               |   ✅   | Ke record                                                                        |
|   2 | `test_ke_request`              |   ✅   | Next-Protocol(NTPv4) + AEAD(AES-SIV-CMAC-256=15) + End-of-Message, all critical. |
|   3 | `test_ke_parse`                |   ✅   | Ke parse                                                                         |
|   4 | `test_extension_field_padding` |   ✅   | 32-byte unique id: 4 + 32 = 36, already a multiple of 4.                         |

</details>

---

## test_dds - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/dds: the RTPS message + submessage framing codec._

|   # | Test                         | Status | Description                                                       |
| --: | :--------------------------- | :----: | :---------------------------------------------------------------- |
|   1 | `test_header`                |   ✅   | Header                                                            |
|   2 | `test_submessage_endianness` |   ✅   | Little-endian (E flag set): octetsToNextHeader = 0x0008 -> 08 00. |
|   3 | `test_parse_message`         |   ✅   | Parse message                                                     |
|   4 | `test_parse_rejects`         |   ✅   | Parse rejects                                                     |

</details>

---

## test_xmpp - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/xmpp: the XMPP stanza builder + minimal reader._

|   # | Test               | Status | Description |
| --: | :----------------- | :----: | :---------- |
|   1 | `test_escape`      |   ✅   | Escape      |
|   2 | `test_message`     |   ✅   | Message     |
|   3 | `test_presence`    |   ✅   | Presence    |
|   4 | `test_iq`          |   ✅   | Iq          |
|   5 | `test_stanza_name` |   ✅   | Stanza name |
|   6 | `test_attr`        |   ✅   | Attr        |

</details>

---

## test_rawl2 - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/rawl2: the Ethernet II / 802.1Q frame codec + the FCS._

|   # | Test                     | Status | Description                                                        |
| --: | :----------------------- | :----: | :----------------------------------------------------------------- |
|   1 | `test_build_ethernet_ii` |   ✅   | Build ethernet ii                                                  |
|   2 | `test_build_vlan`        |   ✅   | pcp 3, dei 0, vid 100 -> TCI 0x6064; PROFINET ethertype.           |
|   3 | `test_parse`             |   ✅   | Parse                                                              |
|   4 | `test_fcs_check_vector`  |   ✅   | The canonical CRC-32 check value: CRC of "123456789" = 0xCBF43926. |

</details>

---

## test_spa_router - ✅ 2 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/spa_router: the single-page-app routing decision._

|   # | Test                 | Status | Description   |
| --: | :------------------- | :----: | :------------ |
|   1 | `test_has_extension` |   ✅   | Has extension |
|   2 | `test_route`         |   ✅   | Route         |

</details>

---

## test_goose - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/goose: the IEC 61850 GOOSE BER PDU + Ethernet frame codec._

|   # | Test                        | Status | Description                                                              |
| --: | :-------------------------- | :----: | :----------------------------------------------------------------------- |
|   1 | `test_pdu_structure`        |   ✅   | Content is 42 octets (see goose.cpp field sizes); PDU = 61 2A <42> = 44. |
|   2 | `test_integer_leading_zero` |   ✅   | Integer leading zero                                                     |
|   3 | `test_frame`                |   ✅   | Frame                                                                    |

</details>

---

## test_mtconnect - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/mtconnect: the MTConnectStreams + MTConnectError document builders._

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_streams_document`      |   ✅   | Streams document      |
|   2 | `test_streams_escapes_value` |   ✅   | Streams escapes value |
|   3 | `test_error_document`        |   ✅   | Error document        |
|   4 | `test_overflow_returns_zero` |   ✅   | Overflow returns zero |

</details>

---

## test_j2735 - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/j2735: the ASN.1 UPER primitive codec + the BSMcore block._

|   # | Test                        | Status | Description                                                                                   |
| --: | :-------------------------- | :----: | :-------------------------------------------------------------------------------------------- |
|   1 | `test_cint_bits`            |   ✅   | Cint bits                                                                                     |
|   2 | `test_bit_writer_pattern`   |   ✅   | Write 0b101 (3 bits) then 0b11 (2 bits): stream 10111 000 -> 0xB8.                            |
|   3 | `test_writer_null_and_zero` |   ✅   | A null buffer (or zero cap) leaves the writer not-ok and must not dereference it.             |
|   4 | `test_cint_roundtrip`       |   ✅   | Cint roundtrip                                                                                |
|   5 | `test_bsm_core_roundtrip`   |   ✅   | Bsm core roundtrip                                                                            |
|   6 | `test_bsm_core_bit_length`  |   ✅   | msgCnt 7 + id 32 + secMark 16 + lat 31 + long 32 + elev 16 + speed 13 + heading 15 = 162 bits |
|   7 | `test_spat_roundtrip`       |   ✅   | Spat roundtrip                                                                                |
|   8 | `test_spat_decode_too_many` |   ✅   | Only room for 1 but 2 encoded -> false.                                                       |
|   9 | `test_map_roundtrip`        |   ✅   | Map roundtrip                                                                                 |

</details>

---

## test_nema_ts2 - ✅ 4 passed

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

## test_snp - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/snp: the GE Fanuc SNP serial frame codec._

|   # | Test                   | Status | Description                                            |
| --: | :--------------------- | :----: | :----------------------------------------------------- |
|   1 | `test_bcc`             |   ✅   | sum = 0x01+0x03+0x10+0x20+0x30 = 0x64.                 |
|   2 | `test_build_and_parse` |   ✅   | [01][03][10 20 30][BCC] ; BCC = 01+03+10+20+30 = 0x64. |
|   3 | `test_empty_data`      |   ✅   | Empty data                                             |
|   4 | `test_parse_rejects`   |   ✅   | Parse rejects                                          |

</details>

---

## test_directnet - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/directnet: the DirectNET serial frame codec._

|   # | Test                        | Status | Description                                                               |
| --: | :-------------------------- | :----: | :------------------------------------------------------------------------ |
|   1 | `test_lrc`                  |   ✅   | Lrc                                                                       |
|   2 | `test_header_frame`         |   ✅   | SOH(1) + slave(2) + type(1) + addr(4) + blocks(2) + ETB(1) + LRC(1) = 12. |
|   3 | `test_data_frame_roundtrip` |   ✅   | STX + ABCD + ETX + LRC = 7.                                               |
|   4 | `test_data_parse_rejects`   |   ✅   | Data parse rejects                                                        |

</details>

---

## test_sep2 - ✅ 5 passed

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

## test_profinet - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/profinet: the PROFINET DCP frame codec._

|   # | Test                          | Status | Description                                                                                    |
| --: | :---------------------------- | :----: | :--------------------------------------------------------------------------------------------- |
|   1 | `test_header_roundtrip`       |   ✅   | Header roundtrip                                                                               |
|   2 | `test_block_even_padding`     |   ✅   | NameOfStation "plc" is 3 bytes (odd) -> padded to an even total, filler not counted in length. |
|   3 | `test_walk_blocks`            |   ✅   | Walk blocks                                                                                    |
|   4 | `test_walk_rejects_truncated` |   ✅   | blockLength claims 10 but only 2 value bytes present.                                          |

</details>

---

## test_ntcip - ✅ 3 passed

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

## test_openadr - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/openadr: the OpenADR 3.0 event / report JSON builders._

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_event`                 |   ✅   | Event                 |
|   2 | `test_report_negative_value` |   ✅   | Report negative value |
|   3 | `test_json_escape`           |   ✅   | Json escape           |
|   4 | `test_overflow`              |   ✅   | Overflow              |

</details>

---

## test_mms - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/mms: the IEC 61850 MMS Read PDU codec._

|   # | Test                           | Status | Description                                                                           |
| --: | :----------------------------- | :----: | :------------------------------------------------------------------------------------ |
|   1 | `test_read_request_structure`  |   ✅   | Read request structure                                                                |
|   2 | `test_read_request_parse`      |   ✅   | Read request parse                                                                    |
|   3 | `test_read_response_roundtrip` |   ✅   | A caller-encoded Data value: boolean-ish [3] BOOLEAN true -> 83 01 FF (context Data). |
|   4 | `test_parse_rejects_bad_tag`   |   ✅   | Parse rejects bad tag                                                                 |

</details>

---

## test_cclink - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/cclink: the CC-Link cyclic frame codec + process-image accessors._

|   # | Test                   | Status | Description     |
| --: | :--------------------- | :----: | :-------------- |
|   1 | `test_sum`             |   ✅   | Sum             |
|   2 | `test_build_and_parse` |   ✅   | Build and parse |
|   3 | `test_bit_accessors`   |   ✅   | Bit accessors   |
|   4 | `test_parse_rejects`   |   ✅   | Parse rejects   |

</details>

---

## test_powerlink - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/powerlink: the Ethernet POWERLINK basic frame codec._

|   # | Test                       | Status | Description                                  |
| --: | :------------------------- | :----: | :------------------------------------------- |
|   1 | `test_soc`                 |   ✅   | Soc                                          |
|   2 | `test_preq_pres_roundtrip` |   ✅   | PReq: MN (240) -> CN 5, carrying output PDO. |
|   3 | `test_parse_rejects`       |   ✅   | Parse rejects                                |

</details>

---

## test_sercos - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/sercos: the SERCOS III telegram + IDN codec._

|   # | Test                           | Status | Description                                              |
| --: | :----------------------------- | :----: | :------------------------------------------------------- |
|   1 | `test_idn_roundtrip`           |   ✅   | S-0-0100 (velocity loop): S-parameter, set 0, block 100. |
|   2 | `test_telegram_roundtrip`      |   ✅   | Telegram roundtrip                                       |
|   3 | `test_at_telegram_and_rejects` |   ✅   | At telegram and rejects                                  |

</details>

---

## test_profibus - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/profibus: the PROFIBUS-DP FDL telegram codec._

|   # | Test                 | Status | Description                             |
| --: | :------------------- | :----: | :-------------------------------------- |
|   1 | `test_fcs`           |   ✅   | Fcs                                     |
|   2 | `test_sd1`           |   ✅   | SD1 DA SA FC FCS ED : 10 03 02 49 4E 16 |
|   3 | `test_sd2_roundtrip` |   ✅   | le = 3 + 3 = 6; total = 4 + 6 + 2 = 12. |
|   4 | `test_parse_rejects` |   ✅   | Parse rejects                           |

</details>

---

## test_lonworks - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/lonworks: the LonTalk NV PDU + SNVT scalar codec._

|   # | Test                                 | Status | Description                                                             |
| --: | :----------------------------------- | :----: | :---------------------------------------------------------------------- |
|   1 | `test_nv_pdu_roundtrip`              |   ✅   | selector 0x1234 is 14-bit -> stored 0x12 0x34.                          |
|   2 | `test_nv_selector_masked_to_14_bits` |   ✅   | The top two bits of the selector byte are not part of the 14-bit value. |
|   3 | `test_snvt_temp`                     |   ✅   | Snvt temp                                                               |
|   4 | `test_snvt_switch`                   |   ✅   | Snvt switch                                                             |

</details>

---

## test_mbplus - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/mbplus: the Modbus Plus HDLC token-bus frame codec._

|   # | Test                          | Status | Description                                           |
| --: | :---------------------------- | :----: | :---------------------------------------------------- |
|   1 | `test_crc_check_vector`       |   ✅   | CRC-16/X-25 check value: CRC of "123456789" = 0x906E. |
|   2 | `test_build_and_parse`        |   ✅   | 7E 05 00 10 03 00 CRClo CRChi 7E = 9 bytes.           |
|   3 | `test_token_frame_no_payload` |   ✅   | Token frame no payload                                |
|   4 | `test_next_token_ring`        |   ✅   | Next token ring                                       |
|   5 | `test_parse_rejects`          |   ✅   | Parse rejects                                         |

</details>

---

## test_interbus - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/interbus: the summation-frame codec._

|   # | Test                    | Status | Description                                                  |
| --: | :---------------------- | :----: | :----------------------------------------------------------- |
|   1 | `test_fcs_check_vector` |   ✅   | CRC-16/CCITT-FALSE check value: CRC of "123456789" = 0x29B1. |
|   2 | `test_build_and_parse`  |   ✅   | Three device slices: 0x1111, 0x2222, 0x3333.                 |
|   3 | `test_empty_frame`      |   ✅   | Empty frame                                                  |
|   4 | `test_parse_rejects`    |   ✅   | Corrupt FCS.                                                 |

</details>

---

## test_iccp - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/iccp: the ICCP / TASE.2 Data_Value codec._

|   # | Test                     | Status | Description                                      |
| --: | :----------------------- | :----: | :----------------------------------------------- |
|   1 | `test_state_q_no_time`   |   ✅   | A2 { 85 01 <sq> } ; sq = (ON=2)<<6               | valid(0) = 0x80. -> A2 03 85 01 80 |
|   2 | `test_state_q_with_time` |   ✅   | State q with time                                |
|   3 | `test_real_q`            |   ✅   | Real q                                           |
|   4 | `test_real_q_negative`   |   ✅   | -1 -> minimal two's complement INTEGER 02 01 FF. |

</details>

---

## test_wave - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/wave: the IEEE 1609 WSMP + 1609.2 envelope + PSID codec._

|   # | Test                      | Status | Description          |
| --: | :------------------------ | :----: | :------------------- |
|   1 | `test_psid_p_encoding`    |   ✅   | 1-octet: 0x20 -> 20. |
|   2 | `test_wsmp_roundtrip`     |   ✅   | Wsmp roundtrip       |
|   3 | `test_1609dot2_wrap`      |   ✅   | 1609dot2 wrap        |
|   4 | `test_wsmp_parse_rejects` |   ✅   | Wsmp parse rejects   |

</details>

---

## test_utmc - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/utmc: the UTMC common-database request/response codec._

|   # | Test                    | Status | Description      |
| --: | :---------------------- | :----: | :--------------- |
|   1 | `test_request`          |   ✅   | Request          |
|   2 | `test_response`         |   ✅   | Response         |
|   3 | `test_response_escapes` |   ✅   | Response escapes |
|   4 | `test_parse_request`    |   ✅   | Parse request    |
|   5 | `test_overflow`         |   ✅   | Overflow         |

</details>

---

## test_ocit - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/ocit: the OCIT-Outstations message codec._

|   # | Test                       | Status | Description                                     |
| --: | :------------------------- | :----: | :---------------------------------------------- |
|   1 | `test_build_and_parse`     |   ✅   | [02][01 02][00 03][04][00 00 12 34] = 10 bytes. |
|   2 | `test_set_u16_helper`      |   ✅   | Set u16 helper                                  |
|   3 | `test_get_no_value`        |   ✅   | Get no value                                    |
|   4 | `test_parse_rejects_short` |   ✅   | Parse rejects short                             |

</details>

---

## test_atc - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/atc: the ATC field-I/O interop snapshot._

|   # | Test                      | Status | Description        |
| --: | :------------------------ | :----: | :----------------- |
|   1 | `test_snapshot_json`      |   ✅   | Snapshot json      |
|   2 | `test_set_output`         |   ✅   | Set an output.     |
|   3 | `test_get`                |   ✅   | Get                |
|   4 | `test_empty_and_overflow` |   ✅   | Empty and overflow |

</details>

---

## test_southbound - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/southbound: the driver registry + name-dispatched read/write facade._

|   # | Test                          | Status | Description                                                   |
| --: | :---------------------------- | :----: | :------------------------------------------------------------ |
|   1 | `test_register_and_find`      |   ✅   | Register and find                                             |
|   2 | `test_read_write_dispatch`    |   ✅   | Read write dispatch                                           |
|   3 | `test_block_atomic`           |   ✅   | Block atomic                                                  |
|   4 | `test_unsupported_capability` |   ✅   | A driver that only implements single-point read.              |
|   5 | `test_registry_full`          |   ✅   | Fill the registry with distinct-named drivers, then overflow. |

</details>

---

## test_exc_decoder - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/exc_decoder: parsing a real ESP32 Guru Meditation panic dump._

|   # | Test                                            | Status | Description                                                                                   |
| --: | :---------------------------------------------- | :----: | :-------------------------------------------------------------------------------------------- |
|   1 | `test_parse_full`                               |   ✅   | Parse full                                                                                    |
|   2 | `test_json`                                     |   ✅   | Json                                                                                          |
|   3 | `test_backtrace_only_and_corrupted`             |   ✅   | No register dump: PC must fall back to the first backtrace frame. Trailing corruption marker. |
|   4 | `test_garbage_returns_false`                    |   ✅   | Garbage returns false                                                                         |
|   5 | `test_json_omits_core_when_absent_and_overflow` |   ✅   | Json omits core when absent and overflow                                                      |

</details>

---

## test_http_delivery - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/http_delivery: RFC 5861 stale-while-revalidate, RFC 7233 byte ranges,_

|   # | Test                 | Status | Description         |
| --: | :------------------- | :----: | :------------------ |
|   1 | `test_swr_decision`  |   ✅   | max-age=60, swr=30. |
|   2 | `test_cache_control` |   ✅   | Cache control       |
|   3 | `test_range_forms`   |   ✅   | X-Y                 |
|   4 | `test_range_rejects` |   ✅   | Range rejects       |
|   5 | `test_content_range` |   ✅   | Content range       |
|   6 | `test_sw_manifest`   |   ✅   | Sw manifest         |

</details>

---

## test_hw_health - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/hw_health: rail droop, SPI CRC backoff, GPIO short, cap leakage._

|   # | Test                      | Status | Description                                       |
| --: | :------------------------ | :----: | :------------------------------------------------ |
|   1 | `test_rail_monitor`       |   ✅   | Rail monitor                                      |
|   2 | `test_spi_backoff`        |   ✅   | Spi backoff                                       |
|   3 | `test_spi_backoff_clamps` |   ✅   | Spi backoff clamps                                |
|   4 | `test_gpio_short`         |   ✅   | Gpio short                                        |
|   5 | `test_cap_leak`           |   ✅   | Expected 100ms decay, 10% tolerance -> [90, 110]. |

</details>

---

## test_mdns_adaptive - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/mdns_adaptive: RF-aware backoff, TTL refresher, auto-sleep beacon._

|   # | Test                       | Status | Description         |
| --: | :------------------------- | :----: | :------------------ |
|   1 | `test_refresh_interval`    |   ✅   | Refresh interval    |
|   2 | `test_backoff_and_recover` |   ✅   | Backoff and recover |
|   3 | `test_due`                 |   ✅   | Due                 |
|   4 | `test_presleep`            |   ✅   | Presleep            |

</details>

---

## test_sockpool - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/sockpool: the LRU connection-slot recycling pool._

|   # | Test                        | Status | Description                         |
| --: | :-------------------------- | :----: | :---------------------------------- |
|   1 | `test_acquire_free`         |   ✅   | Acquire free                        |
|   2 | `test_lru_recycle`          |   ✅   | Fill: id 100@t10, 101@t20, 102@t30. |
|   3 | `test_touch_changes_lru`    |   ✅   | Touch changes lru                   |
|   4 | `test_release_reopens_free` |   ✅   | Release reopens free                |
|   5 | `test_empty_pool_fails`     |   ✅   | Empty pool fails                    |

</details>

---

## test_psram_pool - ✅ 5 passed

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

## test_happy_eyeballs - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/happy_eyeballs: RFC 6724 ordering + RFC 8305 family interleave + attempt gate._

|   # | Test                        | Status | Description                                                                               |
| --: | :-------------------------- | :----: | :---------------------------------------------------------------------------------------- |
|   1 | `test_pref_order`           |   ✅   | Global outranks link-local outranks loopback; within global, native v6 outranks v4.       |
|   2 | `test_order_and_interleave` |   ✅   | Two global v6 + one global v4, given v4-first: sort puts v6 ahead, interleave alternates. |
|   3 | `test_order_single_family`  |   ✅   | All v4: interleave is a no-op, order stays preference-sorted (global before private).     |
|   4 | `test_attempt_due`          |   ✅   | Attempt due                                                                               |

</details>

---

## test_wifi_sniffer - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/wifi_sniffer: 802.11 header decode, traffic tally, roaming decision._

|   # | Test                    | Status | Description                                                          |
| --: | :---------------------- | :----: | :------------------------------------------------------------------- |
|   1 | `test_parse_data`       |   ✅   | Parse data                                                           |
|   2 | `test_parse_beacon`     |   ✅   | Parse beacon                                                         |
|   3 | `test_parse_ctrl_short` |   ✅   | Parse ctrl short                                                     |
|   4 | `test_stats`            |   ✅   | Stats                                                                |
|   5 | `test_roam`             |   ✅   | Current -80 dBm, candidate -70 dBm, 8 dB hysteresis: 10 > 8 -> roam. |

</details>

---

## test_link_manager - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/link_manager: egress selection, graceful escalation, failover._

|   # | Test                           | Status | Description                                             |
| --: | :----------------------------- | :----: | :------------------------------------------------------ |
|   1 | `test_init_none_up`            |   ✅   | Init none up                                            |
|   2 | `test_escalation_and_failover` |   ✅   | WiFi STA comes up first -> it becomes active.           |
|   3 | `test_tie_break_lower_index`   |   ✅   | Two interfaces at equal priority: the lower index wins. |
|   4 | `test_out_of_range_no_change`  |   ✅   | Out of range no change                                  |

</details>

---

## test_cc1101 - ✅ 10 passed

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

</details>

---

## test_fdc2214 - ✅ 4 passed

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

## test_ldc1614 - ✅ 4 passed

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

## test_vl53l0x - ✅ 3 passed

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

## test_radio_sniff - ✅ 4 passed

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

## test_ble_gatt - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/ble_gatt: the ATT PDU codec + GATT characteristic JSON._

|   # | Test                  | Status | Description                 |
| --: | :-------------------- | :----: | :-------------------------- |
|   1 | `test_build_pdus`     |   ✅   | Read Request handle 0x0025. |
|   2 | `test_build_overflow` |   ✅   | Build overflow              |
|   3 | `test_parse`          |   ✅   | Write Request with value.   |
|   4 | `test_char_json`      |   ✅   | Char json                   |

</details>

---

## test_tls_policy - ✅ 4 passed

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

## test_wisun - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host tests for services/wisun: the CoAP client request builder (RFC 7252) + the FAN node registry._

|   # | Test                                           | Status | Description                                                                   |
| --: | :--------------------------------------------- | :----: | :---------------------------------------------------------------------------- |
|   1 | `test_build_coap_get`                          |   ✅   | CON GET "sensors/temp", msg id 0x1234, no token.                              |
|   2 | `test_build_coap_put_with_token_and_payload`   |   ✅   | Header: 0x52 (ver=01, type NON=01, tkl=0010), code 0x03 (PUT), mid 0x00 0x05. |
|   3 | `test_build_coap_long_segment_extended_length` |   ✅   | A 13-char path segment forces the extended-length nibble (0xD).               |
|   4 | `test_build_coap_rejects_bad_args`             |   ✅   | Build coap rejects bad args                                                   |
|   5 | `test_node_registry`                           |   ✅   | Node registry                                                                 |
|   6 | `test_registry_full_and_misses`                |   ✅   | Registry full and misses                                                      |

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

## test_workers - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Phase 2 core-partitioning invariant (built with DETWS_WORKER_COUNT=2): a worker_

|   # | Test                                         | Status | Description                           |
| --: | :------------------------------------------- | :----: | :------------------------------------ |
|   1 | `test_worker_count_is_two`                   |   ✅   | Worker count is two                   |
|   2 | `test_check_timeouts_reaps_only_owned_slots` |   ✅   | Check timeouts reaps only owned slots |
|   3 | `test_pool_init_defaults_owner_zero`         |   ✅   | Pool init defaults owner zero         |

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

## test_qpack - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the QPACK codec (network_drivers/presentation/http3/qpack, RFC 9204): the_

|   # | Test                            | Status | Description                                                                      |
| --: | :------------------------------ | :----: | :------------------------------------------------------------------------------- |
|   1 | `test_appendix_b1_decode`       |   ✅   | Appendix b1 decode                                                               |
|   2 | `test_encode_indexed`           |   ✅   | Encode indexed                                                                   |
|   3 | `test_encode_nameref_roundtrip` |   ✅   | Encode nameref roundtrip                                                         |
|   4 | `test_literal_name`             |   ✅   | Literal name                                                                     |
|   5 | `test_full_section`             |   ✅   | Full section                                                                     |
|   6 | `test_reject_dynamic`           |   ✅   | Reject dynamic                                                                   |
|   7 | `test_encode_edges`             |   ✅   | Encode edges                                                                     |
|   8 | `test_decode_errors`            |   ✅   | Decode errors                                                                    |
|   9 | `test_value_string_paths`       |   ✅   | Value marked Huffman (0x81 = H, len 1) but 0xFF is not a valid single-byte code. |

</details>

---

## test_quic_packet - ✅ 8 passed

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

## test_quic_frame - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the QUIC frame codec (network_drivers/presentation/http3/quic_frame, RFC 9000_

|   # | Test                           | Status | Description                                                                 |
| --: | :----------------------------- | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_simple_frames`           |   ✅   | Simple frames                                                               |
|   2 | `test_ack`                     |   ✅   | Ack                                                                         |
|   3 | `test_crypto`                  |   ✅   | Crypto                                                                      |
|   4 | `test_stream`                  |   ✅   | With offset + FIN.                                                          |
|   5 | `test_max_data_and_close`      |   ✅   | Max data and close                                                          |
|   6 | `test_sequence_and_truncation` |   ✅   | A packet payload: PADDING, PING, then a CRYPTO frame - parse them in order. |
|   7 | `test_builder_overflow`        |   ✅   | Builder overflow                                                            |
|   8 | `test_parse_errors`            |   ✅   | Parse errors                                                                |

</details>

---

## test_quic_crypto - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for QUIC Initial packet crypto (network_drivers/presentation/http3/quic_hkdf,_

|   # | Test                               | Status | Description                 |
| --: | :--------------------------------- | :----: | :-------------------------- |
|   1 | `test_aes128_block_fips197`        |   ✅   | Aes128 block fips197        |
|   2 | `test_aes128_gcm_testcase4`        |   ✅   | Aes128 gcm testcase4        |
|   3 | `test_initial_secrets_appendix_a1` |   ✅   | Initial secrets appendix a1 |
|   4 | `test_server_initial_a3`           |   ✅   | Server initial a3           |
|   5 | `test_client_initial_a2`           |   ✅   | Client initial a2           |
|   6 | `test_retry_integrity_a4`          |   ✅   | Retry integrity a4          |
|   7 | `test_gcm_open_rejects_short`      |   ✅   | Gcm open rejects short      |

</details>

---

## test_tls13_kdf - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the TLS 1.3 key schedule (network_drivers/presentation/http3/tls13_kdf; RFC 8446_

|   # | Test                        | Status | Description               |
| --: | :-------------------------- | :----: | :------------------------ |
|   1 | `test_early_secret`         |   ✅   | Early secret              |
|   2 | `test_handshake_secrets`    |   ✅   | Handshake secrets         |
|   3 | `test_master_secrets`       |   ✅   | Master secrets            |
|   4 | `test_server_hs_write_keys` |   ✅   | Server hs write keys      |
|   5 | `test_server_finished`      |   ✅   | ClientHello (196 octets). |

</details>

---

## test_quic_tp - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the QUIC transport-parameters codec (network_drivers/presentation/http3/quic_tp;_

|   # | Test                        | Status | Description                                                                             |
| --: | :-------------------------- | :----: | :-------------------------------------------------------------------------------------- |
|   1 | `test_defaults`             |   ✅   | Defaults                                                                                |
|   2 | `test_roundtrip`            |   ✅   | Roundtrip                                                                               |
|   3 | `test_parse_bytes`          |   ✅   | Parse bytes                                                                             |
|   4 | `test_skip_unknown`         |   ✅   | id 0x1a (unknown), len 3, value 01 02 03; then 04 01 20 (initial_max_data = 0x20 = 32). |
|   5 | `test_reject_duplicate`     |   ✅   | initial_max_data twice.                                                                 |
|   6 | `test_reject_oversized_cid` |   ✅   | original_destination_connection_id with a 21-byte value (max is 20).                    |
|   7 | `test_reject_bad_values`    |   ✅   | active_connection_id_limit = 1 (must be >= 2).                                          |

</details>

---

## test_tls13_msg - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the TLS 1.3 handshake messages (network_drivers/presentation/http3/tls13_msg;_

|   # | Test                              | Status | Description                                                                                     |
| --: | :-------------------------------- | :----: | :---------------------------------------------------------------------------------------------- |
|   1 | `test_parse_client_hello`         |   ✅   | Parse client hello                                                                              |
|   2 | `test_build_server_hello`         |   ✅   | Build server hello                                                                              |
|   3 | `test_build_certificate`          |   ✅   | Reconstruct the DER cert from the expected message: strip the 11-byte prefix and 2-byte suffix. |
|   4 | `test_build_finished`             |   ✅   | Build finished                                                                                  |
|   5 | `test_encrypted_extensions`       |   ✅   | Encrypted extensions                                                                            |
|   6 | `test_cert_verify_content`        |   ✅   | Cert verify content                                                                             |
|   7 | `test_cert_verify_sign_roundtrip` |   ✅   | Cert verify sign roundtrip                                                                      |

</details>

---

## test_quic_tls - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the TLS 1.3 server handshake state machine (network_drivers/presentation/http3/_

|   # | Test                                 | Status | Description                   |
| --: | :----------------------------------- | :----: | :---------------------------- |
|   1 | `test_full_handshake_roundtrip`      |   ✅   | Full handshake roundtrip      |
|   2 | `test_reject_bad_client_finished`    |   ✅   | Reject bad client finished    |
|   3 | `test_reject_no_h3_alpn`             |   ✅   | Reject no h3 alpn             |
|   4 | `test_partial_client_hello`          |   ✅   | Partial client hello          |
|   5 | `test_reject_no_tls13`               |   ✅   | Reject no tls13               |
|   6 | `test_reject_no_key_share`           |   ✅   | Reject no key share           |
|   7 | `test_reject_no_x25519_group`        |   ✅   | Reject no x25519 group        |
|   8 | `test_reject_no_ed25519`             |   ✅   | Reject no ed25519             |
|   9 | `test_reject_no_transport_params`    |   ✅   | Reject no transport params    |
|  10 | `test_reject_bad_transport_params`   |   ✅   | Reject bad transport params   |
|  11 | `test_reject_malformed_client_hello` |   ✅   | Reject malformed client hello |

</details>

---

## test_quic_conn - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the QUIC v1 server connection engine (network_drivers/presentation/http3/quic_conn;_

|   # | Test                                       | Status | Description                         |
| --: | :----------------------------------------- | :----: | :---------------------------------- |
|   1 | `test_full_handshake_and_stream`           |   ✅   | Full handshake and stream           |
|   2 | `test_pto_retransmits_flight`              |   ✅   | Pto retransmits flight              |
|   3 | `test_connection_close_api`                |   ✅   | Connection close api                |
|   4 | `test_connection_close_on_malformed_frame` |   ✅   | Connection close on malformed frame |

</details>

---

## test_h3_conn - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the HTTP/3 application engine (network_drivers/presentation/http3/h3_conn; RFC_

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

## test_h3_e2e - ✅ 1 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_End-to-end capstone for the whole HTTP/3 stack: a QUIC client (in the test) completes the TLS 1.3_

|   # | Test                        | Status | Description          |
| --: | :-------------------------- | :----: | :------------------- |
|   1 | `test_http3_get_end_to_end` |   ✅   | Http3 get end to end |

</details>

---

## test_quic_server - ✅ 2 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_HTTP/3 server-glue test: the same end-to-end flow as test_h3_e2e (a QUIC client completes the_

|   # | Test                          | Status | Description            |
| --: | :---------------------------- | :----: | :--------------------- |
|   1 | `test_quic_server_http3_get`  |   ✅   | Quic server http3 get  |
|   2 | `test_idle_connection_reaped` |   ✅   | Idle connection reaped |

</details>

---

## test_h3_server - ✅ 1 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_HTTP/3 dispatch-bridge test: proves an HTTP/3 request served by a *real DetWebServer route*. A_

|   # | Test                              | Status | Description                |
| --: | :-------------------------------- | :----: | :------------------------- |
|   1 | `test_h3_request_served_by_route` |   ✅   | H3 request served by route |

</details>

---

## test_ssh_chachapoly - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the chacha20-poly1305@openssh.com cipher and its primitives:_

|   # | Test                              | Status | Description                |
| --: | :-------------------------------- | :----: | :------------------------- |
|   1 | `test_chacha20_block_rfc8439`     |   ✅   | Chacha20 block rfc8439     |
|   2 | `test_poly1305_rfc8439`           |   ✅   | Poly1305 rfc8439           |
|   3 | `test_chachapoly_roundtrip`       |   ✅   | Chachapoly roundtrip       |
|   4 | `test_chachapoly_tamper_rejected` |   ✅   | Chachapoly tamper rejected |

</details>

---

## Raw Output

<details>
<summary>Expand full pio output</summary>

```
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_canopen in native_canopen environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_canopen/test_canopen.cpp:384: test_nmt_start_node                                                     [PASSED]
test/test_canopen/test_canopen.cpp:385: test_sync                                                               [PASSED]
test/test_canopen/test_canopen.cpp:386: test_heartbeat_roundtrip                                                [PASSED]
test/test_canopen/test_canopen.cpp:387: test_emcy_roundtrip                                                     [PASSED]
test/test_canopen/test_canopen.cpp:388: test_pdo_roundtrip                                                      [PASSED]
test/test_canopen/test_canopen.cpp:389: test_sdo_read_request                                                   [PASSED]
test/test_canopen/test_canopen.cpp:390: test_sdo_write_expedited                                                [PASSED]
test/test_canopen/test_canopen.cpp:391: test_sdo_upload_response_expedited                                      [PASSED]
test/test_canopen/test_canopen.cpp:392: test_sdo_abort_roundtrip                                                [PASSED]
test/test_canopen/test_canopen.cpp:393: test_sdo_download_ack                                                   [PASSED]
test/test_canopen/test_canopen.cpp:394: test_parse_classifies                                                   [PASSED]
test/test_canopen/test_canopen.cpp:395: test_build_arg_validation                                               [PASSED]
test/test_canopen/test_canopen.cpp:396: test_emcy_build_null_msef                                               [PASSED]
test/test_canopen/test_canopen.cpp:397: test_parse_all_function_codes                                           [PASSED]
test/test_canopen/test_canopen.cpp:398: test_parse_emcy_rejections                                              [PASSED]
test/test_canopen/test_canopen.cpp:399: test_parse_heartbeat_rejections                                         [PASSED]
test/test_canopen/test_canopen.cpp:400: test_parse_sdo_response_variants                                        [PASSED]
native_canopen:test_canopen Took 3.77 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_canopen  test_canopen  PASSED    00:00:03.768
================= 17 test cases: 17 succeeded in 00:00:03.768 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_det_primitives in native_det_primitives environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_det_primitives/test_det_primitives.cpp:94: test_strtol                                                [PASSED]
test/test_det_primitives/test_det_primitives.cpp:95: test_strtoul                                               [PASSED]
test/test_det_primitives/test_det_primitives.cpp:96: test_strtof                                                [PASSED]
test/test_det_primitives/test_det_primitives.cpp:97: test_utf8_valid                                            [PASSED]
test/test_det_primitives/test_det_primitives.cpp:98: test_utf8_invalid                                          [PASSED]
native_det_primitives:test_det_primitives Took 0.80 seconds --------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment            Test                 Status    Duration
---------------------  -------------------  --------  ------------
native_det_primitives  test_det_primitives  PASSED    00:00:00.798
================== 5 test cases: 5 succeeded in 00:00:00.798 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_det_ip in native_det_ip environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_det_ip/test_det_ip.cpp:220: test_v4_round_trip                                                        [PASSED]
test/test_det_ip/test_det_ip.cpp:221: test_from_v6_bytes                                                        [PASSED]
test/test_det_ip/test_det_ip.cpp:222: test_is_unspecified                                                       [PASSED]
test/test_det_ip/test_det_ip.cpp:223: test_prefix_match                                                         [PASSED]
test/test_det_ip/test_det_ip.cpp:224: test_v6_canonical_5952                                                    [PASSED]
test/test_det_ip/test_det_ip.cpp:225: test_v4_mapped                                                            [PASSED]
test/test_det_ip/test_det_ip.cpp:226: test_classify_v4                                                          [PASSED]
test/test_det_ip/test_det_ip.cpp:227: test_classify_v6                                                          [PASSED]
test/test_det_ip/test_det_ip.cpp:228: test_reject_malformed                                                     [PASSED]
test/test_det_ip/test_det_ip.cpp:229: test_equal_and_from_v4                                                    [PASSED]
native_det_ip:test_det_ip Took 0.78 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_det_ip  test_det_ip  PASSED    00:00:00.783
================= 10 test cases: 10 succeeded in 00:00:00.783 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_det_arena in native_det_arena environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_det_arena/test_det_arena.cpp:272: test_persist_basic_alloc                                            [PASSED]
test/test_det_arena/test_det_arena.cpp:273: test_persist_zeroed                                                 [PASSED]
test/test_det_arena/test_det_arena.cpp:274: test_persist_first_fit_reuse                                        [PASSED]
test/test_det_arena/test_det_arena.cpp:275: test_persist_coalesce                                               [PASSED]
test/test_det_arena/test_det_arena.cpp:276: test_persist_free_shrinks_boundary                                  [PASSED]
test/test_det_arena/test_det_arena.cpp:277: test_scratch_bump_and_reset                                         [PASSED]
test/test_det_arena/test_det_arena.cpp:278: test_scratch_mark_release                                           [PASSED]
test/test_det_arena/test_det_arena.cpp:279: test_persist_and_scratch_no_overlap                                 [PASSED]
test/test_det_arena/test_det_arena.cpp:280: test_boundary_collision_fail_closed                                 [PASSED]
test/test_det_arena/test_det_arena.cpp:281: test_scratch_reset_frees_middle_for_persist                         [PASSED]
test/test_det_arena/test_det_arena.cpp:282: test_alignment_various_sizes                                        [PASSED]
test/test_det_arena/test_det_arena.cpp:283: test_scratch_alignment_16                                           [PASSED]
test/test_det_arena/test_det_arena.cpp:284: test_zero_size_and_null_free                                        [PASSED]
test/test_det_arena/test_det_arena.cpp:285: test_set_add_limits                                                 [PASSED]
test/test_det_arena/test_det_arena.cpp:286: test_set_persist_overflow_and_prefer                                [PASSED]
test/test_det_arena/test_det_arena.cpp:287: test_set_persist_free_routes_by_address                             [PASSED]
test/test_det_arena/test_det_arena.cpp:288: test_set_scratch_overflow_and_unwind                                [PASSED]
native_det_arena:test_det_arena Took 0.83 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_det_arena  test_det_arena  PASSED    00:00:00.827
================= 17 test cases: 17 succeeded in 00:00:00.827 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ssh_ed25519 in native_ssh_ed25519 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_ed25519/test_ssh_ed25519.cpp:280: test_sha512_empty                                               [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:281: test_sha512_abc                                                 [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:282: test_sha512_one_block_boundary                                  [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:283: test_sha512_two_block_boundary                                  [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:284: test_sha512_million_a_streaming                                 [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:285: test_sha512_streaming_matches_oneshot                           [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:286: test_x25519_rfc7748_vector1                                     [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:287: test_x25519_rfc7748_vector2                                     [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:288: test_x25519_iterated_1                                          [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:289: test_x25519_iterated_1000                                       [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:290: test_x25519_dh_agreement                                        [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:291: test_ed25519_vector_empty_msg                                   [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:292: test_ed25519_vector_rfc8032_test2                               [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:293: test_ed25519_vector_zero_seed                                   [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:294: test_ed25519_verify_rejects_tampering                           [PASSED]
test/test_ssh_ed25519/test_ssh_ed25519.cpp:295: test_ed25519_roundtrip_long                                     [PASSED]
native_ssh_ed25519:test_ssh_ed25519 Took 4.92 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_ssh_ed25519  test_ssh_ed25519  PASSED    00:00:04.918
================= 16 test cases: 16 succeeded in 00:00:04.918 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_promisc in native_promisc environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_promisc/test_promisc.cpp:162: test_beacon_mgmt                                                        [PASSED]
test/test_promisc/test_promisc.cpp:163: test_data_from_ds                                                       [PASSED]
test/test_promisc/test_promisc.cpp:164: test_data_to_ds                                                         [PASSED]
test/test_promisc/test_promisc.cpp:165: test_qos_data_header_len                                                [PASSED]
test/test_promisc/test_promisc.cpp:166: test_wds_four_address                                                   [PASSED]
test/test_promisc/test_promisc.cpp:167: test_control_frame                                                      [PASSED]
test/test_promisc/test_promisc.cpp:168: test_reject_short                                                       [PASSED]
test/test_promisc/test_promisc.cpp:169: test_pcap_headers                                                       [PASSED]
native_promisc:test_promisc Took 0.79 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_promisc  test_promisc  PASSED    00:00:00.785
================== 8 test cases: 8 succeeded in 00:00:00.785 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_bus_capture in native_bus_capture environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_bus_capture/test_bus_capture.cpp:112: test_standard_data_frame                                        [PASSED]
test/test_bus_capture/test_bus_capture.cpp:113: test_extended_id_sets_eff                                       [PASSED]
test/test_bus_capture/test_bus_capture.cpp:114: test_rtr_flag_and_no_data                                       [PASSED]
test/test_bus_capture/test_bus_capture.cpp:115: test_masks_and_bounds                                           [PASSED]
test/test_bus_capture/test_bus_capture.cpp:116: test_pcap_can_linktype                                          [PASSED]
native_bus_capture:test_bus_capture Took 0.79 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_bus_capture  test_bus_capture  PASSED    00:00:00.786
================== 5 test cases: 5 succeeded in 00:00:00.786 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_j1939 in native_j1939 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_j1939/test_j1939.cpp:160: test_id_pdu2_roundtrip                                                      [PASSED]
test/test_j1939/test_j1939.cpp:161: test_id_pdu1_roundtrip                                                      [PASSED]
test/test_j1939/test_j1939.cpp:162: test_encode_rejects_bad_args                                                [PASSED]
test/test_j1939/test_j1939.cpp:163: test_build_single_frame                                                     [PASSED]
test/test_j1939/test_j1939.cpp:164: test_request_pgn                                                            [PASSED]
test/test_j1939/test_j1939.cpp:165: test_address_claim_name                                                     [PASSED]
test/test_j1939/test_j1939.cpp:166: test_tp_num_packets                                                         [PASSED]
test/test_j1939/test_j1939.cpp:167: test_tp_bam_roundtrip                                                       [PASSED]
test/test_j1939/test_j1939.cpp:168: test_tp_out_of_sequence_errors                                              [PASSED]
native_j1939:test_j1939 Took 0.78 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_j1939   test_j1939  PASSED    00:00:00.779
================== 9 test cases: 9 succeeded in 00:00:00.779 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_devicenet in native_devicenet environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_devicenet/test_devicenet.cpp:143: test_id_group1                                                      [PASSED]
test/test_devicenet/test_devicenet.cpp:144: test_id_group2                                                      [PASSED]
test/test_devicenet/test_devicenet.cpp:145: test_id_group3_and_4                                                [PASSED]
test/test_devicenet/test_devicenet.cpp:146: test_header_and_frag_octets                                         [PASSED]
test/test_devicenet/test_devicenet.cpp:147: test_build_explicit_single_frame                                    [PASSED]
test/test_devicenet/test_devicenet.cpp:148: test_frag_non_fragmented                                            [PASSED]
test/test_devicenet/test_devicenet.cpp:149: test_frag_reassembly_roundtrip                                      [PASSED]
test/test_devicenet/test_devicenet.cpp:150: test_frag_out_of_order_errors                                       [PASSED]
native_devicenet:test_devicenet Took 0.79 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_devicenet  test_devicenet  PASSED    00:00:00.792
================== 8 test cases: 8 succeeded in 00:00:00.792 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_nmea2000 in native_nmea2000 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_nmea2000/test_nmea2000.cpp:122: test_num_frames                                                       [PASSED]
test/test_nmea2000/test_nmea2000.cpp:123: test_single_frame                                                     [PASSED]
test/test_nmea2000/test_nmea2000.cpp:124: test_fastpacket_roundtrip                                             [PASSED]
test/test_nmea2000/test_nmea2000.cpp:125: test_fastpacket_single_frame_completes                                [PASSED]
test/test_nmea2000/test_nmea2000.cpp:126: test_fastpacket_interleaved_sequence_ignored                          [PASSED]
test/test_nmea2000/test_nmea2000.cpp:127: test_fastpacket_out_of_order_errors                                   [PASSED]
native_nmea2000:test_nmea2000 Took 0.87 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_nmea2000  test_nmea2000  PASSED    00:00:00.873
================== 6 test cases: 6 succeeded in 00:00:00.873 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_mbus in native_mbus environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_mbus/test_mbus.cpp:244: test_ack                                                                      [PASSED]
test/test_mbus/test_mbus.cpp:245: test_short_frame_roundtrip                                                    [PASSED]
test/test_mbus/test_mbus.cpp:246: test_req_ud2_fcb                                                              [PASSED]
test/test_mbus/test_mbus.cpp:247: test_long_frame_roundtrip                                                     [PASSED]
test/test_mbus/test_mbus.cpp:248: test_parse_rejects_corruption                                                 [PASSED]
test/test_mbus/test_mbus.cpp:249: test_dif_data_len                                                             [PASSED]
test/test_mbus/test_mbus.cpp:250: test_record_walk                                                              [PASSED]
test/test_mbus/test_mbus.cpp:251: test_record_truncated_fails                                                   [PASSED]
test/test_mbus/test_mbus.cpp:252: test_build_and_parse_guards                                                   [PASSED]
test/test_mbus/test_mbus.cpp:253: test_dif_data_len_remaining                                                   [PASSED]
test/test_mbus/test_mbus.cpp:254: test_record_edges                                                             [PASSED]
native_mbus:test_mbus Took 0.81 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_mbus    test_mbus  PASSED    00:00:00.805
================= 11 test cases: 11 succeeded in 00:00:00.805 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_iec60870 in native_iec60870 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_iec60870/test_iec60870.cpp:158: test_104_i_format_roundtrip                                           [PASSED]
test/test_iec60870/test_iec60870.cpp:159: test_104_s_format                                                     [PASSED]
test/test_iec60870/test_iec60870.cpp:160: test_104_u_format                                                     [PASSED]
test/test_iec60870/test_iec60870.cpp:161: test_104_sequence_numbers_15bit                                       [PASSED]
test/test_iec60870/test_iec60870.cpp:162: test_asdu_header_roundtrip                                            [PASSED]
test/test_iec60870/test_iec60870.cpp:163: test_ioa_roundtrip                                                    [PASSED]
test/test_iec60870/test_iec60870.cpp:164: test_101_fixed_frame                                                  [PASSED]
test/test_iec60870/test_iec60870.cpp:165: test_101_variable_frame_roundtrip                                     [PASSED]
native_iec60870:test_iec60870 Took 0.81 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_iec60870  test_iec60870  PASSED    00:00:00.810
================== 8 test cases: 8 succeeded in 00:00:00.810 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_sdi12 in native_sdi12 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_sdi12/test_sdi12.cpp:114: test_command_builders                                                       [PASSED]
test/test_sdi12/test_sdi12.cpp:115: test_parse_measure_m                                                        [PASSED]
test/test_sdi12/test_sdi12.cpp:116: test_parse_measure_concurrent_two_digit_count                               [PASSED]
test/test_sdi12/test_sdi12.cpp:117: test_parse_values                                                           [PASSED]
test/test_sdi12/test_sdi12.cpp:118: test_crc_roundtrip                                                          [PASSED]
test/test_sdi12/test_sdi12.cpp:119: test_crc_encode_printable                                                   [PASSED]
native_sdi12:test_sdi12 Took 0.81 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_sdi12   test_sdi12  PASSED    00:00:00.814
================== 6 test cases: 6 succeeded in 00:00:00.814 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_dmx in native_dmx environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_dmx/test_dmx.cpp:130: test_dmx_build_and_get                                                          [PASSED]
test/test_dmx/test_dmx.cpp:131: test_rdm_uid                                                                    [PASSED]
test/test_dmx/test_dmx.cpp:132: test_rdm_get_roundtrip                                                          [PASSED]
test/test_dmx/test_dmx.cpp:133: test_rdm_set_with_data                                                          [PASSED]
test/test_dmx/test_dmx.cpp:134: test_rdm_parse_rejects_bad                                                      [PASSED]
native_dmx:test_dmx Took 0.79 seconds ------------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_dmx     test_dmx  PASSED    00:00:00.791
================== 5 test cases: 5 succeeded in 00:00:00.791 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_nmea0183 in native_nmea0183 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_nmea0183/test_nmea0183.cpp:105: test_checksum_known_vector                                            [PASSED]
test/test_nmea0183/test_nmea0183.cpp:106: test_build                                                            [PASSED]
test/test_nmea0183/test_nmea0183.cpp:107: test_parse_gga                                                        [PASSED]
test/test_nmea0183/test_nmea0183.cpp:108: test_field_helpers                                                    [PASSED]
test/test_nmea0183/test_nmea0183.cpp:109: test_parse_rejects_bad_checksum                                       [PASSED]
test/test_nmea0183/test_nmea0183.cpp:110: test_parse_rejects_no_dollar                                          [PASSED]
test/test_nmea0183/test_nmea0183.cpp:111: test_build_then_parse                                                 [PASSED]
native_nmea0183:test_nmea0183 Took 0.79 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_nmea0183  test_nmea0183  PASSED    00:00:00.789
================== 7 test cases: 7 succeeded in 00:00:00.789 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_iolink in native_iolink environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_iolink/test_iolink.cpp:80: test_mc_octet                                                              [PASSED]
test/test_iolink/test_iolink.cpp:81: test_ckt_cks_octets                                                        [PASSED]
test/test_iolink/test_iolink.cpp:82: test_checksum_known_vector                                                 [PASSED]
test/test_iolink/test_iolink.cpp:83: test_finalize_preserves_type_and_detects_corruption                        [PASSED]
test/test_iolink/test_iolink.cpp:84: test_device_reply_cks_roundtrip                                            [PASSED]
native_iolink:test_iolink Took 0.80 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_iolink  test_iolink  PASSED    00:00:00.797
================== 5 test cases: 5 succeeded in 00:00:00.797 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_sse in native environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_sse/test_sse.cpp:372: test_sse_pool_size                                                              [PASSED]
test/test_sse/test_sse.cpp:373: test_sse_ids_match_indices_after_init                                           [PASSED]
test/test_sse/test_sse.cpp:374: test_sse_all_inactive_after_init                                                [PASSED]
test/test_sse/test_sse.cpp:375: test_sse_path_empty_after_init                                                  [PASSED]
test/test_sse/test_sse.cpp:378: test_sse_alloc_returns_non_null                                                 [PASSED]
test/test_sse/test_sse.cpp:379: test_sse_alloc_sets_active                                                      [PASSED]
test/test_sse/test_sse.cpp:380: test_sse_alloc_sets_slot_id                                                     [PASSED]
test/test_sse/test_sse.cpp:381: test_sse_alloc_stores_path                                                      [PASSED]
test/test_sse/test_sse.cpp:382: test_sse_alloc_stores_different_paths_per_slot                                  [PASSED]
test/test_sse/test_sse.cpp:383: test_sse_alloc_path_truncated_to_max                                            [PASSED]
test/test_sse/test_sse.cpp:384: test_sse_alloc_pool_full_returns_null                                           [PASSED]
test/test_sse/test_sse.cpp:385: test_sse_alloc_sse_id_is_pool_index                                             [PASSED]
test/test_sse/test_sse.cpp:388: test_sse_find_returns_correct_conn                                              [PASSED]
test/test_sse/test_sse.cpp:389: test_sse_find_returns_null_when_empty                                           [PASSED]
test/test_sse/test_sse.cpp:390: test_sse_find_returns_null_for_different_slot                                   [PASSED]
test/test_sse/test_sse.cpp:391: test_sse_find_after_both_slots_allocated                                        [PASSED]
test/test_sse/test_sse.cpp:392: test_sse_find_checks_slot_id_not_sse_id                                         [PASSED]
test/test_sse/test_sse.cpp:395: test_sse_free_deactivates_slot                                                  [PASSED]
test/test_sse/test_sse.cpp:396: test_sse_free_restores_sse_id                                                   [PASSED]
test/test_sse/test_sse.cpp:397: test_sse_free_makes_slot_findable_as_null                                       [PASSED]
test/test_sse/test_sse.cpp:398: test_sse_free_clears_path                                                       [PASSED]
test/test_sse/test_sse.cpp:399: test_sse_free_nop_on_unallocated                                                [PASSED]
test/test_sse/test_sse.cpp:400: test_sse_alloc_after_free_succeeds                                              [PASSED]
test/test_sse/test_sse.cpp:401: test_sse_free_only_frees_matching_slot                                          [PASSED]
test/test_sse/test_sse.cpp:404: test_sse_write_null_data_returns_false                                          [PASSED]
test/test_sse/test_sse.cpp:405: test_sse_write_returns_false_when_conn_not_active                               [PASSED]
test/test_sse/test_sse.cpp:406: test_sse_write_returns_false_when_pcb_null                                      [PASSED]
test/test_sse/test_sse.cpp:407: test_sse_write_data_only_returns_true                                           [PASSED]
test/test_sse/test_sse.cpp:408: test_sse_write_with_event_returns_true                                          [PASSED]
test/test_sse/test_sse.cpp:409: test_sse_write_with_id_returns_true                                             [PASSED]
test/test_sse/test_sse.cpp:410: test_sse_write_with_all_fields_returns_true                                     [PASSED]
test/test_sse/test_sse.cpp:411: test_sse_write_does_not_affect_other_slots                                      [PASSED]
test/test_sse/test_sse.cpp:414: stress_sse_alloc_free_100_cycles                                                [PASSED]
test/test_sse/test_sse.cpp:415: stress_sse_alloc_free_both_slots_alternating                                    [PASSED]
test/test_sse/test_sse.cpp:416: stress_sse_write_100_calls                                                      [PASSED]
test/test_sse/test_sse.cpp:417: stress_sse_find_with_full_pool                                                  [PASSED]
test/test_sse/test_sse.cpp:418: stress_sse_write_slot_isolation                                                 [PASSED]
native:test_sse Took 1.44 seconds ----------------------------------------------------------------------------- [PASSED]

Processing test_session in native environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_session/test_session.cpp:348: test_empty_queue_does_not_crash                                         [PASSED]
test/test_session/test_session.cpp:349: test_pool_initializes_to_parse_method                                   [PASSED]
test/test_session/test_session.cpp:350: test_reset_clears_mid_parse_state                                       [PASSED]
test/test_session/test_session.cpp:351: test_tick_fires_check_timeouts_stale_slot_freed                         [PASSED]
test/test_session/test_session.cpp:352: test_tick_does_not_free_fresh_connection                                [PASSED]
test/test_session/test_session.cpp:355: test_fn_tick_timeout_before_event_drain_ordering                        [PASSED]
test/test_session/test_session.cpp:356: test_fn_tick_only_active_slots_expire                                   [PASSED]
test/test_session/test_session.cpp:359: stress_1000_idle_ticks_stable                                           [PASSED]
test/test_session/test_session.cpp:360: stress_timeout_all_slots_10_cycles                                      [PASSED]
test/test_session/test_session.cpp:361: stress_mixed_fresh_stale_slots_many_ticks                               [PASSED]
test/test_session/test_session.cpp:364: test_evt_connect_calls_http_reset                                       [PASSED]
test/test_session/test_session.cpp:365: test_evt_disconnect_calls_http_reset                                    [PASSED]
test/test_session/test_session.cpp:366: test_evt_error_calls_http_reset                                         [PASSED]
test/test_session/test_session.cpp:367: test_evt_data_calls_http_parse                                          [PASSED]
test/test_session/test_session.cpp:368: test_multiple_events_drained_in_one_tick                                [PASSED]
test/test_session/test_session.cpp:371: race_external_free_between_ticks                                        [PASSED]
test/test_session/test_session.cpp:372: race_activity_update_saves_slot_from_timeout                            [PASSED]
test/test_session/test_session.cpp:373: race_all_expire_then_idle_tick                                          [PASSED]
test/test_session/test_session.cpp:374: race_millis_wraparound_no_spurious_timeout                              [PASSED]
native:test_session Took 0.71 seconds ------------------------------------------------------------------------- [PASSED]

Processing test_presentation in native environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_presentation/test_presentation.cpp:844: test_fn_reset_sets_parse_state_to_method                      [PASSED]
test/test_presentation/test_presentation.cpp:845: test_fn_reset_sets_slot_id                                    [PASSED]
test/test_presentation/test_presentation.cpp:846: test_fn_reset_clears_method                                   [PASSED]
test/test_presentation/test_presentation.cpp:847: test_fn_reset_clears_path_and_idx                             [PASSED]
test/test_presentation/test_presentation.cpp:848: test_fn_reset_clears_query_raw_and_params                     [PASSED]
test/test_presentation/test_presentation.cpp:849: test_fn_reset_clears_all_header_slots                         [PASSED]
test/test_presentation/test_presentation.cpp:850: test_fn_reset_clears_body_fields                              [PASSED]
test/test_presentation/test_presentation.cpp:851: test_fn_reset_out_of_range_is_nop                             [PASSED]
test/test_presentation/test_presentation.cpp:852: test_fn_reset_is_idempotent                                   [PASSED]
test/test_presentation/test_presentation.cpp:855: test_fn_get_header_null_when_no_headers                       [PASSED]
test/test_presentation/test_presentation.cpp:856: test_fn_get_header_finds_single_header                        [PASSED]
test/test_presentation/test_presentation.cpp:857: test_fn_get_header_finds_first_of_many                        [PASSED]
test/test_presentation/test_presentation.cpp:858: test_fn_get_header_finds_middle_of_many                       [PASSED]
test/test_presentation/test_presentation.cpp:859: test_fn_get_header_finds_last_of_many                         [PASSED]
test/test_presentation/test_presentation.cpp:860: test_fn_get_header_case_insensitive_lowercase                 [PASSED]
test/test_presentation/test_presentation.cpp:861: test_fn_get_header_case_insensitive_uppercase                 [PASSED]
test/test_presentation/test_presentation.cpp:862: test_fn_get_header_returns_null_for_absent_key                [PASSED]
test/test_presentation/test_presentation.cpp:863: test_fn_get_header_does_not_bleed_across_slots                [PASSED]
test/test_presentation/test_presentation.cpp:866: test_fn_get_query_null_when_no_params                         [PASSED]
test/test_presentation/test_presentation.cpp:867: test_fn_get_query_finds_single_param                          [PASSED]
test/test_presentation/test_presentation.cpp:868: test_fn_get_query_finds_first_param                           [PASSED]
test/test_presentation/test_presentation.cpp:869: test_fn_get_query_finds_middle_param                          [PASSED]
test/test_presentation/test_presentation.cpp:870: test_fn_get_query_finds_last_param                            [PASSED]
test/test_presentation/test_presentation.cpp:871: test_fn_get_query_returns_null_for_absent_key                 [PASSED]
test/test_presentation/test_presentation.cpp:872: test_fn_get_query_empty_value                                 [PASSED]
test/test_presentation/test_presentation.cpp:873: test_fn_get_query_does_not_bleed_across_slots                 [PASSED]
test/test_presentation/test_presentation.cpp:876: test_get_parses_complete                                      [PASSED]
test/test_presentation/test_presentation.cpp:877: test_post_body_stored                                         [PASSED]
test/test_presentation/test_presentation.cpp:878: test_put_parses_complete                                      [PASSED]
test/test_presentation/test_presentation.cpp:879: test_delete_parses_complete                                   [PASSED]
test/test_presentation/test_presentation.cpp:880: test_patch_parses_complete                                    [PASSED]
test/test_presentation/test_presentation.cpp:881: test_head_parses_complete                                     [PASSED]
test/test_presentation/test_presentation.cpp:882: test_query_single_param                                       [PASSED]
test/test_presentation/test_presentation.cpp:883: test_query_multiple_params                                    [PASSED]
test/test_presentation/test_presentation.cpp:884: test_body_null_terminated                                     [PASSED]
test/test_presentation/test_presentation.cpp:885: test_body_over_buf_size_is_413                                [PASSED]
test/test_presentation/test_presentation.cpp:886: test_overflow_method_sets_error                               [PASSED]
test/test_presentation/test_presentation.cpp:887: test_overflow_path_sets_414                                   [PASSED]
test/test_presentation/test_presentation.cpp:888: test_bad_lf_after_cr_sets_error                               [PASSED]
test/test_presentation/test_presentation.cpp:889: test_headers_beyond_max_are_dropped                           [PASSED]
test/test_presentation/test_presentation.cpp:890: test_query_params_beyond_max_are_dropped                      [PASSED]
test/test_presentation/test_presentation.cpp:891: test_incremental_two_pushes_completes                         [PASSED]
test/test_presentation/test_presentation.cpp:892: test_body_starting_with_newline_stored                        [PASSED]
test/test_presentation/test_presentation.cpp:893: test_put_body_stored                                          [PASSED]
test/test_presentation/test_presentation.cpp:894: test_content_length_header_stored_in_headers_array            [PASSED]
test/test_presentation/test_presentation.cpp:897: stress_parse_reset_100_cycles                                 [PASSED]
test/test_presentation/test_presentation.cpp:898: stress_all_slots_parse_simultaneously                         [PASSED]
test/test_presentation/test_presentation.cpp:899: stress_method_at_max_7_chars_no_error                         [PASSED]
test/test_presentation/test_presentation.cpp:900: stress_path_at_exact_limit_no_error                           [PASSED]
test/test_presentation/test_presentation.cpp:901: stress_body_exactly_buf_size_all_stored                       [PASSED]
test/test_presentation/test_presentation.cpp:902: stress_exactly_max_headers_all_stored                         [PASSED]
test/test_presentation/test_presentation.cpp:903: stress_exactly_max_query_params_all_stored                    [PASSED]
test/test_presentation/test_presentation.cpp:904: stress_incremental_byte_by_byte_no_error                      [PASSED]
test/test_presentation/test_presentation.cpp:905: stress_sequential_requests_no_state_leak                      [PASSED]
test/test_presentation/test_presentation.cpp:908: race_interleaved_producer_consumer_ring_buffer                [PASSED]
test/test_presentation/test_presentation.cpp:909: race_ring_buffer_full_prevents_write                          [PASSED]
test/test_presentation/test_presentation.cpp:910: race_aba_slot_reuse_fresh_timestamp                           [PASSED]
test/test_presentation/test_presentation.cpp:911: race_double_free_is_nop                                       [PASSED]
test/test_presentation/test_presentation.cpp:912: race_concurrent_slot_parse_isolation                          [PASSED]
test/test_presentation/test_presentation.cpp:913: race_reset_during_parse_header_val                            [PASSED]
test/test_presentation/test_presentation.cpp:914: race_reset_during_parse_query                                 [PASSED]
test/test_presentation/test_presentation.cpp:915: race_reset_during_parse_body                                  [PASSED]
test/test_presentation/test_presentation.cpp:916: race_parse_after_complete_is_nop                              [PASSED]
native:test_presentation Took 0.76 seconds -------------------------------------------------------------------- [PASSED]

Processing test_transport in native environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_transport/test_transport.cpp:606: test_pool_capacity_is_four                                          [PASSED]
test/test_transport/test_transport.cpp:607: test_rx_buffer_size_is_one_kb                                       [PASSED]
test/test_transport/test_transport.cpp:608: test_timeout_constant_is_5000ms                                     [PASSED]
test/test_transport/test_transport.cpp:609: test_all_slots_free_after_init                                      [PASSED]
test/test_transport/test_transport.cpp:610: test_all_pcbs_null_after_init                                       [PASSED]
test/test_transport/test_transport.cpp:611: test_all_ring_buffers_empty_after_init                              [PASSED]
test/test_transport/test_transport.cpp:612: test_slot_ids_match_indices                                         [PASSED]
test/test_transport/test_transport.cpp:613: test_ring_empty_when_head_equals_tail                               [PASSED]
test/test_transport/test_transport.cpp:614: test_ring_wrap_at_boundary                                          [PASSED]
test/test_transport/test_transport.cpp:615: test_ring_full_sentinel_one_slot_reserved                           [PASSED]
test/test_transport/test_transport.cpp:616: test_ring_can_store_size_minus_one_bytes                            [PASSED]
test/test_transport/test_transport.cpp:617: test_event_types_are_distinct                                       [PASSED]
test/test_transport/test_transport.cpp:618: test_timeout_does_not_fire_on_free_slot                             [PASSED]
test/test_transport/test_transport.cpp:619: test_timeout_does_not_fire_before_deadline                          [PASSED]
test/test_transport/test_transport.cpp:620: test_timeout_fires_at_deadline                                      [PASSED]
test/test_transport/test_transport.cpp:621: test_timeout_fires_only_on_stale_slots                              [PASSED]
test/test_transport/test_transport.cpp:622: test_init_succeeds_on_native                                        [PASSED]
test/test_transport/test_transport.cpp:623: test_all_last_activity_ms_zero_after_init                           [PASSED]
test/test_transport/test_transport.cpp:624: test_queue_not_null_after_init                                      [PASSED]
test/test_transport/test_transport.cpp:627: stress_ring_buffer_fill_drain_integrity                             [PASSED]
test/test_transport/test_transport.cpp:628: stress_ring_buffer_multi_cycle_no_corruption                        [PASSED]
test/test_transport/test_transport.cpp:629: stress_all_slots_timeout_simultaneously                             [PASSED]
test/test_transport/test_transport.cpp:630: stress_timeout_arm_recover_cycle                                    [PASSED]
test/test_transport/test_transport.cpp:631: stress_check_timeouts_high_call_rate                                [PASSED]
test/test_transport/test_transport.cpp:632: stress_ring_buffer_byte_by_byte_fill_and_drain                      [PASSED]
test/test_transport/test_transport.cpp:635: test_accept_throttle_blocks_over_budget                             [PASSED]
test/test_transport/test_transport.cpp:636: test_accept_throttle_window_refills                                 [PASSED]
test/test_transport/test_transport.cpp:637: test_accept_throttle_handles_rollover                               [PASSED]
test/test_transport/test_transport.cpp:640: test_per_ip_throttle_blocks_over_budget                             [PASSED]
test/test_transport/test_transport.cpp:641: test_per_ip_throttle_isolates_addresses                             [PASSED]
test/test_transport/test_transport.cpp:642: test_per_ip_throttle_window_refills                                 [PASSED]
test/test_transport/test_transport.cpp:643: test_per_ip_throttle_evicts_when_full                               [PASSED]
test/test_transport/test_transport.cpp:644: test_per_ip_throttle_zero_ip_always_allowed                         [PASSED]
test/test_transport/test_transport.cpp:645: test_per_ip_throttle_v6_distinct                                    [PASSED]
test/test_transport/test_transport.cpp:646: test_per_ip_throttle_handles_rollover                               [PASSED]
test/test_transport/test_transport.cpp:649: test_ip_allowlist_empty_allows_all                                  [PASSED]
test/test_transport/test_transport.cpp:650: test_ip_allowlist_host_match                                        [PASSED]
test/test_transport/test_transport.cpp:651: test_ip_allowlist_cidr_match                                        [PASSED]
test/test_transport/test_transport.cpp:652: test_ip_allowlist_masks_host_bits                                   [PASSED]
test/test_transport/test_transport.cpp:653: test_ip_allowlist_multiple_rules                                    [PASSED]
test/test_transport/test_transport.cpp:654: test_ip_allowlist_zero_prefix_matches_all                           [PASSED]
test/test_transport/test_transport.cpp:655: test_ip_allowlist_v6_cidr                                           [PASSED]
test/test_transport/test_transport.cpp:656: test_ip_allowlist_rejects_bad_prefix                                [PASSED]
test/test_transport/test_transport.cpp:657: test_ip_allowlist_table_full                                        [PASSED]
native:test_transport Took 0.75 seconds ----------------------------------------------------------------------- [PASSED]

Processing test_websocket in native environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_websocket/test_websocket.cpp:1068: test_sha1_empty_string                                             [PASSED]
test/test_websocket/test_websocket.cpp:1069: test_sha1_abc                                                      [PASSED]
test/test_websocket/test_websocket.cpp:1070: test_sha1_rfc6455_handshake_key                                    [PASSED]
test/test_websocket/test_websocket.cpp:1071: test_sha1_different_inputs_different_digests                       [PASSED]
test/test_websocket/test_websocket.cpp:1074: test_base64_encode_one_byte                                        [PASSED]
test/test_websocket/test_websocket.cpp:1075: test_base64_encode_two_bytes                                       [PASSED]
test/test_websocket/test_websocket.cpp:1076: test_base64_encode_three_bytes                                     [PASSED]
test/test_websocket/test_websocket.cpp:1077: test_base64_encode_ws_accept_key                                   [PASSED]
test/test_websocket/test_websocket.cpp:1078: test_base64_decode_one_byte                                        [PASSED]
test/test_websocket/test_websocket.cpp:1079: test_base64_decode_two_bytes                                       [PASSED]
test/test_websocket/test_websocket.cpp:1080: test_base64_decode_three_bytes                                     [PASSED]
test/test_websocket/test_websocket.cpp:1081: test_base64_decode_ws_accept_key                                   [PASSED]
test/test_websocket/test_websocket.cpp:1082: test_base64_decode_rejects_misplaced_padding                       [PASSED]
test/test_websocket/test_websocket.cpp:1083: test_base64_decode_respects_capacity                               [PASSED]
test/test_websocket/test_websocket.cpp:1084: test_base64_round_trip                                             [PASSED]
test/test_websocket/test_websocket.cpp:1087: test_ws_pool_size                                                  [PASSED]
test/test_websocket/test_websocket.cpp:1088: test_ws_ids_match_indices_after_init                               [PASSED]
test/test_websocket/test_websocket.cpp:1089: test_ws_all_inactive_after_init                                    [PASSED]
test/test_websocket/test_websocket.cpp:1090: test_ws_alloc_returns_non_null                                     [PASSED]
test/test_websocket/test_websocket.cpp:1091: test_ws_alloc_sets_active                                          [PASSED]
test/test_websocket/test_websocket.cpp:1092: test_ws_alloc_sets_slot_id                                         [PASSED]
test/test_websocket/test_websocket.cpp:1093: test_ws_alloc_sets_parse_state_header1                             [PASSED]
test/test_websocket/test_websocket.cpp:1094: test_ws_alloc_pool_full_returns_null                               [PASSED]
test/test_websocket/test_websocket.cpp:1095: test_ws_find_returns_correct_conn                                  [PASSED]
test/test_websocket/test_websocket.cpp:1096: test_ws_find_returns_null_when_empty                               [PASSED]
test/test_websocket/test_websocket.cpp:1097: test_ws_find_returns_null_for_different_slot                       [PASSED]
test/test_websocket/test_websocket.cpp:1098: test_ws_find_after_both_slots_allocated                            [PASSED]
test/test_websocket/test_websocket.cpp:1099: test_ws_free_deactivates_slot                                      [PASSED]
test/test_websocket/test_websocket.cpp:1100: test_ws_free_restores_ws_id                                        [PASSED]
test/test_websocket/test_websocket.cpp:1101: test_ws_free_makes_slot_findable_as_null                           [PASSED]
test/test_websocket/test_websocket.cpp:1102: test_ws_free_nop_on_unallocated                                    [PASSED]
test/test_websocket/test_websocket.cpp:1103: test_ws_alloc_after_free_succeeds                                  [PASSED]
test/test_websocket/test_websocket.cpp:1106: test_ws_parse_text_frame_sets_ready                                [PASSED]
test/test_websocket/test_websocket.cpp:1107: test_ws_parse_payload_stored_correctly                             [PASSED]
test/test_websocket/test_websocket.cpp:1108: test_ws_parse_binary_frame_sets_ready                              [PASSED]
test/test_websocket/test_websocket.cpp:1109: test_ws_parse_zero_length_unmasked_frame                           [PASSED]
test/test_websocket/test_websocket.cpp:1110: test_ws_parse_zero_length_masked_frame                             [PASSED]
test/test_websocket/test_websocket.cpp:1111: test_ws_reject_unmasked_data_frame                                 [PASSED]
test/test_websocket/test_websocket.cpp:1112: test_ws_reject_reserved_opcode                                     [PASSED]
test/test_websocket/test_websocket.cpp:1113: test_ws_reject_fragmented_control_frame                            [PASSED]
test/test_websocket/test_websocket.cpp:1114: test_ws_reject_oversized_control_frame                             [PASSED]
test/test_websocket/test_websocket.cpp:1115: test_ws_parse_16bit_length_frame                                   [PASSED]
test/test_websocket/test_websocket.cpp:1116: test_ws_parse_rsv1_set_closes_protocol                             [PASSED]
test/test_websocket/test_websocket.cpp:1117: test_ws_parse_rsv2_set_closes_protocol                             [PASSED]
test/test_websocket/test_websocket.cpp:1118: test_ws_parse_rsv3_set_closes_protocol                             [PASSED]
test/test_websocket/test_websocket.cpp:1119: test_ws_parse_64bit_length_closes_too_big                          [PASSED]
test/test_websocket/test_websocket.cpp:1120: test_ws_parse_oversized_16bit_length_closes_too_big                [PASSED]
test/test_websocket/test_websocket.cpp:1121: test_ws_fragment_start_waits_for_continuation                      [PASSED]
test/test_websocket/test_websocket.cpp:1122: test_ws_fragmented_message_reassembled                             [PASSED]
test/test_websocket/test_websocket.cpp:1123: test_ws_control_frame_interleaved_in_fragments                     [PASSED]
test/test_websocket/test_websocket.cpp:1124: test_ws_fragment_accumulation_overflow_rejected                    [PASSED]
test/test_websocket/test_websocket.cpp:1125: test_ws_continuation_without_start_rejected                        [PASSED]
test/test_websocket/test_websocket.cpp:1126: test_ws_new_data_frame_during_fragmentation_rejected               [PASSED]
test/test_websocket/test_websocket.cpp:1127: test_ws_parse_ping_auto_pong_resets_frame                          [PASSED]
test/test_websocket/test_websocket.cpp:1128: test_ws_parse_pong_silently_ignored                                [PASSED]
test/test_websocket/test_websocket.cpp:1129: test_ws_parse_close_marks_ws_closed                                [PASSED]
test/test_websocket/test_websocket.cpp:1130: test_ws_parse_stops_at_frame_ready                                 [PASSED]
test/test_websocket/test_websocket.cpp:1131: test_ws_reset_frame_clears_fields                                  [PASSED]
test/test_websocket/test_websocket.cpp:1132: test_ws_parse_mask_applied_correctly                               [PASSED]
test/test_websocket/test_websocket.cpp:1133: test_ws_text_invalid_utf8_rejected                                 [PASSED]
test/test_websocket/test_websocket.cpp:1134: test_ws_text_valid_utf8_accepted                                   [PASSED]
test/test_websocket/test_websocket.cpp:1135: test_ws_binary_arbitrary_bytes_accepted                            [PASSED]
test/test_websocket/test_websocket.cpp:1143: test_ws_outbound_fragmentation                                     [PASSED]
test/test_websocket/test_websocket.cpp:1146: stress_ws_parse_reset_100_cycles                                   [PASSED]
test/test_websocket/test_websocket.cpp:1147: stress_ws_alloc_free_pool_cycle                                    [PASSED]
test/test_websocket/test_websocket.cpp:1148: stress_ws_parse_incremental_byte_by_byte                           [PASSED]
test/test_websocket/test_websocket.cpp:1149: stress_ws_parse_max_payload                                        [PASSED]
test/test_websocket/test_websocket.cpp:1150: stress_ws_parse_two_consecutive_frames                             [PASSED]
native:test_websocket Took 0.82 seconds ----------------------------------------------------------------------- [PASSED]

Processing test_http_parser in native environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_http_parser/test_http_parser.cpp:1013: test_accessor_null_guards                                      [PASSED]
test/test_http_parser/test_http_parser.cpp:1014: test_cookie_parse_edges                                        [PASSED]
test/test_http_parser/test_http_parser.cpp:1015: test_forwarded_ip_whitespace_and_invalid                       [PASSED]
test/test_http_parser/test_http_parser.cpp:1018: test_reset_sets_parse_method_state                             [PASSED]
test/test_http_parser/test_http_parser.cpp:1019: test_reset_preserves_slot_id                                   [PASSED]
test/test_http_parser/test_http_parser.cpp:1020: test_reset_clears_method                                       [PASSED]
test/test_http_parser/test_http_parser.cpp:1021: test_reset_clears_path                                         [PASSED]
test/test_http_parser/test_http_parser.cpp:1022: test_reset_clears_header_count                                 [PASSED]
test/test_http_parser/test_http_parser.cpp:1023: test_reset_clears_body                                         [PASSED]
test/test_http_parser/test_http_parser.cpp:1024: test_reset_clears_query_count                                  [PASSED]
test/test_http_parser/test_http_parser.cpp:1027: test_feed_after_complete_does_not_change_state                 [PASSED]
test/test_http_parser/test_http_parser.cpp:1028: test_feed_after_error_does_not_change_state                    [PASSED]
test/test_http_parser/test_http_parser.cpp:1029: test_feed_after_entity_too_large_does_not_change_state         [PASSED]
test/test_http_parser/test_http_parser.cpp:1032: test_method_get                                                [PASSED]
test/test_http_parser/test_http_parser.cpp:1033: test_method_post                                               [PASSED]
test/test_http_parser/test_http_parser.cpp:1034: test_method_put                                                [PASSED]
test/test_http_parser/test_http_parser.cpp:1035: test_method_delete                                             [PASSED]
test/test_http_parser/test_http_parser.cpp:1036: test_method_patch                                              [PASSED]
test/test_http_parser/test_http_parser.cpp:1037: test_method_head                                               [PASSED]
test/test_http_parser/test_http_parser.cpp:1038: test_method_options                                            [PASSED]
test/test_http_parser/test_http_parser.cpp:1039: test_method_overflow_is_error                                  [PASSED]
test/test_http_parser/test_http_parser.cpp:1042: test_path_root                                                 [PASSED]
test/test_http_parser/test_http_parser.cpp:1043: test_path_segments                                             [PASSED]
test/test_http_parser/test_http_parser.cpp:1044: test_path_without_query                                        [PASSED]
test/test_http_parser/test_http_parser.cpp:1045: test_path_overflow_is_414                                      [PASSED]
test/test_http_parser/test_http_parser.cpp:1048: test_single_query_param                                        [PASSED]
test/test_http_parser/test_http_parser.cpp:1049: test_two_query_params                                          [PASSED]
test/test_http_parser/test_http_parser.cpp:1050: test_query_key_not_found_returns_null                          [PASSED]
test/test_http_parser/test_http_parser.cpp:1051: test_query_empty_value                                         [PASSED]
test/test_http_parser/test_http_parser.cpp:1054: test_single_header_stored                                      [PASSED]
test/test_http_parser/test_http_parser.cpp:1055: test_header_lookup_case_insensitive                            [PASSED]
test/test_http_parser/test_http_parser.cpp:1056: test_cookie_basic_and_positions                                [PASSED]
test/test_http_parser/test_http_parser.cpp:1057: test_cookie_missing_and_no_header                              [PASSED]
test/test_http_parser/test_http_parser.cpp:1058: test_cookie_exact_name_not_substring                           [PASSED]
test/test_http_parser/test_http_parser.cpp:1059: test_cookie_quoted_and_value_with_equals                       [PASSED]
test/test_http_parser/test_http_parser.cpp:1060: test_forwarded_rfc7239                                         [PASSED]
test/test_http_parser/test_http_parser.cpp:1061: test_forwarded_leftmost_client                                 [PASSED]
test/test_http_parser/test_http_parser.cpp:1062: test_forwarded_strips_quotes_and_port                          [PASSED]
test/test_http_parser/test_http_parser.cpp:1063: test_forwarded_ipv6_recovered_unknown_rejected                 [PASSED]
test/test_http_parser/test_http_parser.cpp:1064: test_header_leading_space_stripped                             [PASSED]
test/test_http_parser/test_http_parser.cpp:1065: test_content_length_header_parsed                              [PASSED]
test/test_http_parser/test_http_parser.cpp:1066: test_content_length_in_headers_array                           [PASSED]
test/test_http_parser/test_http_parser.cpp:1067: test_multiple_headers_stored                                   [PASSED]
test/test_http_parser/test_http_parser.cpp:1068: test_missing_header_returns_null                               [PASSED]
test/test_http_parser/test_http_parser.cpp:1071: test_get_no_body_completes                                     [PASSED]
test/test_http_parser/test_http_parser.cpp:1072: test_post_with_body                                            [PASSED]
test/test_http_parser/test_http_parser.cpp:1073: test_put_with_body                                             [PASSED]
test/test_http_parser/test_http_parser.cpp:1074: test_body_starting_with_newline                                [PASSED]
test/test_http_parser/test_http_parser.cpp:1075: test_post_content_length_zero                                  [PASSED]
test/test_http_parser/test_http_parser.cpp:1076: test_body_exactly_at_buffer_limit                              [PASSED]
test/test_http_parser/test_http_parser.cpp:1077: test_body_null_terminated_after_complete                       [PASSED]
test/test_http_parser/test_http_parser.cpp:1080: test_body_one_over_limit_is_413                                [PASSED]
test/test_http_parser/test_http_parser.cpp:1081: test_body_far_over_limit_is_413                                [PASSED]
test/test_http_parser/test_http_parser.cpp:1082: test_413_no_body_bytes_fed                                     [PASSED]
test/test_http_parser/test_http_parser.cpp:1083: test_413_header_still_stored                                   [PASSED]
test/test_http_parser/test_http_parser.cpp:1084: test_body_exactly_at_limit_is_not_413                          [PASSED]
test/test_http_parser/test_http_parser.cpp:1087: test_path_overflow_stops_feeding                               [PASSED]
test/test_http_parser/test_http_parser.cpp:1088: test_414_path_filled_to_capacity                               [PASSED]
test/test_http_parser/test_http_parser.cpp:1091: test_method_nul_byte_is_error                                  [PASSED]
test/test_http_parser/test_http_parser.cpp:1092: test_method_control_char_is_error                              [PASSED]
test/test_http_parser/test_http_parser.cpp:1093: test_method_del_byte_is_error                                  [PASSED]
test/test_http_parser/test_http_parser.cpp:1094: test_method_non_tchar_symbol_is_error                          [PASSED]
test/test_http_parser/test_http_parser.cpp:1095: test_method_tchar_symbols_accepted                             [PASSED]
test/test_http_parser/test_http_parser.cpp:1098: test_path_nul_byte_is_error                                    [PASSED]
test/test_http_parser/test_http_parser.cpp:1099: test_path_control_char_is_error                                [PASSED]
test/test_http_parser/test_http_parser.cpp:1100: test_path_del_byte_is_error                                    [PASSED]
test/test_http_parser/test_http_parser.cpp:1101: test_query_nul_byte_is_error                                   [PASSED]
test/test_http_parser/test_http_parser.cpp:1102: test_query_control_char_is_error                               [PASSED]
test/test_http_parser/test_http_parser.cpp:1105: test_header_key_space_is_error                                 [PASSED]
test/test_http_parser/test_http_parser.cpp:1106: test_header_key_nul_byte_is_error                              [PASSED]
test/test_http_parser/test_http_parser.cpp:1107: test_header_key_control_char_is_error                          [PASSED]
test/test_http_parser/test_http_parser.cpp:1108: test_header_key_mid_cr_is_error                                [PASSED]
test/test_http_parser/test_http_parser.cpp:1109: test_header_key_colon_at_start_skips_header                    [PASSED]
test/test_http_parser/test_http_parser.cpp:1110: test_long_standard_header_key_accepted                         [PASSED]
test/test_http_parser/test_http_parser.cpp:1111: test_overlong_header_key_truncated_not_error                   [PASSED]
test/test_http_parser/test_http_parser.cpp:1114: test_header_val_nul_byte_is_error                              [PASSED]
test/test_http_parser/test_http_parser.cpp:1115: test_header_val_control_char_is_error                          [PASSED]
test/test_http_parser/test_http_parser.cpp:1116: test_header_val_del_byte_is_error                              [PASSED]
test/test_http_parser/test_http_parser.cpp:1117: test_header_val_htab_mid_value_allowed                         [PASSED]
test/test_http_parser/test_http_parser.cpp:1118: test_header_val_leading_htab_stripped                          [PASSED]
test/test_http_parser/test_http_parser.cpp:1119: test_header_val_obs_text_allowed                               [PASSED]
test/test_http_parser/test_http_parser.cpp:1122: test_version_http11_recognized                                 [PASSED]
test/test_http_parser/test_http_parser.cpp:1123: test_version_http10_recognized                                 [PASSED]
test/test_http_parser/test_http_parser.cpp:1124: test_version_unknown_is_http_unknown                           [PASSED]
test/test_http_parser/test_http_parser.cpp:1125: test_version_reset_to_unknown                                  [PASSED]
test/test_http_parser/test_http_parser.cpp:1128: test_bad_expect_lf_is_error                                    [PASSED]
test/test_http_parser/test_http_parser.cpp:1129: test_blank_line_non_lf_is_error                                [PASSED]
test/test_http_parser/test_http_parser.cpp:1132: test_slots_are_independent                                     [PASSED]
test/test_http_parser/test_http_parser.cpp:1135: test_incremental_byte_by_byte                                  [PASSED]
test/test_http_parser/test_http_parser.cpp:1136: test_incremental_two_chunks                                    [PASSED]
test/test_http_parser/test_http_parser.cpp:1139: stress_many_requests_same_slot                                 [PASSED]
test/test_http_parser/test_http_parser.cpp:1140: stress_max_headers                                             [PASSED]
test/test_http_parser/test_http_parser.cpp:1141: stress_max_query_params                                        [PASSED]
native:test_http_parser Took 0.75 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test               Status    Duration
-------------  -----------------  --------  ------------
native         test_sse           PASSED    00:00:01.439
native         test_session       PASSED    00:00:00.710
native         test_presentation  PASSED    00:00:00.762
native         test_transport     PASSED    00:00:00.751
native         test_websocket     PASSED    00:00:00.816
native         test_http_parser   PASSED    00:00:00.746
================ 324 test cases: 324 succeeded in 00:00:05.225 ================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_observability in native_observability environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_observability/test_observability.cpp:294: test_transition_fires_hook_with_args                        [PASSED]
test/test_observability/test_observability.cpp:295: test_each_reason_bumps_its_counter                          [PASSED]
test/test_observability/test_observability.cpp:296: test_closing_gauge_is_derived_from_pool                     [PASSED]
test/test_observability/test_observability.cpp:297: test_reset_clears_cumulative_not_derived_gauge              [PASSED]
test/test_observability/test_observability.cpp:298: test_no_hook_after_unregister                               [PASSED]
test/test_observability/test_observability.cpp:299: test_recv_fin_counts_remote_close                           [PASSED]
test/test_observability/test_observability.cpp:300: test_err_cb_counts_error_close                              [PASSED]
test/test_observability/test_observability.cpp:301: test_timeout_sweep_counts_timeout                           [PASSED]
test/test_observability/test_observability.cpp:302: test_local_close_counts_local                               [PASSED]
test/test_observability/test_observability.cpp:303: test_abort_slot_counts_abort_and_frees                      [PASSED]
test/test_observability/test_observability.cpp:304: test_abort_slot_noop_on_free_slot                           [PASSED]
test/test_observability/test_observability.cpp:305: test_backpressure_counts_when_ring_full                     [PASSED]
test/test_observability/test_observability.cpp:307: test_begin_close_dwells_then_drains_on_ack                  [PASSED]
test/test_observability/test_observability.cpp:308: test_begin_close_finalizes_immediately_when_already_drained [PASSED]
test/test_observability/test_observability.cpp:309: test_begin_close_noop_if_not_active                         [PASSED]
test/test_observability/test_observability.cpp:310: test_closing_timeout_reaps_stuck_slot                       [PASSED]
test/test_observability/test_observability.cpp:311: test_recv_during_closing_is_drained_not_processed           [PASSED]
native_observability:test_observability Took 0.97 seconds ----------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment           Test                Status    Duration
--------------------  ------------------  --------  ------------
native_observability  test_observability  PASSED    00:00:00.971
================= 17 test cases: 17 succeeded in 00:00:00.971 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_accept_gate in native_accept_gate environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_accept_gate/test_accept_gate.cpp:230: test_accept_throttle_window                                     [PASSED]
test/test_accept_gate/test_accept_gate.cpp:231: test_accept_throttle_rollover                                   [PASSED]
test/test_accept_gate/test_accept_gate.cpp:232: test_per_ip_independent_budgets                                 [PASSED]
test/test_accept_gate/test_accept_gate.cpp:233: test_per_ip_v6_distinct_buckets                                 [PASSED]
test/test_accept_gate/test_accept_gate.cpp:234: test_per_ip_window_rollover                                     [PASSED]
test/test_accept_gate/test_accept_gate.cpp:235: test_per_ip_unspecified_defers                                  [PASSED]
test/test_accept_gate/test_accept_gate.cpp:236: test_per_ip_eviction_bounded                                    [PASSED]
test/test_accept_gate/test_accept_gate.cpp:237: test_ip_allowlist_empty_allows_all                              [PASSED]
test/test_accept_gate/test_accept_gate.cpp:238: test_ip_allowlist_cidr                                          [PASSED]
test/test_accept_gate/test_accept_gate.cpp:239: test_ip_allowlist_cidr_string                                   [PASSED]
test/test_accept_gate/test_accept_gate.cpp:240: test_ip_allowlist_family_isolation                              [PASSED]
test/test_accept_gate/test_accept_gate.cpp:241: test_ip_allowlist_host_and_zero_prefix                          [PASSED]
test/test_accept_gate/test_accept_gate.cpp:242: test_ip_allowlist_rejects_bad_and_full                          [PASSED]
native_accept_gate:test_accept_gate Took 1.33 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_accept_gate  test_accept_gate  PASSED    00:00:01.328
================= 13 test cases: 13 succeeded in 00:00:01.328 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_http_ota in native_ota environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_http_ota/test_http_ota.cpp:95: test_large_body_streams_to_completion                                  [PASSED]
test/test_http_ota/test_http_ota.cpp:96: test_no_hooks_large_body_is_413                                        [PASSED]
test/test_http_ota/test_http_ota.cpp:97: test_nonmatching_path_not_streamed                                     [PASSED]
native_ota:test_http_ota Took 0.86 seconds -------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test           Status    Duration
-------------  -------------  --------  ------------
native_ota     test_http_ota  PASSED    00:00:00.865
================== 3 test cases: 3 succeeded in 00:00:00.865 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_provisioning in native_prov environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_provisioning/test_provisioning.cpp:66: test_plain_fields                                              [PASSED]
test/test_provisioning/test_provisioning.cpp:67: test_url_decoding                                              [PASSED]
test/test_provisioning/test_provisioning.cpp:68: test_missing_field                                             [PASSED]
test/test_provisioning/test_provisioning.cpp:69: test_no_substring_match                                        [PASSED]
test/test_provisioning/test_provisioning.cpp:70: test_capacity_bound                                            [PASSED]
native_prov:test_provisioning Took 0.80 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test               Status    Duration
-------------  -----------------  --------  ------------
native_prov    test_provisioning  PASSED    00:00:00.803
================== 5 test cases: 5 succeeded in 00:00:00.803 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ssh_crypto in native_ssh environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_crypto/test_ssh_crypto.cpp:1100: test_sha256_empty                                                [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1101: test_sha256_abc                                                  [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1102: test_sha256_448bit                                               [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1103: test_sha256_streaming                                            [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1106: test_hmac_sha256_tc1                                             [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1107: test_hmac_sha256_tc2                                             [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1108: test_hmac_sha256_tc3                                             [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1109: test_hmac_sha256_streaming                                       [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1110: test_hmac_sha512_tc1                                             [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1111: test_hmac_sha512_tc2                                             [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1112: test_hmac_sha512_streaming                                       [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1115: test_aes256ctr_encrypt                                           [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1116: test_aes256ctr_decrypt                                           [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1117: test_aes256ctr_multi_block                                       [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1118: test_aes256ctr_wipe                                              [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1121: test_bn_roundtrip                                                [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1122: test_bn_cmp_equal                                                [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1123: test_bn_cmp_less                                                 [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1124: test_bn_cmp_greater                                              [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1125: test_bn_is_zero                                                  [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1126: test_bn_dh_validate_rejects_zero                                 [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1127: test_bn_dh_validate_rejects_one                                  [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1128: test_bn_dh_validate_accepts_two                                  [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1131: test_expmod_exp1                                                 [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1132: test_expmod_exp2                                                 [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1133: test_expmod_exp3                                                 [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1134: test_expmod_commutative                                          [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1137: test_rsa_pkcs1_pad_structure                                     [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1138: test_rsa_sign_verify_roundtrip                                   [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1139: test_rsa_encode_pubkey                                           [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1140: test_rsa_verify_valid_signature                                  [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1141: test_rsa_verify_rejects_tampered_signature                       [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1142: test_rsa_verify_rejects_wrong_message                            [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1145: test_pkt_send_recv_unencrypted                                   [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1146: test_pkt_padding_alignment                                       [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1147: test_pkt_seq_increments                                          [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1148: test_pkt_disconnect_zeroes_state                                 [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1149: test_pkt_encrypted_roundtrip                                     [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1150: test_pkt_chacha20poly1305_roundtrip                              [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1151: test_pkt_aes_etm_sha256_roundtrip                                [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1152: test_pkt_aes_etm_sha512_roundtrip                                [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1153: test_pkt_encrypted_fragmented                                    [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1154: test_pkt_encrypted_two_packets                                   [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1155: test_ssh_kdf_canonical_mpint_k                                   [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:1156: test_ssh_kdf_extension_chain                                     [PASSED]
native_ssh:test_ssh_crypto Took 4.50 seconds ------------------------------------------------------------------ [PASSED]

Processing test_ssh_auth in native_ssh environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_ssh_auth/test_ssh_auth.cpp:646: test_service_request_errors                                           [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:647: test_build_response_guards                                            [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:648: test_parse_request_truncations                                        [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:649: test_pubkey_blob_parse_failures                                       [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:650: test_pubkey_oversized_signed_prefix                                   [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:651: test_handle_request_index_and_parse_guards                            [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:652: test_service_request_accept                                           [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:653: test_service_request_rejects_unknown                                  [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:654: test_parse_password_request                                           [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:655: test_parse_none_request                                               [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:656: test_handle_request_success                                           [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:657: test_handle_request_wrong_password_fails                              [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:658: test_handle_none_request_fails_without_auth                           [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:659: test_handle_request_no_callback_fails                                 [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:660: test_pubkey_probe_returns_pk_ok                                       [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:661: test_pubkey_valid_signature_succeeds                                  [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:662: test_pubkey_ed25519_valid_signature_succeeds                          [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:663: test_pubkey_tampered_signature_fails                                  [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:664: test_pubkey_unauthorized_key_fails                                    [PASSED]
native_ssh:test_ssh_auth Took 0.80 seconds -------------------------------------------------------------------- [PASSED]

Processing test_ssh_server in native_ssh environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_ssh_server/test_ssh_server.cpp:614: test_ssh_pkt_index_and_cap_guards                                 [PASSED]
test/test_ssh_server/test_ssh_server.cpp:615: test_ssh_pkt_recv_unencrypted_errors                              [PASSED]
test/test_ssh_server/test_ssh_server.cpp:616: test_ssh_pkt_seq_overflow_guards                                  [PASSED]
test/test_ssh_server/test_ssh_server.cpp:617: test_ssh_pkt_encrypted_roundtrip_and_mac_fail                     [PASSED]
test/test_ssh_server/test_ssh_server.cpp:618: test_full_handshake_to_channel_data                               [PASSED]
test/test_ssh_server/test_ssh_server.cpp:619: test_extinfo_build_advertises_server_sig_algs                     [PASSED]
test/test_ssh_server/test_ssh_server.cpp:620: test_extinfo_not_sent_without_ext_info_c                          [PASSED]
test/test_ssh_server/test_ssh_server.cpp:621: test_inbound_ext_info_ignored                                     [PASSED]
test/test_ssh_server/test_ssh_server.cpp:622: test_large_client_kexinit_accepted                                [PASSED]
test/test_ssh_server/test_ssh_server.cpp:623: test_channel_open_before_auth_rejected                            [PASSED]
test/test_ssh_server/test_ssh_server.cpp:624: test_disconnect_closes                                            [PASSED]
test/test_ssh_server/test_ssh_server.cpp:625: test_ignore_is_noop                                               [PASSED]
test/test_ssh_server/test_ssh_server.cpp:626: test_auth_bruteforce_disconnect                                   [PASSED]
test/test_ssh_server/test_ssh_server.cpp:627: test_auth_success_after_failures                                  [PASSED]
test/test_ssh_server/test_ssh_server.cpp:628: test_unimplemented_reply_for_unknown_message                      [PASSED]
test/test_ssh_server/test_ssh_server.cpp:629: test_inbound_close_emits_eof_then_close_separately                [PASSED]
native_ssh:test_ssh_server Took 1.02 seconds ------------------------------------------------------------------ [PASSED]

Processing test_ssh_transport in native_ssh environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_ssh_transport/test_ssh_transport.cpp:865: test_transport_index_guards                                 [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:866: test_banner_and_build_caps                                  [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:867: test_kexinit_parse_field_and_trunc                          [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:868: test_kexdh_parse_and_handle_errors                          [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:869: test_server_banner_format                                   [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:870: test_recv_banner_complete                                   [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:871: test_recv_banner_bare_lf                                    [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:872: test_recv_banner_split_across_reads                         [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:873: test_recv_banner_skips_preamble_lines                       [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:874: test_kexinit_build_starts_with_msg_and_stores_is            [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:875: test_kexinit_parse_accepts_supported                        [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:876: test_kexinit_parse_accepts_when_ours_listed_among_others    [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:877: test_kexinit_parse_rejects_missing_kex                      [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:878: test_kexinit_parse_rejects_hostkey_we_lack                  [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:879: test_kexinit_parse_steers_to_curve_ed25519                  [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:880: test_kexinit_parse_rejects_missing_cipher                   [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:881: test_kexinit_parse_selects_chacha20poly1305                 [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:882: test_kexinit_parse_selects_etm_mac                          [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:883: test_kexinit_parse_rejects_truncated                        [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:884: test_exchange_hash_matches_independent_assembly             [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:885: test_exchange_hash_changes_with_input                       [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:886: test_kexdh_parse_init_extracts_e_with_padding               [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:887: test_kexdh_parse_init_extracts_small_e                      [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:888: test_kexdh_parse_init_rejects_wrong_type                    [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:889: test_kexdh_parse_init_rejects_oversized_e                   [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:890: test_kexdh_build_reply_structure                            [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:891: test_kexdh_handle_produces_reply_and_installs_keys          [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:892: test_kexdh_handle_rejects_invalid_e                         [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:893: test_kexdh_handle_curve25519_ed25519_end_to_end             [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:894: test_kexdh_handle_curve25519_rejects_low_order              [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:895: test_derive_keys_session_id_affects_output                  [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:896: test_rekey_needed_threshold                                 [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:897: test_rekey_due_volume_and_time                              [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:898: test_begin_rekey_preserves_session_and_auth                 [PASSED]
native_ssh:test_ssh_transport Took 1.35 seconds --------------------------------------------------------------- [PASSED]

Processing test_ssh_channel in native_ssh environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_ssh_channel/test_ssh_channel.cpp:782: test_open_session_confirms                                      [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:783: test_open_unknown_type_fails                                    [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:784: test_direct_tcpip_no_cb_prohibited                              [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:785: test_direct_tcpip_accept_confirms                               [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:786: test_direct_tcpip_refused_connect_failed                        [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:787: test_forward_data_routes_to_forward_cb                          [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:788: test_shell_request_success_with_reply                           [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:789: test_unknown_request_failure                                    [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:790: test_request_no_reply_produces_nothing                          [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:791: test_inbound_data_invokes_callback                              [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:792: test_inbound_data_window_replenish                              [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:793: test_inbound_data_exceeding_window_rejected                     [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:794: test_outbound_data_frames_and_decrements_window                 [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:795: test_outbound_data_exceeding_peer_window_rejected               [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:796: test_window_adjust_grows_peer_window                            [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:797: test_build_close_emits_eof_and_close                            [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:798: test_inbound_close_routes_to_channel                            [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:799: test_multiplex_two_channels_route_independently                 [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:800: test_pool_full_open_fails                                       [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:801: test_data_to_unknown_channel_rejected                           [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:802: test_rforward_no_cb_refused                                     [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:803: test_rforward_accept_specific_port                              [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:804: test_rforward_port0_echoes_allocated                            [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:805: test_rforward_no_reply_silent                                   [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:806: test_rforward_cancel                                            [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:807: test_global_unknown_request                                     [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:808: test_global_malformed                                           [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:809: test_forwarded_open_builds_channel                              [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:810: test_forwarded_confirm_opens_channel                            [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:811: test_forwarded_failure_frees_channel                            [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:812: test_forwarded_confirm_unknown_rejected                         [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:813: test_forwarded_inbound_data_routes_to_forward_cb                [PASSED]
native_ssh:test_ssh_channel Took 0.76 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test                Status    Duration
-------------  ------------------  --------  ------------
native_ssh     test_ssh_crypto     PASSED    00:00:04.503
native_ssh     test_ssh_auth       PASSED    00:00:00.796
native_ssh     test_ssh_server     PASSED    00:00:01.023
native_ssh     test_ssh_transport  PASSED    00:00:01.346
native_ssh     test_ssh_channel    PASSED    00:00:00.758
================ 146 test cases: 146 succeeded in 00:00:08.426 ================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ssh_hardening in native_ssh_hardened environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_hardening/test_ssh_hardening.cpp:87: test_password_refused_even_with_correct_callback             [PASSED]
test/test_ssh_hardening/test_ssh_hardening.cpp:88: test_failure_advertises_publickey_only                       [PASSED]
native_ssh_hardened:test_ssh_hardening Took 1.34 seconds ------------------------------------------------------ [PASSED]

=================================== SUMMARY ===================================
Environment          Test                Status    Duration
-------------------  ------------------  --------  ------------
native_ssh_hardened  test_ssh_hardening  PASSED    00:00:01.337
================== 2 test cases: 2 succeeded in 00:00:01.337 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ssh_conn in native_ssh_conn environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_conn/test_ssh_conn.cpp:179: test_accept_sends_server_banner                                       [PASSED]
test/test_ssh_conn/test_ssh_conn.cpp:180: test_banner_then_kexinit_advances_and_replies                         [PASSED]
test/test_ssh_conn/test_ssh_conn.cpp:181: test_poll_triggers_server_rekey                                       [PASSED]
native_ssh_conn:test_ssh_conn Took 2.07 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_ssh_conn  test_ssh_conn  PASSED    00:00:02.067
================== 3 test cases: 3 succeeded in 00:00:02.067 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_regex in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
src/dwserver.cpp: In member function ‘void DetWebServer::serve_file_internal(uint8_t, bool, fs::FS&, const char*, const char*, const char*)’:
src/dwserver.cpp:3138:76: warning: ‘snprintf’ output may be truncated before the last format character [-Wformat-truncation=]
 3138 |         snprintf(lastmod_line, sizeof(lastmod_line), "Last-Modified: %s\r\n", lm_date);
      |                                                                            ^
src/dwserver.cpp:3138:17: note: ‘snprintf’ output between 18 and 57 bytes into a destination of size 56
 3138 |         snprintf(lastmod_line, sizeof(lastmod_line), "Last-Modified: %s\r\n", lm_date);
      |         ~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Testing...
test/test_regex/test_regex.cpp:157: test_numeric_class_plus                                                     [PASSED]
test/test_regex/test_regex.cpp:158: test_dot_star_matches_rest                                                  [PASSED]
test/test_regex/test_regex.cpp:159: test_escaped_dot_extension                                                  [PASSED]
test/test_regex/test_regex.cpp:160: test_optional_quantifier                                                    [PASSED]
test/test_regex/test_regex.cpp:161: test_range_class_only                                                       [PASSED]
test/test_regex/test_regex.cpp:162: test_negated_class                                                          [PASSED]
test/test_regex/test_regex.cpp:163: test_anchored_full_match                                                    [PASSED]
test/test_regex/test_regex.cpp:164: test_method_still_enforced                                                  [PASSED]
test/test_regex/test_regex.cpp:165: test_pathological_pattern_terminates_no_match                               [PASSED]
native_app:test_regex Took 1.66 seconds ----------------------------------------------------------------------- [PASSED]

Processing test_template in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_template/test_template.cpp:153: test_basic_substitution                                               [PASSED]
test/test_template/test_template.cpp:154: test_multiple_placeholders                                            [PASSED]
test/test_template/test_template.cpp:155: test_unknown_placeholder_is_empty                                     [PASSED]
test/test_template/test_template.cpp:156: test_unterminated_placeholder_is_literal                              [PASSED]
test/test_template/test_template.cpp:157: test_null_resolver_empties_all                                        [PASSED]
test/test_template/test_template.cpp:158: test_head_suppresses_body_keeps_length                                [PASSED]
native_app:test_template Took 0.79 seconds -------------------------------------------------------------------- [PASSED]

Processing test_path_params in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_path_params/test_path_params.cpp:170: test_single_param_captured                                      [PASSED]
test/test_path_params/test_path_params.cpp:171: test_multiple_params_captured                                   [PASSED]
test/test_path_params/test_path_params.cpp:172: test_missing_param_returns_null                                 [PASSED]
test/test_path_params/test_path_params.cpp:173: test_literal_segment_mismatch_404                               [PASSED]
test/test_path_params/test_path_params.cpp:174: test_extra_segment_does_not_match                               [PASSED]
test/test_path_params/test_path_params.cpp:175: test_empty_param_value_does_not_match                           [PASSED]
test/test_path_params/test_path_params.cpp:176: test_exact_route_still_matches                                  [PASSED]
test/test_path_params/test_path_params.cpp:177: test_param_route_wrong_method_405                               [PASSED]
native_app:test_path_params Took 0.76 seconds ----------------------------------------------------------------- [PASSED]

Processing test_digest_vectors in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_digest_vectors/test_digest_vectors.cpp:108: test_sha256_fips_kats                                     [PASSED]
test/test_digest_vectors/test_digest_vectors.cpp:109: test_ha1_matches_openssl                                  [PASSED]
test/test_digest_vectors/test_digest_vectors.cpp:110: test_ha2_matches_openssl                                  [PASSED]
test/test_digest_vectors/test_digest_vectors.cpp:111: test_response_matches_openssl                             [PASSED]
native_app:test_digest_vectors Took 0.70 seconds -------------------------------------------------------------- [PASSED]

Processing test_form_params in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_form_params/test_form_params.cpp:135: test_form_fields_parsed                                         [PASSED]
test/test_form_params/test_form_params.cpp:136: test_form_missing_key_returns_false                             [PASSED]
test/test_form_params/test_form_params.cpp:137: test_form_empty_value                                           [PASSED]
test/test_form_params/test_form_params.cpp:138: test_form_wrong_content_type_ignored                            [PASSED]
test/test_form_params/test_form_params.cpp:139: test_form_value_truncated_to_buffer                             [PASSED]
native_app:test_form_params Took 0.79 seconds ----------------------------------------------------------------- [PASSED]

Processing test_iface in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_iface/test_iface.cpp:163: test_ap_only_matches_on_ap                                                  [PASSED]
test/test_iface/test_iface.cpp:164: test_ap_only_hidden_on_sta                                                  [PASSED]
test/test_iface/test_iface.cpp:165: test_sta_only_matches_on_sta                                                [PASSED]
test/test_iface/test_iface.cpp:166: test_sta_only_hidden_on_ap                                                  [PASSED]
test/test_iface/test_iface.cpp:167: test_unfiltered_route_matches_any_interface                                 [PASSED]
test/test_iface/test_iface.cpp:168: test_same_path_two_interfaces_picks_correct                                 [PASSED]
test/test_iface/test_iface.cpp:169: test_set_ap_ip_updates_global                                               [PASSED]
native_app:test_iface Took 0.76 seconds ----------------------------------------------------------------------- [PASSED]

Processing test_json in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_json/test_json.cpp:285: test_writer_simple_object                                                     [PASSED]
test/test_json/test_json.cpp:286: test_writer_nested_and_array                                                  [PASSED]
test/test_json/test_json.cpp:287: test_writer_value_types                                                       [PASSED]
test/test_json/test_json.cpp:288: test_writer_escapes_strings                                                   [PASSED]
test/test_json/test_json.cpp:289: test_writer_control_char_unicode_escape                                       [PASSED]
test/test_json/test_json.cpp:290: test_writer_overflow_sets_not_ok_and_stays_terminated                         [PASSED]
test/test_json/test_json.cpp:291: test_writer_depth_overflow_sets_not_ok                                        [PASSED]
test/test_json/test_json.cpp:292: test_reader_get_string                                                        [PASSED]
test/test_json/test_json.cpp:293: test_reader_get_int                                                           [PASSED]
test/test_json/test_json.cpp:294: test_reader_get_bool                                                          [PASSED]
test/test_json/test_json.cpp:295: test_reader_only_matches_top_level_key                                        [PASSED]
test/test_json/test_json.cpp:296: test_reader_missing_key                                                       [PASSED]
test/test_json/test_json.cpp:297: test_reader_type_mismatch                                                     [PASSED]
test/test_json/test_json.cpp:298: test_reader_unescapes_value                                                   [PASSED]
test/test_json/test_json.cpp:299: test_reader_unicode_escape_to_byte                                            [PASSED]
test/test_json/test_json.cpp:300: test_reader_truncates_to_capacity                                             [PASSED]
test/test_json/test_json.cpp:301: test_reader_negative_int                                                      [PASSED]
test/test_json/test_json.cpp:302: test_writer_null_and_remaining_escapes                                        [PASSED]
test/test_json/test_json.cpp:303: test_reader_null_guards                                                       [PASSED]
test/test_json/test_json.cpp:304: test_reader_all_escapes                                                       [PASSED]
test/test_json/test_json.cpp:305: test_reader_unicode_hex_case                                                  [PASSED]
test/test_json/test_json.cpp:306: test_reader_false_bool                                                        [PASSED]
test/test_json/test_json.cpp:307: test_reader_malformed                                                         [PASSED]
native_app:test_json Took 0.71 seconds ------------------------------------------------------------------------ [PASSED]

Processing test_response_headers in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_response_headers/test_response_headers.cpp:251: test_date_header_emitted_when_time_set                [PASSED]
test/test_response_headers/test_response_headers.cpp:252: test_date_header_omitted_when_clockless               [PASSED]
test/test_response_headers/test_response_headers.cpp:253: test_single_custom_header_present                     [PASSED]
test/test_response_headers/test_response_headers.cpp:254: test_multiple_custom_headers_present                  [PASSED]
test/test_response_headers/test_response_headers.cpp:255: test_set_cookie_basic                                 [PASSED]
test/test_response_headers/test_response_headers.cpp:256: test_set_cookie_with_attrs                            [PASSED]
test/test_response_headers/test_response_headers.cpp:257: test_custom_header_on_send_empty                      [PASSED]
test/test_response_headers/test_response_headers.cpp:258: test_custom_header_on_redirect                        [PASSED]
test/test_response_headers/test_response_headers.cpp:259: test_headers_do_not_leak_across_requests              [PASSED]
test/test_response_headers/test_response_headers.cpp:260: test_clear_response_headers                           [PASSED]
test/test_response_headers/test_response_headers.cpp:261: test_oversized_header_dropped_whole                   [PASSED]
native_app:test_response_headers Took 0.78 seconds ------------------------------------------------------------ [PASSED]

Processing test_middleware in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_middleware/test_middleware.cpp:247: test_middleware_runs_then_handler                                 [PASSED]
test/test_middleware/test_middleware.cpp:248: test_middleware_runs_for_unmatched_route                          [PASSED]
test/test_middleware/test_middleware.cpp:249: test_middleware_can_inject_response_header                        [PASSED]
test/test_middleware/test_middleware.cpp:250: test_middleware_halt_short_circuits_handler                       [PASSED]
test/test_middleware/test_middleware.cpp:251: test_middleware_runs_in_registration_order                        [PASSED]
test/test_middleware/test_middleware.cpp:252: test_use_respects_capacity_cap                                    [PASSED]
test/test_middleware/test_middleware.cpp:253: test_rate_limit_allows_then_rejects                               [PASSED]
test/test_middleware/test_middleware.cpp:254: test_rate_limit_window_resets                                     [PASSED]
test/test_middleware/test_middleware.cpp:255: test_rate_limit_disabled_by_default                               [PASSED]
native_app:test_middleware Took 0.77 seconds ------------------------------------------------------------------ [PASSED]

Processing test_digest_auth in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_digest_auth/test_digest_auth.cpp:396: test_challenge_is_digest_sha256                                 [PASSED]
test/test_digest_auth/test_digest_auth.cpp:397: test_valid_digest_authenticates                                 [PASSED]
test/test_digest_auth/test_digest_auth.cpp:398: test_wrong_password_rejected                                    [PASSED]
test/test_digest_auth/test_digest_auth.cpp:399: test_bad_nonce_rejected                                         [PASSED]
test/test_digest_auth/test_digest_auth.cpp:400: test_wrong_username_rejected                                    [PASSED]
test/test_digest_auth/test_digest_auth.cpp:401: test_wrong_qop_rejected                                         [PASSED]
test/test_digest_auth/test_digest_auth.cpp:402: test_missing_response_field_rejected                            [PASSED]
test/test_digest_auth/test_digest_auth.cpp:403: test_basic_scheme_on_digest_route_rejected                      [PASSED]
test/test_digest_auth/test_digest_auth.cpp:404: test_uri_mismatch_rejected                                      [PASSED]
test/test_digest_auth/test_digest_auth.cpp:405: test_nonce_is_stateless_timestamped                             [PASSED]
test/test_digest_auth/test_digest_auth.cpp:406: test_stale_nonce_triggers_transparent_retry                     [PASSED]
native_app:test_digest_auth Took 0.80 seconds ----------------------------------------------------------------- [PASSED]

Processing test_web_terminal in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_web_terminal/test_web_terminal.cpp:204: test_serves_terminal_page                                     [PASSED]
test/test_web_terminal/test_web_terminal.cpp:205: test_ws_upgrade_tracks_client                                 [PASSED]
test/test_web_terminal/test_web_terminal.cpp:206: test_ws_upgrade_requires_connection_token                     [PASSED]
test/test_web_terminal/test_web_terminal.cpp:207: test_ws_upgrade_rejects_bad_key_length                        [PASSED]
test/test_web_terminal/test_web_terminal.cpp:208: test_command_delivered_to_callback                            [PASSED]
test/test_web_terminal/test_web_terminal.cpp:209: test_broadcast_reaches_client                                 [PASSED]
test/test_web_terminal/test_web_terminal.cpp:210: test_printf_broadcast                                         [PASSED]
test/test_web_terminal/test_web_terminal.cpp:211: test_no_broadcast_without_clients                             [PASSED]
test/test_web_terminal/test_web_terminal.cpp:212: test_close_clears_client                                      [PASSED]
native_app:test_web_terminal Took 0.79 seconds ---------------------------------------------------------------- [PASSED]

Processing test_defer in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_defer/test_defer.cpp:56: test_defer_runs_inline_on_host                                               [PASSED]
test/test_defer/test_defer.cpp:57: test_server_defer_routes_by_owner                                            [PASSED]
test/test_defer/test_defer.cpp:58: test_defer_null_fn_fails                                                     [PASSED]
native_app:test_defer Took 0.74 seconds ----------------------------------------------------------------------- [PASSED]

Processing test_multipart in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_multipart/test_multipart.cpp:495: test_no_content_type_returns_false                                  [PASSED]
test/test_multipart/test_multipart.cpp:496: test_no_boundary_in_content_type_returns_false                      [PASSED]
test/test_multipart/test_multipart.cpp:497: test_body_missing_delimiter_returns_false                           [PASSED]
test/test_multipart/test_multipart.cpp:498: test_single_text_field_parsed                                       [PASSED]
test/test_multipart/test_multipart.cpp:499: test_two_text_fields_parsed                                         [PASSED]
test/test_multipart/test_multipart.cpp:500: test_three_text_fields_parsed                                       [PASSED]
test/test_multipart/test_multipart.cpp:501: test_file_upload_part                                               [PASSED]
test/test_multipart/test_multipart.cpp:502: test_file_upload_with_text_field                                    [PASSED]
test/test_multipart/test_multipart.cpp:503: test_get_field_found                                                [PASSED]
test/test_multipart/test_multipart.cpp:504: test_get_field_not_found_returns_null                               [PASSED]
test/test_multipart/test_multipart.cpp:505: test_get_field_multiple_fields                                      [PASSED]
test/test_multipart/test_multipart.cpp:506: test_data_len_is_correct                                            [PASSED]
test/test_multipart/test_multipart.cpp:507: test_max_parts_captured                                             [PASSED]
test/test_multipart/test_multipart.cpp:508: test_empty_field_value                                              [PASSED]
test/test_multipart/test_multipart.cpp:509: test_part_without_filename_has_null_filename                        [PASSED]
test/test_multipart/test_multipart.cpp:510: test_part_without_content_type_has_null_type                        [PASSED]
test/test_multipart/test_multipart.cpp:511: test_long_boundary_string                                           [PASSED]
test/test_multipart/test_multipart.cpp:512: stress_parse_100_requests                                           [PASSED]
test/test_multipart/test_multipart.cpp:513: stress_get_field_100_lookups                                        [PASSED]
native_app:test_multipart Took 0.77 seconds ------------------------------------------------------------------- [PASSED]

Processing test_auth in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_auth/test_auth.cpp:255: test_unprotected_route_fires_handler                                          [PASSED]
test/test_auth/test_auth.cpp:256: test_protected_route_no_header_returns_401                                    [PASSED]
test/test_auth/test_auth.cpp:257: test_protected_route_wrong_password_returns_401                               [PASSED]
test/test_auth/test_auth.cpp:258: test_protected_route_wrong_username_returns_401                               [PASSED]
test/test_auth/test_auth.cpp:259: test_protected_route_valid_credentials_fires_handler                          [PASSED]
test/test_auth/test_auth.cpp:260: test_401_includes_www_authenticate_header                                     [PASSED]
test/test_auth/test_auth.cpp:261: test_non_basic_scheme_returns_401                                             [PASSED]
test/test_auth/test_auth.cpp:262: test_credentials_without_colon_returns_401                                    [PASSED]
test/test_auth/test_auth.cpp:263: test_protected_and_unprotected_routes_coexist                                 [PASSED]
test/test_auth/test_auth.cpp:264: test_auth_route_returns_404_for_wrong_path                                    [PASSED]
test/test_auth/test_auth.cpp:265: test_auth_checked_per_method                                                  [PASSED]
test/test_auth/test_auth.cpp:267: stress_auth_50_valid_requests                                                 [PASSED]
test/test_auth/test_auth.cpp:268: stress_auth_50_invalid_requests                                               [PASSED]
native_app:test_auth Took 0.76 seconds ------------------------------------------------------------------------ [PASSED]

Processing test_file_serving in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_file_serving/test_file_serving.cpp:342: test_missing_file_returns_404                                 [PASSED]
test/test_file_serving/test_file_serving.cpp:343: test_existing_file_returns_200                                [PASSED]
test/test_file_serving/test_file_serving.cpp:344: test_response_includes_content_type_html                      [PASSED]
test/test_file_serving/test_file_serving.cpp:345: test_response_includes_content_type_js                        [PASSED]
test/test_file_serving/test_file_serving.cpp:346: test_content_length_matches_file_size                         [PASSED]
test/test_file_serving/test_file_serving.cpp:347: test_file_body_is_sent                                        [PASSED]
test/test_file_serving/test_file_serving.cpp:348: test_empty_file_returns_200_with_zero_length                  [PASSED]
test/test_file_serving/test_file_serving.cpp:349: test_large_file_body_fully_sent                               [PASSED]
test/test_file_serving/test_file_serving.cpp:350: test_serve_file_does_not_affect_other_routes                  [PASSED]
test/test_file_serving/test_file_serving.cpp:351: test_multiple_content_types                                   [PASSED]
test/test_file_serving/test_file_serving.cpp:352: stress_serve_file_50_requests                                 [PASSED]
test/test_file_serving/test_file_serving.cpp:353: stress_alternate_missing_and_found                            [PASSED]
native_app:test_file_serving Took 0.77 seconds ---------------------------------------------------------------- [PASSED]

Processing test_dispatch in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_dispatch/test_dispatch.cpp:193: test_method_mismatch_returns_405                                      [PASSED]
test/test_dispatch/test_dispatch.cpp:194: test_405_includes_allow_header                                        [PASSED]
test/test_dispatch/test_dispatch.cpp:195: test_405_allow_lists_all_methods_for_path                             [PASSED]
test/test_dispatch/test_dispatch.cpp:196: test_unknown_path_still_404_not_405                                   [PASSED]
test/test_dispatch/test_dispatch.cpp:197: test_unknown_method_returns_501                                       [PASSED]
test/test_dispatch/test_dispatch.cpp:198: test_unknown_method_not_treated_as_get                                [PASSED]
test/test_dispatch/test_dispatch.cpp:199: test_head_runs_get_handler_without_body                               [PASSED]
test/test_dispatch/test_dispatch.cpp:200: test_get_route_advertises_head_in_allow                               [PASSED]
test/test_dispatch/test_dispatch.cpp:201: test_head_on_post_only_route_405                                      [PASSED]
test/test_dispatch/test_dispatch.cpp:203: test_http_parse_skips_ws_upgraded_slot                                [PASSED]
test/test_dispatch/test_dispatch.cpp:205: test_correct_method_still_dispatches                                  [PASSED]
native_app:test_dispatch Took 0.76 seconds -------------------------------------------------------------------- [PASSED]

Processing test_chunked in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_chunked/test_chunked.cpp:337: test_headers_announce_chunked_no_content_length                         [PASSED]
test/test_chunked/test_chunked.cpp:338: test_single_chunk_framing                                               [PASSED]
test/test_chunked/test_chunked.cpp:339: test_multiple_chunks_in_order                                           [PASSED]
test/test_chunked/test_chunked.cpp:340: test_printf_chunk                                                       [PASSED]
test/test_chunked/test_chunked.cpp:341: test_single_piece_then_terminator                                       [PASSED]
test/test_chunked/test_chunked.cpp:342: test_empty_body_is_just_terminator                                      [PASSED]
test/test_chunked/test_chunked.cpp:343: test_large_chunked_body_not_truncated                                   [PASSED]
test/test_chunked/test_chunked.cpp:344: test_head_sends_headers_only                                            [PASSED]
test/test_chunked/test_chunked.cpp:345: test_custom_header_injected_into_chunked                                [PASSED]
test/test_chunked/test_chunked.cpp:346: test_log_hook_reports_total_body_length                                 [PASSED]
test/test_chunked/test_chunked.cpp:347: test_http10_falls_back_to_close_delimited                               [PASSED]
test/test_chunked/test_chunked.cpp:348: test_http10_large_body_not_truncated                                    [PASSED]
native_app:test_chunked Took 0.78 seconds --------------------------------------------------------------------- [PASSED]

Processing test_application in native_app environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_application/test_application.cpp:1065: test_handler_reads_body                                        [PASSED]
test/test_application/test_application.cpp:1066: test_handler_reads_query_param                                 [PASSED]
test/test_application/test_application.cpp:1067: test_handler_reads_header                                      [PASSED]
test/test_application/test_application.cpp:1068: test_wildcard_before_exact_wildcard_wins                       [PASSED]
test/test_application/test_application.cpp:1071: test_fn_on_registers_and_dispatches                            [PASSED]
test/test_application/test_application.cpp:1072: test_fn_on_path_copied_null_terminated                         [PASSED]
test/test_application/test_application.cpp:1073: test_fn_on_table_full_extra_routes_dropped                     [PASSED]
test/test_application/test_application.cpp:1074: test_fn_on_same_path_different_methods_are_distinct            [PASSED]
test/test_application/test_application.cpp:1077: test_fn_on_not_found_called_when_no_match                      [PASSED]
test/test_application/test_application.cpp:1078: test_fn_on_not_found_not_called_when_match_exists              [PASSED]
test/test_application/test_application.cpp:1081: test_fn_set_cors_options_preflight_clears_slot                 [PASSED]
test/test_application/test_application.cpp:1082: test_fn_set_cors_empty_string_disables                         [PASSED]
test/test_application/test_application.cpp:1085: test_wrong_method_does_not_match                               [PASSED]
test/test_application/test_application.cpp:1086: test_wrong_path_does_not_match                                 [PASSED]
test/test_application/test_application.cpp:1087: test_all_http_methods_dispatched                               [PASSED]
test/test_application/test_application.cpp:1088: test_root_path_matches_exactly                                 [PASSED]
test/test_application/test_application.cpp:1089: test_root_path_does_not_match_subpath                          [PASSED]
test/test_application/test_application.cpp:1090: test_wildcard_matches_any_suffix                               [PASSED]
test/test_application/test_application.cpp:1091: test_wildcard_does_not_match_unrelated_prefix                  [PASSED]
test/test_application/test_application.cpp:1092: test_exact_route_wins_when_registered_first                    [PASSED]
test/test_application/test_application.cpp:1093: test_slot_not_stuck_in_complete_after_handle                   [PASSED]
test/test_application/test_application.cpp:1094: test_parse_error_slot_auto_reset                               [PASSED]
test/test_application/test_application.cpp:1097: stress_last_route_dispatched_in_full_table                     [PASSED]
test/test_application/test_application.cpp:1098: stress_sequential_requests_no_state_leak                       [PASSED]
test/test_application/test_application.cpp:1099: stress_all_slots_dispatched_simultaneously                     [PASSED]
test/test_application/test_application.cpp:1100: stress_wildcard_matches_many_paths                             [PASSED]
test/test_application/test_application.cpp:1101: stress_handle_with_no_complete_slots_is_nop                    [PASSED]
test/test_application/test_application.cpp:1104: race_slot_complete_between_handle_calls                        [PASSED]
test/test_application/test_application.cpp:1105: race_conn_freed_after_parse_complete                           [PASSED]
test/test_application/test_application.cpp:1106: race_double_handle_no_double_dispatch                          [PASSED]
test/test_application/test_application.cpp:1107: race_error_and_valid_slot_in_same_handle                       [PASSED]
test/test_application/test_application.cpp:1108: race_callback_manually_resets_slot                             [PASSED]
test/test_application/test_application.cpp:1111: test_uri_too_long_auto_resets_slot                             [PASSED]
test/test_application/test_application.cpp:1114: test_transfer_encoding_chunked_is_501                          [PASSED]
test/test_application/test_application.cpp:1115: test_transfer_encoding_identity_is_501                         [PASSED]
test/test_application/test_application.cpp:1117: test_redirect_emits_location_and_status                        [PASSED]
test/test_application/test_application.cpp:1118: test_redirect_invalid_code_defaults_to_302                     [PASSED]
test/test_application/test_application.cpp:1119: test_mime_type_detection                                       [PASSED]
test/test_application/test_application.cpp:1121: test_serve_static_file_and_mime                                [PASSED]
test/test_application/test_application.cpp:1122: test_serve_static_index_fallback                               [PASSED]
test/test_application/test_application.cpp:1123: test_serve_static_gzip_when_accepted                           [PASSED]
test/test_application/test_application.cpp:1124: test_serve_static_no_gzip_when_not_accepted                    [PASSED]
test/test_application/test_application.cpp:1125: test_serve_static_traversal_not_leaked                         [PASSED]
test/test_application/test_application.cpp:1126: test_serve_static_missing_is_404                               [PASSED]
test/test_application/test_application.cpp:1127: test_serve_static_etag_conditional_get                         [PASSED]
test/test_application/test_application.cpp:1128: test_serve_static_inm_star_list_weak                           [PASSED]
test/test_application/test_application.cpp:1129: test_serve_static_last_modified_conditional_get                [PASSED]
test/test_application/test_application.cpp:1130: test_serve_static_if_modified_since_malformed                  [PASSED]
test/test_application/test_application.cpp:1131: test_serve_static_cache_control                                [PASSED]
test/test_application/test_application.cpp:1133: test_request_log_hook_fires                                    [PASSED]
test/test_application/test_application.cpp:1134: test_stats_endpoint_emits_json                                 [PASSED]
test/test_application/test_application.cpp:1137: test_sse_broadcast_after_upgrade_matches_path                  [PASSED]
test/test_application/test_application.cpp:1140: test_metrics_emits_prometheus                                  [PASSED]
native_app:test_application Took 0.92 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test                   Status    Duration
-------------  ---------------------  --------  ------------
native_app     test_regex             PASSED    00:00:01.664
native_app     test_template          PASSED    00:00:00.788
native_app     test_path_params       PASSED    00:00:00.761
native_app     test_digest_vectors    PASSED    00:00:00.698
native_app     test_form_params       PASSED    00:00:00.791
native_app     test_iface             PASSED    00:00:00.764
native_app     test_json              PASSED    00:00:00.712
native_app     test_response_headers  PASSED    00:00:00.780
native_app     test_middleware        PASSED    00:00:00.773
native_app     test_digest_auth       PASSED    00:00:00.800
native_app     test_web_terminal      PASSED    00:00:00.791
native_app     test_defer             PASSED    00:00:00.738
native_app     test_multipart         PASSED    00:00:00.774
native_app     test_auth              PASSED    00:00:00.758
native_app     test_file_serving      PASSED    00:00:00.775
native_app     test_dispatch          PASSED    00:00:00.755
native_app     test_chunked           PASSED    00:00:00.785
native_app     test_application       PASSED    00:00:00.920
================ 225 test cases: 225 succeeded in 00:00:14.826 ================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_webdav_handler in native_webdav_handler environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_webdav_handler/test_webdav_handler.cpp:270: test_copy_collection_recursive                            [PASSED]
test/test_webdav_handler/test_webdav_handler.cpp:271: test_copy_collection_depth0_shallow                       [PASSED]
test/test_webdav_handler/test_webdav_handler.cpp:272: test_copy_overwrite_semantics                             [PASSED]
test/test_webdav_handler/test_webdav_handler.cpp:273: test_move_collection_recursive                            [PASSED]
test/test_webdav_handler/test_webdav_handler.cpp:274: test_delete_collection_recursive                          [PASSED]
test/test_webdav_handler/test_webdav_handler.cpp:275: test_propfind_depth0_collection_only                      [PASSED]
test/test_webdav_handler/test_webdav_handler.cpp:276: test_propfind_depth1_lists_members                        [PASSED]
test/test_webdav_handler/test_webdav_handler.cpp:277: test_mkcol_create_and_conflict                            [PASSED]
test/test_webdav_handler/test_webdav_handler.cpp:278: test_delete_single_file                                   [PASSED]
test/test_webdav_handler/test_webdav_handler.cpp:279: test_options_advertises_dav                               [PASSED]
test/test_webdav_handler/test_webdav_handler.cpp:280: test_get_file_through_mount                               [PASSED]
test/test_webdav_handler/test_webdav_handler.cpp:281: test_lock_unlock_advisory                                 [PASSED]
native_webdav_handler:test_webdav_handler Took 1.72 seconds --------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment            Test                 Status    Duration
---------------------  -------------------  --------  ------------
native_webdav_handler  test_webdav_handler  PASSED    00:00:01.723
================= 12 test cases: 12 succeeded in 00:00:01.723 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_diag in native_diag environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
src/dwserver.cpp: In member function ‘void DetWebServer::serve_file_internal(uint8_t, bool, fs::FS&, const char*, const char*, const char*)’:
src/dwserver.cpp:3138:76: warning: ‘snprintf’ output may be truncated before the last format character [-Wformat-truncation=]
 3138 |         snprintf(lastmod_line, sizeof(lastmod_line), "Last-Modified: %s\r\n", lm_date);
      |                                                                            ^
src/dwserver.cpp:3138:17: note: ‘snprintf’ output between 18 and 57 bytes into a destination of size 56
 3138 |         snprintf(lastmod_line, sizeof(lastmod_line), "Last-Modified: %s\r\n", lm_date);
      |         ~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Testing...
test/test_diag/test_diag.cpp:92: test_diag_serves_build_info_json                                               [PASSED]
test/test_diag/test_diag.cpp:93: test_diag_json_braces_balanced                                                 [PASSED]
native_diag:test_diag Took 1.67 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_diag    test_diag  PASSED    00:00:01.675
================== 2 test cases: 2 succeeded in 00:00:01.675 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_snmp_ber in native_snmp environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_snmp_ber/test_snmp_ber.cpp:298: test_integer_vectors                                                  [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:299: test_oid_vector                                                       [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:300: test_octet_string_and_null                                            [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:301: test_counter32_keeps_unsigned                                         [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:302: test_sequence_roundtrip                                               [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:303: test_oid_roundtrip                                                    [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:304: test_large_arc_roundtrip                                              [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:305: test_oid_large_first_subidentifier_roundtrip                          [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:306: test_encoder_overflow_sets_not_ok                                     [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:307: test_decoder_truncated_length_fails                                   [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:308: test_decoder_longform_length_count_past_buffer_fails                  [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:309: test_decoder_longform_length_too_wide_fails                           [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:310: test_decoder_longform_length_content_past_buffer_fails                [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:311: test_decoder_longform_length_max_uint32_fails                         [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:312: test_decoder_indefinite_length_fails                                  [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:313: test_decoder_oversized_integer_fails                                  [PASSED]
native_snmp:test_snmp_ber Took 0.90 seconds ------------------------------------------------------------------- [PASSED]

Processing test_snmp_agent in native_snmp environment
------------------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_snmp_agent/test_snmp_agent.cpp:518: test_registration_and_rw_edges                                    [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:519: test_ipaddress_value_encodes                                      [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:520: test_set_wrong_type_and_unknown                                   [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:521: test_getbulk_variants                                             [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:522: test_dispatch_value_types_and_malformed                           [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:523: test_get_string_v2c                                               [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:524: test_get_unknown_v2c_exception                                    [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:525: test_get_bad_instance_v2c_nosuchinstance                          [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:526: test_get_unknown_v1_error                                         [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:527: test_getnext_walks_to_first                                       [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:528: test_getnext_past_end_endofmibview                                [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:529: test_set_without_rw_community_denied                              [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:530: test_set_with_rw_community_invokes_setter                         [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:531: test_set_readonly_not_writable                                    [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:532: test_getbulk_returns_multiple                                     [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:533: test_dynamic_counter_value                                        [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:534: test_uptime_is_timeticks                                          [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:535: test_unknown_community_no_response                                [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:536: test_v3_message_dropped                                           [PASSED]
native_snmp:test_snmp_agent Took 0.68 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test             Status    Duration
-------------  ---------------  --------  ------------
native_snmp    test_snmp_ber    PASSED    00:00:00.895
native_snmp    test_snmp_agent  PASSED    00:00:00.678
================= 35 test cases: 35 succeeded in 00:00:01.573 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_snmp_v3 in native_snmp_v3 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_snmp_v3/test_snmp_v3.cpp:604: test_v3_message_structure_rejections                                    [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:605: test_v3_init_and_boots_accessors                                        [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:606: test_v3_discovery_variants                                              [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:607: test_v3_priv_not_configured                                             [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:608: test_v3_notify_paths                                                    [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:609: test_localize_key_sha256_vector                                         [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:610: test_aes128_fips197_vector                                              [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:611: test_aes_cfb_roundtrip_partial_block                                    [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:612: test_discovery_reports_engine_id                                        [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:613: test_authnopriv_get                                                     [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:614: test_authpriv_get                                                       [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:615: test_wrong_auth_password_reports_wrong_digest                           [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:616: test_unknown_user_reports                                               [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:617: test_not_in_time_window_reports                                         [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:618: test_inform_v3_builds_informrequest                                     [PASSED]
native_snmp_v3:test_snmp_v3 Took 2.44 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_snmp_v3  test_snmp_v3  PASSED    00:00:02.442
================= 15 test cases: 15 succeeded in 00:00:02.442 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_telnet in native_telnet environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_telnet/test_telnet.cpp:253: test_accept_negotiates_echo_and_sga                                       [PASSED]
test/test_telnet/test_telnet.cpp:254: test_line_echoed_and_dispatched                                           [PASSED]
test/test_telnet/test_telnet.cpp:255: test_backspace_first_line                                                 [PASSED]
test/test_telnet/test_telnet.cpp:256: test_iac_will_gets_dont                                                   [PASSED]
test/test_telnet/test_telnet.cpp:257: test_iac_do_unsupported_gets_wont                                         [PASSED]
test/test_telnet/test_telnet.cpp:258: test_iac_do_echo_is_silent                                                [PASSED]
test/test_telnet/test_telnet.cpp:259: test_iac_stripped_from_data                                               [PASSED]
test/test_telnet/test_telnet.cpp:260: test_print_broadcast                                                      [PASSED]
test/test_telnet/test_telnet.cpp:261: test_unknown_slot_is_noop                                                 [PASSED]
test/test_telnet/test_telnet.cpp:262: test_cr_and_control_ignored                                               [PASSED]
test/test_telnet/test_telnet.cpp:263: test_iac_escaped_literal                                                  [PASSED]
test/test_telnet/test_telnet.cpp:264: test_subnegotiation_consumed                                              [PASSED]
test/test_telnet/test_telnet.cpp:265: test_accept_no_capacity                                                   [PASSED]
test/test_telnet/test_telnet.cpp:266: test_output_escaping_and_printf                                           [PASSED]
test/test_telnet/test_telnet.cpp:267: test_inactive_conn_sends_nothing                                          [PASSED]
native_telnet:test_telnet Took 1.01 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_telnet  test_telnet  PASSED    00:00:01.013
================= 15 test cases: 15 succeeded in 00:00:01.013 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_coap in native_coap environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_coap/test_coap.cpp:1037: test_add_resource_limits                                                     [PASSED]
test/test_coap/test_coap.cpp:1038: test_short_and_truncated_token                                               [PASSED]
test/test_coap/test_coap.cpp:1039: test_malformed_options_bad_request                                           [PASSED]
test/test_coap/test_coap.cpp:1040: test_extended_delta_and_length_ignored                                       [PASSED]
test/test_coap/test_coap.cpp:1041: test_oversized_path_and_query                                                [PASSED]
test/test_coap/test_coap.cpp:1042: test_block_option_too_wide                                                   [PASSED]
test/test_coap/test_coap.cpp:1043: test_block1_reserved_szx                                                     [PASSED]
test/test_coap/test_coap.cpp:1044: test_block1_continue_no_space                                                [PASSED]
test/test_coap/test_coap.cpp:1045: test_response_payload_clamped                                                [PASSED]
test/test_coap/test_coap.cpp:1046: test_response_buffer_too_small                                               [PASSED]
test/test_coap/test_coap.cpp:1047: test_well_known_core_truncates                                               [PASSED]
test/test_coap/test_coap.cpp:1048: test_observe_large_seq_encoding                                              [PASSED]
test/test_coap/test_coap.cpp:1049: test_block2_explicit_paging                                                  [PASSED]
test/test_coap/test_coap.cpp:1050: test_block2_auto_when_large                                                  [PASSED]
test/test_coap/test_coap.cpp:1051: test_block2_szx_clamped                                                      [PASSED]
test/test_coap/test_coap.cpp:1052: test_block2_absent_for_small                                                 [PASSED]
test/test_coap/test_coap.cpp:1053: test_block2_out_of_range                                                     [PASSED]
test/test_coap/test_coap.cpp:1054: test_block2_reserved_szx                                                     [PASSED]
test/test_coap/test_coap.cpp:1055: test_block1_upload_two_blocks                                                [PASSED]
test/test_coap/test_coap.cpp:1056: test_block1_out_of_order                                                     [PASSED]
test/test_coap/test_coap.cpp:1057: test_block1_too_large                                                        [PASSED]
test/test_coap/test_coap.cpp:1058: test_observe_option_in_response                                              [PASSED]
test/test_coap/test_coap.cpp:1059: test_no_observe_option_when_seq_negative                                     [PASSED]
test/test_coap/test_coap.cpp:1060: test_get_content                                                             [PASSED]
test/test_coap/test_coap.cpp:1061: test_not_found                                                               [PASSED]
test/test_coap/test_coap.cpp:1062: test_method_not_allowed                                                      [PASSED]
test/test_coap/test_coap.cpp:1063: test_non_request_type                                                        [PASSED]
test/test_coap/test_coap.cpp:1064: test_put_with_payload                                                        [PASSED]
test/test_coap/test_coap.cpp:1065: test_multi_segment_path                                                      [PASSED]
test/test_coap/test_coap.cpp:1066: test_uri_query                                                               [PASSED]
test/test_coap/test_coap.cpp:1067: test_empty_con_ping_rst                                                      [PASSED]
test/test_coap/test_coap.cpp:1068: test_bad_version_rst                                                         [PASSED]
test/test_coap/test_coap.cpp:1069: test_delete                                                                  [PASSED]
test/test_coap/test_coap.cpp:1070: test_token_8_bytes                                                           [PASSED]
test/test_coap/test_coap.cpp:1071: test_extended_option_length                                                  [PASSED]
test/test_coap/test_coap.cpp:1072: test_ack_ignored                                                             [PASSED]
test/test_coap/test_coap.cpp:1073: test_root_path                                                               [PASSED]
test/test_coap/test_coap.cpp:1074: test_unknown_method_not_allowed                                              [PASSED]
test/test_coap/test_coap.cpp:1075: test_unknown_critical_option_bad_option                                      [PASSED]
test/test_coap/test_coap.cpp:1076: test_well_known_core_discovery                                               [PASSED]
test/test_coap/test_coap.cpp:1077: test_well_known_core_rejects_post                                            [PASSED]
native_coap:test_coap Took 1.12 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_coap    test_coap  PASSED    00:00:01.119
================= 41 test cases: 41 succeeded in 00:00:01.119 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_coap in native_coap_observe environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_coap/test_coap.cpp:1037: test_add_resource_limits                                                     [PASSED]
test/test_coap/test_coap.cpp:1038: test_short_and_truncated_token                                               [PASSED]
test/test_coap/test_coap.cpp:1039: test_malformed_options_bad_request                                           [PASSED]
test/test_coap/test_coap.cpp:1040: test_extended_delta_and_length_ignored                                       [PASSED]
test/test_coap/test_coap.cpp:1041: test_oversized_path_and_query                                                [PASSED]
test/test_coap/test_coap.cpp:1042: test_block_option_too_wide                                                   [PASSED]
test/test_coap/test_coap.cpp:1043: test_block1_reserved_szx                                                     [PASSED]
test/test_coap/test_coap.cpp:1044: test_block1_continue_no_space                                                [PASSED]
test/test_coap/test_coap.cpp:1045: test_response_payload_clamped                                                [PASSED]
test/test_coap/test_coap.cpp:1046: test_response_buffer_too_small                                               [PASSED]
test/test_coap/test_coap.cpp:1047: test_well_known_core_truncates                                               [PASSED]
test/test_coap/test_coap.cpp:1048: test_observe_large_seq_encoding                                              [PASSED]
test/test_coap/test_coap.cpp:1049: test_block2_explicit_paging                                                  [PASSED]
test/test_coap/test_coap.cpp:1050: test_block2_auto_when_large                                                  [PASSED]
test/test_coap/test_coap.cpp:1051: test_block2_szx_clamped                                                      [PASSED]
test/test_coap/test_coap.cpp:1052: test_block2_absent_for_small                                                 [PASSED]
test/test_coap/test_coap.cpp:1053: test_block2_out_of_range                                                     [PASSED]
test/test_coap/test_coap.cpp:1054: test_block2_reserved_szx                                                     [PASSED]
test/test_coap/test_coap.cpp:1055: test_block1_upload_two_blocks                                                [PASSED]
test/test_coap/test_coap.cpp:1056: test_block1_out_of_order                                                     [PASSED]
test/test_coap/test_coap.cpp:1057: test_block1_too_large                                                        [PASSED]
test/test_coap/test_coap.cpp:1058: test_observe_option_in_response                                              [PASSED]
test/test_coap/test_coap.cpp:1059: test_no_observe_option_when_seq_negative                                     [PASSED]
test/test_coap/test_coap.cpp:1060: test_get_content                                                             [PASSED]
test/test_coap/test_coap.cpp:1061: test_not_found                                                               [PASSED]
test/test_coap/test_coap.cpp:1062: test_method_not_allowed                                                      [PASSED]
test/test_coap/test_coap.cpp:1063: test_non_request_type                                                        [PASSED]
test/test_coap/test_coap.cpp:1064: test_put_with_payload                                                        [PASSED]
test/test_coap/test_coap.cpp:1065: test_multi_segment_path                                                      [PASSED]
test/test_coap/test_coap.cpp:1066: test_uri_query                                                               [PASSED]
test/test_coap/test_coap.cpp:1067: test_empty_con_ping_rst                                                      [PASSED]
test/test_coap/test_coap.cpp:1068: test_bad_version_rst                                                         [PASSED]
test/test_coap/test_coap.cpp:1069: test_delete                                                                  [PASSED]
test/test_coap/test_coap.cpp:1070: test_token_8_bytes                                                           [PASSED]
test/test_coap/test_coap.cpp:1071: test_extended_option_length                                                  [PASSED]
test/test_coap/test_coap.cpp:1072: test_ack_ignored                                                             [PASSED]
test/test_coap/test_coap.cpp:1073: test_root_path                                                               [PASSED]
test/test_coap/test_coap.cpp:1074: test_unknown_method_not_allowed                                              [PASSED]
test/test_coap/test_coap.cpp:1075: test_unknown_critical_option_bad_option                                      [PASSED]
test/test_coap/test_coap.cpp:1076: test_well_known_core_discovery                                               [PASSED]
test/test_coap/test_coap.cpp:1077: test_well_known_core_rejects_post                                            [PASSED]
native_coap_observe:test_coap Took 0.95 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment          Test       Status    Duration
-------------------  ---------  --------  ------------
native_coap_observe  test_coap  PASSED    00:00:00.949
================= 41 test cases: 41 succeeded in 00:00:00.949 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_webdav in native_webdav environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_webdav/test_webdav.cpp:291: test_method_classification                                                [PASSED]
test/test_webdav/test_webdav.cpp:292: test_depth_parsing                                                        [PASSED]
test/test_webdav/test_webdav.cpp:293: test_xml_escape                                                           [PASSED]
test/test_webdav/test_webdav.cpp:294: test_xml_escape_truncates_safely                                          [PASSED]
test/test_webdav/test_webdav.cpp:295: test_dest_absolute_uri                                                    [PASSED]
test/test_webdav/test_webdav.cpp:296: test_dest_percent_decoded                                                 [PASSED]
test/test_webdav/test_webdav.cpp:297: test_dest_abs_path                                                        [PASSED]
test/test_webdav/test_webdav.cpp:298: test_dest_rejects_malformed                                               [PASSED]
test/test_webdav/test_webdav.cpp:299: test_multistatus_file_and_collection                                      [PASSED]
test/test_webdav/test_webdav.cpp:300: test_multistatus_escapes_href                                             [PASSED]
test/test_webdav/test_webdav.cpp:301: test_multistatus_entry_stops_when_full                                    [PASSED]
test/test_webdav/test_webdav.cpp:302: test_proppatch_windows_timestamp                                          [PASSED]
test/test_webdav/test_webdav.cpp:303: test_proppatch_multiple_and_self_closed                                   [PASSED]
test/test_webdav/test_webdav.cpp:304: test_proppatch_remove_block                                               [PASSED]
test/test_webdav/test_webdav.cpp:305: test_proppatch_escapes_href                                               [PASSED]
test/test_webdav/test_webdav.cpp:306: test_proppatch_empty_body_is_valid                                        [PASSED]
test/test_webdav/test_webdav.cpp:307: test_proppatch_rejects_injection                                          [PASSED]
test/test_webdav/test_webdav.cpp:308: test_proppatch_fuzz_bounded                                               [PASSED]
test/test_webdav/test_webdav.cpp:309: test_proppatch_stops_when_full                                            [PASSED]
native_webdav:test_webdav Took 0.82 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_webdav  test_webdav  PASSED    00:00:00.822
================= 19 test cases: 19 succeeded in 00:00:00.822 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_modbus in native_modbus environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_modbus/test_modbus.cpp:424: test_read_holding_registers                                               [PASSED]
test/test_modbus/test_modbus.cpp:425: test_read_input_registers                                                 [PASSED]
test/test_modbus/test_modbus.cpp:426: test_read_coils_packs_bits                                                [PASSED]
test/test_modbus/test_modbus.cpp:427: test_write_single_coil                                                    [PASSED]
test/test_modbus/test_modbus.cpp:428: test_write_single_register                                                [PASSED]
test/test_modbus/test_modbus.cpp:429: test_write_multiple_registers                                             [PASSED]
test/test_modbus/test_modbus.cpp:430: test_write_multiple_coils                                                 [PASSED]
test/test_modbus/test_modbus.cpp:431: test_exception_illegal_function                                           [PASSED]
test/test_modbus/test_modbus.cpp:432: test_exception_illegal_address                                            [PASSED]
test/test_modbus/test_modbus.cpp:433: test_exception_illegal_value                                              [PASSED]
test/test_modbus/test_modbus.cpp:434: test_write_single_coil_bad_value                                          [PASSED]
test/test_modbus/test_modbus.cpp:435: test_non_modbus_protocol_id_ignored                                       [PASSED]
test/test_modbus/test_modbus.cpp:436: test_truncated_frame_ignored                                              [PASSED]
test/test_modbus/test_modbus.cpp:437: test_discrete_and_input_accessors                                         [PASSED]
test/test_modbus/test_modbus.cpp:438: test_exceptions_per_function                                              [PASSED]
test/test_modbus/test_modbus.cpp:439: test_small_response_buffer                                                [PASSED]
test/test_modbus/test_modbus.cpp:441: test_rtu_crc16_known_vector                                               [PASSED]
test/test_modbus/test_modbus.cpp:442: test_rtu_read_holding_roundtrip                                           [PASSED]
test/test_modbus/test_modbus.cpp:443: test_rtu_bad_crc_dropped                                                  [PASSED]
test/test_modbus/test_modbus.cpp:444: test_rtu_wrong_address_dropped                                            [PASSED]
test/test_modbus/test_modbus.cpp:445: test_rtu_broadcast_executes_without_reply                                 [PASSED]
test/test_modbus/test_modbus.cpp:446: test_rtu_edge_cases                                                       [PASSED]
native_modbus:test_modbus Took 0.80 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_modbus  test_modbus  PASSED    00:00:00.801
================= 22 test cases: 22 succeeded in 00:00:00.801 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_cloudevents in native_cloudevents environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_cloudevents/test_cloudevents.cpp:130: test_build_minimal                                              [PASSED]
test/test_cloudevents/test_cloudevents.cpp:131: test_build_requires_id_source_type                              [PASSED]
test/test_cloudevents/test_cloudevents.cpp:132: test_build_with_json_data                                       [PASSED]
test/test_cloudevents/test_cloudevents.cpp:133: test_build_with_string_data                                     [PASSED]
test/test_cloudevents/test_cloudevents.cpp:134: test_build_overflow_fails_closed                                [PASSED]
test/test_cloudevents/test_cloudevents.cpp:135: test_from_headers_binary_mode                                   [PASSED]
test/test_cloudevents/test_cloudevents.cpp:136: test_from_headers_missing_required                              [PASSED]
native_cloudevents:test_cloudevents Took 0.93 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_cloudevents  test_cloudevents  PASSED    00:00:00.929
================== 7 test cases: 7 succeeded in 00:00:00.929 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_redis_resp in native_redis environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_redis_resp/test_redis_resp.cpp:127: test_encode_command                                               [PASSED]
test/test_redis_resp/test_redis_resp.cpp:128: test_encode_binary_safe                                           [PASSED]
test/test_redis_resp/test_redis_resp.cpp:129: test_encode_overflow_fails_closed                                 [PASSED]
test/test_redis_resp/test_redis_resp.cpp:130: test_parse_simple_and_error                                       [PASSED]
test/test_redis_resp/test_redis_resp.cpp:131: test_parse_integer                                                [PASSED]
test/test_redis_resp/test_redis_resp.cpp:132: test_parse_bulk_and_nil                                           [PASSED]
test/test_redis_resp/test_redis_resp.cpp:133: test_parse_array_cursor                                           [PASSED]
test/test_redis_resp/test_redis_resp.cpp:134: test_parse_incomplete_and_malformed                               [PASSED]
native_redis:test_redis_resp Took 0.78 seconds ---------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test             Status    Duration
-------------  ---------------  --------  ------------
native_redis   test_redis_resp  PASSED    00:00:00.781
================== 8 test cases: 8 succeeded in 00:00:00.781 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_stomp in native_stomp environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_stomp/test_stomp.cpp:214: test_build_send                                                             [PASSED]
test/test_stomp/test_stomp.cpp:215: test_build_cr_escape_and_guards                                             [PASSED]
test/test_stomp/test_stomp.cpp:216: test_parse_more_edges                                                       [PASSED]
test/test_stomp/test_stomp.cpp:217: test_header_and_unescape_null                                               [PASSED]
test/test_stomp/test_stomp.cpp:218: test_build_no_headers_no_body                                               [PASSED]
test/test_stomp/test_stomp.cpp:219: test_build_escapes_header                                                   [PASSED]
test/test_stomp/test_stomp.cpp:220: test_build_overflow_fails_closed                                            [PASSED]
test/test_stomp/test_stomp.cpp:221: test_round_trip                                                             [PASSED]
test/test_stomp/test_stomp.cpp:222: test_parse_message_crlf                                                     [PASSED]
test/test_stomp/test_stomp.cpp:223: test_parse_content_length_body_with_nul                                     [PASSED]
test/test_stomp/test_stomp.cpp:224: test_parse_skips_leading_heartbeats                                         [PASSED]
test/test_stomp/test_stomp.cpp:225: test_parse_incomplete_and_malformed                                         [PASSED]
test/test_stomp/test_stomp.cpp:226: test_unescape                                                               [PASSED]
test/test_stomp/test_stomp.cpp:227: test_unescape_rejects_bad                                                   [PASSED]
native_stomp:test_stomp Took 0.81 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_stomp   test_stomp  PASSED    00:00:00.806
================= 14 test cases: 14 succeeded in 00:00:00.806 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_mqtt_sn in native_mqtt_sn environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_mqtt_sn/test_mqtt_sn.cpp:288: test_make_flags                                                         [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:289: test_build_connect_bytes                                                [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:290: test_build_publish_bytes                                                [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:291: test_register_round_trip                                                [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:292: test_parse_connack_regack_suback_publish                                [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:293: test_three_octet_length                                                 [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:294: test_optional_fields                                                    [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:295: test_overflow_and_malformed                                             [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:296: test_build_regack_puback                                                [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:297: test_build_subscribe_variants                                           [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:298: test_pingreq_with_client_id                                             [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:299: test_build_guards                                                       [PASSED]
test/test_mqtt_sn/test_mqtt_sn.cpp:300: test_parse_typed_rejections                                             [PASSED]
native_mqtt_sn:test_mqtt_sn Took 0.78 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_mqtt_sn  test_mqtt_sn  PASSED    00:00:00.778
================= 13 test cases: 13 succeeded in 00:00:00.778 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_flow_export in native_flow_export environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_flow_export/test_flow_export.cpp:158: test_v5_header_bytes                                            [PASSED]
test/test_flow_export/test_flow_export.cpp:159: test_v5_record_bytes                                            [PASSED]
test/test_flow_export/test_flow_export.cpp:160: test_v5_overflow_fails_closed                                   [PASSED]
test/test_flow_export/test_flow_export.cpp:161: test_ipfix_message_bytes                                        [PASSED]
test/test_flow_export/test_flow_export.cpp:162: test_v9_count_and_padding                                       [PASSED]
test/test_flow_export/test_flow_export.cpp:163: test_finish_overflow_fails_closed                               [PASSED]
native_flow_export:test_flow_export Took 0.78 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_flow_export  test_flow_export  PASSED    00:00:00.777
================== 6 test cases: 6 succeeded in 00:00:00.777 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_protobuf in native_protobuf environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_protobuf/test_protobuf.cpp:285: test_writer_error_paths                                               [PASSED]
test/test_protobuf/test_protobuf.cpp:286: test_reader_error_paths                                               [PASSED]
test/test_protobuf/test_protobuf.cpp:287: test_float_bits_helper                                                [PASSED]
test/test_protobuf/test_protobuf.cpp:288: test_vector_field1_150                                                [PASSED]
test/test_protobuf/test_protobuf.cpp:289: test_vector_string_testing                                            [PASSED]
test/test_protobuf/test_protobuf.cpp:290: test_zigzag_mapping                                                   [PASSED]
test/test_protobuf/test_protobuf.cpp:291: test_fixed_and_float_bytes                                            [PASSED]
test/test_protobuf/test_protobuf.cpp:292: test_round_trip_reader                                                [PASSED]
test/test_protobuf/test_protobuf.cpp:293: test_int64_negative                                                   [PASSED]
test/test_protobuf/test_protobuf.cpp:294: test_varint_and_overflow                                              [PASSED]
test/test_protobuf/test_protobuf.cpp:295: test_malformed_reads                                                  [PASSED]
test/test_protobuf/test_protobuf.cpp:296: test_varint_width_boundary                                            [PASSED]
test/test_protobuf/test_protobuf.cpp:297: test_empty_length_field                                               [PASSED]
native_protobuf:test_protobuf Took 0.77 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_protobuf  test_protobuf  PASSED    00:00:00.775
================= 13 test cases: 13 succeeded in 00:00:00.775 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_preempt_queue in native_preempt_queue environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_preempt_queue/test_preempt_queue.cpp:213: test_start_validates_and_runs                               [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:214: test_fifo_order                                             [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:215: test_urgent_goes_to_front                                   [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:216: test_fail_closed_when_full                                  [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:217: test_high_water_tracks_peak                                 [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:218: test_from_isr_enqueues                                      [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:219: test_drain_empties_and_reuses                               [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:220: test_internal_lanes_outrank_user                            [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:221: test_lanes_are_isolated                                     [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:222: test_lane_start_stop_running_independent                    [PASSED]
test/test_preempt_queue/test_preempt_queue.cpp:223: test_lane_high_water_is_per_lane                            [PASSED]
native_preempt_queue:test_preempt_queue Took 0.85 seconds ----------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment           Test                Status    Duration
--------------------  ------------------  --------  ------------
native_preempt_queue  test_preempt_queue  PASSED    00:00:00.854
================= 11 test cases: 11 succeeded in 00:00:00.854 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_dma in native_dma environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_dma/test_dma.cpp:251: test_open_validates                                                             [PASSED]
test/test_dma/test_dma.cpp:252: test_ingress_emits_rx_event                                                     [PASSED]
test/test_dma/test_dma.cpp:253: test_buffer_fills_then_partial_flush                                            [PASSED]
test/test_dma/test_dma.cpp:254: test_ping_pong_flips_buffer                                                     [PASSED]
test/test_dma/test_dma.cpp:255: test_egress_captures_tx                                                         [PASSED]
test/test_dma/test_dma.cpp:256: test_tx_one_in_flight_fail_closed                                               [PASSED]
test/test_dma/test_dma.cpp:257: test_tx_rejects_bad_len                                                         [PASSED]
test/test_dma/test_dma.cpp:258: test_loopback_round_trip                                                        [PASSED]
test/test_dma/test_dma.cpp:259: test_feed_fail_closed_when_full                                                 [PASSED]
test/test_dma/test_dma.cpp:260: test_closed_channel_is_inert                                                    [PASSED]
test/test_dma/test_dma.cpp:261: test_two_channels_independent                                                   [PASSED]
native_dma:test_dma Took 0.94 seconds ------------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_dma     test_dma  PASSED    00:00:00.942
================= 11 test cases: 11 succeeded in 00:00:00.942 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_forward in native_forward environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_forward/test_forward.cpp:256: test_default_deny                                                       [PASSED]
test/test_forward/test_forward.cpp:257: test_allow_forwards                                                     [PASSED]
test/test_forward/test_forward.cpp:258: test_no_self_forward                                                    [PASSED]
test/test_forward/test_forward.cpp:259: test_deny_wins_over_allow                                               [PASSED]
test/test_forward/test_forward.cpp:260: test_multi_destination_fanout                                           [PASSED]
test/test_forward/test_forward.cpp:261: test_rate_cap_drops_then_reopens                                        [PASSED]
test/test_forward/test_forward.cpp:262: test_send_failure_counted                                               [PASSED]
test/test_forward/test_forward.cpp:263: test_add_if_validation_and_table_full                                   [PASSED]
test/test_forward/test_forward.cpp:264: test_add_rule_table_full                                                [PASSED]
test/test_forward/test_forward.cpp:265: test_unregistered_destination_is_inert                                  [PASSED]
test/test_forward/test_forward.cpp:266: test_acl_deny_by_byte_pattern                                           [PASSED]
test/test_forward/test_forward.cpp:267: test_acl_allowlist_default_deny                                         [PASSED]
test/test_forward/test_forward.cpp:268: test_acl_first_match_wins                                               [PASSED]
test/test_forward/test_forward.cpp:269: test_acl_src_any_content_wildcard                                       [PASSED]
test/test_forward/test_forward.cpp:270: test_acl_short_frame_skips_entry                                        [PASSED]
test/test_forward/test_forward.cpp:271: test_acl_add_validation_and_table_full                                  [PASSED]
native_forward:test_forward Took 1.03 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_forward  test_forward  PASSED    00:00:01.032
================= 16 test cases: 16 succeeded in 00:00:01.032 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_gateway in native_gateway environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_gateway/test_gateway.cpp:234: test_uplink_envelopes_and_publishes                                     [PASSED]
test/test_gateway/test_gateway.cpp:235: test_uplink_no_sink_drops                                               [PASSED]
test/test_gateway/test_gateway.cpp:236: test_uplink_unknown_port_drops                                          [PASSED]
test/test_gateway/test_gateway.cpp:237: test_uplink_rate_cap                                                    [PASSED]
test/test_gateway/test_gateway.cpp:238: test_uplink_sink_refusal_counted                                        [PASSED]
test/test_gateway/test_gateway.cpp:239: test_downlink_transmits                                                 [PASSED]
test/test_gateway/test_gateway.cpp:240: test_downlink_no_tx_or_unknown_port_drops                               [PASSED]
test/test_gateway/test_gateway.cpp:241: test_downlink_tx_refusal_counted                                        [PASSED]
test/test_gateway/test_gateway.cpp:242: test_topic_format                                                       [PASSED]
test/test_gateway/test_gateway.cpp:243: test_add_port_validation_and_table_full                                 [PASSED]
test/test_gateway/test_gateway.cpp:244: test_seq_increments_per_uplink                                          [PASSED]
native_gateway:test_gateway Took 0.94 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_gateway  test_gateway  PASSED    00:00:00.943
================= 11 test cases: 11 succeeded in 00:00:00.943 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_lora in native_lora environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_lora/test_lora.cpp:210: test_frame_build_then_parse                                                   [PASSED]
test/test_lora/test_lora.cpp:211: test_frame_parse_rejects_short                                                [PASSED]
test/test_lora/test_lora.cpp:212: test_frame_build_bounds                                                       [PASSED]
test/test_lora/test_lora.cpp:213: test_init_verifies_chip_and_lands_in_standby                                  [PASSED]
test/test_lora/test_lora.cpp:214: test_init_fails_on_wrong_version                                              [PASSED]
test/test_lora/test_lora.cpp:215: test_init_programs_frequency                                                  [PASSED]
test/test_lora/test_lora.cpp:216: test_send_loads_fifo_and_starts_tx                                            [PASSED]
test/test_lora/test_lora.cpp:217: test_tx_done_flag                                                             [PASSED]
test/test_lora/test_lora.cpp:218: test_set_rx_enters_continuous                                                 [PASSED]
test/test_lora/test_lora.cpp:219: test_recv_reads_frame_and_rssi                                                [PASSED]
test/test_lora/test_lora.cpp:220: test_recv_no_packet                                                           [PASSED]
test/test_lora/test_lora.cpp:221: test_recv_crc_error_dropped                                                   [PASSED]
test/test_lora/test_lora.cpp:222: test_recv_truncates_to_cap                                                    [PASSED]
native_lora:test_lora Took 0.77 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_lora    test_lora  PASSED    00:00:00.767
================= 13 test cases: 13 succeeded in 00:00:00.767 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_nrf24 in native_nrf24 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_nrf24/test_nrf24.cpp:188: test_init_configures_and_powers_up                                          [PASSED]
test/test_nrf24/test_nrf24.cpp:189: test_init_fails_when_absent                                                 [PASSED]
test/test_nrf24/test_nrf24.cpp:190: test_send_pads_to_width_and_keys_tx                                         [PASSED]
test/test_nrf24/test_nrf24.cpp:191: test_send_rejects_oversize                                                  [PASSED]
test/test_nrf24/test_nrf24.cpp:192: test_tx_done_flag                                                           [PASSED]
test/test_nrf24/test_nrf24.cpp:193: test_set_rx_enters_prx                                                      [PASSED]
test/test_nrf24/test_nrf24.cpp:194: test_recv_reads_payload_and_pipe                                            [PASSED]
test/test_nrf24/test_nrf24.cpp:195: test_recv_no_packet                                                         [PASSED]
test/test_nrf24/test_nrf24.cpp:196: test_recv_fifo_empty_pipe                                                   [PASSED]
test/test_nrf24/test_nrf24.cpp:197: test_recv_truncates_to_cap                                                  [PASSED]
native_nrf24:test_nrf24 Took 0.77 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_nrf24   test_nrf24  PASSED    00:00:00.767
================= 10 test cases: 10 succeeded in 00:00:00.767 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_enocean in native_enocean environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_enocean/test_enocean.cpp:120: test_crc8_known_answers                                                 [PASSED]
test/test_enocean/test_enocean.cpp:121: test_build_then_parse_round_trip                                        [PASSED]
test/test_enocean/test_enocean.cpp:122: test_parse_rejects_bad_sync                                             [PASSED]
test/test_enocean/test_enocean.cpp:123: test_parse_rejects_bad_header_crc                                       [PASSED]
test/test_enocean/test_enocean.cpp:124: test_parse_rejects_bad_data_crc                                         [PASSED]
test/test_enocean/test_enocean.cpp:125: test_parse_needs_more_bytes                                             [PASSED]
test/test_enocean/test_enocean.cpp:126: test_parse_rejects_over_length                                          [PASSED]
test/test_enocean/test_enocean.cpp:127: test_parse_resynchronises_after_junk                                    [PASSED]
test/test_enocean/test_enocean.cpp:128: test_build_bounds                                                       [PASSED]
native_enocean:test_enocean Took 0.79 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_enocean  test_enocean  PASSED    00:00:00.790
================== 9 test cases: 9 succeeded in 00:00:00.790 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_pn532 in native_pn532 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_pn532/test_pn532.cpp:139: test_build_getfirmwareversion_kat                                           [PASSED]
test/test_pn532/test_pn532.cpp:140: test_parse_getfirmwareversion_response_kat                                  [PASSED]
test/test_pn532/test_pn532.cpp:141: test_build_then_parse_round_trip                                            [PASSED]
test/test_pn532/test_pn532.cpp:142: test_parse_rejects_bad_preamble_and_start                                   [PASSED]
test/test_pn532/test_pn532.cpp:143: test_parse_rejects_bad_lcs                                                  [PASSED]
test/test_pn532/test_pn532.cpp:144: test_parse_rejects_bad_dcs                                                  [PASSED]
test/test_pn532/test_pn532.cpp:145: test_parse_needs_more_bytes                                                 [PASSED]
test/test_pn532/test_pn532.cpp:146: test_parse_rejects_over_length                                              [PASSED]
test/test_pn532/test_pn532.cpp:147: test_ack_frame                                                              [PASSED]
test/test_pn532/test_pn532.cpp:148: test_build_bounds                                                           [PASSED]
native_pn532:test_pn532 Took 0.77 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_pn532   test_pn532  PASSED    00:00:00.774
================= 10 test cases: 10 succeeded in 00:00:00.774 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_sigfox in native_sigfox environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_sigfox/test_sigfox.cpp:74: test_build_uplink_hex_encode                                               [PASSED]
test/test_sigfox/test_sigfox.cpp:75: test_build_uplink_single_byte                                              [PASSED]
test/test_sigfox/test_sigfox.cpp:76: test_build_uplink_bounds                                                   [PASSED]
test/test_sigfox/test_sigfox.cpp:77: test_parse_response_ok                                                     [PASSED]
test/test_sigfox/test_sigfox.cpp:78: test_parse_response_error                                                  [PASSED]
test/test_sigfox/test_sigfox.cpp:79: test_parse_response_pending                                                [PASSED]
test/test_sigfox/test_sigfox.cpp:80: test_parse_response_error_wins                                             [PASSED]
native_sigfox:test_sigfox Took 0.77 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_sigfox  test_sigfox  PASSED    00:00:00.772
================== 7 test cases: 7 succeeded in 00:00:00.772 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_zwave in native_zwave environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_zwave/test_zwave.cpp:109: test_build_getversion_kat                                                   [PASSED]
test/test_zwave/test_zwave.cpp:110: test_build_then_parse_round_trip                                            [PASSED]
test/test_zwave/test_zwave.cpp:111: test_parse_getversion_kat                                                   [PASSED]
test/test_zwave/test_zwave.cpp:112: test_parse_rejects_bad_sof                                                  [PASSED]
test/test_zwave/test_zwave.cpp:113: test_parse_rejects_bad_checksum                                             [PASSED]
test/test_zwave/test_zwave.cpp:114: test_parse_needs_more_bytes                                                 [PASSED]
test/test_zwave/test_zwave.cpp:115: test_parse_rejects_over_length                                              [PASSED]
test/test_zwave/test_zwave.cpp:116: test_control_bytes                                                          [PASSED]
test/test_zwave/test_zwave.cpp:117: test_build_bounds                                                           [PASSED]
native_zwave:test_zwave Took 0.78 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_zwave   test_zwave  PASSED    00:00:00.779
================== 9 test cases: 9 succeeded in 00:00:00.779 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_zigbee in native_zigbee environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_zigbee/test_zigbee.cpp:125: test_crc16_rst_kat                                                        [PASSED]
test/test_zigbee/test_zigbee.cpp:126: test_encode_rst_frame_kat                                                 [PASSED]
test/test_zigbee/test_zigbee.cpp:127: test_encode_decode_round_trip                                             [PASSED]
test/test_zigbee/test_zigbee.cpp:128: test_byte_stuffing_round_trip                                             [PASSED]
test/test_zigbee/test_zigbee.cpp:129: test_decode_needs_more_without_flag                                       [PASSED]
test/test_zigbee/test_zigbee.cpp:130: test_decode_rejects_bad_crc                                               [PASSED]
test/test_zigbee/test_zigbee.cpp:131: test_decode_rejects_dangling_escape                                       [PASSED]
test/test_zigbee/test_zigbee.cpp:132: test_decode_rejects_small_payload_buffer                                  [PASSED]
test/test_zigbee/test_zigbee.cpp:133: test_encode_bounds                                                        [PASSED]
native_zigbee:test_zigbee Took 0.78 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_zigbee  test_zigbee  PASSED    00:00:00.782
================== 9 test cases: 9 succeeded in 00:00:00.782 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_thread in native_thread environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_thread/test_thread.cpp:199: test_fcs_x25_check_value                                                  [PASSED]
test/test_thread/test_thread.cpp:200: test_encode_decode_round_trip                                             [PASSED]
test/test_thread/test_thread.cpp:201: test_byte_stuffing_round_trip                                             [PASSED]
test/test_thread/test_thread.cpp:202: test_decode_needs_more_without_flag                                       [PASSED]
test/test_thread/test_thread.cpp:203: test_decode_rejects_bad_fcs                                               [PASSED]
test/test_thread/test_thread.cpp:204: test_decode_rejects_dangling_escape                                       [PASSED]
test/test_thread/test_thread.cpp:205: test_decode_rejects_small_payload_buffer                                  [PASSED]
test/test_thread/test_thread.cpp:206: test_encode_bounds                                                        [PASSED]
test/test_thread/test_thread.cpp:207: test_spinel_pack_uint_kats                                                [PASSED]
test/test_thread/test_thread.cpp:208: test_spinel_pack_unpack_round_trip                                        [PASSED]
test/test_thread/test_thread.cpp:209: test_spinel_unpack_needs_more_and_overflow                                [PASSED]
test/test_thread/test_thread.cpp:210: test_spinel_command_build_parse_round_trip                                [PASSED]
test/test_thread/test_thread.cpp:211: test_spinel_command_through_hdlc                                          [PASSED]
native_thread:test_thread Took 0.79 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_thread  test_thread  PASSED    00:00:00.790
================= 13 test cases: 13 succeeded in 00:00:00.790 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_wamp in native_wamp environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_wamp/test_wamp.cpp:171: test_build_hello                                                              [PASSED]
test/test_wamp/test_wamp.cpp:172: test_build_subscribe_default_options                                          [PASSED]
test/test_wamp/test_wamp.cpp:173: test_build_publish_with_args                                                  [PASSED]
test/test_wamp/test_wamp.cpp:174: test_build_publish_kwargs_only                                                [PASSED]
test/test_wamp/test_wamp.cpp:175: test_build_call_and_register_and_yield                                        [PASSED]
test/test_wamp/test_wamp.cpp:176: test_build_unsubscribe_and_goodbye                                            [PASSED]
test/test_wamp/test_wamp.cpp:177: test_build_overflow_fails_closed                                              [PASSED]
test/test_wamp/test_wamp.cpp:178: test_parse_type_and_id                                                        [PASSED]
test/test_wamp/test_wamp.cpp:179: test_parse_event_positions                                                    [PASSED]
test/test_wamp/test_wamp.cpp:180: test_parse_get_uri_and_nesting                                                [PASSED]
test/test_wamp/test_wamp.cpp:181: test_parse_malformed                                                          [PASSED]
test/test_wamp/test_wamp.cpp:182: test_get_uri_dest_bounds                                                      [PASSED]
native_wamp:test_wamp Took 0.84 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_wamp    test_wamp  PASSED    00:00:00.843
================= 12 test cases: 12 succeeded in 00:00:00.843 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_sunspec in native_sunspec environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_sunspec/test_sunspec.cpp:130: test_build_and_walk                                                     [PASSED]
test/test_sunspec/test_sunspec.cpp:131: test_two_models                                                         [PASSED]
test/test_sunspec/test_sunspec.cpp:132: test_string_point                                                       [PASSED]
test/test_sunspec/test_sunspec.cpp:133: test_marker_and_truncation                                              [PASSED]
test/test_sunspec/test_sunspec.cpp:134: test_writer_overflow_fails_closed                                       [PASSED]
native_sunspec:test_sunspec Took 0.78 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_sunspec  test_sunspec  PASSED    00:00:00.778
================== 5 test cases: 5 succeeded in 00:00:00.778 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_c37118 in native_c37118 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_c37118/test_c37118.cpp:117: test_crc_check_value                                                      [PASSED]
test/test_c37118/test_c37118.cpp:118: test_build_command_bytes                                                  [PASSED]
test/test_c37118/test_c37118.cpp:119: test_command_round_trip                                                   [PASSED]
test/test_c37118/test_c37118.cpp:120: test_data_frame_payload                                                   [PASSED]
test/test_c37118/test_c37118.cpp:121: test_parse_rejects_bad                                                    [PASSED]
test/test_c37118/test_c37118.cpp:122: test_build_overflow_fails_closed                                          [PASSED]
native_c37118:test_c37118 Took 0.76 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_c37118  test_c37118  PASSED    00:00:00.759
================== 6 test cases: 6 succeeded in 00:00:00.759 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_dnp3 in native_dnp3 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_dnp3/test_dnp3.cpp:157: test_dnp3_parse_guards                                                        [PASSED]
test/test_dnp3/test_dnp3.cpp:158: test_crc_check_value                                                          [PASSED]
test/test_dnp3/test_dnp3.cpp:159: test_build_header_bytes                                                       [PASSED]
test/test_dnp3/test_dnp3.cpp:160: test_round_trip_single_block                                                  [PASSED]
test/test_dnp3/test_dnp3.cpp:161: test_round_trip_multi_block                                                   [PASSED]
test/test_dnp3/test_dnp3.cpp:162: test_header_only_frame                                                        [PASSED]
test/test_dnp3/test_dnp3.cpp:163: test_parse_rejects_bad                                                        [PASSED]
test/test_dnp3/test_dnp3.cpp:164: test_build_overflow_fails_closed                                              [PASSED]
native_dnp3:test_dnp3 Took 0.77 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_dnp3    test_dnp3  PASSED    00:00:00.774
================== 8 test cases: 8 succeeded in 00:00:00.774 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_grpcweb in native_grpcweb environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_grpcweb/test_grpcweb.cpp:116: test_frame_message_bytes                                                [PASSED]
test/test_grpcweb/test_grpcweb.cpp:117: test_compressed_flag                                                    [PASSED]
test/test_grpcweb/test_grpcweb.cpp:118: test_trailer_frame                                                      [PASSED]
test/test_grpcweb/test_grpcweb.cpp:119: test_trailer_status_only                                                [PASSED]
test/test_grpcweb/test_grpcweb.cpp:120: test_parse_stream                                                       [PASSED]
test/test_grpcweb/test_grpcweb.cpp:121: test_parse_incomplete                                                   [PASSED]
test/test_grpcweb/test_grpcweb.cpp:122: test_frame_overflow_fails_closed                                        [PASSED]
native_grpcweb:test_grpcweb Took 0.76 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_grpcweb  test_grpcweb  PASSED    00:00:00.760
================== 7 test cases: 7 succeeded in 00:00:00.760 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_lwm2m_tlv in native_lwm2m_tlv environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:266: test_write_int_1byte                                                [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:267: test_write_int_2byte                                                [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:268: test_write_string_8bit_length                                       [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:269: test_write_16bit_id                                                 [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:270: test_round_trip_and_value_int                                       [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:271: test_object_instance_nested                                         [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:272: test_write_16bit_length                                             [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:273: test_read_24bit_length                                              [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:274: test_value_int_4_and_8_byte                                         [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:275: test_zero_length_value                                              [PASSED]
test/test_lwm2m_tlv/test_lwm2m_tlv.cpp:276: test_overflow_and_malformed                                         [PASSED]
native_lwm2m_tlv:test_lwm2m_tlv Took 0.78 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_lwm2m_tlv  test_lwm2m_tlv  PASSED    00:00:00.777
================= 11 test cases: 11 succeeded in 00:00:00.777 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_fins in native_fins environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_fins/test_fins.cpp:135: test_build_command_bytes                                                      [PASSED]
test/test_fins/test_fins.cpp:136: test_memory_area_read                                                         [PASSED]
test/test_fins/test_fins.cpp:137: test_parse_command                                                            [PASSED]
test/test_fins/test_fins.cpp:138: test_parse_response_ok                                                        [PASSED]
test/test_fins/test_fins.cpp:139: test_parse_response_error                                                     [PASSED]
test/test_fins/test_fins.cpp:140: test_overflow_and_truncation                                                  [PASSED]
native_fins:test_fins Took 0.77 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_fins    test_fins  PASSED    00:00:00.774
================== 6 test cases: 6 succeeded in 00:00:00.774 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_hostlink in native_hostlink environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_hostlink/test_hostlink.cpp:112: test_fcs_vector                                                       [PASSED]
test/test_hostlink/test_hostlink.cpp:113: test_build_dm_read                                                    [PASSED]
test/test_hostlink/test_hostlink.cpp:114: test_build_node_digits                                                [PASSED]
test/test_hostlink/test_hostlink.cpp:115: test_round_trip                                                       [PASSED]
test/test_hostlink/test_hostlink.cpp:116: test_parse_response_end_code                                          [PASSED]
test/test_hostlink/test_hostlink.cpp:117: test_parse_rejects_bad                                                [PASSED]
test/test_hostlink/test_hostlink.cpp:118: test_build_overflow_fails_closed                                      [PASSED]
native_hostlink:test_hostlink Took 0.76 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_hostlink  test_hostlink  PASSED    00:00:00.757
================== 7 test cases: 7 succeeded in 00:00:00.757 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_senml in native_senml environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_senml/test_senml.cpp:239: test_json_canonical                                                         [PASSED]
test/test_senml/test_senml.cpp:240: test_json_base_time_and_none                                                [PASSED]
test/test_senml/test_senml.cpp:241: test_cbor_all_kinds                                                         [PASSED]
test/test_senml/test_senml.cpp:242: test_senml_null_args                                                        [PASSED]
test/test_senml/test_senml.cpp:243: test_json_multi_record                                                      [PASSED]
test/test_senml/test_senml.cpp:244: test_json_string_bool_time                                                  [PASSED]
test/test_senml/test_senml.cpp:245: test_cbor_round_trip                                                        [PASSED]
test/test_senml/test_senml.cpp:246: test_cbor_base_name_key                                                     [PASSED]
test/test_senml/test_senml.cpp:247: test_overflow_fails_closed                                                  [PASSED]
native_senml:test_senml Took 0.86 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_senml   test_senml  PASSED    00:00:00.863
================== 9 test cases: 9 succeeded in 00:00:00.863 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_df1 in native_df1 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_df1/test_df1.cpp:131: test_bcc_vector                                                                 [PASSED]
test/test_df1/test_df1.cpp:132: test_crc_vector                                                                 [PASSED]
test/test_df1/test_df1.cpp:133: test_build_bcc_frame                                                            [PASSED]
test/test_df1/test_df1.cpp:134: test_build_dle_stuffing                                                         [PASSED]
test/test_df1/test_df1.cpp:135: test_round_trip_bcc                                                             [PASSED]
test/test_df1/test_df1.cpp:136: test_round_trip_crc                                                             [PASSED]
test/test_df1/test_df1.cpp:137: test_empty_data_frame                                                           [PASSED]
test/test_df1/test_df1.cpp:138: test_parse_rejects_bad                                                          [PASSED]
test/test_df1/test_df1.cpp:139: test_build_overflow_fails_closed                                                [PASSED]
native_df1:test_df1 Took 0.76 seconds ------------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_df1     test_df1  PASSED    00:00:00.763
================== 9 test cases: 9 succeeded in 00:00:00.763 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_cotp in native_cotp environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_cotp/test_cotp.cpp:122: test_tpkt_bytes                                                               [PASSED]
test/test_cotp/test_cotp.cpp:123: test_cotp_dt_bytes                                                            [PASSED]
test/test_cotp/test_cotp.cpp:124: test_cotp_cr_bytes                                                            [PASSED]
test/test_cotp/test_cotp.cpp:125: test_cotp_cr_with_tsaps                                                       [PASSED]
test/test_cotp/test_cotp.cpp:126: test_full_stack                                                               [PASSED]
test/test_cotp/test_cotp.cpp:127: test_parse_rejects_bad                                                        [PASSED]
native_cotp:test_cotp Took 0.78 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_cotp    test_cotp  PASSED    00:00:00.779
================== 6 test cases: 6 succeeded in 00:00:00.779 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_s7comm in native_s7comm environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_s7comm/test_s7comm.cpp:158: test_build_setup                                                          [PASSED]
test/test_s7comm/test_s7comm.cpp:159: test_build_read_request                                                   [PASSED]
test/test_s7comm/test_s7comm.cpp:160: test_read_request_bit_address                                             [PASSED]
test/test_s7comm/test_s7comm.cpp:161: test_parse_response_single                                                [PASSED]
test/test_s7comm/test_s7comm.cpp:162: test_parse_response_padding                                               [PASSED]
test/test_s7comm/test_s7comm.cpp:163: test_parse_octet_and_error                                                [PASSED]
test/test_s7comm/test_s7comm.cpp:164: test_parse_rejects_bad                                                    [PASSED]
test/test_s7comm/test_s7comm.cpp:165: test_build_overflow_fails_closed                                          [PASSED]
native_s7comm:test_s7comm Took 0.77 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_s7comm  test_s7comm  PASSED    00:00:00.768
================== 8 test cases: 8 succeeded in 00:00:00.768 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_melsec in native_melsec environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_melsec/test_melsec.cpp:101: test_build_read_bytes                                                     [PASSED]
test/test_melsec/test_melsec.cpp:102: test_head_device_24bit                                                    [PASSED]
test/test_melsec/test_melsec.cpp:103: test_parse_response_ok                                                    [PASSED]
test/test_melsec/test_melsec.cpp:104: test_parse_response_error                                                 [PASSED]
test/test_melsec/test_melsec.cpp:105: test_parse_rejects_bad                                                    [PASSED]
test/test_melsec/test_melsec.cpp:106: test_build_overflow_fails_closed                                          [PASSED]
native_melsec:test_melsec Took 0.77 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_melsec  test_melsec  PASSED    00:00:00.766
================== 6 test cases: 6 succeeded in 00:00:00.766 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_bacnet in native_bacnet environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_bacnet/test_bacnet.cpp:180: test_bacnet_guards_and_truncations                                        [PASSED]
test/test_bacnet/test_bacnet.cpp:181: test_bvlc_bytes                                                           [PASSED]
test/test_bacnet/test_bacnet.cpp:182: test_npdu_local                                                           [PASSED]
test/test_bacnet/test_bacnet.cpp:183: test_npdu_dest                                                            [PASSED]
test/test_bacnet/test_bacnet.cpp:184: test_npdu_broadcast                                                       [PASSED]
test/test_bacnet/test_bacnet.cpp:185: test_npdu_parse_with_source                                               [PASSED]
test/test_bacnet/test_bacnet.cpp:186: test_full_stack                                                           [PASSED]
test/test_bacnet/test_bacnet.cpp:187: test_parse_rejects_bad                                                    [PASSED]
test/test_bacnet/test_bacnet.cpp:188: test_overflow_fails_closed                                                [PASSED]
native_bacnet:test_bacnet Took 0.79 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_bacnet  test_bacnet  PASSED    00:00:00.789
================== 9 test cases: 9 succeeded in 00:00:00.789 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_enip in native_enip environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_enip/test_enip.cpp:135: test_header_round_trip                                                        [PASSED]
test/test_enip/test_enip.cpp:136: test_register_session                                                         [PASSED]
test/test_enip/test_enip.cpp:137: test_send_rr_data_bytes                                                       [PASSED]
test/test_enip/test_enip.cpp:138: test_send_rr_data_round_trip                                                  [PASSED]
test/test_enip/test_enip.cpp:139: test_parse_rejects_bad                                                        [PASSED]
test/test_enip/test_enip.cpp:140: test_build_overflow_fails_closed                                              [PASSED]
native_enip:test_enip Took 0.77 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_enip    test_enip  PASSED    00:00:00.770
================== 6 test cases: 6 succeeded in 00:00:00.770 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_amqp in native_amqp environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_amqp/test_amqp.cpp:134: test_protocol_header                                                          [PASSED]
test/test_amqp/test_amqp.cpp:135: test_build_method_bytes                                                       [PASSED]
test/test_amqp/test_amqp.cpp:136: test_method_round_trip                                                        [PASSED]
test/test_amqp/test_amqp.cpp:137: test_heartbeat                                                                [PASSED]
test/test_amqp/test_amqp.cpp:138: test_parse_stream                                                             [PASSED]
test/test_amqp/test_amqp.cpp:139: test_parse_rejects_bad                                                        [PASSED]
test/test_amqp/test_amqp.cpp:140: test_build_overflow_fails_closed                                              [PASSED]
native_amqp:test_amqp Took 0.75 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_amqp    test_amqp  PASSED    00:00:00.751
================== 7 test cases: 7 succeeded in 00:00:00.751 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_cip in native_cip environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_cip/test_cip.cpp:128: test_cip_build_guards                                                           [PASSED]
test/test_cip/test_cip.cpp:129: test_epath_8bit                                                                 [PASSED]
test/test_cip/test_cip.cpp:130: test_epath_16bit                                                                [PASSED]
test/test_cip/test_cip.cpp:131: test_get_attr_single                                                            [PASSED]
test/test_cip/test_cip.cpp:132: test_build_request_with_data                                                    [PASSED]
test/test_cip/test_cip.cpp:133: test_parse_response_ok                                                          [PASSED]
test/test_cip/test_cip.cpp:134: test_parse_response_additional_status                                           [PASSED]
test/test_cip/test_cip.cpp:135: test_parse_response_error                                                       [PASSED]
test/test_cip/test_cip.cpp:136: test_rejects_bad                                                                [PASSED]
native_cip:test_cip Took 0.77 seconds ------------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_cip     test_cip  PASSED    00:00:00.770
================== 9 test cases: 9 succeeded in 00:00:00.770 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_nats in native_nats environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_nats/test_nats.cpp:186: test_build_connect                                                            [PASSED]
test/test_nats/test_nats.cpp:187: test_build_ping_pong                                                          [PASSED]
test/test_nats/test_nats.cpp:188: test_build_null_args                                                          [PASSED]
test/test_nats/test_nats.cpp:189: test_build_overflow_put_ch                                                    [PASSED]
test/test_nats/test_nats.cpp:190: test_parse_edges                                                              [PASSED]
test/test_nats/test_nats.cpp:191: test_build_pub                                                                [PASSED]
test/test_nats/test_nats.cpp:192: test_build_pub_with_reply                                                     [PASSED]
test/test_nats/test_nats.cpp:193: test_build_pub_empty_payload                                                  [PASSED]
test/test_nats/test_nats.cpp:194: test_build_sub_and_unsub                                                      [PASSED]
test/test_nats/test_nats.cpp:195: test_parse_msg                                                                [PASSED]
test/test_nats/test_nats.cpp:196: test_parse_msg_with_reply                                                     [PASSED]
test/test_nats/test_nats.cpp:197: test_parse_control_lines                                                      [PASSED]
test/test_nats/test_nats.cpp:198: test_parse_incomplete                                                         [PASSED]
test/test_nats/test_nats.cpp:199: test_build_overflow_fails_closed                                              [PASSED]
native_nats:test_nats Took 0.77 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_nats    test_nats  PASSED    00:00:00.767
================= 14 test cases: 14 succeeded in 00:00:00.767 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_proxy_protocol in native_proxy_protocol environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_proxy_protocol/test_proxy_protocol.cpp:120: test_v1_build                                             [PASSED]
test/test_proxy_protocol/test_proxy_protocol.cpp:121: test_v1_round_trip                                        [PASSED]
test/test_proxy_protocol/test_proxy_protocol.cpp:122: test_v2_build_bytes                                       [PASSED]
test/test_proxy_protocol/test_proxy_protocol.cpp:123: test_v2_round_trip                                        [PASSED]
test/test_proxy_protocol/test_proxy_protocol.cpp:124: test_v1_unknown                                           [PASSED]
test/test_proxy_protocol/test_proxy_protocol.cpp:125: test_not_a_proxy_header                                   [PASSED]
test/test_proxy_protocol/test_proxy_protocol.cpp:126: test_incomplete                                           [PASSED]
test/test_proxy_protocol/test_proxy_protocol.cpp:127: test_build_overflow_fails_closed                          [PASSED]
native_proxy_protocol:test_proxy_protocol Took 0.79 seconds --------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment            Test                 Status    Duration
---------------------  -------------------  --------  ------------
native_proxy_protocol  test_proxy_protocol  PASSED    00:00:00.788
================== 8 test cases: 8 succeeded in 00:00:00.788 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_sparkplug in native_sparkplug environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_sparkplug/test_sparkplug.cpp:201: test_spb_error_and_kind_paths                                       [PASSED]
test/test_sparkplug/test_sparkplug.cpp:202: test_topic                                                          [PASSED]
test/test_sparkplug/test_sparkplug.cpp:203: test_metric_bytes                                                   [PASSED]
test/test_sparkplug/test_sparkplug.cpp:204: test_payload_round_trip                                             [PASSED]
test/test_sparkplug/test_sparkplug.cpp:205: test_metric_int_and_string                                          [PASSED]
test/test_sparkplug/test_sparkplug.cpp:206: test_metric_alias                                                   [PASSED]
test/test_sparkplug/test_sparkplug.cpp:207: test_overflow_fails_closed                                          [PASSED]
native_sparkplug:test_sparkplug Took 0.80 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_sparkplug  test_sparkplug  PASSED    00:00:00.798
================== 7 test cases: 7 succeeded in 00:00:00.798 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_modbus_master in native_modbus_master environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_modbus_master/test_modbus_master.cpp:85: test_build_read_bytes                                        [PASSED]
test/test_modbus_master/test_modbus_master.cpp:86: test_build_rejects_bad_args                                  [PASSED]
test/test_modbus_master/test_modbus_master.cpp:87: test_round_trip_holding_regs                                 [PASSED]
test/test_modbus_master/test_modbus_master.cpp:88: test_round_trip_exception                                    [PASSED]
test/test_modbus_master/test_modbus_master.cpp:89: test_parse_short_frame_fails                                 [PASSED]
native_modbus_master:test_modbus_master Took 0.82 seconds ----------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment           Test                Status    Duration
--------------------  ------------------  --------  ------------
native_modbus_master  test_modbus_master  PASSED    00:00:00.817
================== 5 test cases: 5 succeeded in 00:00:00.817 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ota_rollback in native_ota_rollback environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ota_rollback/test_ota_rollback.cpp:49: test_not_pending_waits                                         [PASSED]
test/test_ota_rollback/test_ota_rollback.cpp:50: test_pending_self_test_ok_commits                              [PASSED]
test/test_ota_rollback/test_ota_rollback.cpp:51: test_pending_within_window_waits                               [PASSED]
test/test_ota_rollback/test_ota_rollback.cpp:52: test_pending_window_elapsed_rolls_back                         [PASSED]
test/test_ota_rollback/test_ota_rollback.cpp:53: test_self_test_ok_beats_window                                 [PASSED]
native_ota_rollback:test_ota_rollback Took 0.75 seconds ------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_ota_rollback  test_ota_rollback  PASSED    00:00:00.750
================== 5 test cases: 5 succeeded in 00:00:00.750 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_totp in native_totp environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_totp/test_totp.cpp:68: test_rfc6238_vectors                                                           [PASSED]
test/test_totp/test_totp.cpp:69: test_verify_window                                                             [PASSED]
test/test_totp/test_totp.cpp:70: test_base32_decode                                                             [PASSED]
test/test_totp/test_totp.cpp:71: test_base32_rejects_invalid                                                    [PASSED]
native_totp:test_totp Took 0.80 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_totp    test_totp  PASSED    00:00:00.798
================== 4 test cases: 4 succeeded in 00:00:00.798 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_webhook in native_webhook environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_webhook/test_webhook.cpp:65: test_ifttt_url                                                           [PASSED]
test/test_webhook/test_webhook.cpp:66: test_payload_three_values                                                [PASSED]
test/test_webhook/test_webhook.cpp:67: test_payload_omits_nulls                                                 [PASSED]
test/test_webhook/test_webhook.cpp:68: test_payload_escapes_json                                                [PASSED]
test/test_webhook/test_webhook.cpp:69: test_overflow_fails_closed                                               [PASSED]
native_webhook:test_webhook Took 0.79 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_webhook  test_webhook  PASSED    00:00:00.788
================== 5 test cases: 5 succeeded in 00:00:00.788 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_radio_power in native_radio_power environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_radio_power/test_radio_power.cpp:34: test_ps_names                                                    [PASSED]
test/test_radio_power/test_radio_power.cpp:35: test_apply_is_noop_on_host                                       [PASSED]
native_radio_power:test_radio_power Took 0.75 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_radio_power  test_radio_power  PASSED    00:00:00.748
================== 2 test cases: 2 succeeded in 00:00:00.748 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_dns_resolver in native_dns_resolver environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_dns_resolver/test_dns_resolver.cpp:59: test_classify                                                  [PASSED]
test/test_dns_resolver/test_dns_resolver.cpp:60: test_verify_rejects_suspicious                                 [PASSED]
test/test_dns_resolver/test_dns_resolver.cpp:61: test_verify_accepts_plausible                                  [PASSED]
test/test_dns_resolver/test_dns_resolver.cpp:62: test_resolve_is_noop_on_host                                   [PASSED]
native_dns_resolver:test_dns_resolver Took 0.79 seconds ------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_dns_resolver  test_dns_resolver  PASSED    00:00:00.786
================== 4 test cases: 4 succeeded in 00:00:00.786 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_audit_log in native_audit_log environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_audit_log/test_audit_log.cpp:246: test_append_assigns_monotonic_seq                                   [PASSED]
test/test_audit_log/test_audit_log.cpp:247: test_chain_verifies_when_untouched                                  [PASSED]
test/test_audit_log/test_audit_log.cpp:248: test_tampered_message_breaks_chain                                  [PASSED]
test/test_audit_log/test_audit_log.cpp:249: test_tampered_hash_breaks_chain                                     [PASSED]
test/test_audit_log/test_audit_log.cpp:250: test_tampered_category_breaks_chain                                 [PASSED]
test/test_audit_log/test_audit_log.cpp:251: test_ring_evicts_oldest_and_still_verifies                          [PASSED]
test/test_audit_log/test_audit_log.cpp:252: test_tamper_after_wrap_detected_at_oldest                           [PASSED]
test/test_audit_log/test_audit_log.cpp:253: test_reset_clears_everything                                        [PASSED]
test/test_audit_log/test_audit_log.cpp:254: test_sink_receives_each_record                                      [PASSED]
test/test_audit_log/test_audit_log.cpp:255: test_format_and_dump_json                                           [PASSED]
test/test_audit_log/test_audit_log.cpp:256: test_dump_json_reports_broken_chain                                 [PASSED]
test/test_audit_log/test_audit_log.cpp:257: test_format_fails_closed_on_small_buffer                            [PASSED]
test/test_audit_log/test_audit_log.cpp:258: test_null_msg_and_categories                                        [PASSED]
test/test_audit_log/test_audit_log.cpp:259: test_json_escape_all_chars                                          [PASSED]
test/test_audit_log/test_audit_log.cpp:260: test_format_fails_closed_all_stages                                 [PASSED]
test/test_audit_log/test_audit_log.cpp:261: test_dump_fails_closed_all_stages                                   [PASSED]
native_audit_log:test_audit_log Took 0.85 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_audit_log  test_audit_log  PASSED    00:00:00.852
================= 16 test cases: 16 succeeded in 00:00:00.852 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_oidc in native_oidc environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_oidc/test_oidc.cpp:252: test_jwks_malformed_keys                                                      [PASSED]
test/test_oidc/test_oidc.cpp:253: test_token_kid_guards                                                         [PASSED]
test/test_oidc/test_oidc.cpp:254: test_jwks_find_guards                                                         [PASSED]
test/test_oidc/test_oidc.cpp:255: test_verify_guards_and_malformed                                              [PASSED]
test/test_oidc/test_oidc.cpp:256: test_token_kid                                                                [PASSED]
test/test_oidc/test_oidc.cpp:257: test_jwks_find                                                                [PASSED]
test/test_oidc/test_oidc.cpp:258: test_jwks_find_missing_kid_fails                                              [PASSED]
test/test_oidc/test_oidc.cpp:259: test_verify_valid_token_and_claims                                            [PASSED]
test/test_oidc/test_oidc.cpp:260: test_verify_aud_array                                                         [PASSED]
test/test_oidc/test_oidc.cpp:261: test_reject_expired                                                           [PASSED]
test/test_oidc/test_oidc.cpp:262: test_reject_wrong_issuer                                                      [PASSED]
test/test_oidc/test_oidc.cpp:263: test_reject_wrong_audience                                                    [PASSED]
test/test_oidc/test_oidc.cpp:264: test_reject_non_rs256_header                                                  [PASSED]
test/test_oidc/test_oidc.cpp:265: test_reject_tampered_payload                                                  [PASSED]
test/test_oidc/test_oidc.cpp:266: test_reject_tampered_signature                                                [PASSED]
test/test_oidc/test_oidc.cpp:267: test_reject_unknown_key                                                       [PASSED]
test/test_oidc/test_oidc.cpp:268: test_reject_malformed                                                         [PASSED]
native_oidc:test_oidc Took 1.10 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_oidc    test_oidc  PASSED    00:00:01.105
================= 17 test cases: 17 succeeded in 00:00:01.105 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_vfs in native_vfs environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_vfs/test_vfs.cpp:174: test_write_then_read_file                                                       [PASSED]
test/test_vfs/test_vfs.cpp:175: test_streamed_write_and_read                                                    [PASSED]
test/test_vfs/test_vfs.cpp:176: test_write_mode_truncates                                                       [PASSED]
test/test_vfs/test_vfs.cpp:177: test_append_extends                                                             [PASSED]
test/test_vfs/test_vfs.cpp:178: test_remove_and_rename                                                          [PASSED]
test/test_vfs/test_vfs.cpp:179: test_missing_file_fails_closed                                                  [PASSED]
test/test_vfs/test_vfs.cpp:180: test_read_buffer_too_small_fails_closed                                         [PASSED]
test/test_vfs/test_vfs.cpp:181: test_file_full_is_bounded                                                       [PASSED]
test/test_vfs/test_vfs.cpp:182: test_file_pool_exhaustion                                                       [PASSED]
test/test_vfs/test_vfs.cpp:183: test_handle_pool_exhaustion                                                     [PASSED]
test/test_vfs/test_vfs.cpp:184: test_unmounted_fails_closed                                                     [PASSED]
native_vfs:test_vfs Took 0.80 seconds ------------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_vfs     test_vfs  PASSED    00:00:00.796
================= 11 test cases: 11 succeeded in 00:00:00.796 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_graphql in native_graphql environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_graphql/test_graphql.cpp:396: test_malformed_tokens_fail                                              [PASSED]
test/test_graphql/test_graphql.cpp:397: test_query_keyword_forms_fail                                           [PASSED]
test/test_graphql/test_graphql.cpp:398: test_pool_limits                                                        [PASSED]
test/test_graphql/test_graphql.cpp:399: test_string_pool_exhaustion                                             [PASSED]
test/test_graphql/test_graphql.cpp:400: test_resolver_null_typed_value                                          [PASSED]
test/test_graphql/test_graphql.cpp:401: test_resolver_path_overflow                                             [PASSED]
test/test_graphql/test_graphql.cpp:402: test_arg_accessors_edges                                                [PASSED]
test/test_graphql/test_graphql.cpp:403: test_flat_selection                                                     [PASSED]
test/test_graphql/test_graphql.cpp:404: test_string_escapes_decoded                                             [PASSED]
test/test_graphql/test_graphql.cpp:405: test_number_arg_variants_parse                                          [PASSED]
test/test_graphql/test_graphql.cpp:406: test_bool_args                                                          [PASSED]
test/test_graphql/test_graphql.cpp:407: test_null_arg_value                                                     [PASSED]
test/test_graphql/test_graphql.cpp:408: test_control_char_is_unicode_escaped                                    [PASSED]
test/test_graphql/test_graphql.cpp:409: test_unterminated_string_arg_fails                                      [PASSED]
test/test_graphql/test_graphql.cpp:410: test_arg_missing_colon_fails                                            [PASSED]
test/test_graphql/test_graphql.cpp:411: test_bad_arg_value_fails                                                [PASSED]
test/test_graphql/test_graphql.cpp:412: test_trailing_junk_fails                                                [PASSED]
test/test_graphql/test_graphql.cpp:413: test_long_field_name_hits_limit                                         [PASSED]
test/test_graphql/test_graphql.cpp:414: test_null_inputs_fail_closed                                            [PASSED]
test/test_graphql/test_graphql.cpp:415: test_unknown_operation_keyword_fails                                    [PASSED]
test/test_graphql/test_graphql.cpp:416: test_selection_is_honored                                               [PASSED]
test/test_graphql/test_graphql.cpp:417: test_nested_object                                                      [PASSED]
test/test_graphql/test_graphql.cpp:418: test_args_collected_along_path                                          [PASSED]
test/test_graphql/test_graphql.cpp:419: test_scalar_types                                                       [PASSED]
test/test_graphql/test_graphql.cpp:420: test_string_arg_and_escaping                                            [PASSED]
test/test_graphql/test_graphql.cpp:421: test_unresolved_field_is_null                                           [PASSED]
test/test_graphql/test_graphql.cpp:422: test_query_keyword_and_name                                             [PASSED]
test/test_graphql/test_graphql.cpp:423: test_comments_and_commas                                                [PASSED]
test/test_graphql/test_graphql.cpp:424: test_parse_error_reports_errors                                         [PASSED]
test/test_graphql/test_graphql.cpp:425: test_mutation_rejected                                                  [PASSED]
test/test_graphql/test_graphql.cpp:426: test_depth_limit                                                        [PASSED]
test/test_graphql/test_graphql.cpp:427: test_overflow_fails_closed                                              [PASSED]
native_graphql:test_graphql Took 0.83 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_graphql  test_graphql  PASSED    00:00:00.826
================= 32 test cases: 32 succeeded in 00:00:00.826 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_espnow in native_espnow environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_espnow/test_espnow.cpp:121: test_encode_decode_roundtrip                                              [PASSED]
test/test_espnow/test_espnow.cpp:122: test_encode_zero_length                                                   [PASSED]
test/test_espnow/test_espnow.cpp:123: test_encode_rejects_oversize_and_small_buffer                             [PASSED]
test/test_espnow/test_espnow.cpp:124: test_decode_rejects_corrupt                                               [PASSED]
test/test_espnow/test_espnow.cpp:125: test_peer_registry                                                        [PASSED]
test/test_espnow/test_espnow.cpp:126: test_peer_table_full_fails_closed                                         [PASSED]
test/test_espnow/test_espnow.cpp:127: test_broadcast_address                                                    [PASSED]
native_espnow:test_espnow Took 0.82 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_espnow  test_espnow  PASSED    00:00:00.816
================== 7 test cases: 7 succeeded in 00:00:00.816 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_oauth2 in native_oauth2 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_oauth2/test_oauth2.cpp:99: test_build_code_request_minimal                                            [PASSED]
test/test_oauth2/test_oauth2.cpp:100: test_build_code_request_with_secret_encodes_specials                      [PASSED]
test/test_oauth2/test_oauth2.cpp:101: test_build_code_request_pkce                                              [PASSED]
test/test_oauth2/test_oauth2.cpp:102: test_build_refresh_request                                                [PASSED]
test/test_oauth2/test_oauth2.cpp:103: test_build_overflows_fail_closed                                          [PASSED]
test/test_oauth2/test_oauth2.cpp:104: test_parse_token_response                                                 [PASSED]
test/test_oauth2/test_oauth2.cpp:105: test_parse_minimal_response                                               [PASSED]
test/test_oauth2/test_oauth2.cpp:106: test_parse_error_response_fails                                           [PASSED]
native_oauth2:test_oauth2 Took 0.85 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_oauth2  test_oauth2  PASSED    00:00:00.852
================== 8 test cases: 8 succeeded in 00:00:00.852 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_opcua in native_opcua environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_opcua/test_opcua.cpp:1365: test_parse_read_optional_fields                                            [PASSED]
test/test_opcua/test_opcua.cpp:1366: test_parse_rejections                                                      [PASSED]
test/test_opcua/test_opcua.cpp:1367: test_build_guards_and_overflow                                             [PASSED]
test/test_opcua/test_opcua.cpp:1368: test_setters_and_endpoint_url                                              [PASSED]
test/test_opcua/test_opcua.cpp:1369: test_variant_scalar_types                                                  [PASSED]
test/test_opcua/test_opcua.cpp:1370: test_variant_errors                                                        [PASSED]
test/test_opcua/test_opcua.cpp:1371: test_datavalue_all_masks                                                   [PASSED]
test/test_opcua/test_opcua.cpp:1372: test_nodeid_encodings                                                      [PASSED]
test/test_opcua/test_opcua.cpp:1373: test_reader_underruns                                                      [PASSED]
test/test_opcua/test_opcua.cpp:1374: test_codec_roundtrip                                                       [PASSED]
test/test_opcua/test_opcua.cpp:1375: test_string_null_roundtrip                                                 [PASSED]
test/test_opcua/test_opcua.cpp:1376: test_reader_underrun_latches                                               [PASSED]
test/test_opcua/test_opcua.cpp:1377: test_writer_overflow_fails_closed                                          [PASSED]
test/test_opcua/test_opcua.cpp:1378: test_parse_header                                                          [PASSED]
test/test_opcua/test_opcua.cpp:1379: test_parse_hello                                                           [PASSED]
test/test_opcua/test_opcua.cpp:1380: test_parse_hello_rejects_short                                             [PASSED]
test/test_opcua/test_opcua.cpp:1381: test_build_ack_negotiates                                                  [PASSED]
test/test_opcua/test_opcua.cpp:1382: test_nodeid_roundtrip                                                      [PASSED]
test/test_opcua/test_opcua.cpp:1383: test_filetime_from_unix                                                    [PASSED]
test/test_opcua/test_opcua.cpp:1384: test_parse_open                                                            [PASSED]
test/test_opcua/test_opcua.cpp:1385: test_parse_open_rejects_wrong_type                                         [PASSED]
test/test_opcua/test_opcua.cpp:1386: test_build_open_response                                                   [PASSED]
test/test_opcua/test_opcua.cpp:1387: test_parse_msg                                                             [PASSED]
test/test_opcua/test_opcua.cpp:1388: test_parse_msg_rejects_non_msg                                             [PASSED]
test/test_opcua/test_opcua.cpp:1389: test_build_create_session_response                                         [PASSED]
test/test_opcua/test_opcua.cpp:1390: test_build_activate_session_response                                       [PASSED]
test/test_opcua/test_opcua.cpp:1391: test_datavalue_good_int32                                                  [PASSED]
test/test_opcua/test_opcua.cpp:1392: test_datavalue_bad_status                                                  [PASSED]
test/test_opcua/test_opcua.cpp:1393: test_parse_read                                                            [PASSED]
test/test_opcua/test_opcua.cpp:1394: test_build_read_response                                                   [PASSED]
test/test_opcua/test_opcua.cpp:1395: test_parse_browse                                                          [PASSED]
test/test_opcua/test_opcua.cpp:1396: test_build_browse_response                                                 [PASSED]
test/test_opcua/test_opcua.cpp:1397: test_build_browse_response_unknown                                         [PASSED]
test/test_opcua/test_opcua.cpp:1398: test_build_close_session_response                                          [PASSED]
test/test_opcua/test_opcua.cpp:1399: test_build_get_endpoints                                                   [PASSED]
test/test_opcua/test_opcua.cpp:1400: test_build_service_fault                                                   [PASSED]
test/test_opcua/test_opcua.cpp:1401: test_datavalue_roundtrip                                                   [PASSED]
test/test_opcua/test_opcua.cpp:1402: test_parse_and_build_write                                                 [PASSED]
native_opcua:test_opcua Took 0.95 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_opcua   test_opcua  PASSED    00:00:00.954
================= 38 test cases: 38 succeeded in 00:00:00.954 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_opcua_client in native_opcua_client environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_opcua_client/test_opcua_client.cpp:413: test_on_read_all_variant_types                                [PASSED]
test/test_opcua_client/test_opcua_client.cpp:414: test_client_parsers_reject_fault                              [PASSED]
test/test_opcua_client/test_opcua_client.cpp:415: test_client_parsers_reject_malformed                          [PASSED]
test/test_opcua_client/test_opcua_client.cpp:416: test_hello_ack_roundtrip                                      [PASSED]
test/test_opcua_client/test_opcua_client.cpp:417: test_open_roundtrip                                           [PASSED]
test/test_opcua_client/test_opcua_client.cpp:418: test_session_roundtrip                                        [PASSED]
test/test_opcua_client/test_opcua_client.cpp:419: test_get_endpoints_roundtrip                                  [PASSED]
test/test_opcua_client/test_opcua_client.cpp:420: test_service_fault_rejected_by_parsers                        [PASSED]
test/test_opcua_client/test_opcua_client.cpp:421: test_read_roundtrip                                           [PASSED]
test/test_opcua_client/test_opcua_client.cpp:422: test_browse_roundtrip                                         [PASSED]
test/test_opcua_client/test_opcua_client.cpp:423: test_write_roundtrip                                          [PASSED]
test/test_opcua_client/test_opcua_client.cpp:424: test_close_session_roundtrip                                  [PASSED]
test/test_opcua_client/test_opcua_client.cpp:425: test_close_channel_is_clo                                     [PASSED]
test/test_opcua_client/test_opcua_client.cpp:426: test_seq_and_request_id_increment                             [PASSED]
native_opcua_client:test_opcua_client Took 0.87 seconds ------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_opcua_client  test_opcua_client  PASSED    00:00:00.870
================= 14 test cases: 14 succeeded in 00:00:00.870 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_keepalive in native_keepalive environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_keepalive/test_keepalive.cpp:194: test_http11_default_keeps_alive                                     [PASSED]
test/test_keepalive/test_keepalive.cpp:195: test_http11_explicit_close                                          [PASSED]
test/test_keepalive/test_keepalive.cpp:196: test_http10_default_closes                                          [PASSED]
test/test_keepalive/test_keepalive.cpp:197: test_http10_explicit_keepalive                                      [PASSED]
test/test_keepalive/test_keepalive.cpp:198: test_connection_token_list_close                                    [PASSED]
test/test_keepalive/test_keepalive.cpp:199: test_two_sequential_requests_same_slot                              [PASSED]
test/test_keepalive/test_keepalive.cpp:200: test_pipelined_requests                                             [PASSED]
test/test_keepalive/test_keepalive.cpp:201: test_404_still_keeps_alive                                          [PASSED]
test/test_keepalive/test_keepalive.cpp:202: test_max_requests_cap_closes                                        [PASSED]
test/test_keepalive/test_keepalive.cpp:203: test_fresh_connection_resets_count                                  [PASSED]
native_keepalive:test_keepalive Took 1.55 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_keepalive  test_keepalive  PASSED    00:00:01.554
================= 10 test cases: 10 succeeded in 00:00:01.554 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_range in native_range environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_range/test_range.cpp:219: test_no_range_full_200                                                      [PASSED]
test/test_range/test_range.cpp:220: test_range_prefix                                                           [PASSED]
test/test_range/test_range.cpp:221: test_range_open_ended                                                       [PASSED]
test/test_range/test_range.cpp:222: test_range_suffix                                                           [PASSED]
test/test_range/test_range.cpp:223: test_range_single_byte                                                      [PASSED]
test/test_range/test_range.cpp:224: test_range_clamped_to_eof                                                   [PASSED]
test/test_range/test_range.cpp:225: test_range_unsatisfiable_416                                                [PASSED]
test/test_range/test_range.cpp:226: test_malformed_range_ignored                                                [PASSED]
test/test_range/test_range.cpp:227: test_range_overflow_start_unsatisfiable                                     [PASSED]
test/test_range/test_range.cpp:228: test_range_overflow_end_clamps                                              [PASSED]
test/test_range/test_range.cpp:229: test_range_suffix_zero_unsatisfiable                                        [PASSED]
test/test_range/test_range.cpp:230: test_multirange_falls_back_to_200                                           [PASSED]
test/test_range/test_range.cpp:231: test_head_with_range_no_body                                                [PASSED]
native_range:test_range Took 1.58 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_range   test_range  PASSED    00:00:01.585
================= 13 test cases: 13 succeeded in 00:00:01.585 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_syslog in native_syslog environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_syslog/test_syslog.cpp:125: test_pri_local0_info                                                      [PASSED]
test/test_syslog/test_syslog.cpp:126: test_pri_computation_varies                                               [PASSED]
test/test_syslog/test_syslog.cpp:127: test_nilvalue_for_empty_fields                                            [PASSED]
test/test_syslog/test_syslog.cpp:128: test_empty_message_ok                                                     [PASSED]
test/test_syslog/test_syslog.cpp:129: test_overflow_returns_zero                                                [PASSED]
test/test_syslog/test_syslog.cpp:130: test_length_matches_strlen                                                [PASSED]
test/test_syslog/test_syslog.cpp:131: test_init_and_log_captured                                                [PASSED]
test/test_syslog/test_syslog.cpp:132: test_log_not_ready_when_no_server                                         [PASSED]
test/test_syslog/test_syslog.cpp:133: test_format_null_and_pri_clamp                                            [PASSED]
test/test_syslog/test_syslog.cpp:134: test_init_truncates_long_fields                                           [PASSED]
native_syslog:test_syslog Took 0.88 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_syslog  test_syslog  PASSED    00:00:00.877
================= 10 test cases: 10 succeeded in 00:00:00.877 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_smtp in native_smtp environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_smtp/test_smtp.cpp:231: test_happy_path_no_auth                                                       [PASSED]
test/test_smtp/test_smtp.cpp:232: test_auth_login                                                               [PASSED]
test/test_smtp/test_smtp.cpp:233: test_auth_rejected                                                            [PASSED]
test/test_smtp/test_smtp.cpp:234: test_greeting_not_ready                                                       [PASSED]
test/test_smtp/test_smtp.cpp:235: test_rcpt_rejected                                                            [PASSED]
test/test_smtp/test_smtp.cpp:236: test_data_refused                                                             [PASSED]
test/test_smtp/test_smtp.cpp:237: test_dot_stuffing                                                             [PASSED]
test/test_smtp/test_smtp.cpp:238: test_multiline_reply_and_lf_body                                              [PASSED]
test/test_smtp/test_smtp.cpp:239: test_partial_reads_dribble                                                    [PASSED]
test/test_smtp/test_smtp.cpp:240: test_missing_required_arg                                                     [PASSED]
test/test_smtp/test_smtp.cpp:241: test_io_error_when_server_hangs                                               [PASSED]
native_smtp:test_smtp Took 0.99 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_smtp    test_smtp  PASSED    00:00:00.991
================= 11 test cases: 11 succeeded in 00:00:00.991 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ntp_server in native_ntp_server environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ntp_server/test_ntp_server.cpp:141: test_happy_path_fields                                            [PASSED]
test/test_ntp_server/test_ntp_server.cpp:142: test_origin_is_client_transmit                                    [PASSED]
test/test_ntp_server/test_ntp_server.cpp:143: test_version_echo                                                 [PASSED]
test/test_ntp_server/test_ntp_server.cpp:144: test_poll_echo_and_default                                        [PASSED]
test/test_ntp_server/test_ntp_server.cpp:145: test_stratum_passthrough                                          [PASSED]
test/test_ntp_server/test_ntp_server.cpp:146: test_big_endian_encoding                                          [PASSED]
test/test_ntp_server/test_ntp_server.cpp:147: test_length_guards                                                [PASSED]
test/test_ntp_server/test_ntp_server.cpp:148: test_root_dispersion_advertised                                   [PASSED]
native_ntp_server:test_ntp_server Took 0.85 seconds ----------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_ntp_server  test_ntp_server  PASSED    00:00:00.846
================== 8 test cases: 8 succeeded in 00:00:00.846 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_dns_server in native_dns_server environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_dns_server/test_dns_server.cpp:175: test_a_record_answer                                              [PASSED]
test/test_dns_server/test_dns_server.cpp:176: test_nxdomain                                                     [PASSED]
test/test_dns_server/test_dns_server.cpp:177: test_non_a_query_no_error                                         [PASSED]
test/test_dns_server/test_dns_server.cpp:178: test_multilabel_name_reaches_resolver                             [PASSED]
test/test_dns_server/test_dns_server.cpp:179: test_malformed_guards                                             [PASSED]
test/test_dns_server/test_dns_server.cpp:180: test_table_add_lookup_case_insensitive                            [PASSED]
test/test_dns_server/test_dns_server.cpp:181: test_end_to_end_with_table                                        [PASSED]
native_dns_server:test_dns_server Took 0.80 seconds ----------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_dns_server  test_dns_server  PASSED    00:00:00.799
================== 7 test cases: 7 succeeded in 00:00:00.799 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_rtc in native_rtc environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_rtc/test_rtc.cpp:124: test_known_epoch_2000                                                           [PASSED]
test/test_rtc/test_rtc.cpp:125: test_decode_datetime                                                            [PASSED]
test/test_rtc/test_rtc.cpp:126: test_12hour_mode_equivalence                                                    [PASSED]
test/test_rtc/test_rtc.cpp:127: test_12hour_midnight_and_noon                                                   [PASSED]
test/test_rtc/test_rtc.cpp:128: test_roundtrip_over_range                                                       [PASSED]
test/test_rtc/test_rtc.cpp:129: test_leap_day                                                                   [PASSED]
test/test_rtc/test_rtc.cpp:130: test_masks_ch_and_century                                                       [PASSED]
test/test_rtc/test_rtc.cpp:131: test_invalid_guards                                                             [PASSED]
native_rtc:test_rtc Took 0.77 seconds ------------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_rtc     test_rtc  PASSED    00:00:00.769
================== 8 test cases: 8 succeeded in 00:00:00.769 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ld2410 in native_ld2410 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ld2410/test_ld2410.cpp:204: test_parse_basic                                                          [PASSED]
test/test_ld2410/test_ld2410.cpp:205: test_parse_engineering                                                    [PASSED]
test/test_ld2410/test_ld2410.cpp:206: test_reject_malformed                                                     [PASSED]
test/test_ld2410/test_ld2410.cpp:207: test_stream_resync_and_split                                              [PASSED]
test/test_ld2410/test_ld2410.cpp:208: test_stream_absurd_length_drops                                           [PASSED]
test/test_ld2410/test_ld2410.cpp:209: test_helpers                                                              [PASSED]
test/test_ld2410/test_ld2410.cpp:210: test_command_encoders                                                     [PASSED]
native_ld2410:test_ld2410 Took 0.85 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_ld2410  test_ld2410  PASSED    00:00:00.848
================== 7 test cases: 7 succeeded in 00:00:00.848 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_mpr121 in native_mpr121 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_mpr121/test_mpr121.cpp:101: test_touched_decode                                                       [PASSED]
test/test_mpr121/test_mpr121.cpp:102: test_prox_and_overcurrent_masked                                          [PASSED]
test/test_mpr121/test_mpr121.cpp:103: test_word10                                                               [PASSED]
test/test_mpr121/test_mpr121.cpp:104: test_build_init_bytes                                                     [PASSED]
test/test_mpr121/test_mpr121.cpp:105: test_build_init_guards                                                    [PASSED]
native_mpr121:test_mpr121 Took 0.79 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_mpr121  test_mpr121  PASSED    00:00:00.791
================== 5 test cases: 5 succeeded in 00:00:00.791 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_sht3x in native_sht3x environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_sht3x/test_sht3x.cpp:83: test_crc8_datasheet_vector                                                   [PASSED]
test/test_sht3x/test_sht3x.cpp:84: test_conversion                                                              [PASSED]
test/test_sht3x/test_sht3x.cpp:85: test_parse_valid                                                             [PASSED]
test/test_sht3x/test_sht3x.cpp:86: test_parse_bad_crc                                                           [PASSED]
test/test_sht3x/test_sht3x.cpp:87: test_parse_null_out                                                          [PASSED]
native_sht3x:test_sht3x Took 0.80 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_sht3x   test_sht3x  PASSED    00:00:00.796
================== 5 test cases: 5 succeeded in 00:00:00.796 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_pca9685 in native_pca9685 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_pca9685/test_pca9685.cpp:73: test_prescale                                                            [PASSED]
test/test_pca9685/test_pca9685.cpp:74: test_channel_reg                                                         [PASSED]
test/test_pca9685/test_pca9685.cpp:75: test_us_to_count                                                         [PASSED]
test/test_pca9685/test_pca9685.cpp:76: test_set_pwm_bytes                                                       [PASSED]
native_pca9685:test_pca9685 Took 0.77 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_pca9685  test_pca9685  PASSED    00:00:00.771
================== 4 test cases: 4 succeeded in 00:00:00.771 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ads1115 in native_ads1115 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ads1115/test_ads1115.cpp:51: test_config_word                                                         [PASSED]
test/test_ads1115/test_ads1115.cpp:52: test_config_fallbacks                                                    [PASSED]
test/test_ads1115/test_ads1115.cpp:53: test_raw_to_uv                                                           [PASSED]
native_ads1115:test_ads1115 Took 0.76 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_ads1115  test_ads1115  PASSED    00:00:00.763
================== 3 test cases: 3 succeeded in 00:00:00.763 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ina219 in native_ina219 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ina219/test_ina219.cpp:55: test_bus_mv                                                                [PASSED]
test/test_ina219/test_ina219.cpp:56: test_shunt_uv                                                              [PASSED]
test/test_ina219/test_ina219.cpp:57: test_calibration                                                           [PASSED]
test/test_ina219/test_ina219.cpp:58: test_current_and_power                                                     [PASSED]
native_ina219:test_ina219 Took 0.77 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_ina219  test_ina219  PASSED    00:00:00.774
================== 4 test cases: 4 succeeded in 00:00:00.774 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_hpack in native_hpack environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_hpack/test_hpack.cpp:186: test_int_coding                                                             [PASSED]
test/test_hpack/test_hpack.cpp:187: test_huffman                                                                [PASSED]
test/test_hpack/test_hpack.cpp:188: test_decode_c31_and_index                                                   [PASSED]
test/test_hpack/test_hpack.cpp:189: test_dynamic_eviction                                                       [PASSED]
test/test_hpack/test_hpack.cpp:190: test_encode_static                                                          [PASSED]
test/test_hpack/test_hpack.cpp:191: test_encode_decode_roundtrip                                                [PASSED]
test/test_hpack/test_hpack.cpp:192: test_reject_malformed                                                       [PASSED]
native_hpack:test_hpack Took 0.95 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_hpack   test_hpack  PASSED    00:00:00.946
================== 7 test cases: 7 succeeded in 00:00:00.946 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_h2_frame in native_h2frame environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_h2_frame/test_h2_frame.cpp:131: test_header_roundtrip                                                 [PASSED]
test/test_h2_frame/test_h2_frame.cpp:132: test_settings_build_parse                                             [PASSED]
test/test_h2_frame/test_h2_frame.cpp:133: test_settings_validation                                              [PASSED]
test/test_h2_frame/test_h2_frame.cpp:134: test_control_frames                                                   [PASSED]
test/test_h2_frame/test_h2_frame.cpp:135: test_headers_and_data                                                 [PASSED]
test/test_h2_frame/test_h2_frame.cpp:136: test_preface                                                          [PASSED]
native_h2frame:test_h2_frame Took 0.78 seconds ---------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test           Status    Duration
--------------  -------------  --------  ------------
native_h2frame  test_h2_frame  PASSED    00:00:00.780
================== 6 test cases: 6 succeeded in 00:00:00.780 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_h2_conn in native_h2conn environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_h2_conn/test_h2_conn.cpp:260: test_init_and_request                                                   [PASSED]
test/test_h2_conn/test_h2_conn.cpp:261: test_respond_roundtrip                                                  [PASSED]
test/test_h2_conn/test_h2_conn.cpp:262: test_ping_and_split_recv                                                [PASSED]
test/test_h2_conn/test_h2_conn.cpp:263: test_bad_preface                                                        [PASSED]
native_h2conn:test_h2_conn Took 1.11 seconds ------------------------------------------------------------------ [PASSED]

=================================== SUMMARY ===================================
Environment    Test          Status    Duration
-------------  ------------  --------  ------------
native_h2conn  test_h2_conn  PASSED    00:00:01.114
================== 4 test cases: 4 succeeded in 00:00:01.114 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_quic_varint in native_quic_varint environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_quic_varint/test_quic_varint.cpp:82: test_rfc_examples                                                [PASSED]
test/test_quic_varint/test_quic_varint.cpp:83: test_non_minimal_decode                                          [PASSED]
test/test_quic_varint/test_quic_varint.cpp:84: test_boundaries_and_guards                                       [PASSED]
native_quic_varint:test_quic_varint Took 0.83 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_quic_varint  test_quic_varint  PASSED    00:00:00.831
================== 3 test cases: 3 succeeded in 00:00:00.831 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_h3_frame in native_h3frame environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_h3_frame/test_h3_frame.cpp:140: test_header_roundtrip                                                 [PASSED]
test/test_h3_frame/test_h3_frame.cpp:141: test_build_data_and_goaway                                            [PASSED]
test/test_h3_frame/test_h3_frame.cpp:142: test_settings_roundtrip                                               [PASSED]
test/test_h3_frame/test_h3_frame.cpp:143: test_reserved                                                         [PASSED]
test/test_h3_frame/test_h3_frame.cpp:144: test_build_headers                                                    [PASSED]
test/test_h3_frame/test_h3_frame.cpp:145: test_builder_overflow                                                 [PASSED]
test/test_h3_frame/test_h3_frame.cpp:146: test_parse_errors                                                     [PASSED]
native_h3frame:test_h3_frame Took 0.85 seconds ---------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test           Status    Duration
--------------  -------------  --------  ------------
native_h3frame  test_h3_frame  PASSED    00:00:00.853
================== 7 test cases: 7 succeeded in 00:00:00.853 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_jwt in native_jwt environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_jwt/test_jwt.cpp:335: test_base64url_strict_alphabet                                                  [PASSED]
test/test_jwt/test_jwt.cpp:336: test_verify_malformed_headers                                                   [PASSED]
test/test_jwt/test_jwt.cpp:337: test_bearer_extra_spaces                                                        [PASSED]
test/test_jwt/test_jwt.cpp:338: test_claim_int_edges                                                            [PASSED]
test/test_jwt/test_jwt.cpp:339: test_claim_str_edges                                                            [PASSED]
test/test_jwt/test_jwt.cpp:340: test_valid_token_accepts                                                        [PASSED]
test/test_jwt/test_jwt.cpp:341: test_wrong_secret_rejects                                                       [PASSED]
test/test_jwt/test_jwt.cpp:342: test_tampered_payload_rejects                                                   [PASSED]
test/test_jwt/test_jwt.cpp:343: test_tampered_signature_rejects                                                 [PASSED]
test/test_jwt/test_jwt.cpp:344: test_malformed_rejected                                                         [PASSED]
test/test_jwt/test_jwt.cpp:345: test_alg_not_hs256_rejected                                                     [PASSED]
test/test_jwt/test_jwt.cpp:346: test_bearer_header                                                              [PASSED]
test/test_jwt/test_jwt.cpp:347: test_claim_int                                                                  [PASSED]
test/test_jwt/test_jwt.cpp:348: test_claim_missing                                                              [PASSED]
test/test_jwt/test_jwt.cpp:349: test_claim_str                                                                  [PASSED]
test/test_jwt/test_jwt.cpp:350: test_scope_allows                                                               [PASSED]
test/test_jwt/test_jwt.cpp:351: test_time_no_clock_skips_claims                                                 [PASSED]
test/test_jwt/test_jwt.cpp:352: test_time_exp_enforced                                                          [PASSED]
test/test_jwt/test_jwt.cpp:353: test_time_nbf_enforced                                                          [PASSED]
test/test_jwt/test_jwt.cpp:354: test_time_no_claims_valid                                                       [PASSED]
test/test_jwt/test_jwt.cpp:355: test_bearer_valid_at                                                            [PASSED]
native_jwt:test_jwt Took 0.88 seconds ------------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_jwt     test_jwt  PASSED    00:00:00.882
================= 21 test cases: 21 succeeded in 00:00:00.882 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_upload in native_upload environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_upload/test_upload.cpp:111: test_upload_streams_body_to_file                                          [PASSED]
test/test_upload/test_upload.cpp:112: test_small_body_single_chunk                                              [PASSED]
test/test_upload/test_upload.cpp:113: test_empty_body_not_streamed                                              [PASSED]
native_upload:test_upload Took 1.60 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_upload  test_upload  PASSED    00:00:01.601
================== 3 test cases: 3 succeeded in 00:00:01.601 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_http_client in native_http_client environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_http_client/test_http_client.cpp:228: test_url_edge_rejections                                        [PASSED]
test/test_http_client/test_http_client.cpp:229: test_build_edge_rejections                                      [PASSED]
test/test_http_client/test_http_client.cpp:230: test_response_edge_rejections                                   [PASSED]
test/test_http_client/test_http_client.cpp:231: test_host_transport_stubs                                       [PASSED]
test/test_http_client/test_http_client.cpp:232: test_url_http_default                                           [PASSED]
test/test_http_client/test_http_client.cpp:233: test_url_https_port_nopath                                      [PASSED]
test/test_http_client/test_http_client.cpp:234: test_url_bad_scheme                                             [PASSED]
test/test_http_client/test_http_client.cpp:235: test_build_get                                                  [PASSED]
test/test_http_client/test_http_client.cpp:236: test_build_post_with_body_and_port                              [PASSED]
test/test_http_client/test_http_client.cpp:237: test_parse_content_length                                       [PASSED]
test/test_http_client/test_http_client.cpp:238: test_parse_status_404                                           [PASSED]
test/test_http_client/test_http_client.cpp:239: test_parse_chunked                                              [PASSED]
test/test_http_client/test_http_client.cpp:240: test_parse_chunked_oversize_size_clamped                        [PASSED]
test/test_http_client/test_http_client.cpp:241: test_parse_connection_close_body                                [PASSED]
test/test_http_client/test_http_client.cpp:242: test_parse_malformed                                            [PASSED]
native_http_client:test_http_client Took 0.84 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_http_client  test_http_client  PASSED    00:00:00.839
================= 15 test cases: 15 succeeded in 00:00:00.839 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_compliance in native_compliance environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_compliance/test_compliance.cpp:168: test_http11_missing_host_rejected                                 [PASSED]
test/test_compliance/test_compliance.cpp:169: test_http11_with_host_ok                                          [PASSED]
test/test_compliance/test_compliance.cpp:170: test_http10_missing_host_ok                                       [PASSED]
test/test_compliance/test_compliance.cpp:171: test_duplicate_host_rejected                                      [PASSED]
test/test_compliance/test_compliance.cpp:172: test_duplicate_host_rejected_http10                               [PASSED]
test/test_compliance/test_compliance.cpp:173: test_host_beyond_max_headers_still_counted                        [PASSED]
test/test_compliance/test_compliance.cpp:174: test_duplicate_host_with_one_beyond_cap_rejected                  [PASSED]
test/test_compliance/test_compliance.cpp:176: test_content_length_non_digit_rejected                            [PASSED]
test/test_compliance/test_compliance.cpp:177: test_content_length_empty_rejected                                [PASSED]
test/test_compliance/test_compliance.cpp:178: test_content_length_conflicting_duplicate_rejected                [PASSED]
test/test_compliance/test_compliance.cpp:179: test_content_length_matching_duplicate_ok                         [PASSED]
test/test_compliance/test_compliance.cpp:180: test_content_length_valid_body                                    [PASSED]
test/test_compliance/test_compliance.cpp:182: test_transfer_encoding_chunked_rejected                           [PASSED]
test/test_compliance/test_compliance.cpp:183: test_transfer_encoding_with_content_length_rejected               [PASSED]
test/test_compliance/test_compliance.cpp:184: test_transfer_encoding_case_insensitive_rejected                  [PASSED]
native_compliance:test_compliance Took 0.82 seconds ----------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_compliance  test_compliance  PASSED    00:00:00.823
================= 15 test cases: 15 succeeded in 00:00:00.823 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_mqtt in native_mqtt environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_mqtt/test_mqtt.cpp:410: test_build_guards_and_overflow                                                [PASSED]
test/test_mqtt/test_mqtt.cpp:411: test_parse_guards                                                             [PASSED]
test/test_mqtt/test_mqtt.cpp:412: test_host_transport_stubs                                                     [PASSED]
test/test_mqtt/test_mqtt.cpp:413: test_remlen_boundaries                                                        [PASSED]
test/test_mqtt/test_mqtt.cpp:414: test_remlen_too_big                                                           [PASSED]
test/test_mqtt/test_mqtt.cpp:415: test_remlen_decode_incomplete                                                 [PASSED]
test/test_mqtt/test_mqtt.cpp:416: test_remlen_decode_malformed                                                  [PASSED]
test/test_mqtt/test_mqtt.cpp:417: test_connect_minimal                                                          [PASSED]
test/test_mqtt/test_mqtt.cpp:418: test_connect_full                                                             [PASSED]
test/test_mqtt/test_mqtt.cpp:419: test_publish_qos0_roundtrip                                                   [PASSED]
test/test_mqtt/test_mqtt.cpp:420: test_publish_qos1_flags_and_id                                                [PASSED]
test/test_mqtt/test_mqtt.cpp:421: test_publish_topic_overflow_rejected                                          [PASSED]
test/test_mqtt/test_mqtt.cpp:422: test_publish_qos3_rejected                                                    [PASSED]
test/test_mqtt/test_mqtt.cpp:423: test_publish_wildcard_topic_rejected                                          [PASSED]
test/test_mqtt/test_mqtt.cpp:424: test_publish_topic_nul_or_bad_utf8_rejected                                   [PASSED]
test/test_mqtt/test_mqtt.cpp:425: test_subscribe                                                                [PASSED]
test/test_mqtt/test_mqtt.cpp:426: test_unsubscribe                                                              [PASSED]
test/test_mqtt/test_mqtt.cpp:427: test_ack_packets                                                              [PASSED]
test/test_mqtt/test_mqtt.cpp:428: test_connack                                                                  [PASSED]
test/test_mqtt/test_mqtt.cpp:429: test_suback                                                                   [PASSED]
test/test_mqtt/test_mqtt.cpp:430: test_ping_disconnect                                                          [PASSED]
test/test_mqtt/test_mqtt.cpp:431: test_fixed_header_multibyte_remlen                                            [PASSED]
native_mqtt:test_mqtt Took 0.82 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_mqtt    test_mqtt  PASSED    00:00:00.819
================= 22 test cases: 22 succeeded in 00:00:00.819 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ws_client in native_ws_client environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ws_client/test_ws_client.cpp:246: test_accept_for_key_guards                                          [PASSED]
test/test_ws_client/test_ws_client.cpp:247: test_build_handshake_guards                                         [PASSED]
test/test_ws_client/test_ws_client.cpp:248: test_check_response_guards                                          [PASSED]
test/test_ws_client/test_ws_client.cpp:249: test_build_frame_guards_and_64bit                                   [PASSED]
test/test_ws_client/test_ws_client.cpp:250: test_parse_frame_edges                                              [PASSED]
test/test_ws_client/test_ws_client.cpp:251: test_host_transport_stubs                                           [PASSED]
test/test_ws_client/test_ws_client.cpp:252: test_accept_rfc_example                                             [PASSED]
test/test_ws_client/test_ws_client.cpp:253: test_build_handshake                                                [PASSED]
test/test_ws_client/test_ws_client.cpp:254: test_check_response_ok                                              [PASSED]
test/test_ws_client/test_ws_client.cpp:255: test_check_response_bad_accept                                      [PASSED]
test/test_ws_client/test_ws_client.cpp:256: test_check_response_not_101                                         [PASSED]
test/test_ws_client/test_ws_client.cpp:257: test_build_frame_masked                                             [PASSED]
test/test_ws_client/test_ws_client.cpp:258: test_build_frame_extended_len                                       [PASSED]
test/test_ws_client/test_ws_client.cpp:259: test_parse_frame_server_text                                        [PASSED]
test/test_ws_client/test_ws_client.cpp:260: test_parse_frame_incomplete                                         [PASSED]
test/test_ws_client/test_ws_client.cpp:261: test_parse_frame_extended_len                                       [PASSED]
native_ws_client:test_ws_client Took 0.91 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_ws_client  test_ws_client  PASSED    00:00:00.906
================= 16 test cases: 16 succeeded in 00:00:00.906 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_scratch in native_scratch environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_scratch/test_scratch.cpp:160: test_alloc_returns_nonnull_and_advances_used                            [PASSED]
test/test_scratch/test_scratch.cpp:161: test_sequential_allocs_are_distinct_and_ordered                         [PASSED]
test/test_scratch/test_scratch.cpp:162: test_reset_frees_all_and_reuses_base                                    [PASSED]
test/test_scratch/test_scratch.cpp:163: test_alignment_is_honored                                               [PASSED]
test/test_scratch/test_scratch.cpp:164: test_exhaustion_returns_null_without_corrupting_arena                   [PASSED]
test/test_scratch/test_scratch.cpp:165: test_alloc_larger_than_capacity_returns_null                            [PASSED]
test/test_scratch/test_scratch.cpp:166: test_alignment_padding_cannot_overflow_arena                            [PASSED]
test/test_scratch/test_scratch.cpp:167: test_high_water_bounds                                                  [PASSED]
test/test_scratch/test_scratch.cpp:168: test_zero_size_alloc_returns_nonnull_when_space                         [PASSED]
test/test_scratch/test_scratch.cpp:169: test_mark_release_reclaims                                              [PASSED]
test/test_scratch/test_scratch.cpp:170: test_release_allows_reuse_of_same_region                                [PASSED]
test/test_scratch/test_scratch.cpp:171: test_scratch_scope_releases_on_scope_exit                               [PASSED]
test/test_scratch/test_scratch.cpp:172: test_nested_scopes_reclaim_lifo                                         [PASSED]
test/test_scratch/test_scratch.cpp:173: test_sequential_scopes_do_not_accumulate                                [PASSED]
native_scratch:test_scratch Took 0.87 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_scratch  test_scratch  PASSED    00:00:00.870
================= 14 test cases: 14 succeeded in 00:00:00.870 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_snmp_trap in native_snmp_trap environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_snmp_trap/test_snmp_trap.cpp:226: test_trap_v2c_structure                                             [PASSED]
test/test_snmp_trap/test_snmp_trap.cpp:227: test_all_varbind_types                                              [PASSED]
test/test_snmp_trap/test_snmp_trap.cpp:228: test_invalid_varbind_type                                           [PASSED]
test/test_snmp_trap/test_snmp_trap.cpp:229: test_build_v2c_null_args                                            [PASSED]
test/test_snmp_trap/test_snmp_trap.cpp:230: test_host_transport_stubs                                           [PASSED]
test/test_snmp_trap/test_snmp_trap.cpp:231: test_inform_tag                                                     [PASSED]
test/test_snmp_trap/test_snmp_trap.cpp:232: test_buffer_too_small                                               [PASSED]
native_snmp_trap:test_snmp_trap Took 0.86 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_snmp_trap  test_snmp_trap  PASSED    00:00:00.865
================== 7 test cases: 7 succeeded in 00:00:00.865 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_inflate in native_inflate environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_inflate/test_inflate.cpp:169: test_fixed_huffman                                                      [PASSED]
test/test_inflate/test_inflate.cpp:170: test_back_references                                                    [PASSED]
test/test_inflate/test_inflate.cpp:171: test_stored_block                                                       [PASSED]
test/test_inflate/test_inflate.cpp:172: test_dynamic_huffman                                                    [PASSED]
test/test_inflate/test_inflate.cpp:173: test_empty_message                                                      [PASSED]
test/test_inflate/test_inflate.cpp:174: test_permessage_deflate_marker                                          [PASSED]
test/test_inflate/test_inflate.cpp:175: test_permessage_deflate_back_references                                 [PASSED]
test/test_inflate/test_inflate.cpp:176: test_output_overflow_fails_closed                                       [PASSED]
test/test_inflate/test_inflate.cpp:177: test_scratch_too_small_fails_closed                                     [PASSED]
test/test_inflate/test_inflate.cpp:178: test_truncated_input_is_malformed                                       [PASSED]
test/test_inflate/test_inflate.cpp:179: test_reserved_block_type_is_malformed                                   [PASSED]
test/test_inflate/test_inflate.cpp:180: test_corrupt_stored_nlen_is_malformed                                   [PASSED]
native_inflate:test_inflate Took 0.84 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_inflate  test_inflate  PASSED    00:00:00.843
================= 12 test cases: 12 succeeded in 00:00:00.843 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_deflate in native_deflate environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_deflate/test_deflate.cpp:161: test_roundtrip_text                                                     [PASSED]
test/test_deflate/test_deflate.cpp:162: test_roundtrip_empty                                                    [PASSED]
test/test_deflate/test_deflate.cpp:163: test_roundtrip_single_byte                                              [PASSED]
test/test_deflate/test_deflate.cpp:164: test_roundtrip_all_byte_values                                          [PASSED]
test/test_deflate/test_deflate.cpp:165: test_compresses_repetitive                                              [PASSED]
test/test_deflate/test_deflate.cpp:166: test_compresses_json                                                    [PASSED]
test/test_deflate/test_deflate.cpp:167: test_fuzz_roundtrip                                                     [PASSED]
test/test_deflate/test_deflate.cpp:168: test_fuzz_low_entropy_roundtrip                                         [PASSED]
test/test_deflate/test_deflate.cpp:169: test_output_overflow_fails_closed                                       [PASSED]
test/test_deflate/test_deflate.cpp:170: test_scratch_too_small_fails_closed                                     [PASSED]
native_deflate:test_deflate Took 0.87 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_deflate  test_deflate  PASSED    00:00:00.866
================= 10 test cases: 10 succeeded in 00:00:00.866 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ssh_zlib in native_ssh_zlib environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_zlib/test_ssh_zlib.cpp:210: test_session_roundtrip_and_context_takeover                           [PASSED]
test/test_ssh_zlib/test_ssh_zlib.cpp:211: test_empty_payloads                                                   [PASSED]
test/test_ssh_zlib/test_ssh_zlib.cpp:212: test_all_byte_values                                                  [PASSED]
test/test_ssh_zlib/test_ssh_zlib.cpp:213: test_window_slide_long_session                                        [PASSED]
test/test_ssh_zlib/test_ssh_zlib.cpp:214: test_max_input_payload                                                [PASSED]
test/test_ssh_zlib/test_ssh_zlib.cpp:215: test_fuzz_stream_roundtrip                                            [PASSED]
test/test_ssh_zlib/test_ssh_zlib.cpp:216: test_fuzz_low_entropy_stream                                          [PASSED]
test/test_ssh_zlib/test_ssh_zlib.cpp:217: test_oversize_input_rejected                                          [PASSED]
test/test_ssh_zlib/test_ssh_zlib.cpp:218: test_output_overflow_fails_closed                                     [PASSED]
native_ssh_zlib:test_ssh_zlib Took 0.87 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_ssh_zlib  test_ssh_zlib  PASSED    00:00:00.874
================== 9 test cases: 9 succeeded in 00:00:00.874 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ssh_comp in native_ssh_comp environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_comp/test_ssh_comp.cpp:138: test_delayed_activation                                               [PASSED]
test/test_ssh_comp/test_ssh_comp.cpp:139: test_immediate_activation                                             [PASSED]
test/test_ssh_comp/test_ssh_comp.cpp:140: test_none_never_activates                                             [PASSED]
test/test_ssh_comp/test_ssh_comp.cpp:141: test_packet_layer_stream_roundtrip                                    [PASSED]
test/test_ssh_comp/test_ssh_comp.cpp:142: test_packet_layer_window_slide                                        [PASSED]
native_ssh_comp:test_ssh_comp Took 1.42 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_ssh_comp  test_ssh_comp  PASSED    00:00:01.421
================== 5 test cases: 5 succeeded in 00:00:01.421 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_websocket in native_ws_deflate environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_websocket/test_websocket.cpp:1068: test_sha1_empty_string                                             [PASSED]
test/test_websocket/test_websocket.cpp:1069: test_sha1_abc                                                      [PASSED]
test/test_websocket/test_websocket.cpp:1070: test_sha1_rfc6455_handshake_key                                    [PASSED]
test/test_websocket/test_websocket.cpp:1071: test_sha1_different_inputs_different_digests                       [PASSED]
test/test_websocket/test_websocket.cpp:1074: test_base64_encode_one_byte                                        [PASSED]
test/test_websocket/test_websocket.cpp:1075: test_base64_encode_two_bytes                                       [PASSED]
test/test_websocket/test_websocket.cpp:1076: test_base64_encode_three_bytes                                     [PASSED]
test/test_websocket/test_websocket.cpp:1077: test_base64_encode_ws_accept_key                                   [PASSED]
test/test_websocket/test_websocket.cpp:1078: test_base64_decode_one_byte                                        [PASSED]
test/test_websocket/test_websocket.cpp:1079: test_base64_decode_two_bytes                                       [PASSED]
test/test_websocket/test_websocket.cpp:1080: test_base64_decode_three_bytes                                     [PASSED]
test/test_websocket/test_websocket.cpp:1081: test_base64_decode_ws_accept_key                                   [PASSED]
test/test_websocket/test_websocket.cpp:1082: test_base64_decode_rejects_misplaced_padding                       [PASSED]
test/test_websocket/test_websocket.cpp:1083: test_base64_decode_respects_capacity                               [PASSED]
test/test_websocket/test_websocket.cpp:1084: test_base64_round_trip                                             [PASSED]
test/test_websocket/test_websocket.cpp:1087: test_ws_pool_size                                                  [PASSED]
test/test_websocket/test_websocket.cpp:1088: test_ws_ids_match_indices_after_init                               [PASSED]
test/test_websocket/test_websocket.cpp:1089: test_ws_all_inactive_after_init                                    [PASSED]
test/test_websocket/test_websocket.cpp:1090: test_ws_alloc_returns_non_null                                     [PASSED]
test/test_websocket/test_websocket.cpp:1091: test_ws_alloc_sets_active                                          [PASSED]
test/test_websocket/test_websocket.cpp:1092: test_ws_alloc_sets_slot_id                                         [PASSED]
test/test_websocket/test_websocket.cpp:1093: test_ws_alloc_sets_parse_state_header1                             [PASSED]
test/test_websocket/test_websocket.cpp:1094: test_ws_alloc_pool_full_returns_null                               [PASSED]
test/test_websocket/test_websocket.cpp:1095: test_ws_find_returns_correct_conn                                  [PASSED]
test/test_websocket/test_websocket.cpp:1096: test_ws_find_returns_null_when_empty                               [PASSED]
test/test_websocket/test_websocket.cpp:1097: test_ws_find_returns_null_for_different_slot                       [PASSED]
test/test_websocket/test_websocket.cpp:1098: test_ws_find_after_both_slots_allocated                            [PASSED]
test/test_websocket/test_websocket.cpp:1099: test_ws_free_deactivates_slot                                      [PASSED]
test/test_websocket/test_websocket.cpp:1100: test_ws_free_restores_ws_id                                        [PASSED]
test/test_websocket/test_websocket.cpp:1101: test_ws_free_makes_slot_findable_as_null                           [PASSED]
test/test_websocket/test_websocket.cpp:1102: test_ws_free_nop_on_unallocated                                    [PASSED]
test/test_websocket/test_websocket.cpp:1103: test_ws_alloc_after_free_succeeds                                  [PASSED]
test/test_websocket/test_websocket.cpp:1106: test_ws_parse_text_frame_sets_ready                                [PASSED]
test/test_websocket/test_websocket.cpp:1107: test_ws_parse_payload_stored_correctly                             [PASSED]
test/test_websocket/test_websocket.cpp:1108: test_ws_parse_binary_frame_sets_ready                              [PASSED]
test/test_websocket/test_websocket.cpp:1109: test_ws_parse_zero_length_unmasked_frame                           [PASSED]
test/test_websocket/test_websocket.cpp:1110: test_ws_parse_zero_length_masked_frame                             [PASSED]
test/test_websocket/test_websocket.cpp:1111: test_ws_reject_unmasked_data_frame                                 [PASSED]
test/test_websocket/test_websocket.cpp:1112: test_ws_reject_reserved_opcode                                     [PASSED]
test/test_websocket/test_websocket.cpp:1113: test_ws_reject_fragmented_control_frame                            [PASSED]
test/test_websocket/test_websocket.cpp:1114: test_ws_reject_oversized_control_frame                             [PASSED]
test/test_websocket/test_websocket.cpp:1115: test_ws_parse_16bit_length_frame                                   [PASSED]
test/test_websocket/test_websocket.cpp:1116: test_ws_parse_rsv1_set_closes_protocol                             [PASSED]
test/test_websocket/test_websocket.cpp:1117: test_ws_parse_rsv2_set_closes_protocol                             [PASSED]
test/test_websocket/test_websocket.cpp:1118: test_ws_parse_rsv3_set_closes_protocol                             [PASSED]
test/test_websocket/test_websocket.cpp:1119: test_ws_parse_64bit_length_closes_too_big                          [PASSED]
test/test_websocket/test_websocket.cpp:1120: test_ws_parse_oversized_16bit_length_closes_too_big                [PASSED]
test/test_websocket/test_websocket.cpp:1121: test_ws_fragment_start_waits_for_continuation                      [PASSED]
test/test_websocket/test_websocket.cpp:1122: test_ws_fragmented_message_reassembled                             [PASSED]
test/test_websocket/test_websocket.cpp:1123: test_ws_control_frame_interleaved_in_fragments                     [PASSED]
test/test_websocket/test_websocket.cpp:1124: test_ws_fragment_accumulation_overflow_rejected                    [PASSED]
test/test_websocket/test_websocket.cpp:1125: test_ws_continuation_without_start_rejected                        [PASSED]
test/test_websocket/test_websocket.cpp:1126: test_ws_new_data_frame_during_fragmentation_rejected               [PASSED]
test/test_websocket/test_websocket.cpp:1127: test_ws_parse_ping_auto_pong_resets_frame                          [PASSED]
test/test_websocket/test_websocket.cpp:1128: test_ws_parse_pong_silently_ignored                                [PASSED]
test/test_websocket/test_websocket.cpp:1129: test_ws_parse_close_marks_ws_closed                                [PASSED]
test/test_websocket/test_websocket.cpp:1130: test_ws_parse_stops_at_frame_ready                                 [PASSED]
test/test_websocket/test_websocket.cpp:1131: test_ws_reset_frame_clears_fields                                  [PASSED]
test/test_websocket/test_websocket.cpp:1132: test_ws_parse_mask_applied_correctly                               [PASSED]
test/test_websocket/test_websocket.cpp:1133: test_ws_text_invalid_utf8_rejected                                 [PASSED]
test/test_websocket/test_websocket.cpp:1134: test_ws_text_valid_utf8_accepted                                   [PASSED]
test/test_websocket/test_websocket.cpp:1135: test_ws_binary_arbitrary_bytes_accepted                            [PASSED]
test/test_websocket/test_websocket.cpp:1137: test_ws_permessage_deflate_inbound                                 [PASSED]
test/test_websocket/test_websocket.cpp:1138: test_ws_rsv1_without_negotiation_closes                            [PASSED]
test/test_websocket/test_websocket.cpp:1139: test_ws_permessage_deflate_outbound                                [PASSED]
test/test_websocket/test_websocket.cpp:1140: test_ws_outbound_incompressible_not_flagged                        [PASSED]
test/test_websocket/test_websocket.cpp:1143: test_ws_outbound_fragmentation                                     [PASSED]
test/test_websocket/test_websocket.cpp:1146: stress_ws_parse_reset_100_cycles                                   [PASSED]
test/test_websocket/test_websocket.cpp:1147: stress_ws_alloc_free_pool_cycle                                    [PASSED]
test/test_websocket/test_websocket.cpp:1148: stress_ws_parse_incremental_byte_by_byte                           [PASSED]
test/test_websocket/test_websocket.cpp:1149: stress_ws_parse_max_payload                                        [PASSED]
test/test_websocket/test_websocket.cpp:1150: stress_ws_parse_two_consecutive_frames                             [PASSED]
native_ws_deflate:test_websocket Took 1.47 seconds ------------------------------------------------------------ [PASSED]

=================================== SUMMARY ===================================
Environment        Test            Status    Duration
-----------------  --------------  --------  ------------
native_ws_deflate  test_websocket  PASSED    00:00:01.468
================= 72 test cases: 72 succeeded in 00:00:01.468 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_time_source in native_time_source environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_time_source/test_time_source.cpp:129: test_single_source                                              [PASSED]
test/test_time_source/test_time_source.cpp:130: test_priority_order_lowest_value_wins                           [PASSED]
test/test_time_source/test_time_source.cpp:131: test_falls_back_when_primary_unavailable                        [PASSED]
test/test_time_source/test_time_source.cpp:132: test_all_unavailable_returns_zero                               [PASSED]
test/test_time_source/test_time_source.cpp:133: test_first_valid_short_circuits                                 [PASSED]
test/test_time_source/test_time_source.cpp:134: test_fallback_queries_in_priority_order                         [PASSED]
test/test_time_source/test_time_source.cpp:135: test_table_full_rejects                                         [PASSED]
test/test_time_source/test_time_source.cpp:136: test_null_fn_rejected                                           [PASSED]
test/test_time_source/test_time_source.cpp:137: test_reset_clears_sources                                       [PASSED]
native_time_source:test_time_source Took 0.82 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_time_source  test_time_source  PASSED    00:00:00.818
================== 9 test cases: 9 succeeded in 00:00:00.818 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_config_store in native_config_store environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_config_store/test_config_store.cpp:149: test_str_round_trip                                           [PASSED]
test/test_config_store/test_config_store.cpp:150: test_str_default_when_missing                                 [PASSED]
test/test_config_store/test_config_store.cpp:151: test_str_overwrite                                            [PASSED]
test/test_config_store/test_config_store.cpp:152: test_str_truncates_to_capacity                                [PASSED]
test/test_config_store/test_config_store.cpp:153: test_u32_round_trip                                           [PASSED]
test/test_config_store/test_config_store.cpp:154: test_u32_default_when_missing                                 [PASSED]
test/test_config_store/test_config_store.cpp:155: test_blob_round_trip                                          [PASSED]
test/test_config_store/test_config_store.cpp:156: test_blob_bounded_by_capacity                                 [PASSED]
test/test_config_store/test_config_store.cpp:157: test_blob_missing_returns_zero                                [PASSED]
test/test_config_store/test_config_store.cpp:158: test_erase_removes_key                                        [PASSED]
test/test_config_store/test_config_store.cpp:159: test_clear_wipes_namespace                                    [PASSED]
test/test_config_store/test_config_store.cpp:160: test_table_full_rejects_new_key                               [PASSED]
test/test_config_store/test_config_store.cpp:161: test_existing_key_overwrites_even_when_full                   [PASSED]
test/test_config_store/test_config_store.cpp:162: test_key_too_long_rejected                                    [PASSED]
native_config_store:test_config_store Took 0.83 seconds ------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_config_store  test_config_store  PASSED    00:00:00.830
================= 14 test cases: 14 succeeded in 00:00:00.830 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_device_id in native_device_id environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_device_id/test_device_id.cpp:58: test_uuid_matches_reference_aabbccddeeff                             [PASSED]
test/test_device_id/test_device_id.cpp:59: test_uuid_matches_reference_001122334455                             [PASSED]
test/test_device_id/test_device_id.cpp:60: test_uuid_is_deterministic                                           [PASSED]
test/test_device_id/test_device_id.cpp:61: test_uuid_version_and_variant_bits                                   [PASSED]
native_device_id:test_device_id Took 0.86 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_device_id  test_device_id  PASSED    00:00:00.861
================== 4 test cases: 4 succeeded in 00:00:00.861 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_auth_lockout in native_auth_lockout environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_auth_lockout/test_auth_lockout.cpp:183: test_below_threshold_not_locked                               [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:184: test_locks_at_threshold                                       [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:185: test_exponential_backoff                                      [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:186: test_caps_at_max                                              [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:187: test_expires_after_window                                     [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:188: test_success_clears                                           [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:189: test_isolates_addresses                                       [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:190: test_v6_distinct_from_v4_and_each_other                       [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:191: test_zero_ip_never_locked                                     [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:192: test_table_full_tracks_new_address                            [PASSED]
test/test_auth_lockout/test_auth_lockout.cpp:193: test_active_lockout_survives_eviction                         [PASSED]
native_auth_lockout:test_auth_lockout Took 0.89 seconds ------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_auth_lockout  test_auth_lockout  PASSED    00:00:00.885
================= 11 test cases: 11 succeeded in 00:00:00.885 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_csrf in native_csrf environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_csrf/test_csrf.cpp:114: test_issue_verify_roundtrip                                                   [PASSED]
test/test_csrf/test_csrf.cpp:115: test_token_format_and_length                                                  [PASSED]
test/test_csrf/test_csrf.cpp:116: test_verify_rejects_tampered_sig                                              [PASSED]
test/test_csrf/test_csrf.cpp:117: test_verify_rejects_tampered_nonce                                            [PASSED]
test/test_csrf/test_csrf.cpp:118: test_verify_rejects_garbage                                                   [PASSED]
test/test_csrf/test_csrf.cpp:119: test_different_secret_rejects                                                 [PASSED]
test/test_csrf/test_csrf.cpp:120: test_no_secret_fails_closed                                                   [PASSED]
test/test_csrf/test_csrf.cpp:121: test_issue_unique                                                             [PASSED]
test/test_csrf/test_csrf.cpp:122: test_issue_rejects_small_buffer                                               [PASSED]
native_csrf:test_csrf Took 0.89 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_csrf    test_csrf  PASSED    00:00:00.886
================== 9 test cases: 9 succeeded in 00:00:00.886 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_telemetry in native_telemetry environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_telemetry/test_telemetry.cpp:121: test_window_classic_stats                                           [PASSED]
test/test_telemetry/test_telemetry.cpp:122: test_window_empty                                                   [PASSED]
test/test_telemetry/test_telemetry.cpp:123: test_window_single_sample                                           [PASSED]
test/test_telemetry/test_telemetry.cpp:124: test_window_eviction                                                [PASSED]
test/test_telemetry/test_telemetry.cpp:125: test_rate_basic                                                     [PASSED]
test/test_telemetry/test_telemetry.cpp:126: test_rate_zero_dt                                                   [PASSED]
test/test_telemetry/test_telemetry.cpp:127: test_totalizer_constant_rate                                        [PASSED]
test/test_telemetry/test_telemetry.cpp:128: test_totalizer_trapezoid_and_reset                                  [PASSED]
native_telemetry:test_telemetry Took 0.87 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_telemetry  test_telemetry  PASSED    00:00:00.874
================== 8 test cases: 8 succeeded in 00:00:00.874 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_dashboard in native_dashboard environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_dashboard/test_dashboard.cpp:220: test_layout_bar_sparkline_types                                     [PASSED]
test/test_dashboard/test_dashboard.cpp:221: test_null_widget_table_guards                                       [PASSED]
test/test_dashboard/test_dashboard.cpp:222: test_json_overflow_paths                                            [PASSED]
test/test_dashboard/test_dashboard.cpp:223: test_parse_control_edges                                            [PASSED]
test/test_dashboard/test_dashboard.cpp:224: test_layout_json                                                    [PASSED]
test/test_dashboard/test_dashboard.cpp:225: test_values_json_initial_zero                                       [PASSED]
test/test_dashboard/test_dashboard.cpp:226: test_set_and_values                                                 [PASSED]
test/test_dashboard/test_dashboard.cpp:227: test_set_unknown_key                                                [PASSED]
test/test_dashboard/test_dashboard.cpp:228: test_configure_resets_values                                        [PASSED]
test/test_dashboard/test_dashboard.cpp:229: test_small_buffer_fails_closed                                      [PASSED]
test/test_dashboard/test_dashboard.cpp:230: test_parse_control_ok                                               [PASSED]
test/test_dashboard/test_dashboard.cpp:231: test_parse_control_float                                            [PASSED]
test/test_dashboard/test_dashboard.cpp:232: test_parse_control_rejects_malformed                                [PASSED]
test/test_dashboard/test_dashboard.cpp:233: test_dispatch_control_invokes_cb                                    [PASSED]
test/test_dashboard/test_dashboard.cpp:234: test_layout_control_types                                           [PASSED]
native_dashboard:test_dashboard Took 0.83 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_dashboard  test_dashboard  PASSED    00:00:00.831
================= 15 test cases: 15 succeeded in 00:00:00.831 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_net_egress in native_net_egress environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_net_egress/test_net_egress.cpp:60: test_classify_sta                                                  [PASSED]
test/test_net_egress/test_net_egress.cpp:61: test_classify_ap                                                   [PASSED]
test/test_net_egress/test_net_egress.cpp:62: test_classify_eth                                                  [PASSED]
test/test_net_egress/test_net_egress.cpp:63: test_classify_none                                                 [PASSED]
test/test_net_egress/test_net_egress.cpp:64: test_egress_host_stub                                              [PASSED]
test/test_net_egress/test_net_egress.cpp:65: test_eth_host_stub                                                 [PASSED]
native_net_egress:test_net_egress Took 0.80 seconds ----------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_net_egress  test_net_egress  PASSED    00:00:00.801
================== 6 test cases: 6 succeeded in 00:00:00.801 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_partition_monitor in native_partition environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_partition_monitor/test_partition_monitor.cpp:71: test_kind_app                                        [PASSED]
test/test_partition_monitor/test_partition_monitor.cpp:72: test_kind_data                                       [PASSED]
test/test_partition_monitor/test_partition_monitor.cpp:73: test_json                                            [PASSED]
test/test_partition_monitor/test_partition_monitor.cpp:74: test_json_small_buffer_fails_closed                  [PASSED]
test/test_partition_monitor/test_partition_monitor.cpp:75: test_collect_host_stub                               [PASSED]
native_partition:test_partition_monitor Took 0.82 seconds ----------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test                    Status    Duration
----------------  ----------------------  --------  ------------
native_partition  test_partition_monitor  PASSED    00:00:00.818
================== 5 test cases: 5 succeeded in 00:00:00.818 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_cbor in native_cbor environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_cbor/test_cbor.cpp:372: test_uint                                                                     [PASSED]
test/test_cbor/test_cbor.cpp:373: test_peek_each_type                                                           [PASSED]
test/test_cbor/test_cbor.cpp:374: test_uint_8byte                                                               [PASSED]
test/test_cbor/test_cbor.cpp:375: test_read_double_encoded_float                                                [PASSED]
test/test_cbor/test_cbor.cpp:376: test_read_map_type_mismatch                                                   [PASSED]
test/test_cbor/test_cbor.cpp:377: test_int                                                                      [PASSED]
test/test_cbor/test_cbor.cpp:378: test_text                                                                     [PASSED]
test/test_cbor/test_cbor.cpp:379: test_bytes                                                                    [PASSED]
test/test_cbor/test_cbor.cpp:380: test_simple                                                                   [PASSED]
test/test_cbor/test_cbor.cpp:381: test_float                                                                    [PASSED]
test/test_cbor/test_cbor.cpp:382: test_array_and_map                                                            [PASSED]
test/test_cbor/test_cbor.cpp:383: test_overflow_fails_closed                                                    [PASSED]
test/test_cbor/test_cbor.cpp:384: test_decode_uint                                                              [PASSED]
test/test_cbor/test_cbor.cpp:385: test_decode_int                                                               [PASSED]
test/test_cbor/test_cbor.cpp:386: test_decode_float_roundtrip                                                   [PASSED]
test/test_cbor/test_cbor.cpp:387: test_decode_roundtrip_map                                                     [PASSED]
test/test_cbor/test_cbor.cpp:388: test_decode_truncated                                                         [PASSED]
test/test_cbor/test_cbor.cpp:389: test_decode_type_mismatch                                                     [PASSED]
native_cbor:test_cbor Took 0.86 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_cbor    test_cbor  PASSED    00:00:00.859
================= 18 test cases: 18 succeeded in 00:00:00.859 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_msgpack in native_msgpack environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_msgpack/test_msgpack.cpp:495: test_uint                                                               [PASSED]
test/test_msgpack/test_msgpack.cpp:496: test_wide_roundtrip                                                     [PASSED]
test/test_msgpack/test_msgpack.cpp:497: test_decode_wide_fails_closed                                           [PASSED]
test/test_msgpack/test_msgpack.cpp:498: test_int                                                                [PASSED]
test/test_msgpack/test_msgpack.cpp:499: test_str                                                                [PASSED]
test/test_msgpack/test_msgpack.cpp:500: test_bytes                                                              [PASSED]
test/test_msgpack/test_msgpack.cpp:501: test_simple                                                             [PASSED]
test/test_msgpack/test_msgpack.cpp:502: test_float                                                              [PASSED]
test/test_msgpack/test_msgpack.cpp:503: test_array_and_map                                                      [PASSED]
test/test_msgpack/test_msgpack.cpp:504: test_overflow_fails_closed                                              [PASSED]
test/test_msgpack/test_msgpack.cpp:505: test_decode_uint                                                        [PASSED]
test/test_msgpack/test_msgpack.cpp:506: test_decode_int                                                         [PASSED]
test/test_msgpack/test_msgpack.cpp:507: test_decode_str_and_bytes                                               [PASSED]
test/test_msgpack/test_msgpack.cpp:508: test_decode_simple_and_float                                            [PASSED]
test/test_msgpack/test_msgpack.cpp:509: test_decode_array_and_map                                               [PASSED]
test/test_msgpack/test_msgpack.cpp:510: test_decode_roundtrip                                                   [PASSED]
test/test_msgpack/test_msgpack.cpp:511: test_decode_fails_closed                                                [PASSED]
native_msgpack:test_msgpack Took 0.83 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_msgpack  test_msgpack  PASSED    00:00:00.833
================= 17 test cases: 17 succeeded in 00:00:00.833 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_gpio_map in native_gpio_map environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_gpio_map/test_gpio_map.cpp:107: test_dir_name                                                         [PASSED]
test/test_gpio_map/test_gpio_map.cpp:108: test_json                                                             [PASSED]
test/test_gpio_map/test_gpio_map.cpp:109: test_json_empty                                                       [PASSED]
test/test_gpio_map/test_gpio_map.cpp:110: test_json_small_buffer_fails_closed                                   [PASSED]
test/test_gpio_map/test_gpio_map.cpp:111: test_parse_set                                                        [PASSED]
test/test_gpio_map/test_gpio_map.cpp:112: test_parse_set_rejects_partial                                        [PASSED]
test/test_gpio_map/test_gpio_map.cpp:113: test_parse_set_no_prefix_match                                        [PASSED]
test/test_gpio_map/test_gpio_map.cpp:114: test_is_output                                                        [PASSED]
native_gpio_map:test_gpio_map Took 0.86 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_gpio_map  test_gpio_map  PASSED    00:00:00.865
================== 8 test cases: 8 succeeded in 00:00:00.865 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_udp_telemetry in native_udp_telemetry environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_udp_telemetry/test_udp_telemetry.cpp:101: test_int_and_uint_fields                                    [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:102: test_float_field                                            [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:103: test_no_fields_not_ok                                       [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:104: test_overflow_fails_closed                                  [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:105: test_tags_and_timestamp                                     [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:106: test_tag_escaping                                           [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:107: test_tag_after_field_fails_closed                           [PASSED]
native_udp_telemetry:test_udp_telemetry Took 0.83 seconds ----------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment           Test                Status    Duration
--------------------  ------------------  --------  ------------
native_udp_telemetry  test_udp_telemetry  PASSED    00:00:00.829
================== 7 test cases: 7 succeeded in 00:00:00.829 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_statsd in native_statsd environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_statsd/test_statsd.cpp:132: test_format_types                                                         [PASSED]
test/test_statsd/test_statsd.cpp:133: test_format_sample_rate                                                   [PASSED]
test/test_statsd/test_statsd.cpp:134: test_format_tags_and_both                                                 [PASSED]
test/test_statsd/test_statsd.cpp:135: test_format_guards                                                        [PASSED]
test/test_statsd/test_statsd.cpp:136: test_emit_counter_and_negative                                            [PASSED]
test/test_statsd/test_statsd.cpp:137: test_emit_gauge_and_delta                                                 [PASSED]
test/test_statsd/test_statsd.cpp:138: test_emit_timing_set_sampled                                              [PASSED]
test/test_statsd/test_statsd.cpp:139: test_emit_global_tags                                                     [PASSED]
test/test_statsd/test_statsd.cpp:140: test_emit_noop_until_begin                                                [PASSED]
native_statsd:test_statsd Took 0.86 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_statsd  test_statsd  PASSED    00:00:00.860
================== 9 test cases: 9 succeeded in 00:00:00.860 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_guardrails in native_guardrails environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_guardrails/test_guardrails.cpp:63: test_eval_all_clear                                                [PASSED]
test/test_guardrails/test_guardrails.cpp:64: test_eval_heap_breach                                              [PASSED]
test/test_guardrails/test_guardrails.cpp:65: test_eval_frag_and_stack                                           [PASSED]
test/test_guardrails/test_guardrails.cpp:66: test_eval_all_breached                                             [PASSED]
test/test_guardrails/test_guardrails.cpp:67: test_json                                                          [PASSED]
test/test_guardrails/test_guardrails.cpp:68: test_json_small_buffer_fails_closed                                [PASSED]
native_guardrails:test_guardrails Took 0.82 seconds ----------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_guardrails  test_guardrails  PASSED    00:00:00.820
================== 6 test cases: 6 succeeded in 00:00:00.820 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_failsafe in native_failsafe environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_failsafe/test_failsafe.cpp:114: test_overdue_predicate                                                [PASSED]
test/test_failsafe/test_failsafe.cpp:115: test_register_and_not_overdue_when_fresh                              [PASSED]
test/test_failsafe/test_failsafe.cpp:116: test_breach_fires_once_then_clears_on_feed                            [PASSED]
test/test_failsafe/test_failsafe.cpp:117: test_registry_full                                                    [PASSED]
test/test_failsafe/test_failsafe.cpp:118: test_feed_bad_id                                                      [PASSED]
test/test_failsafe/test_failsafe.cpp:119: test_json                                                             [PASSED]
native_failsafe:test_failsafe Took 0.86 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_failsafe  test_failsafe  PASSED    00:00:00.856
================== 6 test cases: 6 succeeded in 00:00:00.856 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_sleep_sched in native_sleep_sched environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_sleep_sched/test_sleep_sched.cpp:79: test_awake_when_recent                                           [PASSED]
test/test_sleep_sched/test_sleep_sched.cpp:80: test_min_window_at_threshold                                     [PASSED]
test/test_sleep_sched/test_sleep_sched.cpp:81: test_ramp_doubles                                                [PASSED]
test/test_sleep_sched/test_sleep_sched.cpp:82: test_clamps_to_ceiling                                           [PASSED]
test/test_sleep_sched/test_sleep_sched.cpp:83: test_no_ramp_jumps_to_ceiling                                    [PASSED]
test/test_sleep_sched/test_sleep_sched.cpp:84: test_degenerate_max_below_min                                    [PASSED]
test/test_sleep_sched/test_sleep_sched.cpp:85: test_wrap_safe                                                   [PASSED]
test/test_sleep_sched/test_sleep_sched.cpp:86: test_null_cfg                                                    [PASSED]
native_sleep_sched:test_sleep_sched Took 0.82 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_sleep_sched  test_sleep_sched  PASSED    00:00:00.821
================== 8 test cases: 8 succeeded in 00:00:00.821 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_wearlevel in native_wearlevel environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_wearlevel/test_wearlevel.cpp:68: test_pick_least_worn_ties_lowest_index                               [PASSED]
test/test_wearlevel/test_wearlevel.cpp:69: test_pick_edge                                                       [PASSED]
test/test_wearlevel/test_wearlevel.cpp:70: test_pick_plus_mark_levels_the_region                                [PASSED]
test/test_wearlevel/test_wearlevel.cpp:71: test_mark_saturates_and_bounds                                       [PASSED]
test/test_wearlevel/test_wearlevel.cpp:72: test_spread                                                          [PASSED]
native_wearlevel:test_wearlevel Took 0.80 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_wearlevel  test_wearlevel  PASSED    00:00:00.796
================== 5 test cases: 5 succeeded in 00:00:00.796 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_netadapt in native_netadapt environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_netadapt/test_netadapt.cpp:59: test_window_floor_when_low_heap                                        [PASSED]
test/test_netadapt/test_netadapt.cpp:60: test_window_scales_with_heap                                           [PASSED]
test/test_netadapt/test_netadapt.cpp:61: test_window_clamps_to_ceiling                                          [PASSED]
test/test_netadapt/test_netadapt.cpp:62: test_window_degenerate_max_below_min                                   [PASSED]
test/test_netadapt/test_netadapt.cpp:63: test_dhcp_fallback_on_timeout                                          [PASSED]
test/test_netadapt/test_netadapt.cpp:64: test_dhcp_fallback_on_attempts                                         [PASSED]
native_netadapt:test_netadapt Took 0.81 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_netadapt  test_netadapt  PASSED    00:00:00.808
================== 6 test cases: 6 succeeded in 00:00:00.808 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_dshot in native_dshot environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_dshot/test_dshot.cpp:92: test_encode_known_vector                                                     [PASSED]
test/test_dshot/test_dshot.cpp:93: test_encode_telemetry_bit                                                    [PASSED]
test/test_dshot/test_dshot.cpp:94: test_encode_bidirectional_inverts_crc                                        [PASSED]
test/test_dshot/test_dshot.cpp:95: test_value_masked_to_11_bits                                                 [PASSED]
test/test_dshot/test_dshot.cpp:96: test_decode_roundtrip_and_crc                                                [PASSED]
test/test_dshot/test_dshot.cpp:97: test_bit_timing                                                              [PASSED]
test/test_dshot/test_dshot.cpp:98: test_esc_pwm_mapping                                                         [PASSED]
native_dshot:test_dshot Took 0.82 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_dshot   test_dshot  PASSED    00:00:00.818
================== 7 test cases: 7 succeeded in 00:00:00.818 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_hart in native_hart environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_hart/test_hart.cpp:92: test_checksum                                                                  [PASSED]
test/test_hart/test_hart.cpp:93: test_build_command0_short                                                      [PASSED]
test/test_hart/test_hart.cpp:94: test_build_with_data                                                           [PASSED]
test/test_hart/test_hart.cpp:95: test_build_long_address                                                        [PASSED]
test/test_hart/test_hart.cpp:96: test_parse_roundtrip_and_bad_checksum                                          [PASSED]
test/test_hart/test_hart.cpp:97: test_hartip_header                                                             [PASSED]
native_hart:test_hart Took 0.82 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_hart    test_hart  PASSED    00:00:00.817
================== 6 test cases: 6 succeeded in 00:00:00.817 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_nts in native_nts environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_nts/test_nts.cpp:102: test_ke_record                                                                  [PASSED]
test/test_nts/test_nts.cpp:103: test_ke_request                                                                 [PASSED]
test/test_nts/test_nts.cpp:104: test_ke_parse                                                                   [PASSED]
test/test_nts/test_nts.cpp:105: test_extension_field_padding                                                    [PASSED]
native_nts:test_nts Took 0.83 seconds ------------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_nts     test_nts  PASSED    00:00:00.832
================== 4 test cases: 4 succeeded in 00:00:00.832 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_dds in native_dds environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_dds/test_dds.cpp:101: test_header                                                                     [PASSED]
test/test_dds/test_dds.cpp:102: test_submessage_endianness                                                      [PASSED]
test/test_dds/test_dds.cpp:103: test_parse_message                                                              [PASSED]
test/test_dds/test_dds.cpp:104: test_parse_rejects                                                              [PASSED]
native_dds:test_dds Took 0.81 seconds ------------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_dds     test_dds  PASSED    00:00:00.814
================== 4 test cases: 4 succeeded in 00:00:00.814 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_xmpp in native_xmpp environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_xmpp/test_xmpp.cpp:81: test_escape                                                                    [PASSED]
test/test_xmpp/test_xmpp.cpp:82: test_message                                                                   [PASSED]
test/test_xmpp/test_xmpp.cpp:83: test_presence                                                                  [PASSED]
test/test_xmpp/test_xmpp.cpp:84: test_iq                                                                        [PASSED]
test/test_xmpp/test_xmpp.cpp:85: test_stanza_name                                                               [PASSED]
test/test_xmpp/test_xmpp.cpp:86: test_attr                                                                      [PASSED]
native_xmpp:test_xmpp Took 0.84 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_xmpp    test_xmpp  PASSED    00:00:00.841
================== 6 test cases: 6 succeeded in 00:00:00.841 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_rawl2 in native_rawl2 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_rawl2/test_rawl2.cpp:81: test_build_ethernet_ii                                                       [PASSED]
test/test_rawl2/test_rawl2.cpp:82: test_build_vlan                                                              [PASSED]
test/test_rawl2/test_rawl2.cpp:83: test_parse                                                                   [PASSED]
test/test_rawl2/test_rawl2.cpp:84: test_fcs_check_vector                                                        [PASSED]
native_rawl2:test_rawl2 Took 0.81 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_rawl2   test_rawl2  PASSED    00:00:00.811
================== 4 test cases: 4 succeeded in 00:00:00.811 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_spa_router in native_spa_router environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_spa_router/test_spa_router.cpp:47: test_has_extension                                                 [PASSED]
test/test_spa_router/test_spa_router.cpp:48: test_route                                                         [PASSED]
native_spa_router:test_spa_router Took 0.77 seconds ----------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_spa_router  test_spa_router  PASSED    00:00:00.774
================== 2 test cases: 2 succeeded in 00:00:00.774 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_goose in native_goose environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_goose/test_goose.cpp:106: test_pdu_structure                                                          [PASSED]
test/test_goose/test_goose.cpp:107: test_integer_leading_zero                                                   [PASSED]
test/test_goose/test_goose.cpp:108: test_frame                                                                  [PASSED]
native_goose:test_goose Took 0.79 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_goose   test_goose  PASSED    00:00:00.790
================== 3 test cases: 3 succeeded in 00:00:00.790 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_mtconnect in native_mtconnect environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_mtconnect/test_mtconnect.cpp:79: test_streams_document                                                [PASSED]
test/test_mtconnect/test_mtconnect.cpp:80: test_streams_escapes_value                                           [PASSED]
test/test_mtconnect/test_mtconnect.cpp:81: test_error_document                                                  [PASSED]
test/test_mtconnect/test_mtconnect.cpp:82: test_overflow_returns_zero                                           [PASSED]
native_mtconnect:test_mtconnect Took 0.84 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_mtconnect  test_mtconnect  PASSED    00:00:00.838
================== 4 test cases: 4 succeeded in 00:00:00.838 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_j2735 in native_j2735 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_j2735/test_j2735.cpp:168: test_cint_bits                                                              [PASSED]
test/test_j2735/test_j2735.cpp:169: test_bit_writer_pattern                                                     [PASSED]
test/test_j2735/test_j2735.cpp:170: test_writer_null_and_zero                                                   [PASSED]
test/test_j2735/test_j2735.cpp:171: test_cint_roundtrip                                                         [PASSED]
test/test_j2735/test_j2735.cpp:172: test_bsm_core_roundtrip                                                     [PASSED]
test/test_j2735/test_j2735.cpp:173: test_bsm_core_bit_length                                                    [PASSED]
test/test_j2735/test_j2735.cpp:174: test_spat_roundtrip                                                         [PASSED]
test/test_j2735/test_j2735.cpp:175: test_spat_decode_too_many                                                   [PASSED]
test/test_j2735/test_j2735.cpp:176: test_map_roundtrip                                                          [PASSED]
native_j2735:test_j2735 Took 0.82 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_j2735   test_j2735  PASSED    00:00:00.819
================== 9 test cases: 9 succeeded in 00:00:00.819 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_nema_ts2 in native_nema_ts2 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_nema_ts2/test_nema_ts2.cpp:68: test_crc_check_vector                                                  [PASSED]
test/test_nema_ts2/test_nema_ts2.cpp:69: test_build_and_parse                                                   [PASSED]
test/test_nema_ts2/test_nema_ts2.cpp:70: test_no_data_frame                                                     [PASSED]
test/test_nema_ts2/test_nema_ts2.cpp:71: test_parse_rejects_bad_crc_and_short                                   [PASSED]
native_nema_ts2:test_nema_ts2 Took 0.78 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_nema_ts2  test_nema_ts2  PASSED    00:00:00.784
================== 4 test cases: 4 succeeded in 00:00:00.784 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_snp in native_snp environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_snp/test_snp.cpp:72: test_bcc                                                                         [PASSED]
test/test_snp/test_snp.cpp:73: test_build_and_parse                                                             [PASSED]
test/test_snp/test_snp.cpp:74: test_empty_data                                                                  [PASSED]
test/test_snp/test_snp.cpp:75: test_parse_rejects                                                               [PASSED]
native_snp:test_snp Took 0.79 seconds ------------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_snp     test_snp  PASSED    00:00:00.794
================== 4 test cases: 4 succeeded in 00:00:00.794 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_directnet in native_directnet environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_directnet/test_directnet.cpp:72: test_lrc                                                             [PASSED]
test/test_directnet/test_directnet.cpp:73: test_header_frame                                                    [PASSED]
test/test_directnet/test_directnet.cpp:74: test_data_frame_roundtrip                                            [PASSED]
test/test_directnet/test_directnet.cpp:75: test_data_parse_rejects                                              [PASSED]
native_directnet:test_directnet Took 0.79 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_directnet  test_directnet  PASSED    00:00:00.790
================== 4 test cases: 4 succeeded in 00:00:00.790 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_sep2 in native_sep2 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_sep2/test_sep2.cpp:69: test_device_capability                                                         [PASSED]
test/test_sep2/test_sep2.cpp:70: test_end_device                                                                [PASSED]
test/test_sep2/test_sep2.cpp:71: test_der_control_negative_setpoint                                             [PASSED]
test/test_sep2/test_sep2.cpp:72: test_xml_escape_in_href                                                        [PASSED]
test/test_sep2/test_sep2.cpp:73: test_overflow                                                                  [PASSED]
native_sep2:test_sep2 Took 0.81 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_sep2    test_sep2  PASSED    00:00:00.807
================== 5 test cases: 5 succeeded in 00:00:00.807 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_profinet in native_profinet environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_profinet/test_profinet.cpp:96: test_header_roundtrip                                                  [PASSED]
test/test_profinet/test_profinet.cpp:97: test_block_even_padding                                                [PASSED]
test/test_profinet/test_profinet.cpp:98: test_walk_blocks                                                       [PASSED]
test/test_profinet/test_profinet.cpp:99: test_walk_rejects_truncated                                            [PASSED]
native_profinet:test_profinet Took 0.80 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_profinet  test_profinet  PASSED    00:00:00.804
================== 4 test cases: 4 succeeded in 00:00:00.804 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ntcip in native_ntcip environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ntcip/test_ntcip.cpp:63: test_roots_under_nema                                                        [PASSED]
test/test_ntcip/test_ntcip.cpp:64: test_oid_builder_scalar_and_index                                            [PASSED]
test/test_ntcip/test_ntcip.cpp:65: test_oid_builder_overflow                                                    [PASSED]
native_ntcip:test_ntcip Took 0.81 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_ntcip   test_ntcip  PASSED    00:00:00.807
================== 3 test cases: 3 succeeded in 00:00:00.807 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_openadr in native_openadr environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_openadr/test_openadr.cpp:70: test_event                                                               [PASSED]
test/test_openadr/test_openadr.cpp:71: test_report_negative_value                                               [PASSED]
test/test_openadr/test_openadr.cpp:72: test_json_escape                                                         [PASSED]
test/test_openadr/test_openadr.cpp:73: test_overflow                                                            [PASSED]
native_openadr:test_openadr Took 0.82 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_openadr  test_openadr  PASSED    00:00:00.820
================== 4 test cases: 4 succeeded in 00:00:00.820 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_mms in native_mms environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_mms/test_mms.cpp:82: test_read_request_structure                                                      [PASSED]
test/test_mms/test_mms.cpp:83: test_read_request_parse                                                          [PASSED]
test/test_mms/test_mms.cpp:84: test_read_response_roundtrip                                                     [PASSED]
test/test_mms/test_mms.cpp:85: test_parse_rejects_bad_tag                                                       [PASSED]
native_mms:test_mms Took 0.82 seconds ------------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_mms     test_mms  PASSED    00:00:00.821
================== 4 test cases: 4 succeeded in 00:00:00.821 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_cclink in native_cclink environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_cclink/test_cclink.cpp:74: test_sum                                                                   [PASSED]
test/test_cclink/test_cclink.cpp:75: test_build_and_parse                                                       [PASSED]
test/test_cclink/test_cclink.cpp:76: test_bit_accessors                                                         [PASSED]
test/test_cclink/test_cclink.cpp:77: test_parse_rejects                                                         [PASSED]
native_cclink:test_cclink Took 0.81 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_cclink  test_cclink  PASSED    00:00:00.813
================== 4 test cases: 4 succeeded in 00:00:00.813 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_powerlink in native_powerlink environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_powerlink/test_powerlink.cpp:62: test_soc                                                             [PASSED]
test/test_powerlink/test_powerlink.cpp:63: test_preq_pres_roundtrip                                             [PASSED]
test/test_powerlink/test_powerlink.cpp:64: test_parse_rejects                                                   [PASSED]
native_powerlink:test_powerlink Took 0.79 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_powerlink  test_powerlink  PASSED    00:00:00.790
================== 3 test cases: 3 succeeded in 00:00:00.790 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_sercos in native_sercos environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_sercos/test_sercos.cpp:76: test_idn_roundtrip                                                         [PASSED]
test/test_sercos/test_sercos.cpp:77: test_telegram_roundtrip                                                    [PASSED]
test/test_sercos/test_sercos.cpp:78: test_at_telegram_and_rejects                                               [PASSED]
native_sercos:test_sercos Took 0.81 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_sercos  test_sercos  PASSED    00:00:00.807
================== 3 test cases: 3 succeeded in 00:00:00.807 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_profibus in native_profibus environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_profibus/test_profibus.cpp:81: test_fcs                                                               [PASSED]
test/test_profibus/test_profibus.cpp:82: test_sd1                                                               [PASSED]
test/test_profibus/test_profibus.cpp:83: test_sd2_roundtrip                                                     [PASSED]
test/test_profibus/test_profibus.cpp:84: test_parse_rejects                                                     [PASSED]
native_profibus:test_profibus Took 0.78 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_profibus  test_profibus  PASSED    00:00:00.779
================== 4 test cases: 4 succeeded in 00:00:00.779 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_lonworks in native_lonworks environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_lonworks/test_lonworks.cpp:85: test_nv_pdu_roundtrip                                                  [PASSED]
test/test_lonworks/test_lonworks.cpp:86: test_nv_selector_masked_to_14_bits                                     [PASSED]
test/test_lonworks/test_lonworks.cpp:87: test_snvt_temp                                                         [PASSED]
test/test_lonworks/test_lonworks.cpp:88: test_snvt_switch                                                       [PASSED]
native_lonworks:test_lonworks Took 0.77 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_lonworks  test_lonworks  PASSED    00:00:00.767
================== 4 test cases: 4 succeeded in 00:00:00.767 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_mbplus in native_mbplus environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_mbplus/test_mbplus.cpp:82: test_crc_check_vector                                                      [PASSED]
test/test_mbplus/test_mbplus.cpp:83: test_build_and_parse                                                       [PASSED]
test/test_mbplus/test_mbplus.cpp:84: test_token_frame_no_payload                                                [PASSED]
test/test_mbplus/test_mbplus.cpp:85: test_next_token_ring                                                       [PASSED]
test/test_mbplus/test_mbplus.cpp:86: test_parse_rejects                                                         [PASSED]
native_mbplus:test_mbplus Took 0.80 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_mbplus  test_mbplus  PASSED    00:00:00.799
================== 5 test cases: 5 succeeded in 00:00:00.799 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_interbus in native_interbus environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_interbus/test_interbus.cpp:79: test_fcs_check_vector                                                  [PASSED]
test/test_interbus/test_interbus.cpp:80: test_build_and_parse                                                   [PASSED]
test/test_interbus/test_interbus.cpp:81: test_empty_frame                                                       [PASSED]
test/test_interbus/test_interbus.cpp:82: test_parse_rejects                                                     [PASSED]
native_interbus:test_interbus Took 0.78 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_interbus  test_interbus  PASSED    00:00:00.776
================== 4 test cases: 4 succeeded in 00:00:00.776 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_iccp in native_iccp environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_iccp/test_iccp.cpp:80: test_state_q_no_time                                                           [PASSED]
test/test_iccp/test_iccp.cpp:81: test_state_q_with_time                                                         [PASSED]
test/test_iccp/test_iccp.cpp:82: test_real_q                                                                    [PASSED]
test/test_iccp/test_iccp.cpp:83: test_real_q_negative                                                           [PASSED]
native_iccp:test_iccp Took 0.78 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_iccp    test_iccp  PASSED    00:00:00.784
================== 4 test cases: 4 succeeded in 00:00:00.784 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_wave in native_wave environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_wave/test_wave.cpp:84: test_psid_p_encoding                                                           [PASSED]
test/test_wave/test_wave.cpp:85: test_wsmp_roundtrip                                                            [PASSED]
test/test_wave/test_wave.cpp:86: test_1609dot2_wrap                                                             [PASSED]
test/test_wave/test_wave.cpp:87: test_wsmp_parse_rejects                                                        [PASSED]
native_wave:test_wave Took 0.79 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_wave    test_wave  PASSED    00:00:00.785
================== 4 test cases: 4 succeeded in 00:00:00.785 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_utmc in native_utmc environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_utmc/test_utmc.cpp:70: test_request                                                                   [PASSED]
test/test_utmc/test_utmc.cpp:71: test_response                                                                  [PASSED]
test/test_utmc/test_utmc.cpp:72: test_response_escapes                                                          [PASSED]
test/test_utmc/test_utmc.cpp:73: test_parse_request                                                             [PASSED]
test/test_utmc/test_utmc.cpp:74: test_overflow                                                                  [PASSED]
native_utmc:test_utmc Took 0.77 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_utmc    test_utmc  PASSED    00:00:00.774
================== 5 test cases: 5 succeeded in 00:00:00.774 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ocit in native_ocit environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ocit/test_ocit.cpp:71: test_build_and_parse                                                           [PASSED]
test/test_ocit/test_ocit.cpp:72: test_set_u16_helper                                                            [PASSED]
test/test_ocit/test_ocit.cpp:73: test_get_no_value                                                              [PASSED]
test/test_ocit/test_ocit.cpp:74: test_parse_rejects_short                                                       [PASSED]
native_ocit:test_ocit Took 0.81 seconds ----------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_ocit    test_ocit  PASSED    00:00:00.809
================== 4 test cases: 4 succeeded in 00:00:00.809 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_atc in native_atc environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_atc/test_atc.cpp:75: test_snapshot_json                                                               [PASSED]
test/test_atc/test_atc.cpp:76: test_set_output                                                                  [PASSED]
test/test_atc/test_atc.cpp:77: test_get                                                                         [PASSED]
test/test_atc/test_atc.cpp:78: test_empty_and_overflow                                                          [PASSED]
native_atc:test_atc Took 0.82 seconds ------------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_atc     test_atc  PASSED    00:00:00.819
================== 4 test cases: 4 succeeded in 00:00:00.819 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_southbound in native_southbound environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_southbound/test_southbound.cpp:161: test_register_and_find                                            [PASSED]
test/test_southbound/test_southbound.cpp:162: test_read_write_dispatch                                          [PASSED]
test/test_southbound/test_southbound.cpp:163: test_block_atomic                                                 [PASSED]
test/test_southbound/test_southbound.cpp:164: test_unsupported_capability                                       [PASSED]
test/test_southbound/test_southbound.cpp:165: test_registry_full                                                [PASSED]
native_southbound:test_southbound Took 0.80 seconds ----------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_southbound  test_southbound  PASSED    00:00:00.803
================== 5 test cases: 5 succeeded in 00:00:00.803 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_exc_decoder in native_exc_decoder environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_exc_decoder/test_exc_decoder.cpp:94: test_parse_full                                                  [PASSED]
test/test_exc_decoder/test_exc_decoder.cpp:95: test_json                                                        [PASSED]
test/test_exc_decoder/test_exc_decoder.cpp:96: test_backtrace_only_and_corrupted                                [PASSED]
test/test_exc_decoder/test_exc_decoder.cpp:97: test_garbage_returns_false                                       [PASSED]
test/test_exc_decoder/test_exc_decoder.cpp:98: test_json_omits_core_when_absent_and_overflow                    [PASSED]
native_exc_decoder:test_exc_decoder Took 0.82 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_exc_decoder  test_exc_decoder  PASSED    00:00:00.822
================== 5 test cases: 5 succeeded in 00:00:00.822 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_http_delivery in native_http_delivery environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_http_delivery/test_http_delivery.cpp:111: test_swr_decision                                           [PASSED]
test/test_http_delivery/test_http_delivery.cpp:112: test_cache_control                                          [PASSED]
test/test_http_delivery/test_http_delivery.cpp:113: test_range_forms                                            [PASSED]
test/test_http_delivery/test_http_delivery.cpp:114: test_range_rejects                                          [PASSED]
test/test_http_delivery/test_http_delivery.cpp:115: test_content_range                                          [PASSED]
test/test_http_delivery/test_http_delivery.cpp:116: test_sw_manifest                                            [PASSED]
native_http_delivery:test_http_delivery Took 0.83 seconds ----------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment           Test                Status    Duration
--------------------  ------------------  --------  ------------
native_http_delivery  test_http_delivery  PASSED    00:00:00.833
================== 6 test cases: 6 succeeded in 00:00:00.833 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_hw_health in native_hw_health environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_hw_health/test_hw_health.cpp:98: test_rail_monitor                                                    [PASSED]
test/test_hw_health/test_hw_health.cpp:99: test_spi_backoff                                                     [PASSED]
test/test_hw_health/test_hw_health.cpp:100: test_spi_backoff_clamps                                             [PASSED]
test/test_hw_health/test_hw_health.cpp:101: test_gpio_short                                                     [PASSED]
test/test_hw_health/test_hw_health.cpp:102: test_cap_leak                                                       [PASSED]
native_hw_health:test_hw_health Took 0.82 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_hw_health  test_hw_health  PASSED    00:00:00.823
================== 5 test cases: 5 succeeded in 00:00:00.823 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_mdns_adaptive in native_mdns_adaptive environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_mdns_adaptive/test_mdns_adaptive.cpp:66: test_refresh_interval                                        [PASSED]
test/test_mdns_adaptive/test_mdns_adaptive.cpp:67: test_backoff_and_recover                                     [PASSED]
test/test_mdns_adaptive/test_mdns_adaptive.cpp:68: test_due                                                     [PASSED]
test/test_mdns_adaptive/test_mdns_adaptive.cpp:69: test_presleep                                                [PASSED]
native_mdns_adaptive:test_mdns_adaptive Took 0.78 seconds ----------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment           Test                Status    Duration
--------------------  ------------------  --------  ------------
native_mdns_adaptive  test_mdns_adaptive  PASSED    00:00:00.776
================== 4 test cases: 4 succeeded in 00:00:00.776 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_sockpool in native_sockpool environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_sockpool/test_sockpool.cpp:90: test_acquire_free                                                      [PASSED]
test/test_sockpool/test_sockpool.cpp:91: test_lru_recycle                                                       [PASSED]
test/test_sockpool/test_sockpool.cpp:92: test_touch_changes_lru                                                 [PASSED]
test/test_sockpool/test_sockpool.cpp:93: test_release_reopens_free                                              [PASSED]
test/test_sockpool/test_sockpool.cpp:94: test_empty_pool_fails                                                  [PASSED]
native_sockpool:test_sockpool Took 0.80 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_sockpool  test_sockpool  PASSED    00:00:00.803
================== 5 test cases: 5 succeeded in 00:00:00.803 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_psram_pool in native_psram_pool environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_psram_pool/test_psram_pool.cpp:69: test_place_large_prefers_psram                                     [PASSED]
test/test_psram_pool/test_psram_pool.cpp:70: test_place_small_prefers_dram                                      [PASSED]
test/test_psram_pool/test_psram_pool.cpp:71: test_place_dma_forces_dram                                         [PASSED]
test/test_psram_pool/test_psram_pool.cpp:72: test_place_edges                                                   [PASSED]
test/test_psram_pool/test_psram_pool.cpp:73: test_pingpong                                                      [PASSED]
native_psram_pool:test_psram_pool Took 0.78 seconds ----------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_psram_pool  test_psram_pool  PASSED    00:00:00.781
================== 5 test cases: 5 succeeded in 00:00:00.781 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_happy_eyeballs in native_happy_eyeballs environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_happy_eyeballs/test_happy_eyeballs.cpp:71: test_pref_order                                            [PASSED]
test/test_happy_eyeballs/test_happy_eyeballs.cpp:72: test_order_and_interleave                                  [PASSED]
test/test_happy_eyeballs/test_happy_eyeballs.cpp:73: test_order_single_family                                   [PASSED]
test/test_happy_eyeballs/test_happy_eyeballs.cpp:74: test_attempt_due                                           [PASSED]
native_happy_eyeballs:test_happy_eyeballs Took 0.82 seconds --------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment            Test                 Status    Duration
---------------------  -------------------  --------  ------------
native_happy_eyeballs  test_happy_eyeballs  PASSED    00:00:00.816
================== 4 test cases: 4 succeeded in 00:00:00.816 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_wifi_sniffer in native_wifi_sniffer environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_wifi_sniffer/test_wifi_sniffer.cpp:98: test_parse_data                                                [PASSED]
test/test_wifi_sniffer/test_wifi_sniffer.cpp:99: test_parse_beacon                                              [PASSED]
test/test_wifi_sniffer/test_wifi_sniffer.cpp:100: test_parse_ctrl_short                                         [PASSED]
test/test_wifi_sniffer/test_wifi_sniffer.cpp:101: test_stats                                                    [PASSED]
test/test_wifi_sniffer/test_wifi_sniffer.cpp:102: test_roam                                                     [PASSED]
native_wifi_sniffer:test_wifi_sniffer Took 0.79 seconds ------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_wifi_sniffer  test_wifi_sniffer  PASSED    00:00:00.794
================== 5 test cases: 5 succeeded in 00:00:00.794 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_link_manager in native_link_manager environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_link_manager/test_link_manager.cpp:76: test_init_none_up                                              [PASSED]
test/test_link_manager/test_link_manager.cpp:77: test_escalation_and_failover                                   [PASSED]
test/test_link_manager/test_link_manager.cpp:78: test_tie_break_lower_index                                     [PASSED]
test/test_link_manager/test_link_manager.cpp:79: test_out_of_range_no_change                                    [PASSED]
native_link_manager:test_link_manager Took 0.79 seconds ------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_link_manager  test_link_manager  PASSED    00:00:00.787
================== 4 test cases: 4 succeeded in 00:00:00.787 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_cc1101 in native_cc1101 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_cc1101/test_cc1101.cpp:215: test_init_configures_and_detects                                          [PASSED]
test/test_cc1101/test_cc1101.cpp:216: test_init_fails_when_absent                                               [PASSED]
test/test_cc1101/test_cc1101.cpp:217: test_send_writes_fifo_and_strobes_tx                                      [PASSED]
test/test_cc1101/test_cc1101.cpp:218: test_send_rejects_bad_len                                                 [PASSED]
test/test_cc1101/test_cc1101.cpp:219: test_tx_done                                                              [PASSED]
test/test_cc1101/test_cc1101.cpp:220: test_set_rx                                                               [PASSED]
test/test_cc1101/test_cc1101.cpp:221: test_recv_reads_packet_and_rssi                                           [PASSED]
test/test_cc1101/test_cc1101.cpp:222: test_recv_empty                                                           [PASSED]
test/test_cc1101/test_cc1101.cpp:223: test_recv_truncates                                                       [PASSED]
test/test_cc1101/test_cc1101.cpp:224: test_rssi_decode                                                          [PASSED]
native_cc1101:test_cc1101 Took 0.79 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_cc1101  test_cc1101  PASSED    00:00:00.788
================= 10 test cases: 10 succeeded in 00:00:00.788 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_fdc2214 in native_fdc2214 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_fdc2214/test_fdc2214.cpp:62: test_data_combine                                                        [PASSED]
test/test_fdc2214/test_fdc2214.cpp:63: test_freq_scale                                                          [PASSED]
test/test_fdc2214/test_fdc2214.cpp:64: test_build_config                                                        [PASSED]
test/test_fdc2214/test_fdc2214.cpp:65: test_build_config_too_small                                              [PASSED]
native_fdc2214:test_fdc2214 Took 0.78 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_fdc2214  test_fdc2214  PASSED    00:00:00.780
================== 4 test cases: 4 succeeded in 00:00:00.780 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ldc1614 in native_ldc1614 environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ldc1614/test_ldc1614.cpp:54: test_data_combine                                                        [PASSED]
test/test_ldc1614/test_ldc1614.cpp:55: test_freq_scale                                                          [PASSED]
test/test_ldc1614/test_ldc1614.cpp:56: test_build_config                                                        [PASSED]
test/test_ldc1614/test_ldc1614.cpp:57: test_build_config_too_small                                              [PASSED]
native_ldc1614:test_ldc1614 Took 0.79 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_ldc1614  test_ldc1614  PASSED    00:00:00.794
================== 4 test cases: 4 succeeded in 00:00:00.794 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_vl53l0x in native_vl53l0x environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_vl53l0x/test_vl53l0x.cpp:45: test_range_mm                                                            [PASSED]
test/test_vl53l0x/test_vl53l0x.cpp:46: test_data_ready                                                          [PASSED]
test/test_vl53l0x/test_vl53l0x.cpp:47: test_range_status                                                        [PASSED]
native_vl53l0x:test_vl53l0x Took 0.77 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_vl53l0x  test_vl53l0x  PASSED    00:00:00.765
================== 3 test cases: 3 succeeded in 00:00:00.765 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_radio_sniff in native_radio_sniff environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_radio_sniff/test_radio_sniff.cpp:81: test_i2f32                                                       [PASSED]
test/test_radio_sniff/test_radio_sniff.cpp:82: test_global_header                                               [PASSED]
test/test_radio_sniff/test_radio_sniff.cpp:83: test_tap_record                                                  [PASSED]
test/test_radio_sniff/test_radio_sniff.cpp:84: test_tap_record_overflow                                         [PASSED]
native_radio_sniff:test_radio_sniff Took 0.77 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_radio_sniff  test_radio_sniff  PASSED    00:00:00.771
================== 4 test cases: 4 succeeded in 00:00:00.771 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ble_gatt in native_ble_gatt environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ble_gatt/test_ble_gatt.cpp:94: test_build_pdus                                                        [PASSED]
test/test_ble_gatt/test_ble_gatt.cpp:95: test_build_overflow                                                    [PASSED]
test/test_ble_gatt/test_ble_gatt.cpp:96: test_parse                                                             [PASSED]
test/test_ble_gatt/test_ble_gatt.cpp:97: test_char_json                                                         [PASSED]
native_ble_gatt:test_ble_gatt Took 0.79 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_ble_gatt  test_ble_gatt  PASSED    00:00:00.792
================== 4 test cases: 4 succeeded in 00:00:00.792 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_tls_policy in native_tls_policy environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_tls_policy/test_tls_policy.cpp:68: test_negotiate_version                                             [PASSED]
test/test_tls_policy/test_tls_policy.cpp:69: test_version_name                                                  [PASSED]
test/test_tls_policy/test_tls_policy.cpp:70: test_select_cipher                                                 [PASSED]
test/test_tls_policy/test_tls_policy.cpp:71: test_is_aead                                                       [PASSED]
native_tls_policy:test_tls_policy Took 0.77 seconds ----------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_tls_policy  test_tls_policy  PASSED    00:00:00.774
================== 4 test cases: 4 succeeded in 00:00:00.774 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_wisun in native_wisun environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_wisun/test_wisun.cpp:140: test_build_coap_get                                                         [PASSED]
test/test_wisun/test_wisun.cpp:141: test_build_coap_put_with_token_and_payload                                  [PASSED]
test/test_wisun/test_wisun.cpp:142: test_build_coap_long_segment_extended_length                                [PASSED]
test/test_wisun/test_wisun.cpp:143: test_build_coap_rejects_bad_args                                            [PASSED]
test/test_wisun/test_wisun.cpp:144: test_node_registry                                                          [PASSED]
test/test_wisun/test_wisun.cpp:145: test_registry_full_and_misses                                               [PASSED]
native_wisun:test_wisun Took 0.84 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_wisun   test_wisun  PASSED    00:00:00.835
================== 6 test cases: 6 succeeded in 00:00:00.835 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_logbuf in native_logbuf environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_logbuf/test_logbuf.cpp:81: test_append_and_order                                                      [PASSED]
test/test_logbuf/test_logbuf.cpp:82: test_dump                                                                  [PASSED]
test/test_logbuf/test_logbuf.cpp:83: test_rotation_drops_oldest                                                 [PASSED]
test/test_logbuf/test_logbuf.cpp:84: test_trap_threshold                                                        [PASSED]
native_logbuf:test_logbuf Took 0.79 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_logbuf  test_logbuf  PASSED    00:00:00.788
================== 4 test cases: 4 succeeded in 00:00:00.788 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_config_io in native_config_io environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_config_io/test_config_io.cpp:81: test_export_format                                                   [PASSED]
test/test_config_io/test_config_io.cpp:82: test_round_trip                                                      [PASSED]
test/test_config_io/test_config_io.cpp:83: test_import_skips_unknown_keys                                       [PASSED]
test/test_config_io/test_config_io.cpp:84: test_export_overflow_fails_closed                                    [PASSED]
native_config_io:test_config_io Took 0.85 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_config_io  test_config_io  PASSED    00:00:00.845
================== 4 test cases: 4 succeeded in 00:00:00.845 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_workers in native_workers environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_workers/test_workers.cpp:62: test_worker_count_is_two                                                 [PASSED]
test/test_workers/test_workers.cpp:63: test_check_timeouts_reaps_only_owned_slots                               [PASSED]
test/test_workers/test_workers.cpp:64: test_pool_init_defaults_owner_zero                                       [PASSED]
native_workers:test_workers Took 0.93 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_workers  test_workers  PASSED    00:00:00.932
================== 3 test cases: 3 succeeded in 00:00:00.932 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_clock in native_clock environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_clock/test_clock.cpp:126: test_default_is_platform_millis                                             [PASSED]
test/test_clock/test_clock.cpp:127: test_custom_clock_divides_to_1000hz                                         [PASSED]
test/test_clock/test_clock.cpp:128: test_sub_khz_source_not_divided                                             [PASSED]
test/test_clock/test_clock.cpp:129: test_revert_to_default                                                      [PASSED]
test/test_clock/test_clock.cpp:130: test_micros_custom_divides_to_1mhz                                          [PASSED]
test/test_clock/test_clock.cpp:131: test_latency_stat_records_and_budgets                                       [PASSED]
test/test_clock/test_clock.cpp:132: test_latency_budget_zero_disables                                           [PASSED]
native_clock:test_clock Took 0.77 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_clock   test_clock  PASSED    00:00:00.771
================== 7 test cases: 7 succeeded in 00:00:00.771 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_concurrency in native_concurrency environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_concurrency/test_concurrency.cpp:109: test_spsc_ring_no_race                                          [PASSED]
test/test_concurrency/test_concurrency.cpp:110: test_state_handoff_no_race                                      [PASSED]
native_concurrency:test_concurrency Took 0.93 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_concurrency  test_concurrency  PASSED    00:00:00.927
================== 2 test cases: 2 succeeded in 00:00:00.927 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_concurrency in native_tsan environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_concurrency/test_concurrency.cpp:109: test_spsc_ring_no_race                                          [PASSED]
test/test_concurrency/test_concurrency.cpp:110: test_state_handoff_no_race                                      [PASSED]
native_tsan:test_concurrency Took 1.48 seconds ---------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test              Status    Duration
-------------  ----------------  --------  ------------
native_tsan    test_concurrency  PASSED    00:00:01.483
================== 2 test cases: 2 succeeded in 00:00:01.483 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_qpack in native_qpack environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_qpack/test_qpack.cpp:208: test_appendix_b1_decode                                                     [PASSED]
test/test_qpack/test_qpack.cpp:209: test_encode_indexed                                                         [PASSED]
test/test_qpack/test_qpack.cpp:210: test_encode_nameref_roundtrip                                               [PASSED]
test/test_qpack/test_qpack.cpp:211: test_literal_name                                                           [PASSED]
test/test_qpack/test_qpack.cpp:212: test_full_section                                                           [PASSED]
test/test_qpack/test_qpack.cpp:213: test_reject_dynamic                                                         [PASSED]
test/test_qpack/test_qpack.cpp:214: test_encode_edges                                                           [PASSED]
test/test_qpack/test_qpack.cpp:215: test_decode_errors                                                          [PASSED]
test/test_qpack/test_qpack.cpp:216: test_value_string_paths                                                     [PASSED]
native_qpack:test_qpack Took 0.91 seconds --------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_qpack   test_qpack  PASSED    00:00:00.912
================== 9 test cases: 9 succeeded in 00:00:00.912 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_quic_packet in native_quic_packet environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_quic_packet/test_quic_packet.cpp:162: test_long_header_roundtrip                                      [PASSED]
test/test_quic_packet/test_quic_packet.cpp:163: test_version_negotiation                                        [PASSED]
test/test_quic_packet/test_quic_packet.cpp:164: test_short_header_parse                                         [PASSED]
test/test_quic_packet/test_quic_packet.cpp:165: test_pn_encode                                                  [PASSED]
test/test_quic_packet/test_quic_packet.cpp:166: test_pn_decode                                                  [PASSED]
test/test_quic_packet/test_quic_packet.cpp:167: test_reject                                                     [PASSED]
test/test_quic_packet/test_quic_packet.cpp:168: test_build_guards                                               [PASSED]
test/test_quic_packet/test_quic_packet.cpp:169: test_short_header_guards                                        [PASSED]
native_quic_packet:test_quic_packet Took 0.79 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_quic_packet  test_quic_packet  PASSED    00:00:00.793
================== 8 test cases: 8 succeeded in 00:00:00.793 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_quic_frame in native_quic_frame environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_quic_frame/test_quic_frame.cpp:201: test_simple_frames                                                [PASSED]
test/test_quic_frame/test_quic_frame.cpp:202: test_ack                                                          [PASSED]
test/test_quic_frame/test_quic_frame.cpp:203: test_crypto                                                       [PASSED]
test/test_quic_frame/test_quic_frame.cpp:204: test_stream                                                       [PASSED]
test/test_quic_frame/test_quic_frame.cpp:205: test_max_data_and_close                                           [PASSED]
test/test_quic_frame/test_quic_frame.cpp:206: test_sequence_and_truncation                                      [PASSED]
test/test_quic_frame/test_quic_frame.cpp:207: test_builder_overflow                                             [PASSED]
test/test_quic_frame/test_quic_frame.cpp:208: test_parse_errors                                                 [PASSED]
native_quic_frame:test_quic_frame Took 0.81 seconds ----------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_quic_frame  test_quic_frame  PASSED    00:00:00.813
================== 8 test cases: 8 succeeded in 00:00:00.813 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_quic_crypto in native_quic_crypto environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_quic_crypto/test_quic_crypto.cpp:262: test_aes128_block_fips197                                       [PASSED]
test/test_quic_crypto/test_quic_crypto.cpp:263: test_aes128_gcm_testcase4                                       [PASSED]
test/test_quic_crypto/test_quic_crypto.cpp:264: test_initial_secrets_appendix_a1                                [PASSED]
test/test_quic_crypto/test_quic_crypto.cpp:265: test_server_initial_a3                                          [PASSED]
test/test_quic_crypto/test_quic_crypto.cpp:266: test_client_initial_a2                                          [PASSED]
test/test_quic_crypto/test_quic_crypto.cpp:267: test_retry_integrity_a4                                         [PASSED]
test/test_quic_crypto/test_quic_crypto.cpp:268: test_gcm_open_rejects_short                                     [PASSED]
native_quic_crypto:test_quic_crypto Took 0.93 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_quic_crypto  test_quic_crypto  PASSED    00:00:00.932
================== 7 test cases: 7 succeeded in 00:00:00.932 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_tls13_kdf in native_tls13_kdf environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_tls13_kdf/test_tls13_kdf.cpp:205: test_early_secret                                                   [PASSED]
test/test_tls13_kdf/test_tls13_kdf.cpp:206: test_handshake_secrets                                              [PASSED]
test/test_tls13_kdf/test_tls13_kdf.cpp:207: test_master_secrets                                                 [PASSED]
test/test_tls13_kdf/test_tls13_kdf.cpp:208: test_server_hs_write_keys                                           [PASSED]
test/test_tls13_kdf/test_tls13_kdf.cpp:209: test_server_finished                                                [PASSED]
native_tls13_kdf:test_tls13_kdf Took 0.87 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_tls13_kdf  test_tls13_kdf  PASSED    00:00:00.874
================== 5 test cases: 5 succeeded in 00:00:00.874 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_quic_tp in native_quic_tp environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_quic_tp/test_quic_tp.cpp:155: test_defaults                                                           [PASSED]
test/test_quic_tp/test_quic_tp.cpp:156: test_roundtrip                                                          [PASSED]
test/test_quic_tp/test_quic_tp.cpp:157: test_parse_bytes                                                        [PASSED]
test/test_quic_tp/test_quic_tp.cpp:158: test_skip_unknown                                                       [PASSED]
test/test_quic_tp/test_quic_tp.cpp:159: test_reject_duplicate                                                   [PASSED]
test/test_quic_tp/test_quic_tp.cpp:160: test_reject_oversized_cid                                               [PASSED]
test/test_quic_tp/test_quic_tp.cpp:161: test_reject_bad_values                                                  [PASSED]
native_quic_tp:test_quic_tp Took 0.87 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_quic_tp  test_quic_tp  PASSED    00:00:00.870
================== 7 test cases: 7 succeeded in 00:00:00.870 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_tls13_msg in native_tls13_msg environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_tls13_msg/test_tls13_msg.cpp:206: test_parse_client_hello                                             [PASSED]
test/test_tls13_msg/test_tls13_msg.cpp:207: test_build_server_hello                                             [PASSED]
test/test_tls13_msg/test_tls13_msg.cpp:208: test_build_certificate                                              [PASSED]
test/test_tls13_msg/test_tls13_msg.cpp:209: test_build_finished                                                 [PASSED]
test/test_tls13_msg/test_tls13_msg.cpp:210: test_encrypted_extensions                                           [PASSED]
test/test_tls13_msg/test_tls13_msg.cpp:211: test_cert_verify_content                                            [PASSED]
test/test_tls13_msg/test_tls13_msg.cpp:212: test_cert_verify_sign_roundtrip                                     [PASSED]
native_tls13_msg:test_tls13_msg Took 0.97 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_tls13_msg  test_tls13_msg  PASSED    00:00:00.972
================== 7 test cases: 7 succeeded in 00:00:00.972 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_quic_tls in native_quic_tls environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_quic_tls/test_quic_tls.cpp:406: test_full_handshake_roundtrip                                         [PASSED]
test/test_quic_tls/test_quic_tls.cpp:407: test_reject_bad_client_finished                                       [PASSED]
test/test_quic_tls/test_quic_tls.cpp:408: test_reject_no_h3_alpn                                                [PASSED]
test/test_quic_tls/test_quic_tls.cpp:409: test_partial_client_hello                                             [PASSED]
test/test_quic_tls/test_quic_tls.cpp:410: test_reject_no_tls13                                                  [PASSED]
test/test_quic_tls/test_quic_tls.cpp:411: test_reject_no_key_share                                              [PASSED]
test/test_quic_tls/test_quic_tls.cpp:412: test_reject_no_x25519_group                                           [PASSED]
test/test_quic_tls/test_quic_tls.cpp:413: test_reject_no_ed25519                                                [PASSED]
test/test_quic_tls/test_quic_tls.cpp:414: test_reject_no_transport_params                                       [PASSED]
test/test_quic_tls/test_quic_tls.cpp:415: test_reject_bad_transport_params                                      [PASSED]
test/test_quic_tls/test_quic_tls.cpp:416: test_reject_malformed_client_hello                                    [PASSED]
native_quic_tls:test_quic_tls Took 1.26 seconds --------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_quic_tls  test_quic_tls  PASSED    00:00:01.263
================= 11 test cases: 11 succeeded in 00:00:01.263 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_quic_conn in native_quic_conn environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_quic_conn/test_quic_conn.cpp:670: test_full_handshake_and_stream                                      [PASSED]
test/test_quic_conn/test_quic_conn.cpp:671: test_pto_retransmits_flight                                         [PASSED]
test/test_quic_conn/test_quic_conn.cpp:672: test_connection_close_api                                           [PASSED]
test/test_quic_conn/test_quic_conn.cpp:673: test_connection_close_on_malformed_frame                            [PASSED]
native_quic_conn:test_quic_conn Took 1.36 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_quic_conn  test_quic_conn  PASSED    00:00:01.360
================== 4 test cases: 4 succeeded in 00:00:01.360 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_h3_conn in native_h3_conn environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_h3_conn/test_h3_conn.cpp:354: test_request_dispatch_and_response                                      [PASSED]
test/test_h3_conn/test_h3_conn.cpp:355: test_post_with_body                                                     [PASSED]
test/test_h3_conn/test_h3_conn.cpp:356: test_control_stream_settings_sent                                       [PASSED]
test/test_h3_conn/test_h3_conn.cpp:357: test_client_control_stream_settings                                     [PASSED]
test/test_h3_conn/test_h3_conn.cpp:358: test_client_uni_stream_types                                            [PASSED]
test/test_h3_conn/test_h3_conn.cpp:359: test_handshake_done_idempotent                                          [PASSED]
test/test_h3_conn/test_h3_conn.cpp:360: test_malformed_request_frame                                            [PASSED]
test/test_h3_conn/test_h3_conn.cpp:361: test_respond_body_too_large                                             [PASSED]
test/test_h3_conn/test_h3_conn.cpp:362: test_stream_pool_full                                                   [PASSED]
test/test_h3_conn/test_h3_conn.cpp:363: test_uni_stream_partial_type                                            [PASSED]
test/test_h3_conn/test_h3_conn.cpp:364: test_overlong_field_truncated                                           [PASSED]
native_h3_conn:test_h3_conn Took 1.30 seconds ----------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_h3_conn  test_h3_conn  PASSED    00:00:01.297
================= 11 test cases: 11 succeeded in 00:00:01.297 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_h3_e2e in native_h3_e2e environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_h3_e2e/test_h3_e2e.cpp:402: test_http3_get_end_to_end                                                 [PASSED]
native_h3_e2e:test_h3_e2e Took 1.34 seconds ------------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_h3_e2e  test_h3_e2e  PASSED    00:00:01.344
================== 1 test cases: 1 succeeded in 00:00:01.344 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_quic_server in native_quic_server environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_quic_server/test_quic_server.cpp:476: test_quic_server_http3_get                                      [PASSED]
test/test_quic_server/test_quic_server.cpp:477: test_idle_connection_reaped                                     [PASSED]
native_quic_server:test_quic_server Took 1.41 seconds --------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_quic_server  test_quic_server  PASSED    00:00:01.410
================== 2 test cases: 2 succeeded in 00:00:01.410 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_h3_server in native_h3_server environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
src/dwserver.cpp: In member function ‘void DetWebServer::serve_file_internal(uint8_t, bool, fs::FS&, const char*, const char*, const char*)’:
src/dwserver.cpp:3138:76: warning: ‘snprintf’ output may be truncated before the last format character [-Wformat-truncation=]
 3138 |         snprintf(lastmod_line, sizeof(lastmod_line), "Last-Modified: %s\r\n", lm_date);
      |                                                                            ^
src/dwserver.cpp:3138:17: note: ‘snprintf’ output between 18 and 57 bytes into a destination of size 56
 3138 |         snprintf(lastmod_line, sizeof(lastmod_line), "Last-Modified: %s\r\n", lm_date);
      |         ~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Testing...
test/test_h3_server/test_h3_server.cpp:417: test_h3_request_served_by_route                                     [PASSED]
native_h3_server:test_h3_server Took 2.22 seconds ------------------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_h3_server  test_h3_server  PASSED    00:00:02.218
================== 1 test cases: 1 succeeded in 00:00:02.218 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 229 tests

Processing test_ssh_chachapoly in native_ssh_chachapoly environment
------------------------------------------------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_chachapoly/test_ssh_chachapoly.cpp:128: test_chacha20_block_rfc8439                               [PASSED]
test/test_ssh_chachapoly/test_ssh_chachapoly.cpp:129: test_poly1305_rfc8439                                     [PASSED]
test/test_ssh_chachapoly/test_ssh_chachapoly.cpp:130: test_chachapoly_roundtrip                                 [PASSED]
test/test_ssh_chachapoly/test_ssh_chachapoly.cpp:131: test_chachapoly_tamper_rejected                           [PASSED]
native_ssh_chachapoly:test_ssh_chachapoly Took 0.85 seconds --------------------------------------------------- [PASSED]

=================================== SUMMARY ===================================
Environment            Test                 Status    Duration
---------------------  -------------------  --------  ------------
native_ssh_chachapoly  test_ssh_chachapoly  PASSED    00:00:00.850
================== 4 test cases: 4 succeeded in 00:00:00.850 ==================
```

</details>
