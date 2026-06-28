# Test Report

**Generated:** 2026-06-28 10:19:29
**Command:** `pio test` over 62 auto-discovered native envs (excludes native_pentest, native_codeql)
**Result:** ✅ 1172 passed - 109s

---

## Summary

| Suite                    | Environment            | Tests | Status |     Duration |
| :----------------------- | :--------------------- | ----: | :----: | -----------: |
| `test_sse`               | `native`               |    37 |   ✅   | 00:00:15.407 |
| `test_session`           | `native`               |    19 |   ✅   | 00:00:00.530 |
| `test_presentation`      | `native`               |    63 |   ✅   | 00:00:00.568 |
| `test_transport`         | `native`               |    42 |   ✅   | 00:00:00.556 |
| `test_websocket`         | `native`               |    63 |   ✅   | 00:00:00.577 |
| `test_http_parser`       | `native`               |    82 |   ✅   | 00:00:00.532 |
| `test_observability`     | `native_observability` |    15 |   ✅   | 00:00:00.673 |
| `test_http_ota`          | `native_ota`           |     3 |   ✅   | 00:00:00.634 |
| `test_provisioning`      | `native_prov`          |     5 |   ✅   | 00:00:00.679 |
| `test_ssh_crypto`        | `native_ssh`           |    38 |   ✅   | 00:00:03.828 |
| `test_ssh_auth`          | `native_ssh`           |    12 |   ✅   | 00:00:00.548 |
| `test_ssh_server`        | `native_ssh`           |     7 |   ✅   | 00:00:00.655 |
| `test_ssh_transport`     | `native_ssh`           |    23 |   ✅   | 00:00:00.866 |
| `test_ssh_channel`       | `native_ssh`           |    12 |   ✅   | 00:00:00.516 |
| `test_ssh_hardening`     | `native_ssh_hardened`  |     2 |   ✅   | 00:00:00.864 |
| `test_ssh_conn`          | `native_ssh_conn`      |     2 |   ✅   | 00:00:01.224 |
| `test_regex`             | `native_app`           |     9 |   ✅   | 00:00:01.078 |
| `test_template`          | `native_app`           |     6 |   ✅   | 00:00:00.552 |
| `test_path_params`       | `native_app`           |     8 |   ✅   | 00:00:00.550 |
| `test_digest_vectors`    | `native_app`           |     4 |   ✅   | 00:00:00.492 |
| `test_form_params`       | `native_app`           |     5 |   ✅   | 00:00:00.552 |
| `test_iface`             | `native_app`           |     7 |   ✅   | 00:00:00.552 |
| `test_json`              | `native_app`           |    17 |   ✅   | 00:00:00.508 |
| `test_response_headers`  | `native_app`           |     9 |   ✅   | 00:00:00.554 |
| `test_middleware`        | `native_app`           |     9 |   ✅   | 00:00:00.562 |
| `test_digest_auth`       | `native_app`           |     5 |   ✅   | 00:00:00.561 |
| `test_web_terminal`      | `native_app`           |     7 |   ✅   | 00:00:00.552 |
| `test_defer`             | `native_app`           |     3 |   ✅   | 00:00:00.533 |
| `test_multipart`         | `native_app`           |    19 |   ✅   | 00:00:00.565 |
| `test_auth`              | `native_app`           |    13 |   ✅   | 00:00:00.559 |
| `test_file_serving`      | `native_app`           |    12 |   ✅   | 00:00:00.572 |
| `test_dispatch`          | `native_app`           |    11 |   ✅   | 00:00:00.556 |
| `test_chunked`           | `native_app`           |     8 |   ✅   | 00:00:00.551 |
| `test_application`       | `native_app`           |    50 |   ✅   | 00:00:00.647 |
| `test_snmp_ber`          | `native_snmp`          |     9 |   ✅   | 00:00:00.669 |
| `test_snmp_agent`        | `native_snmp`          |    13 |   ✅   | 00:00:00.481 |
| `test_snmp_v3`           | `native_snmp_v3`       |     9 |   ✅   | 00:00:01.362 |
| `test_telnet`            | `native_telnet`        |     8 |   ✅   | 00:00:00.739 |
| `test_coap`              | `native_coap`          |    26 |   ✅   | 00:00:00.664 |
| `test_webdav`            | `native_webdav`        |    11 |   ✅   | 00:00:00.621 |
| `test_modbus`            | `native_modbus`        |    13 |   ✅   | 00:00:00.639 |
| `test_modbus_master`     | `native_modbus_master` |     5 |   ✅   | 00:00:00.648 |
| `test_ota_rollback`      | `native_ota_rollback`  |     5 |   ✅   | 00:00:00.624 |
| `test_totp`              | `native_totp`          |     4 |   ✅   | 00:00:00.652 |
| `test_webhook`           | `native_webhook`       |     5 |   ✅   | 00:00:00.628 |
| `test_radio_power`       | `native_radio_power`   |     2 |   ✅   | 00:00:00.628 |
| `test_dns_resolver`      | `native_dns_resolver`  |     4 |   ✅   | 00:00:00.626 |
| `test_audit_log`         | `native_audit_log`     |    12 |   ✅   | 00:00:00.644 |
| `test_oidc`              | `native_oidc`          |    13 |   ✅   | 00:00:00.797 |
| `test_vfs`               | `native_vfs`           |    11 |   ✅   | 00:00:00.633 |
| `test_graphql`           | `native_graphql`       |    13 |   ✅   | 00:00:00.636 |
| `test_espnow`            | `native_espnow`        |     7 |   ✅   | 00:00:00.627 |
| `test_oauth2`            | `native_oauth2`        |     8 |   ✅   | 00:00:00.662 |
| `test_opcua`             | `native_opcua`         |    29 |   ✅   | 00:00:00.667 |
| `test_opcua_client`      | `native_opcua_client`  |    11 |   ✅   | 00:00:00.676 |
| `test_keepalive`         | `native_keepalive`     |    10 |   ✅   | 00:00:01.007 |
| `test_range`             | `native_range`         |    10 |   ✅   | 00:00:01.014 |
| `test_syslog`            | `native_syslog`        |     6 |   ✅   | 00:00:00.639 |
| `test_jwt`               | `native_jwt`           |    10 |   ✅   | 00:00:00.675 |
| `test_upload`            | `native_upload`        |     3 |   ✅   | 00:00:01.053 |
| `test_http_client`       | `native_http_client`   |    10 |   ✅   | 00:00:00.637 |
| `test_compliance`        | `native_compliance`    |    12 |   ✅   | 00:00:00.636 |
| `test_mqtt`              | `native_mqtt`          |    16 |   ✅   | 00:00:00.638 |
| `test_ws_client`         | `native_ws_client`     |    10 |   ✅   | 00:00:00.667 |
| `test_scratch`           | `native_scratch`       |    14 |   ✅   | 00:00:00.674 |
| `test_snmp_trap`         | `native_snmp_trap`     |     3 |   ✅   | 00:00:00.653 |
| `test_inflate`           | `native_inflate`       |    12 |   ✅   | 00:00:00.632 |
| `test_deflate`           | `native_deflate`       |    10 |   ✅   | 00:00:00.665 |
| `test_websocket`         | `native_ws_deflate`    |    67 |   ✅   | 00:00:01.003 |
| `test_time_source`       | `native_time_source`   |     9 |   ✅   | 00:00:00.629 |
| `test_config_store`      | `native_config_store`  |    14 |   ✅   | 00:00:00.633 |
| `test_device_id`         | `native_device_id`     |     4 |   ✅   | 00:00:00.659 |
| `test_auth_lockout`      | `native_auth_lockout`  |    10 |   ✅   | 00:00:00.640 |
| `test_csrf`              | `native_csrf`          |     9 |   ✅   | 00:00:00.672 |
| `test_telemetry`         | `native_telemetry`     |     8 |   ✅   | 00:00:00.670 |
| `test_dashboard`         | `native_dashboard`     |    11 |   ✅   | 00:00:00.670 |
| `test_net_egress`        | `native_net_egress`    |     5 |   ✅   | 00:00:00.646 |
| `test_partition_monitor` | `native_partition`     |     5 |   ✅   | 00:00:00.624 |
| `test_cbor`              | `native_cbor`          |    14 |   ✅   | 00:00:00.634 |
| `test_msgpack`           | `native_msgpack`       |     8 |   ✅   | 00:00:00.643 |
| `test_gpio_map`          | `native_gpio_map`      |     8 |   ✅   | 00:00:00.639 |
| `test_udp_telemetry`     | `native_udp_telemetry` |     4 |   ✅   | 00:00:00.629 |
| `test_guardrails`        | `native_guardrails`    |     6 |   ✅   | 00:00:00.623 |
| `test_logbuf`            | `native_logbuf`        |     4 |   ✅   | 00:00:00.628 |
| `test_config_io`         | `native_config_io`     |     4 |   ✅   | 00:00:00.655 |
| `test_workers`           | `native_workers`       |     3 |   ✅   | 00:00:00.724 |
| `test_clock`             | `native_clock`         |     4 |   ✅   | 00:00:00.615 |
| `test_concurrency`       | `native_concurrency`   |     2 |   ✅   | 00:00:00.862 |
| `test_concurrency`       | `native_tsan`          |     2 |   ✅   | 00:00:01.299 |

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

## test_transport - ✅ 42 passed

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
|  34 | `test_per_ip_throttle_handles_rollover`          |   ✅   | Per ip throttle handles rollover                 |
|  35 | `test_ip_allowlist_empty_allows_all`             |   ✅   | Ip allowlist empty allows all                    |
|  36 | `test_ip_allowlist_host_match`                   |   ✅   | Ip allowlist host match                          |
|  37 | `test_ip_allowlist_cidr_match`                   |   ✅   | Ip allowlist cidr match                          |
|  38 | `test_ip_allowlist_masks_host_bits`              |   ✅   | Ip allowlist masks host bits                     |
|  39 | `test_ip_allowlist_multiple_rules`               |   ✅   | Ip allowlist multiple rules                      |
|  40 | `test_ip_allowlist_zero_prefix_matches_all`      |   ✅   | Ip allowlist zero prefix matches all             |
|  41 | `test_ip_allowlist_rejects_bad_prefix`           |   ✅   | Ip allowlist rejects bad prefix                  |
|  42 | `test_ip_allowlist_table_full`                   |   ✅   | Ip allowlist table full                          |

</details>

---

## test_websocket - ✅ 63 passed

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
|  51 | `test_ws_continuation_without_start_rejected`          |   ✅   | CONTINUATION with no message in progress (RFC 6455 §5.4) → 1002.          |
|  52 | `test_ws_new_data_frame_during_fragmentation_rejected` |   ✅   | A second TEXT (new message) before finishing the first is illegal.        |
|  53 | `test_ws_parse_ping_auto_pong_resets_frame`            |   ✅   | FIN=1, PING=0x09: byte0 = 0x89                                            |
|  54 | `test_ws_parse_pong_silently_ignored`                  |   ✅   | FIN=1, PONG=0x0A: byte0 = 0x8A                                            |
|  55 | `test_ws_parse_close_marks_ws_closed`                  |   ✅   | FIN=1, CLOSE=0x08: byte0 = 0x88                                           |
|  56 | `test_ws_parse_stops_at_frame_ready`                   |   ✅   | Push two complete frames -- parser should stop after the first            |
|  57 | `test_ws_reset_frame_clears_fields`                    |   ✅   | Ws reset frame clears fields                                              |
|  58 | `test_ws_parse_mask_applied_correctly`                 |   ✅   | Frame with mask key {0x37, 0xFA, 0x21, 0x3D}, payload 'H' XOR 0x37 = 0x7F |
|  59 | `stress_ws_parse_reset_100_cycles`                     |   ✅   | Stress - Ws parse reset 100 cycles                                        |
|  60 | `stress_ws_alloc_free_pool_cycle`                      |   ✅   | Stress - Ws alloc free pool cycle                                         |
|  61 | `stress_ws_parse_incremental_byte_by_byte`             |   ✅   | Stress - Ws parse incremental byte by byte                                |
|  62 | `stress_ws_parse_max_payload`                          |   ✅   | Stress - Ws parse max payload                                             |
|  63 | `stress_ws_parse_two_consecutive_frames`               |   ✅   | First frame                                                               |

</details>

---

## test_http_parser - ✅ 82 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Comprehensive unit tests for the standalone HTTP/1.1 parser._

|   # | Test                                                     | Status | Description                                                                     |
| --: | :------------------------------------------------------- | :----: | :------------------------------------------------------------------------------ |
|   1 | `test_reset_sets_parse_method_state`                     |   ✅   | Reset sets parse method state                                                   |
|   2 | `test_reset_preserves_slot_id`                           |   ✅   | Reset preserves slot id                                                         |
|   3 | `test_reset_clears_method`                               |   ✅   | Reset clears method                                                             |
|   4 | `test_reset_clears_path`                                 |   ✅   | Reset clears path                                                               |
|   5 | `test_reset_clears_header_count`                         |   ✅   | Reset clears header count                                                       |
|   6 | `test_reset_clears_body`                                 |   ✅   | Reset clears body                                                               |
|   7 | `test_reset_clears_query_count`                          |   ✅   | Reset clears query count                                                        |
|   8 | `test_feed_after_complete_does_not_change_state`         |   ✅   | Feed after complete does not change state                                       |
|   9 | `test_feed_after_error_does_not_change_state`            |   ✅   | Feed after error does not change state                                          |
|  10 | `test_feed_after_entity_too_large_does_not_change_state` |   ✅   | Feed after entity too large does not change state                               |
|  11 | `test_method_get`                                        |   ✅   | Method get                                                                      |
|  12 | `test_method_post`                                       |   ✅   | Method post                                                                     |
|  13 | `test_method_put`                                        |   ✅   | Method put                                                                      |
|  14 | `test_method_delete`                                     |   ✅   | Method delete                                                                   |
|  15 | `test_method_patch`                                      |   ✅   | Method patch                                                                    |
|  16 | `test_method_head`                                       |   ✅   | Method head                                                                     |
|  17 | `test_method_options`                                    |   ✅   | Method options                                                                  |
|  18 | `test_method_overflow_is_error`                          |   ✅   | More than 7 chars (sizeof method - 1) before a space → PARSE_ERROR              |
|  19 | `test_path_root`                                         |   ✅   | Path root                                                                       |
|  20 | `test_path_segments`                                     |   ✅   | Path segments                                                                   |
|  21 | `test_path_without_query`                                |   ✅   | Path without query                                                              |
|  22 | `test_path_overflow_is_414`                              |   ✅   | Build a path longer than MAX_PATH_LEN                                           |
|  23 | `test_single_query_param`                                |   ✅   | Single query param                                                              |
|  24 | `test_two_query_params`                                  |   ✅   | Two query params                                                                |
|  25 | `test_query_key_not_found_returns_null`                  |   ✅   | Query key not found returns null                                                |
|  26 | `test_query_empty_value`                                 |   ✅   | Query empty value                                                               |
|  27 | `test_single_header_stored`                              |   ✅   | Single header stored                                                            |
|  28 | `test_header_lookup_case_insensitive`                    |   ✅   | Header lookup case insensitive                                                  |
|  29 | `test_header_leading_space_stripped`                     |   ✅   | Header leading space stripped                                                   |
|  30 | `test_content_length_header_parsed`                      |   ✅   | Content length header parsed                                                    |
|  31 | `test_content_length_in_headers_array`                   |   ✅   | Content length in headers array                                                 |
|  32 | `test_multiple_headers_stored`                           |   ✅   | Multiple headers stored                                                         |
|  33 | `test_missing_header_returns_null`                       |   ✅   | Missing header returns null                                                     |
|  34 | `test_get_no_body_completes`                             |   ✅   | Get no body completes                                                           |
|  35 | `test_post_with_body`                                    |   ✅   | Post with body                                                                  |
|  36 | `test_put_with_body`                                     |   ✅   | Put with body                                                                   |
|  37 | `test_body_starting_with_newline`                        |   ✅   | Body starting with newline                                                      |
|  38 | `test_post_content_length_zero`                          |   ✅   | Post content length zero                                                        |
|  39 | `test_body_exactly_at_buffer_limit`                      |   ✅   | Body of exactly BODY_BUF_SIZE bytes - should succeed                            |
|  40 | `test_body_null_terminated_after_complete`               |   ✅   | Body null terminated after complete                                             |
|  41 | `test_body_one_over_limit_is_413`                        |   ✅   | Content-Length == BODY_BUF_SIZE + 1 → PARSE_ENTITY_TOO_LARGE                    |
|  42 | `test_body_far_over_limit_is_413`                        |   ✅   | Body far over limit is 413                                                      |
|  43 | `test_413_no_body_bytes_fed`                             |   ✅   | Even though we detected 413, no body bytes should have been stored              |
|  44 | `test_413_header_still_stored`                           |   ✅   | Headers before the blank line must be accessible even when 413                  |
|  45 | `test_body_exactly_at_limit_is_not_413`                  |   ✅   | BODY_BUF_SIZE is the max that fits - should NOT trigger 413                     |
|  46 | `test_path_overflow_stops_feeding`                       |   ✅   | Bytes fed after URI_TOO_LONG are ignored - state must not change                |
|  47 | `test_414_path_filled_to_capacity`                       |   ✅   | Buffer fills to MAX_PATH_LEN-1 chars before overflow is detected                |
|  48 | `test_method_nul_byte_is_error`                          |   ✅   | Method nul byte is error                                                        |
|  49 | `test_method_control_char_is_error`                      |   ✅   | Method control char is error                                                    |
|  50 | `test_method_del_byte_is_error`                          |   ✅   | Method del byte is error                                                        |
|  51 | `test_method_non_tchar_symbol_is_error`                  |   ✅   | '(' is VCHAR but not tchar                                                      |
|  52 | `test_method_tchar_symbols_accepted`                     |   ✅   | '-' is a valid tchar; a custom method like "X-CMD" is valid per RFC 7230        |
|  53 | `test_path_nul_byte_is_error`                            |   ✅   | Path nul byte is error                                                          |
|  54 | `test_path_control_char_is_error`                        |   ✅   | Path control char is error                                                      |
|  55 | `test_path_del_byte_is_error`                            |   ✅   | Path del byte is error                                                          |
|  56 | `test_query_nul_byte_is_error`                           |   ✅   | Query nul byte is error                                                         |
|  57 | `test_query_control_char_is_error`                       |   ✅   | Query control char is error                                                     |
|  58 | `test_header_key_space_is_error`                         |   ✅   | Space in a field-name is not a valid tchar                                      |
|  59 | `test_header_key_nul_byte_is_error`                      |   ✅   | Header key nul byte is error                                                    |
|  60 | `test_header_key_control_char_is_error`                  |   ✅   | Header key control char is error                                                |
|  61 | `test_header_key_mid_cr_is_error`                        |   ✅   | CR in the middle of a key name must be PARSE_ERROR, not blank-line detection    |
|  62 | `test_header_key_colon_at_start_skips_header`            |   ✅   | Empty key name (colon immediately after CRLF): transition to val with empty key |
|  63 | `test_long_standard_header_key_accepted`                 |   ✅   | Regression: "Sec-WebSocket-Extensions" (24 chars) is a standard header that     |
|  64 | `test_overlong_header_key_truncated_not_error`           |   ✅   | A header name longer than MAX_KEY_LEN is capped (capacity), not rejected:       |
|  65 | `test_header_val_nul_byte_is_error`                      |   ✅   | Header val nul byte is error                                                    |
|  66 | `test_header_val_control_char_is_error`                  |   ✅   | Header val control char is error                                                |
|  67 | `test_header_val_del_byte_is_error`                      |   ✅   | Header val del byte is error                                                    |
|  68 | `test_header_val_htab_mid_value_allowed`                 |   ✅   | HTAB is valid mid-value (RFC 7230 §3.2)                                         |
|  69 | `test_header_val_leading_htab_stripped`                  |   ✅   | Leading HTAB (OWS) is stripped just like leading SP                             |
|  70 | `test_header_val_obs_text_allowed`                       |   ✅   | obs-text bytes (%x80-FF) are allowed for legacy compatibility (RFC 7230 §3.2.6) |
|  71 | `test_version_http11_recognized`                         |   ✅   | Version http11 recognized                                                       |
|  72 | `test_version_http10_recognized`                         |   ✅   | Version http10 recognized                                                       |
|  73 | `test_version_unknown_is_http_unknown`                   |   ✅   | Version unknown is http unknown                                                 |
|  74 | `test_version_reset_to_unknown`                          |   ✅   | Version reset to unknown                                                        |
|  75 | `test_bad_expect_lf_is_error`                            |   ✅   | CRLF in version line replaced by CR + X (no LF)                                 |
|  76 | `test_blank_line_non_lf_is_error`                        |   ✅   | Header block ends with CR + non-LF in the blank line                            |
|  77 | `test_slots_are_independent`                             |   ✅   | Slots are independent                                                           |
|  78 | `test_incremental_byte_by_byte`                          |   ✅   | Incremental byte by byte                                                        |
|  79 | `test_incremental_two_chunks`                            |   ✅   | Incremental two chunks                                                          |
|  80 | `stress_many_requests_same_slot`                         |   ✅   | Stress - Many requests same slot                                                |
|  81 | `stress_max_headers`                                     |   ✅   | Build a request with MAX_HEADERS header lines                                   |
|  82 | `stress_max_query_params`                                |   ✅   | Build a query string with MAX_QUERY_PARAMS parameters                           |

</details>

---

## test_observability - ✅ 15 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Transport observability (DETWS_ENABLE_OBSERVABILITY): the det_conn_on_event_

|   # | Test                                                          | Status | Description                                            |
| --: | :------------------------------------------------------------ | :----: | :----------------------------------------------------- |
|   1 | `test_transition_fires_hook_with_args`                        |   ✅   | Transition fires hook with args                        |
|   2 | `test_each_reason_bumps_its_counter`                          |   ✅   | Each reason bumps its counter                          |
|   3 | `test_closing_gauge_is_derived_from_pool`                     |   ✅   | Closing gauge is derived from pool                     |
|   4 | `test_reset_clears_cumulative_not_derived_gauge`              |   ✅   | Reset clears cumulative not derived gauge              |
|   5 | `test_no_hook_after_unregister`                               |   ✅   | No hook after unregister                               |
|   6 | `test_recv_fin_counts_remote_close`                           |   ✅   | Recv fin counts remote close                           |
|   7 | `test_err_cb_counts_error_close`                              |   ✅   | Err cb counts error close                              |
|   8 | `test_timeout_sweep_counts_timeout`                           |   ✅   | Timeout sweep counts timeout                           |
|   9 | `test_local_close_counts_local`                               |   ✅   | Local close counts local                               |
|  10 | `test_backpressure_counts_when_ring_full`                     |   ✅   | Backpressure counts when ring full                     |
|  11 | `test_begin_close_dwells_then_drains_on_ack`                  |   ✅   | Begin close dwells then drains on ack                  |
|  12 | `test_begin_close_finalizes_immediately_when_already_drained` |   ✅   | Begin close finalizes immediately when already drained |
|  13 | `test_begin_close_noop_if_not_active`                         |   ✅   | Begin close noop if not active                         |
|  14 | `test_closing_timeout_reaps_stuck_slot`                       |   ✅   | Closing timeout reaps stuck slot                       |
|  15 | `test_recv_during_closing_is_drained_not_processed`           |   ✅   | Recv during closing is drained not processed           |

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

## test_ssh_crypto - ✅ 38 passed

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

</details>

---

## test_ssh_auth - ✅ 12 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH user-authentication tests (RFC 4252): service request/accept, request_

|   # | Test                                          | Status | Description                                       |
| --: | :-------------------------------------------- | :----: | :------------------------------------------------ |
|   1 | `test_service_request_accept`                 |   ✅   | Service request accept                            |
|   2 | `test_service_request_rejects_unknown`        |   ✅   | Service request rejects unknown                   |
|   3 | `test_parse_password_request`                 |   ✅   | Parse password request                            |
|   4 | `test_parse_none_request`                     |   ✅   | Parse none request                                |
|   5 | `test_handle_request_success`                 |   ✅   | Handle request success                            |
|   6 | `test_handle_request_wrong_password_fails`    |   ✅   | Handle request wrong password fails               |
|   7 | `test_handle_none_request_fails_without_auth` |   ✅   | Handle none request fails without auth            |
|   8 | `test_handle_request_no_callback_fails`       |   ✅   | No callback installed → all credentials rejected. |
|   9 | `test_pubkey_probe_returns_pk_ok`             |   ✅   | Pubkey probe returns pk ok                        |
|  10 | `test_pubkey_valid_signature_succeeds`        |   ✅   | Pubkey valid signature succeeds                   |
|  11 | `test_pubkey_tampered_signature_fails`        |   ✅   | Pubkey tampered signature fails                   |
|  12 | `test_pubkey_unauthorized_key_fails`          |   ✅   | Pubkey unauthorized key fails                     |

</details>

---

## test_ssh_server - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_End-to-end SSH server dispatcher test: drives a full handshake_

|   # | Test                                           | Status | Description                                                           |
| --: | :--------------------------------------------- | :----: | :-------------------------------------------------------------------- |
|   1 | `test_full_handshake_to_channel_data`          |   ✅   | Banner exchange already done out-of-band; seed V_C and enter KEXINIT. |
|   2 | `test_channel_open_before_auth_rejected`       |   ✅   | Channel open before auth rejected                                     |
|   3 | `test_disconnect_closes`                       |   ✅   | Disconnect closes                                                     |
|   4 | `test_ignore_is_noop`                          |   ✅   | Ignore is noop                                                        |
|   5 | `test_auth_bruteforce_disconnect`              |   ✅   | The first SSH_MAX_AUTH_ATTEMPTS-1 failures keep the connection open.  |
|   6 | `test_auth_success_after_failures`             |   ✅   | Auth success after failures                                           |
|   7 | `test_unimplemented_reply_for_unknown_message` |   ✅   | Unimplemented reply for unknown message                               |

</details>

---

## test_ssh_transport - ✅ 23 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH transport handshake tests (RFC 4253): identification-string exchange and_

|   # | Test                                                       | Status | Description                                                      |
| --: | :--------------------------------------------------------- | :----: | :--------------------------------------------------------------- |
|   1 | `test_server_banner_format`                                |   ✅   | Server banner format                                             |
|   2 | `test_recv_banner_complete`                                |   ✅   | Recv banner complete                                             |
|   3 | `test_recv_banner_bare_lf`                                 |   ✅   | Recv banner bare lf                                              |
|   4 | `test_recv_banner_split_across_reads`                      |   ✅   | Recv banner split across reads                                   |
|   5 | `test_recv_banner_skips_preamble_lines`                    |   ✅   | RFC 4253 §4.2 allows lines before the SSH identification string. |
|   6 | `test_kexinit_build_starts_with_msg_and_stores_is`         |   ✅   | Kexinit build starts with msg and stores is                      |
|   7 | `test_kexinit_parse_accepts_supported`                     |   ✅   | Kexinit parse accepts supported                                  |
|   8 | `test_kexinit_parse_accepts_when_ours_listed_among_others` |   ✅   | Kexinit parse accepts when ours listed among others              |
|   9 | `test_kexinit_parse_rejects_missing_kex`                   |   ✅   | Kexinit parse rejects missing kex                                |
|  10 | `test_kexinit_parse_rejects_missing_cipher`                |   ✅   | Kexinit parse rejects missing cipher                             |
|  11 | `test_kexinit_parse_rejects_truncated`                     |   ✅   | Kexinit parse rejects truncated                                  |
|  12 | `test_exchange_hash_matches_independent_assembly`          |   ✅   | Populate the session fields the hash reads.                      |
|  13 | `test_exchange_hash_changes_with_input`                    |   ✅   | Exchange hash changes with input                                 |
|  14 | `test_kexdh_parse_init_extracts_e_with_padding`            |   ✅   | Kexdh parse init extracts e with padding                         |
|  15 | `test_kexdh_parse_init_extracts_small_e`                   |   ✅   | Kexdh parse init extracts small e                                |
|  16 | `test_kexdh_parse_init_rejects_wrong_type`                 |   ✅   | Kexdh parse init rejects wrong type                              |
|  17 | `test_kexdh_parse_init_rejects_oversized_e`                |   ✅   | mpint with 300 magnitude bytes → exceeds 2048 bits.              |
|  18 | `test_kexdh_build_reply_structure`                         |   ✅   | Kexdh build reply structure                                      |
|  19 | `test_kexdh_handle_produces_reply_and_installs_keys`       |   ✅   | Kexdh handle produces reply and installs keys                    |
|  20 | `test_kexdh_handle_rejects_invalid_e`                      |   ✅   | Kexdh handle rejects invalid e                                   |
|  21 | `test_derive_keys_session_id_affects_output`               |   ✅   | Derive keys session id affects output                            |
|  22 | `test_rekey_needed_threshold`                              |   ✅   | Rekey needed threshold                                           |
|  23 | `test_begin_rekey_preserves_session_and_auth`              |   ✅   | Begin rekey preserves session and auth                           |

</details>

---

## test_ssh_channel - ✅ 12 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH connection-protocol (channel) tests - RFC 4254._

|   # | Test                                                | Status | Description                                  |
| --: | :-------------------------------------------------- | :----: | :------------------------------------------- |
|   1 | `test_open_session_confirms`                        |   ✅   | Open session confirms                        |
|   2 | `test_open_non_session_fails`                       |   ✅   | Open non session fails                       |
|   3 | `test_shell_request_success_with_reply`             |   ✅   | Shell request success with reply             |
|   4 | `test_unknown_request_failure`                      |   ✅   | Unknown request failure                      |
|   5 | `test_request_no_reply_produces_nothing`            |   ✅   | Request no reply produces nothing            |
|   6 | `test_inbound_data_invokes_callback`                |   ✅   | Inbound data invokes callback                |
|   7 | `test_inbound_data_window_replenish`                |   ✅   | Inbound data window replenish                |
|   8 | `test_inbound_data_exceeding_window_rejected`       |   ✅   | Inbound data exceeding window rejected       |
|   9 | `test_outbound_data_frames_and_decrements_window`   |   ✅   | Outbound data frames and decrements window   |
|  10 | `test_outbound_data_exceeding_peer_window_rejected` |   ✅   | Outbound data exceeding peer window rejected |
|  11 | `test_window_adjust_grows_peer_window`              |   ✅   | Window adjust grows peer window              |
|  12 | `test_build_close_emits_eof_and_close`              |   ✅   | Build close emits eof and close              |

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

## test_ssh_conn - ✅ 2 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_SSH transport-glue test: drives a PROTO_SSH connection through the real_

|   # | Test                                            | Status | Description                              |
| --: | :---------------------------------------------- | :----: | :--------------------------------------- |
|   1 | `test_accept_sends_server_banner`               |   ✅   | Accept sends server banner               |
|   2 | `test_banner_then_kexinit_advances_and_replies` |   ✅   | Banner then kexinit advances and replies |

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

## test_json - ✅ 17 passed

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

</details>

---

## test_response_headers - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for custom response headers and cookies:_

|   # | Test                                       | Status | Description                              |
| --: | :----------------------------------------- | :----: | :--------------------------------------- |
|   1 | `test_single_custom_header_present`        |   ✅   | Single custom header present             |
|   2 | `test_multiple_custom_headers_present`     |   ✅   | Multiple custom headers present          |
|   3 | `test_set_cookie_basic`                    |   ✅   | Set cookie basic                         |
|   4 | `test_set_cookie_with_attrs`               |   ✅   | Set cookie with attrs                    |
|   5 | `test_custom_header_on_send_empty`         |   ✅   | Custom header on send empty              |
|   6 | `test_custom_header_on_redirect`           |   ✅   | Custom header on redirect                |
|   7 | `test_headers_do_not_leak_across_requests` |   ✅   | First request queues X-Custom on slot 0. |
|   8 | `test_clear_response_headers`              |   ✅   | Clear response headers                   |
|   9 | `test_oversized_header_dropped_whole`      |   ✅   | Oversized header dropped whole           |

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

## test_digest_auth - ✅ 5 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for HTTP Digest authentication (RFC 7616, SHA-256, qop=auth)._

|   # | Test                              | Status | Description                                                              |
| --: | :-------------------------------- | :----: | :----------------------------------------------------------------------- |
|   1 | `test_challenge_is_digest_sha256` |   ✅   | Challenge is digest sha256                                               |
|   2 | `test_valid_digest_authenticates` |   ✅   | Valid digest authenticates                                               |
|   3 | `test_wrong_password_rejected`    |   ✅   | Wrong password rejected                                                  |
|   4 | `test_bad_nonce_rejected`         |   ✅   | Bad nonce rejected                                                       |
|   5 | `test_nonce_is_128bit_hex`        |   ✅   | The hardened nonce is SHA-256(CSPRNG + counter + millis) truncated to 16 |

</details>

---

## test_web_terminal - ✅ 7 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the WebSocket web-serial terminal (DETWS_ENABLE_WEB_TERMINAL):_

|   # | Test                                 | Status | Description                                                  |
| --: | :----------------------------------- | :----: | :----------------------------------------------------------- |
|   1 | `test_serves_terminal_page`          |   ✅   | Serves terminal page                                         |
|   2 | `test_ws_upgrade_tracks_client`      |   ✅   | Ws upgrade tracks client                                     |
|   3 | `test_command_delivered_to_callback` |   ✅   | Command delivered to callback                                |
|   4 | `test_broadcast_reaches_client`      |   ✅   | Broadcast reaches client                                     |
|   5 | `test_printf_broadcast`              |   ✅   | Printf broadcast                                             |
|   6 | `test_no_broadcast_without_clients`  |   ✅   | No handshake -> no terminal clients -> print writes nothing. |
|   7 | `test_close_clears_client`           |   ✅   | Close clears client                                          |

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

|   # | Test                                           | Status | Description                                                                |
| --: | :--------------------------------------------- | :----: | :------------------------------------------------------------------------- |
|   1 | `test_missing_file_returns_404`                |   ✅   | Missing file returns 404                                                   |
|   2 | `test_existing_file_returns_200`               |   ✅   | Existing file returns 200                                                  |
|   3 | `test_response_includes_content_type_html`     |   ✅   | Response includes content type html                                        |
|   4 | `test_response_includes_content_type_js`       |   ✅   | Response includes content type js                                          |
|   5 | `test_content_length_matches_file_size`        |   ✅   | Content length matches file size                                           |
|   6 | `test_file_body_is_sent`                       |   ✅   | File body is sent                                                          |
|   7 | `test_empty_file_returns_200_with_zero_length` |   ✅   | Empty file returns 200 with zero length                                    |
|   8 | `test_large_file_body_fully_sent`              |   ✅   | Build a body larger than one FILE_CHUNK_SIZE to exercise chunked streaming |
|   9 | `test_serve_file_does_not_affect_other_routes` |   ✅   | Serve file does not affect other routes                                    |
|  10 | `test_multiple_content_types`                  |   ✅   | Multiple content types                                                     |
|  11 | `stress_serve_file_50_requests`                |   ✅   | Stress - Serve file 50 requests                                            |
|  12 | `stress_alternate_missing_and_found`           |   ✅   | Stress - Alternate missing and found                                       |

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

## test_chunked - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for send_chunked() / ChunkedResponse streaming responses._

|   # | Test                                              | Status | Description                                |
| --: | :------------------------------------------------ | :----: | :----------------------------------------- |
|   1 | `test_headers_announce_chunked_no_content_length` |   ✅   | Headers announce chunked no content length |
|   2 | `test_single_chunk_framing`                       |   ✅   | Single chunk framing                       |
|   3 | `test_multiple_chunks_in_order`                   |   ✅   | Multiple chunks in order                   |
|   4 | `test_printf_chunk`                               |   ✅   | Printf chunk                               |
|   5 | `test_empty_writes_do_not_terminate_early`        |   ✅   | Empty writes do not terminate early        |
|   6 | `test_head_sends_headers_only`                    |   ✅   | Head sends headers only                    |
|   7 | `test_custom_header_injected_into_chunked`        |   ✅   | Custom header injected into chunked        |
|   8 | `test_log_hook_reports_total_body_length`         |   ✅   | Log hook reports total body length         |

</details>

---

## test_application - ✅ 50 passed

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
|  46 | `test_serve_static_cache_control`                     |   ✅   | Serve static cache control                                                 |
|  47 | `test_request_log_hook_fires`                         |   ✅   | Request log hook fires                                                     |
|  48 | `test_stats_endpoint_emits_json`                      |   ✅   | Stats endpoint emits json                                                  |
|  49 | `test_sse_broadcast_after_upgrade_matches_path`       |   ✅   | Sse broadcast after upgrade matches path                                   |
|  50 | `test_metrics_emits_prometheus`                       |   ✅   | Metrics emits prometheus                                                   |

</details>

---

## test_snmp_ber - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SNMP ASN.1 BER codec. Encodings are checked against_

|   # | Test                                  | Status | Description                                                                 |
| --: | :------------------------------------ | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_integer_vectors`                |   ✅   | Integer vectors                                                             |
|   2 | `test_oid_vector`                     |   ✅   | 1.3.6.1 -> 06 03 2B 06 01                                                   |
|   3 | `test_octet_string_and_null`          |   ✅   | Octet string and null                                                       |
|   4 | `test_counter32_keeps_unsigned`       |   ✅   | 0x80000000 has the top bit set -> a leading 0x00 must be added.             |
|   5 | `test_sequence_roundtrip`             |   ✅   | Sequence roundtrip                                                          |
|   6 | `test_oid_roundtrip`                  |   ✅   | Oid roundtrip                                                               |
|   7 | `test_large_arc_roundtrip`            |   ✅   | An arc > 127 exercises multi-byte base-128 encoding (e.g. enterprise 8072). |
|   8 | `test_encoder_overflow_sets_not_ok`   |   ✅   | Encoder overflow sets not ok                                                |
|   9 | `test_decoder_truncated_length_fails` |   ✅   | Claims 10 bytes of content but only 2 are present.                          |

</details>

---

## test_snmp_agent - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SNMP v1/v2c agent core (snmp_agent_process). Each test_

|   # | Test                                        | Status | Description                                                                     |
| --: | :------------------------------------------ | :----: | :------------------------------------------------------------------------------ |
|   1 | `test_get_string_v2c`                       |   ✅   | Get string v2c                                                                  |
|   2 | `test_get_unknown_v2c_exception`            |   ✅   | Get unknown v2c exception                                                       |
|   3 | `test_get_unknown_v1_error`                 |   ✅   | Get unknown v1 error                                                            |
|   4 | `test_getnext_walks_to_first`               |   ✅   | Getnext walks to first                                                          |
|   5 | `test_getnext_past_end_endofmibview`        |   ✅   | Getnext past end endofmibview                                                   |
|   6 | `test_set_without_rw_community_denied`      |   ✅   | Set without rw community denied                                                 |
|   7 | `test_set_with_rw_community_invokes_setter` |   ✅   | Set with rw community invokes setter                                            |
|   8 | `test_set_readonly_not_writable`            |   ✅   | Set readonly not writable                                                       |
|   9 | `test_getbulk_returns_multiple`             |   ✅   | non-repeaters=0, max-repetitions=3, one repeater starting at the system prefix. |
|  10 | `test_dynamic_counter_value`                |   ✅   | Dynamic counter value                                                           |
|  11 | `test_uptime_is_timeticks`                  |   ✅   | Uptime is timeticks                                                             |
|  12 | `test_unknown_community_no_response`        |   ✅   | Unknown community no response                                                   |
|  13 | `test_v3_message_dropped`                   |   ✅   | V3 message dropped                                                              |

</details>

---

## test_snmp_v3 - ✅ 9 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the SNMPv3 USM layer. The test acts as a full SNMP manager:_

|   # | Test                                            | Status | Description                                                                     |
| --: | :---------------------------------------------- | :----: | :------------------------------------------------------------------------------ |
|   1 | `test_localize_key_sha256_vector`               |   ✅   | password "maplesyrup", engineID 80 00 C0 DE 05 01 02 03 04 (the agent default). |
|   2 | `test_aes128_fips197_vector`                    |   ✅   | FIPS-197 C.1. CFB with IV = plaintext and zero input yields E_key(plaintext).   |
|   3 | `test_aes_cfb_roundtrip_partial_block`          |   ✅   | Aes cfb roundtrip partial block                                                 |
|   4 | `test_discovery_reports_engine_id`              |   ✅   | Discovery reports engine id                                                     |
|   5 | `test_authnopriv_get`                           |   ✅   | Authnopriv get                                                                  |
|   6 | `test_authpriv_get`                             |   ✅   | Authpriv get                                                                    |
|   7 | `test_wrong_auth_password_reports_wrong_digest` |   ✅   | Wrong auth password reports wrong digest                                        |
|   8 | `test_unknown_user_reports`                     |   ✅   | Unknown user reports                                                            |
|   9 | `test_not_in_time_window_reports`               |   ✅   | Not in time window reports                                                      |

</details>

---

## test_telnet - ✅ 8 passed

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

</details>

---

## test_coap - ✅ 26 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CoAP server core (coap_server_process). Each test encodes a_

|   # | Test                                       | Status | Description                                                   |
| --: | :----------------------------------------- | :----: | :------------------------------------------------------------ |
|   1 | `test_block2_explicit_paging`              |   ✅   | Block2 explicit paging                                        |
|   2 | `test_block2_auto_when_large`              |   ✅   | Block2 auto when large                                        |
|   3 | `test_block2_szx_clamped`                  |   ✅   | Block2 szx clamped                                            |
|   4 | `test_block2_absent_for_small`             |   ✅   | Block2 absent for small                                       |
|   5 | `test_block2_out_of_range`                 |   ✅   | Block2 out of range                                           |
|   6 | `test_block2_reserved_szx`                 |   ✅   | Block2 reserved szx                                           |
|   7 | `test_block1_upload_two_blocks`            |   ✅   | Block1 upload two blocks                                      |
|   8 | `test_block1_out_of_order`                 |   ✅   | Block1 out of order                                           |
|   9 | `test_block1_too_large`                    |   ✅   | Block1 too large                                              |
|  10 | `test_observe_option_in_response`          |   ✅   | Observe option in response                                    |
|  11 | `test_no_observe_option_when_seq_negative` |   ✅   | No observe option when seq negative                           |
|  12 | `test_get_content`                         |   ✅   | Get content                                                   |
|  13 | `test_not_found`                           |   ✅   | Not found                                                     |
|  14 | `test_method_not_allowed`                  |   ✅   | Method not allowed                                            |
|  15 | `test_non_request_type`                    |   ✅   | Non request type                                              |
|  16 | `test_put_with_payload`                    |   ✅   | Put with payload                                              |
|  17 | `test_multi_segment_path`                  |   ✅   | Multi segment path                                            |
|  18 | `test_uri_query`                           |   ✅   | Uri query                                                     |
|  19 | `test_empty_con_ping_rst`                  |   ✅   | Empty con ping rst                                            |
|  20 | `test_bad_version_rst`                     |   ✅   | Bad version rst                                               |
|  21 | `test_delete`                              |   ✅   | Delete                                                        |
|  22 | `test_token_8_bytes`                       |   ✅   | Token 8 bytes                                                 |
|  23 | `test_extended_option_length`              |   ✅   | Extended option length                                        |
|  24 | `test_ack_ignored`                         |   ✅   | Ack ignored                                                   |
|  25 | `test_root_path`                           |   ✅   | Root path                                                     |
|  26 | `test_unknown_method_not_implemented`      |   ✅   | Code 0.05 (FETCH) is a valid class-0 code we don't implement. |

</details>

---

## test_webdav - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the WebDAV server core (services/webdav): method classification,_

|   # | Test                                     | Status | Description                       |
| --: | :--------------------------------------- | :----: | :-------------------------------- |
|   1 | `test_method_classification`             |   ✅   | Method classification             |
|   2 | `test_depth_parsing`                     |   ✅   | Depth parsing                     |
|   3 | `test_xml_escape`                        |   ✅   | Xml escape                        |
|   4 | `test_xml_escape_truncates_safely`       |   ✅   | Xml escape truncates safely       |
|   5 | `test_dest_absolute_uri`                 |   ✅   | Dest absolute uri                 |
|   6 | `test_dest_percent_decoded`              |   ✅   | Dest percent decoded              |
|   7 | `test_dest_abs_path`                     |   ✅   | Dest abs path                     |
|   8 | `test_dest_rejects_malformed`            |   ✅   | Dest rejects malformed            |
|   9 | `test_multistatus_file_and_collection`   |   ✅   | Multistatus file and collection   |
|  10 | `test_multistatus_escapes_href`          |   ✅   | Multistatus escapes href          |
|  11 | `test_multistatus_entry_stops_when_full` |   ✅   | Multistatus entry stops when full |

</details>

---

## test_modbus - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the Modbus TCP slave core (services/modbus): the data model and_

|   # | Test                                  | Status | Description                                          |
| --: | :------------------------------------ | :----: | :--------------------------------------------------- |
|   1 | `test_read_holding_registers`         |   ✅   | Read holding registers                               |
|   2 | `test_read_input_registers`           |   ✅   | Read input registers                                 |
|   3 | `test_read_coils_packs_bits`          |   ✅   | Read coils packs bits                                |
|   4 | `test_write_single_coil`              |   ✅   | Write single coil                                    |
|   5 | `test_write_single_register`          |   ✅   | Write single register                                |
|   6 | `test_write_multiple_registers`       |   ✅   | Write multiple registers                             |
|   7 | `test_write_multiple_coils`           |   ✅   | qty 5, 1 byte of data: bits 0..4 = 1,0,1,1,0 -> 0x0D |
|   8 | `test_exception_illegal_function`     |   ✅   | Exception illegal function                           |
|   9 | `test_exception_illegal_address`      |   ✅   | Read holding regs beyond the 64-register table.      |
|  10 | `test_exception_illegal_value`        |   ✅   | Exception illegal value                              |
|  11 | `test_write_single_coil_bad_value`    |   ✅   | Write single coil bad value                          |
|  12 | `test_non_modbus_protocol_id_ignored` |   ✅   | Non modbus protocol id ignored                       |
|  13 | `test_truncated_frame_ignored`        |   ✅   | Truncated frame ignored                              |

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

## test_audit_log - ✅ 12 passed

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

</details>

---

## test_oidc - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the OIDC RS256 ID-token verifier (services/oidc). Vectors are_

|   # | Test                                 | Status | Description                                                               |
| --: | :----------------------------------- | :----: | :------------------------------------------------------------------------ |
|   1 | `test_token_kid`                     |   ✅   | Token kid                                                                 |
|   2 | `test_jwks_find`                     |   ✅   | Jwks find                                                                 |
|   3 | `test_jwks_find_missing_kid_fails`   |   ✅   | Jwks find missing kid fails                                               |
|   4 | `test_verify_valid_token_and_claims` |   ✅   | Verify valid token and claims                                             |
|   5 | `test_verify_aud_array`              |   ✅   | Verify aud array                                                          |
|   6 | `test_reject_expired`                |   ✅   | Reject expired                                                            |
|   7 | `test_reject_wrong_issuer`           |   ✅   | Reject wrong issuer                                                       |
|   8 | `test_reject_wrong_audience`         |   ✅   | Reject wrong audience                                                     |
|   9 | `test_reject_non_rs256_header`       |   ✅   | Reject non rs256 header                                                   |
|  10 | `test_reject_tampered_payload`       |   ✅   | Reject tampered payload                                                   |
|  11 | `test_reject_tampered_signature`     |   ✅   | Reject tampered signature                                                 |
|  12 | `test_reject_unknown_key`            |   ✅   | JWKS whose only key has a different kid than the token's.                 |
|  13 | `test_reject_malformed`              |   ✅   | No kid extractable -> the sole JWKS key is selected, then the token shape |

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

## test_graphql - ✅ 13 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the GraphQL query subset (services/graphql): selection shaping,_

|   # | Test                              | Status | Description                                                                |
| --: | :-------------------------------- | :----: | :------------------------------------------------------------------------- |
|   1 | `test_flat_selection`             |   ✅   | Flat selection                                                             |
|   2 | `test_selection_is_honored`       |   ✅   | Only the requested field appears.                                          |
|   3 | `test_nested_object`              |   ✅   | Nested object                                                              |
|   4 | `test_args_collected_along_path`  |   ✅   | `id` is on the object `sensor`; the leaf resolver `sensor.value` reads it. |
|   5 | `test_scalar_types`               |   ✅   | Scalar types                                                               |
|   6 | `test_string_arg_and_escaping`    |   ✅   | String arg is decoded, and the resolver's output string is JSON-escaped.   |
|   7 | `test_unresolved_field_is_null`   |   ✅   | Unresolved field is null                                                   |
|   8 | `test_query_keyword_and_name`     |   ✅   | Query keyword and name                                                     |
|   9 | `test_comments_and_commas`        |   ✅   | Comments and commas                                                        |
|  10 | `test_parse_error_reports_errors` |   ✅   | Parse error reports errors                                                 |
|  11 | `test_mutation_rejected`          |   ✅   | Mutation rejected                                                          |
|  12 | `test_depth_limit`                |   ✅   | Depth limit                                                                |
|  13 | `test_overflow_fails_closed`      |   ✅   | Overflow fails closed                                                      |

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

## test_opcua - ✅ 29 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for OPC UA (services/opcua): the Binary built-in type codec (incl._

|   # | Test                                   | Status | Description                                                                 |
| --: | :------------------------------------- | :----: | :-------------------------------------------------------------------------- |
|   1 | `test_codec_roundtrip`                 |   ✅   | Codec roundtrip                                                             |
|   2 | `test_string_null_roundtrip`           |   ✅   | String null roundtrip                                                       |
|   3 | `test_reader_underrun_latches`         |   ✅   | Reader underrun latches                                                     |
|   4 | `test_writer_overflow_fails_closed`    |   ✅   | Writer overflow fails closed                                                |
|   5 | `test_parse_header`                    |   ✅   | Parse header                                                                |
|   6 | `test_parse_hello`                     |   ✅   | Parse hello                                                                 |
|   7 | `test_parse_hello_rejects_short`       |   ✅   | Parse hello rejects short                                                   |
|   8 | `test_build_ack_negotiates`            |   ✅   | Build ack negotiates                                                        |
|   9 | `test_nodeid_roundtrip`                |   ✅   | Nodeid roundtrip                                                            |
|  10 | `test_filetime_from_unix`              |   ✅   | Filetime from unix                                                          |
|  11 | `test_parse_open`                      |   ✅   | Parse open                                                                  |
|  12 | `test_parse_open_rejects_wrong_type`   |   ✅   | Corrupt the message type so it is no longer "OPN".                          |
|  13 | `test_build_open_response`             |   ✅   | Build open response                                                         |
|  14 | `test_parse_msg`                       |   ✅   | Parse msg                                                                   |
|  15 | `test_parse_msg_rejects_non_msg`       |   ✅   | Parse msg rejects non msg                                                   |
|  16 | `test_build_create_session_response`   |   ✅   | Build create session response                                               |
|  17 | `test_build_activate_session_response` |   ✅   | Build activate session response                                             |
|  18 | `test_datavalue_good_int32`            |   ✅   | Datavalue good int32                                                        |
|  19 | `test_datavalue_bad_status`            |   ✅   | Datavalue bad status                                                        |
|  20 | `test_parse_read`                      |   ✅   | Parse read                                                                  |
|  21 | `test_build_read_response`             |   ✅   | Build read response                                                         |
|  22 | `test_parse_browse`                    |   ✅   | Parse browse                                                                |
|  23 | `test_build_browse_response`           |   ✅   | Build browse response                                                       |
|  24 | `test_build_browse_response_unknown`   |   ✅   | Build browse response unknown                                               |
|  25 | `test_build_close_session_response`    |   ✅   | Build close session response                                                |
|  26 | `test_build_get_endpoints`             |   ✅   | Build get endpoints                                                         |
|  27 | `test_build_service_fault`             |   ✅   | Build service fault                                                         |
|  28 | `test_datavalue_roundtrip`             |   ✅   | Datavalue roundtrip                                                         |
|  29 | `test_parse_and_build_write`           |   ✅   | Build a WriteRequest writing one Int32 to ns=1;i=10 (value-only DataValue). |

</details>

---

## test_opcua_client - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Round-trip tests for the OPC UA client (services/opcua_client): the client builds_

|   # | Test                                     | Status | Description                                                                            |
| --: | :--------------------------------------- | :----: | :------------------------------------------------------------------------------------- |
|   1 | `test_hello_ack_roundtrip`               |   ✅   | Hello ack roundtrip                                                                    |
|   2 | `test_open_roundtrip`                    |   ✅   | Open roundtrip                                                                         |
|   3 | `test_session_roundtrip`                 |   ✅   | Session roundtrip                                                                      |
|   4 | `test_get_endpoints_roundtrip`           |   ✅   | Get endpoints roundtrip                                                                |
|   5 | `test_service_fault_rejected_by_parsers` |   ✅   | An unknown service draws a ServiceFault; a typed parser must reject it (wrong TypeId). |
|   6 | `test_read_roundtrip`                    |   ✅   | Read roundtrip                                                                         |
|   7 | `test_browse_roundtrip`                  |   ✅   | Browse roundtrip                                                                       |
|   8 | `test_write_roundtrip`                   |   ✅   | Write roundtrip                                                                        |
|   9 | `test_close_session_roundtrip`           |   ✅   | Close session roundtrip                                                                |
|  10 | `test_close_channel_is_clo`              |   ✅   | Close channel is clo                                                                   |
|  11 | `test_seq_and_request_id_increment`      |   ✅   | Seq and request id increment                                                           |

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

## test_range - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_HTTP Range requests / 206 Partial Content (DETWS_ENABLE_RANGE). Each test_

|   # | Test                                | Status | Description                  |
| --: | :---------------------------------- | :----: | :--------------------------- |
|   1 | `test_no_range_full_200`            |   ✅   | No range full 200            |
|   2 | `test_range_prefix`                 |   ✅   | Range prefix                 |
|   3 | `test_range_open_ended`             |   ✅   | Range open ended             |
|   4 | `test_range_suffix`                 |   ✅   | Range suffix                 |
|   5 | `test_range_single_byte`            |   ✅   | Range single byte            |
|   6 | `test_range_clamped_to_eof`         |   ✅   | Range clamped to eof         |
|   7 | `test_range_unsatisfiable_416`      |   ✅   | Range unsatisfiable 416      |
|   8 | `test_malformed_range_ignored`      |   ✅   | Malformed range ignored      |
|   9 | `test_multirange_falls_back_to_200` |   ✅   | Multirange falls back to 200 |
|  10 | `test_head_with_range_no_body`      |   ✅   | Head with range no body      |

</details>

---

## test_syslog - ✅ 6 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the RFC 5424 syslog line formatter (syslog_format). No sockets._

|   # | Test                             | Status | Description               |
| --: | :------------------------------- | :----: | :------------------------ |
|   1 | `test_pri_local0_info`           |   ✅   | Pri local0 info           |
|   2 | `test_pri_computation_varies`    |   ✅   | daemon(3)*8 + err(3) = 27 |
|   3 | `test_nilvalue_for_empty_fields` |   ✅   | Nilvalue for empty fields |
|   4 | `test_empty_message_ok`          |   ✅   | Empty message ok          |
|   5 | `test_overflow_returns_zero`     |   ✅   | Overflow returns zero     |
|   6 | `test_length_matches_strlen`     |   ✅   | Length matches strlen     |

</details>

---

## test_jwt - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the JWT HS256 verifier. The reference token below was produced_

|   # | Test                              | Status | Description                |
| --: | :-------------------------------- | :----: | :------------------------- |
|   1 | `test_valid_token_accepts`        |   ✅   | Valid token accepts        |
|   2 | `test_wrong_secret_rejects`       |   ✅   | Wrong secret rejects       |
|   3 | `test_tampered_payload_rejects`   |   ✅   | Tampered payload rejects   |
|   4 | `test_tampered_signature_rejects` |   ✅   | Tampered signature rejects |
|   5 | `test_malformed_rejected`         |   ✅   | Malformed rejected         |
|   6 | `test_bearer_header`              |   ✅   | Bearer header              |
|   7 | `test_claim_int`                  |   ✅   | Claim int                  |
|   8 | `test_claim_missing`              |   ✅   | Claim missing              |
|   9 | `test_claim_str`                  |   ✅   | Claim str                  |
|  10 | `test_scope_allows`               |   ✅   | Scope allows               |

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

## test_http_client - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the outbound HTTP client's pure core: URL parsing, request_

|   # | Test                                 | Status | Description                                                        |
| --: | :----------------------------------- | :----: | :----------------------------------------------------------------- |
|   1 | `test_url_http_default`              |   ✅   | Url http default                                                   |
|   2 | `test_url_https_port_nopath`         |   ✅   | Url https port nopath                                              |
|   3 | `test_url_bad_scheme`                |   ✅   | Url bad scheme                                                     |
|   4 | `test_build_get`                     |   ✅   | Build get                                                          |
|   5 | `test_build_post_with_body_and_port` |   ✅   | Build post with body and port                                      |
|   6 | `test_parse_content_length`          |   ✅   | Parse content length                                               |
|   7 | `test_parse_status_404`              |   ✅   | Parse status 404                                                   |
|   8 | `test_parse_chunked`                 |   ✅   | two chunks "Wiki" (4) + "pedia" (5) -> "Wikipedia"                 |
|   9 | `test_parse_connection_close_body`   |   ✅   | No Content-Length / chunked: body is everything after the headers. |
|  10 | `test_parse_malformed`               |   ✅   | Parse malformed                                                    |

</details>

---

## test_compliance - ✅ 12 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_RFC-compliance suite. Built with production enforcement defaults_

|   # | Test                                                 | Status | Description                                                               |
| --: | :--------------------------------------------------- | :----: | :------------------------------------------------------------------------ |
|   1 | `test_http11_missing_host_rejected`                  |   ✅   | Http11 missing host rejected                                              |
|   2 | `test_http11_with_host_ok`                           |   ✅   | Http11 with host ok                                                       |
|   3 | `test_http10_missing_host_ok`                        |   ✅   | Host is not required for HTTP/1.0.                                        |
|   4 | `test_duplicate_host_rejected`                       |   ✅   | Duplicate host rejected                                                   |
|   5 | `test_duplicate_host_rejected_http10`                |   ✅   | More than one Host is invalid regardless of version.                      |
|   6 | `test_host_beyond_max_headers_still_counted`         |   ✅   | A valid Host that appears after MAX_HEADERS other fields is still counted |
|   7 | `test_duplicate_host_with_one_beyond_cap_rejected`   |   ✅   | First Host is stored; a second Host pushed past MAX_HEADERS must still be |
|   8 | `test_content_length_non_digit_rejected`             |   ✅   | Content length non digit rejected                                         |
|   9 | `test_content_length_empty_rejected`                 |   ✅   | Content length empty rejected                                             |
|  10 | `test_content_length_conflicting_duplicate_rejected` |   ✅   | Content length conflicting duplicate rejected                             |
|  11 | `test_content_length_matching_duplicate_ok`          |   ✅   | Two identical Content-Length values are not a conflict.                   |
|  12 | `test_content_length_valid_body`                     |   ✅   | Content length valid body                                                 |

</details>

---

## test_mqtt - ✅ 16 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host unit tests for the MQTT 3.1.1 packet codec (env:native_mqtt)._

|   # | Test                                   | Status | Description                                        |
| --: | :------------------------------------- | :----: | :------------------------------------------------- |
|   1 | `test_remlen_boundaries`               |   ✅   | Remlen boundaries                                  |
|   2 | `test_remlen_too_big`                  |   ✅   | Remlen too big                                     |
|   3 | `test_remlen_decode_incomplete`        |   ✅   | Remlen decode incomplete                           |
|   4 | `test_remlen_decode_malformed`         |   ✅   | Remlen decode malformed                            |
|   5 | `test_connect_minimal`                 |   ✅   | Connect minimal                                    |
|   6 | `test_connect_full`                    |   ✅   | Connect full                                       |
|   7 | `test_publish_qos0_roundtrip`          |   ✅   | Publish qos0 roundtrip                             |
|   8 | `test_publish_qos1_flags_and_id`       |   ✅   | Publish qos1 flags and id                          |
|   9 | `test_publish_topic_overflow_rejected` |   ✅   | Publish topic overflow rejected                    |
|  10 | `test_subscribe`                       |   ✅   | Subscribe                                          |
|  11 | `test_unsubscribe`                     |   ✅   | Unsubscribe                                        |
|  12 | `test_ack_packets`                     |   ✅   | Ack packets                                        |
|  13 | `test_connack`                         |   ✅   | Connack                                            |
|  14 | `test_suback`                          |   ✅   | Suback                                             |
|  15 | `test_ping_disconnect`                 |   ✅   | Ping disconnect                                    |
|  16 | `test_fixed_header_multibyte_remlen`   |   ✅   | Remaining length 300 -> 2-byte field {0xAC, 0x02}. |

</details>

---

## test_ws_client - ✅ 10 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host unit tests for the outbound WebSocket client codec (env:native_ws_client)._

|   # | Test                             | Status | Description                           |
| --: | :------------------------------- | :----: | :------------------------------------ |
|   1 | `test_accept_rfc_example`        |   ✅   | Accept rfc example                    |
|   2 | `test_build_handshake`           |   ✅   | Build handshake                       |
|   3 | `test_check_response_ok`         |   ✅   | Check response ok                     |
|   4 | `test_check_response_bad_accept` |   ✅   | Check response bad accept             |
|   5 | `test_check_response_not_101`    |   ✅   | Check response not 101                |
|   6 | `test_build_frame_masked`        |   ✅   | Build frame masked                    |
|   7 | `test_build_frame_extended_len`  |   ✅   | Build frame extended len              |
|   8 | `test_parse_frame_server_text`   |   ✅   | Server (unmasked) text frame "hello". |
|   9 | `test_parse_frame_incomplete`    |   ✅   | Parse frame incomplete                |
|  10 | `test_parse_frame_extended_len`  |   ✅   | Parse frame extended len              |

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

## test_snmp_trap - ✅ 3 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Host unit tests for the outbound SNMP notification builder (env:native_snmp_trap)._

|   # | Test                      | Status | Description        |
| --: | :------------------------ | :----: | :----------------- |
|   1 | `test_trap_v2c_structure` |   ✅   | Trap v2c structure |
|   2 | `test_inform_tag`         |   ✅   | Inform tag         |
|   3 | `test_buffer_too_small`   |   ✅   | Buffer too small   |

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

## test_websocket - ✅ 67 passed

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
|  51 | `test_ws_continuation_without_start_rejected`          |   ✅   | CONTINUATION with no message in progress (RFC 6455 §5.4) → 1002.          |
|  52 | `test_ws_new_data_frame_during_fragmentation_rejected` |   ✅   | A second TEXT (new message) before finishing the first is illegal.        |
|  53 | `test_ws_parse_ping_auto_pong_resets_frame`            |   ✅   | FIN=1, PING=0x09: byte0 = 0x89                                            |
|  54 | `test_ws_parse_pong_silently_ignored`                  |   ✅   | FIN=1, PONG=0x0A: byte0 = 0x8A                                            |
|  55 | `test_ws_parse_close_marks_ws_closed`                  |   ✅   | FIN=1, CLOSE=0x08: byte0 = 0x88                                           |
|  56 | `test_ws_parse_stops_at_frame_ready`                   |   ✅   | Push two complete frames -- parser should stop after the first            |
|  57 | `test_ws_reset_frame_clears_fields`                    |   ✅   | Ws reset frame clears fields                                              |
|  58 | `test_ws_parse_mask_applied_correctly`                 |   ✅   | Frame with mask key {0x37, 0xFA, 0x21, 0x3D}, payload 'H' XOR 0x37 = 0x7F |
|  59 | `test_ws_permessage_deflate_inbound`                   |   ✅   | "Hello, World!" as permessage-deflate (SYNC_FLUSH, marker stripped) - the |
|  60 | `test_ws_rsv1_without_negotiation_closes`              |   ✅   | Ws rsv1 without negotiation closes                                        |
|  61 | `test_ws_permessage_deflate_outbound`                  |   ✅   | Ws permessage deflate outbound                                            |
|  62 | `test_ws_outbound_incompressible_not_flagged`          |   ✅   | Ws outbound incompressible not flagged                                    |
|  63 | `stress_ws_parse_reset_100_cycles`                     |   ✅   | Stress - Ws parse reset 100 cycles                                        |
|  64 | `stress_ws_alloc_free_pool_cycle`                      |   ✅   | Stress - Ws alloc free pool cycle                                         |
|  65 | `stress_ws_parse_incremental_byte_by_byte`             |   ✅   | Stress - Ws parse incremental byte by byte                                |
|  66 | `stress_ws_parse_max_payload`                          |   ✅   | Stress - Ws parse max payload                                             |
|  67 | `stress_ws_parse_two_consecutive_frames`               |   ✅   | First frame                                                               |

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

## test_dashboard - ✅ 11 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the dashboard widget-table JSON serializers (services/dashboard_

|   # | Test                                   | Status | Description                     |
| --: | :------------------------------------- | :----: | :------------------------------ |
|   1 | `test_layout_json`                     |   ✅   | Layout json                     |
|   2 | `test_values_json_initial_zero`        |   ✅   | Values json initial zero        |
|   3 | `test_set_and_values`                  |   ✅   | Set and values                  |
|   4 | `test_set_unknown_key`                 |   ✅   | Set unknown key                 |
|   5 | `test_configure_resets_values`         |   ✅   | Configure resets values         |
|   6 | `test_small_buffer_fails_closed`       |   ✅   | Small buffer fails closed       |
|   7 | `test_parse_control_ok`                |   ✅   | Parse control ok                |
|   8 | `test_parse_control_float`             |   ✅   | Parse control float             |
|   9 | `test_parse_control_rejects_malformed` |   ✅   | Parse control rejects malformed |
|  10 | `test_dispatch_control_invokes_cb`     |   ✅   | Dispatch control invokes cb     |
|  11 | `test_layout_control_types`            |   ✅   | Layout control types            |

</details>

---

## test_net_egress - ✅ 5 passed

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

## test_cbor - ✅ 14 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the CBOR encoder (network_drivers/presentation/cbor). Expected_

|   # | Test                          | Status | Description            |
| --: | :---------------------------- | :----: | :--------------------- |
|   1 | `test_uint`                   |   ✅   | Uint                   |
|   2 | `test_int`                    |   ✅   | Int                    |
|   3 | `test_text`                   |   ✅   | Text                   |
|   4 | `test_bytes`                  |   ✅   | Bytes                  |
|   5 | `test_simple`                 |   ✅   | Simple                 |
|   6 | `test_float`                  |   ✅   | Float                  |
|   7 | `test_array_and_map`          |   ✅   | Array and map          |
|   8 | `test_overflow_fails_closed`  |   ✅   | Overflow fails closed  |
|   9 | `test_decode_uint`            |   ✅   | Decode uint            |
|  10 | `test_decode_int`             |   ✅   | Decode int             |
|  11 | `test_decode_float_roundtrip` |   ✅   | Decode float roundtrip |
|  12 | `test_decode_roundtrip_map`   |   ✅   | Decode roundtrip map   |
|  13 | `test_decode_truncated`       |   ✅   | Decode truncated       |
|  14 | `test_decode_type_mismatch`   |   ✅   | Decode type mismatch   |

</details>

---

## test_msgpack - ✅ 8 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the MessagePack encoder (network_drivers/presentation/msgpack)._

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_uint`                  |   ✅   | Uint                  |
|   2 | `test_int`                   |   ✅   | Int                   |
|   3 | `test_str`                   |   ✅   | Str                   |
|   4 | `test_bytes`                 |   ✅   | Bytes                 |
|   5 | `test_simple`                |   ✅   | Simple                |
|   6 | `test_float`                 |   ✅   | Float                 |
|   7 | `test_array_and_map`         |   ✅   | Array and map         |
|   8 | `test_overflow_fails_closed` |   ✅   | Overflow fails closed |

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

## test_udp_telemetry - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the UDP telemetry line builder (services/udp_telemetry): the_

|   # | Test                         | Status | Description           |
| --: | :--------------------------- | :----: | :-------------------- |
|   1 | `test_int_and_uint_fields`   |   ✅   | Int and uint fields   |
|   2 | `test_float_field`           |   ✅   | Float field           |
|   3 | `test_no_fields_not_ok`      |   ✅   | No fields not ok      |
|   4 | `test_overflow_fails_closed` |   ✅   | Overflow fails closed |

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

## test_clock - ✅ 4 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit tests for the pluggable monotonic clock (services/det_clock): the platform_

|   # | Test                                  | Status | Description                    |
| --: | :------------------------------------ | :----: | :----------------------------- |
|   1 | `test_default_is_platform_millis`     |   ✅   | Default is platform millis     |
|   2 | `test_custom_clock_divides_to_1000hz` |   ✅   | Custom clock divides to 1000hz |
|   3 | `test_sub_khz_source_not_divided`     |   ✅   | Sub khz source not divided     |
|   4 | `test_revert_to_default`              |   ✅   | Revert to default              |

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
Collected 88 tests
Platform Manager: Installing native
Downloading 0% 10%
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Platform Manager: native@1.2.1 has been installed!

Processing test_sse in native environment
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
test/test_sse/test_sse.cpp:372: test_sse_pool_size                      [PASSED]
test/test_sse/test_sse.cpp:373: test_sse_ids_match_indices_after_init   [PASSED]
test/test_sse/test_sse.cpp:374: test_sse_all_inactive_after_init        [PASSED]
test/test_sse/test_sse.cpp:375: test_sse_path_empty_after_init          [PASSED]
test/test_sse/test_sse.cpp:378: test_sse_alloc_returns_non_null         [PASSED]
test/test_sse/test_sse.cpp:379: test_sse_alloc_sets_active              [PASSED]
test/test_sse/test_sse.cpp:380: test_sse_alloc_sets_slot_id             [PASSED]
test/test_sse/test_sse.cpp:381: test_sse_alloc_stores_path              [PASSED]
test/test_sse/test_sse.cpp:382: test_sse_alloc_stores_different_paths_per_slot [PASSED]
test/test_sse/test_sse.cpp:383: test_sse_alloc_path_truncated_to_max    [PASSED]
test/test_sse/test_sse.cpp:384: test_sse_alloc_pool_full_returns_null   [PASSED]
test/test_sse/test_sse.cpp:385: test_sse_alloc_sse_id_is_pool_index     [PASSED]
test/test_sse/test_sse.cpp:388: test_sse_find_returns_correct_conn      [PASSED]
test/test_sse/test_sse.cpp:389: test_sse_find_returns_null_when_empty   [PASSED]
test/test_sse/test_sse.cpp:390: test_sse_find_returns_null_for_different_slot [PASSED]
test/test_sse/test_sse.cpp:391: test_sse_find_after_both_slots_allocated [PASSED]
test/test_sse/test_sse.cpp:392: test_sse_find_checks_slot_id_not_sse_id [PASSED]
test/test_sse/test_sse.cpp:395: test_sse_free_deactivates_slot          [PASSED]
test/test_sse/test_sse.cpp:396: test_sse_free_restores_sse_id           [PASSED]
test/test_sse/test_sse.cpp:397: test_sse_free_makes_slot_findable_as_null [PASSED]
test/test_sse/test_sse.cpp:398: test_sse_free_clears_path               [PASSED]
test/test_sse/test_sse.cpp:399: test_sse_free_nop_on_unallocated        [PASSED]
test/test_sse/test_sse.cpp:400: test_sse_alloc_after_free_succeeds      [PASSED]
test/test_sse/test_sse.cpp:401: test_sse_free_only_frees_matching_slot  [PASSED]
test/test_sse/test_sse.cpp:404: test_sse_write_null_data_returns_false  [PASSED]
test/test_sse/test_sse.cpp:405: test_sse_write_returns_false_when_conn_not_active [PASSED]
test/test_sse/test_sse.cpp:406: test_sse_write_returns_false_when_pcb_null [PASSED]
test/test_sse/test_sse.cpp:407: test_sse_write_data_only_returns_true   [PASSED]
test/test_sse/test_sse.cpp:408: test_sse_write_with_event_returns_true  [PASSED]
test/test_sse/test_sse.cpp:409: test_sse_write_with_id_returns_true     [PASSED]
test/test_sse/test_sse.cpp:410: test_sse_write_with_all_fields_returns_true [PASSED]
test/test_sse/test_sse.cpp:411: test_sse_write_does_not_affect_other_slots [PASSED]
test/test_sse/test_sse.cpp:414: stress_sse_alloc_free_100_cycles        [PASSED]
test/test_sse/test_sse.cpp:415: stress_sse_alloc_free_both_slots_alternating [PASSED]
test/test_sse/test_sse.cpp:416: stress_sse_write_100_calls              [PASSED]
test/test_sse/test_sse.cpp:417: stress_sse_find_with_full_pool          [PASSED]
test/test_sse/test_sse.cpp:418: stress_sse_write_slot_isolation         [PASSED]
----------------- native:test_sse [PASSED] Took 15.41 seconds ------------------

Processing test_session in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_session/test_session.cpp:347: test_empty_queue_does_not_crash [PASSED]
test/test_session/test_session.cpp:348: test_pool_initializes_to_parse_method [PASSED]
test/test_session/test_session.cpp:349: test_reset_clears_mid_parse_state [PASSED]
test/test_session/test_session.cpp:350: test_tick_fires_check_timeouts_stale_slot_freed [PASSED]
test/test_session/test_session.cpp:351: test_tick_does_not_free_fresh_connection [PASSED]
test/test_session/test_session.cpp:354: test_fn_tick_timeout_before_event_drain_ordering [PASSED]
test/test_session/test_session.cpp:355: test_fn_tick_only_active_slots_expire [PASSED]
test/test_session/test_session.cpp:358: stress_1000_idle_ticks_stable   [PASSED]
test/test_session/test_session.cpp:359: stress_timeout_all_slots_10_cycles [PASSED]
test/test_session/test_session.cpp:360: stress_mixed_fresh_stale_slots_many_ticks [PASSED]
test/test_session/test_session.cpp:363: test_evt_connect_calls_http_reset [PASSED]
test/test_session/test_session.cpp:364: test_evt_disconnect_calls_http_reset [PASSED]
test/test_session/test_session.cpp:365: test_evt_error_calls_http_reset [PASSED]
test/test_session/test_session.cpp:366: test_evt_data_calls_http_parse  [PASSED]
test/test_session/test_session.cpp:367: test_multiple_events_drained_in_one_tick [PASSED]
test/test_session/test_session.cpp:370: race_external_free_between_ticks [PASSED]
test/test_session/test_session.cpp:371: race_activity_update_saves_slot_from_timeout [PASSED]
test/test_session/test_session.cpp:372: race_all_expire_then_idle_tick  [PASSED]
test/test_session/test_session.cpp:373: race_millis_wraparound_no_spurious_timeout [PASSED]
---------------- native:test_session [PASSED] Took 0.53 seconds ----------------

Processing test_presentation in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_presentation/test_presentation.cpp:844: test_fn_reset_sets_parse_state_to_method [PASSED]
test/test_presentation/test_presentation.cpp:845: test_fn_reset_sets_slot_id [PASSED]
test/test_presentation/test_presentation.cpp:846: test_fn_reset_clears_method [PASSED]
test/test_presentation/test_presentation.cpp:847: test_fn_reset_clears_path_and_idx [PASSED]
test/test_presentation/test_presentation.cpp:848: test_fn_reset_clears_query_raw_and_params [PASSED]
test/test_presentation/test_presentation.cpp:849: test_fn_reset_clears_all_header_slots [PASSED]
test/test_presentation/test_presentation.cpp:850: test_fn_reset_clears_body_fields [PASSED]
test/test_presentation/test_presentation.cpp:851: test_fn_reset_out_of_range_is_nop [PASSED]
test/test_presentation/test_presentation.cpp:852: test_fn_reset_is_idempotent [PASSED]
test/test_presentation/test_presentation.cpp:855: test_fn_get_header_null_when_no_headers [PASSED]
test/test_presentation/test_presentation.cpp:856: test_fn_get_header_finds_single_header [PASSED]
test/test_presentation/test_presentation.cpp:857: test_fn_get_header_finds_first_of_many [PASSED]
test/test_presentation/test_presentation.cpp:858: test_fn_get_header_finds_middle_of_many [PASSED]
test/test_presentation/test_presentation.cpp:859: test_fn_get_header_finds_last_of_many [PASSED]
test/test_presentation/test_presentation.cpp:860: test_fn_get_header_case_insensitive_lowercase [PASSED]
test/test_presentation/test_presentation.cpp:861: test_fn_get_header_case_insensitive_uppercase [PASSED]
test/test_presentation/test_presentation.cpp:862: test_fn_get_header_returns_null_for_absent_key [PASSED]
test/test_presentation/test_presentation.cpp:863: test_fn_get_header_does_not_bleed_across_slots [PASSED]
test/test_presentation/test_presentation.cpp:866: test_fn_get_query_null_when_no_params [PASSED]
test/test_presentation/test_presentation.cpp:867: test_fn_get_query_finds_single_param [PASSED]
test/test_presentation/test_presentation.cpp:868: test_fn_get_query_finds_first_param [PASSED]
test/test_presentation/test_presentation.cpp:869: test_fn_get_query_finds_middle_param [PASSED]
test/test_presentation/test_presentation.cpp:870: test_fn_get_query_finds_last_param [PASSED]
test/test_presentation/test_presentation.cpp:871: test_fn_get_query_returns_null_for_absent_key [PASSED]
test/test_presentation/test_presentation.cpp:872: test_fn_get_query_empty_value [PASSED]
test/test_presentation/test_presentation.cpp:873: test_fn_get_query_does_not_bleed_across_slots [PASSED]
test/test_presentation/test_presentation.cpp:876: test_get_parses_complete [PASSED]
test/test_presentation/test_presentation.cpp:877: test_post_body_stored [PASSED]
test/test_presentation/test_presentation.cpp:878: test_put_parses_complete [PASSED]
test/test_presentation/test_presentation.cpp:879: test_delete_parses_complete [PASSED]
test/test_presentation/test_presentation.cpp:880: test_patch_parses_complete [PASSED]
test/test_presentation/test_presentation.cpp:881: test_head_parses_complete [PASSED]
test/test_presentation/test_presentation.cpp:882: test_query_single_param [PASSED]
test/test_presentation/test_presentation.cpp:883: test_query_multiple_params [PASSED]
test/test_presentation/test_presentation.cpp:884: test_body_null_terminated [PASSED]
test/test_presentation/test_presentation.cpp:885: test_body_over_buf_size_is_413 [PASSED]
test/test_presentation/test_presentation.cpp:886: test_overflow_method_sets_error [PASSED]
test/test_presentation/test_presentation.cpp:887: test_overflow_path_sets_414 [PASSED]
test/test_presentation/test_presentation.cpp:888: test_bad_lf_after_cr_sets_error [PASSED]
test/test_presentation/test_presentation.cpp:889: test_headers_beyond_max_are_dropped [PASSED]
test/test_presentation/test_presentation.cpp:890: test_query_params_beyond_max_are_dropped [PASSED]
test/test_presentation/test_presentation.cpp:891: test_incremental_two_pushes_completes [PASSED]
test/test_presentation/test_presentation.cpp:892: test_body_starting_with_newline_stored [PASSED]
test/test_presentation/test_presentation.cpp:893: test_put_body_stored  [PASSED]
test/test_presentation/test_presentation.cpp:894: test_content_length_header_stored_in_headers_array [PASSED]
test/test_presentation/test_presentation.cpp:897: stress_parse_reset_100_cycles [PASSED]
test/test_presentation/test_presentation.cpp:898: stress_all_slots_parse_simultaneously [PASSED]
test/test_presentation/test_presentation.cpp:899: stress_method_at_max_7_chars_no_error [PASSED]
test/test_presentation/test_presentation.cpp:900: stress_path_at_exact_limit_no_error [PASSED]
test/test_presentation/test_presentation.cpp:901: stress_body_exactly_buf_size_all_stored [PASSED]
test/test_presentation/test_presentation.cpp:902: stress_exactly_max_headers_all_stored [PASSED]
test/test_presentation/test_presentation.cpp:903: stress_exactly_max_query_params_all_stored [PASSED]
test/test_presentation/test_presentation.cpp:904: stress_incremental_byte_by_byte_no_error [PASSED]
test/test_presentation/test_presentation.cpp:905: stress_sequential_requests_no_state_leak [PASSED]
test/test_presentation/test_presentation.cpp:908: race_interleaved_producer_consumer_ring_buffer [PASSED]
test/test_presentation/test_presentation.cpp:909: race_ring_buffer_full_prevents_write [PASSED]
test/test_presentation/test_presentation.cpp:910: race_aba_slot_reuse_fresh_timestamp [PASSED]
test/test_presentation/test_presentation.cpp:911: race_double_free_is_nop [PASSED]
test/test_presentation/test_presentation.cpp:912: race_concurrent_slot_parse_isolation [PASSED]
test/test_presentation/test_presentation.cpp:913: race_reset_during_parse_header_val [PASSED]
test/test_presentation/test_presentation.cpp:914: race_reset_during_parse_query [PASSED]
test/test_presentation/test_presentation.cpp:915: race_reset_during_parse_body [PASSED]
test/test_presentation/test_presentation.cpp:916: race_parse_after_complete_is_nop [PASSED]
------------- native:test_presentation [PASSED] Took 0.57 seconds --------------

Processing test_transport in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_transport/test_transport.cpp:539: test_pool_capacity_is_four  [PASSED]
test/test_transport/test_transport.cpp:540: test_rx_buffer_size_is_one_kb [PASSED]
test/test_transport/test_transport.cpp:541: test_timeout_constant_is_5000ms [PASSED]
test/test_transport/test_transport.cpp:542: test_all_slots_free_after_init [PASSED]
test/test_transport/test_transport.cpp:543: test_all_pcbs_null_after_init [PASSED]
test/test_transport/test_transport.cpp:544: test_all_ring_buffers_empty_after_init [PASSED]
test/test_transport/test_transport.cpp:545: test_slot_ids_match_indices [PASSED]
test/test_transport/test_transport.cpp:546: test_ring_empty_when_head_equals_tail [PASSED]
test/test_transport/test_transport.cpp:547: test_ring_wrap_at_boundary  [PASSED]
test/test_transport/test_transport.cpp:548: test_ring_full_sentinel_one_slot_reserved [PASSED]
test/test_transport/test_transport.cpp:549: test_ring_can_store_size_minus_one_bytes [PASSED]
test/test_transport/test_transport.cpp:550: test_event_types_are_distinct [PASSED]
test/test_transport/test_transport.cpp:551: test_timeout_does_not_fire_on_free_slot [PASSED]
test/test_transport/test_transport.cpp:552: test_timeout_does_not_fire_before_deadline [PASSED]
test/test_transport/test_transport.cpp:553: test_timeout_fires_at_deadline [PASSED]
test/test_transport/test_transport.cpp:554: test_timeout_fires_only_on_stale_slots [PASSED]
test/test_transport/test_transport.cpp:555: test_init_succeeds_on_native [PASSED]
test/test_transport/test_transport.cpp:556: test_all_last_activity_ms_zero_after_init [PASSED]
test/test_transport/test_transport.cpp:557: test_queue_not_null_after_init [PASSED]
test/test_transport/test_transport.cpp:560: stress_ring_buffer_fill_drain_integrity [PASSED]
test/test_transport/test_transport.cpp:561: stress_ring_buffer_multi_cycle_no_corruption [PASSED]
test/test_transport/test_transport.cpp:562: stress_all_slots_timeout_simultaneously [PASSED]
test/test_transport/test_transport.cpp:563: stress_timeout_arm_recover_cycle [PASSED]
test/test_transport/test_transport.cpp:564: stress_check_timeouts_high_call_rate [PASSED]
test/test_transport/test_transport.cpp:565: stress_ring_buffer_byte_by_byte_fill_and_drain [PASSED]
test/test_transport/test_transport.cpp:568: test_accept_throttle_blocks_over_budget [PASSED]
test/test_transport/test_transport.cpp:569: test_accept_throttle_window_refills [PASSED]
test/test_transport/test_transport.cpp:570: test_accept_throttle_handles_rollover [PASSED]
test/test_transport/test_transport.cpp:573: test_per_ip_throttle_blocks_over_budget [PASSED]
test/test_transport/test_transport.cpp:574: test_per_ip_throttle_isolates_addresses [PASSED]
test/test_transport/test_transport.cpp:575: test_per_ip_throttle_window_refills [PASSED]
test/test_transport/test_transport.cpp:576: test_per_ip_throttle_evicts_when_full [PASSED]
test/test_transport/test_transport.cpp:577: test_per_ip_throttle_zero_ip_always_allowed [PASSED]
test/test_transport/test_transport.cpp:578: test_per_ip_throttle_handles_rollover [PASSED]
test/test_transport/test_transport.cpp:581: test_ip_allowlist_empty_allows_all [PASSED]
test/test_transport/test_transport.cpp:582: test_ip_allowlist_host_match [PASSED]
test/test_transport/test_transport.cpp:583: test_ip_allowlist_cidr_match [PASSED]
test/test_transport/test_transport.cpp:584: test_ip_allowlist_masks_host_bits [PASSED]
test/test_transport/test_transport.cpp:585: test_ip_allowlist_multiple_rules [PASSED]
test/test_transport/test_transport.cpp:586: test_ip_allowlist_zero_prefix_matches_all [PASSED]
test/test_transport/test_transport.cpp:587: test_ip_allowlist_rejects_bad_prefix [PASSED]
test/test_transport/test_transport.cpp:588: test_ip_allowlist_table_full [PASSED]
--------------- native:test_transport [PASSED] Took 0.56 seconds ---------------

Processing test_websocket in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_websocket/test_websocket.cpp:962: test_sha1_empty_string      [PASSED]
test/test_websocket/test_websocket.cpp:963: test_sha1_abc               [PASSED]
test/test_websocket/test_websocket.cpp:964: test_sha1_rfc6455_handshake_key [PASSED]
test/test_websocket/test_websocket.cpp:965: test_sha1_different_inputs_different_digests [PASSED]
test/test_websocket/test_websocket.cpp:968: test_base64_encode_one_byte [PASSED]
test/test_websocket/test_websocket.cpp:969: test_base64_encode_two_bytes [PASSED]
test/test_websocket/test_websocket.cpp:970: test_base64_encode_three_bytes [PASSED]
test/test_websocket/test_websocket.cpp:971: test_base64_encode_ws_accept_key [PASSED]
test/test_websocket/test_websocket.cpp:972: test_base64_decode_one_byte [PASSED]
test/test_websocket/test_websocket.cpp:973: test_base64_decode_two_bytes [PASSED]
test/test_websocket/test_websocket.cpp:974: test_base64_decode_three_bytes [PASSED]
test/test_websocket/test_websocket.cpp:975: test_base64_decode_ws_accept_key [PASSED]
test/test_websocket/test_websocket.cpp:976: test_base64_decode_rejects_misplaced_padding [PASSED]
test/test_websocket/test_websocket.cpp:977: test_base64_decode_respects_capacity [PASSED]
test/test_websocket/test_websocket.cpp:978: test_base64_round_trip      [PASSED]
test/test_websocket/test_websocket.cpp:981: test_ws_pool_size           [PASSED]
test/test_websocket/test_websocket.cpp:982: test_ws_ids_match_indices_after_init [PASSED]
test/test_websocket/test_websocket.cpp:983: test_ws_all_inactive_after_init [PASSED]
test/test_websocket/test_websocket.cpp:984: test_ws_alloc_returns_non_null [PASSED]
test/test_websocket/test_websocket.cpp:985: test_ws_alloc_sets_active   [PASSED]
test/test_websocket/test_websocket.cpp:986: test_ws_alloc_sets_slot_id  [PASSED]
test/test_websocket/test_websocket.cpp:987: test_ws_alloc_sets_parse_state_header1 [PASSED]
test/test_websocket/test_websocket.cpp:988: test_ws_alloc_pool_full_returns_null [PASSED]
test/test_websocket/test_websocket.cpp:989: test_ws_find_returns_correct_conn [PASSED]
test/test_websocket/test_websocket.cpp:990: test_ws_find_returns_null_when_empty [PASSED]
test/test_websocket/test_websocket.cpp:991: test_ws_find_returns_null_for_different_slot [PASSED]
test/test_websocket/test_websocket.cpp:992: test_ws_find_after_both_slots_allocated [PASSED]
test/test_websocket/test_websocket.cpp:993: test_ws_free_deactivates_slot [PASSED]
test/test_websocket/test_websocket.cpp:994: test_ws_free_restores_ws_id [PASSED]
test/test_websocket/test_websocket.cpp:995: test_ws_free_makes_slot_findable_as_null [PASSED]
test/test_websocket/test_websocket.cpp:996: test_ws_free_nop_on_unallocated [PASSED]
test/test_websocket/test_websocket.cpp:997: test_ws_alloc_after_free_succeeds [PASSED]
test/test_websocket/test_websocket.cpp:1000: test_ws_parse_text_frame_sets_ready [PASSED]
test/test_websocket/test_websocket.cpp:1001: test_ws_parse_payload_stored_correctly [PASSED]
test/test_websocket/test_websocket.cpp:1002: test_ws_parse_binary_frame_sets_ready [PASSED]
test/test_websocket/test_websocket.cpp:1003: test_ws_parse_zero_length_unmasked_frame [PASSED]
test/test_websocket/test_websocket.cpp:1004: test_ws_parse_zero_length_masked_frame [PASSED]
test/test_websocket/test_websocket.cpp:1005: test_ws_reject_unmasked_data_frame [PASSED]
test/test_websocket/test_websocket.cpp:1006: test_ws_reject_reserved_opcode [PASSED]
test/test_websocket/test_websocket.cpp:1007: test_ws_reject_fragmented_control_frame [PASSED]
test/test_websocket/test_websocket.cpp:1008: test_ws_reject_oversized_control_frame [PASSED]
test/test_websocket/test_websocket.cpp:1009: test_ws_parse_16bit_length_frame [PASSED]
test/test_websocket/test_websocket.cpp:1010: test_ws_parse_rsv1_set_closes_protocol [PASSED]
test/test_websocket/test_websocket.cpp:1011: test_ws_parse_rsv2_set_closes_protocol [PASSED]
test/test_websocket/test_websocket.cpp:1012: test_ws_parse_rsv3_set_closes_protocol [PASSED]
test/test_websocket/test_websocket.cpp:1013: test_ws_parse_64bit_length_closes_too_big [PASSED]
test/test_websocket/test_websocket.cpp:1014: test_ws_parse_oversized_16bit_length_closes_too_big [PASSED]
test/test_websocket/test_websocket.cpp:1015: test_ws_fragment_start_waits_for_continuation [PASSED]
test/test_websocket/test_websocket.cpp:1016: test_ws_fragmented_message_reassembled [PASSED]
test/test_websocket/test_websocket.cpp:1017: test_ws_control_frame_interleaved_in_fragments [PASSED]
test/test_websocket/test_websocket.cpp:1018: test_ws_continuation_without_start_rejected [PASSED]
test/test_websocket/test_websocket.cpp:1019: test_ws_new_data_frame_during_fragmentation_rejected [PASSED]
test/test_websocket/test_websocket.cpp:1020: test_ws_parse_ping_auto_pong_resets_frame [PASSED]
test/test_websocket/test_websocket.cpp:1021: test_ws_parse_pong_silently_ignored [PASSED]
test/test_websocket/test_websocket.cpp:1022: test_ws_parse_close_marks_ws_closed [PASSED]
test/test_websocket/test_websocket.cpp:1023: test_ws_parse_stops_at_frame_ready [PASSED]
test/test_websocket/test_websocket.cpp:1024: test_ws_reset_frame_clears_fields [PASSED]
test/test_websocket/test_websocket.cpp:1025: test_ws_parse_mask_applied_correctly [PASSED]
test/test_websocket/test_websocket.cpp:1034: stress_ws_parse_reset_100_cycles [PASSED]
test/test_websocket/test_websocket.cpp:1035: stress_ws_alloc_free_pool_cycle [PASSED]
test/test_websocket/test_websocket.cpp:1036: stress_ws_parse_incremental_byte_by_byte [PASSED]
test/test_websocket/test_websocket.cpp:1037: stress_ws_parse_max_payload [PASSED]
test/test_websocket/test_websocket.cpp:1038: stress_ws_parse_two_consecutive_frames [PASSED]
--------------- native:test_websocket [PASSED] Took 0.58 seconds ---------------

Processing test_http_parser in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_http_parser/test_http_parser.cpp:853: test_reset_sets_parse_method_state [PASSED]
test/test_http_parser/test_http_parser.cpp:854: test_reset_preserves_slot_id [PASSED]
test/test_http_parser/test_http_parser.cpp:855: test_reset_clears_method [PASSED]
test/test_http_parser/test_http_parser.cpp:856: test_reset_clears_path  [PASSED]
test/test_http_parser/test_http_parser.cpp:857: test_reset_clears_header_count [PASSED]
test/test_http_parser/test_http_parser.cpp:858: test_reset_clears_body  [PASSED]
test/test_http_parser/test_http_parser.cpp:859: test_reset_clears_query_count [PASSED]
test/test_http_parser/test_http_parser.cpp:862: test_feed_after_complete_does_not_change_state [PASSED]
test/test_http_parser/test_http_parser.cpp:863: test_feed_after_error_does_not_change_state [PASSED]
test/test_http_parser/test_http_parser.cpp:864: test_feed_after_entity_too_large_does_not_change_state [PASSED]
test/test_http_parser/test_http_parser.cpp:867: test_method_get         [PASSED]
test/test_http_parser/test_http_parser.cpp:868: test_method_post        [PASSED]
test/test_http_parser/test_http_parser.cpp:869: test_method_put         [PASSED]
test/test_http_parser/test_http_parser.cpp:870: test_method_delete      [PASSED]
test/test_http_parser/test_http_parser.cpp:871: test_method_patch       [PASSED]
test/test_http_parser/test_http_parser.cpp:872: test_method_head        [PASSED]
test/test_http_parser/test_http_parser.cpp:873: test_method_options     [PASSED]
test/test_http_parser/test_http_parser.cpp:874: test_method_overflow_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:877: test_path_root          [PASSED]
test/test_http_parser/test_http_parser.cpp:878: test_path_segments      [PASSED]
test/test_http_parser/test_http_parser.cpp:879: test_path_without_query [PASSED]
test/test_http_parser/test_http_parser.cpp:880: test_path_overflow_is_414 [PASSED]
test/test_http_parser/test_http_parser.cpp:883: test_single_query_param [PASSED]
test/test_http_parser/test_http_parser.cpp:884: test_two_query_params   [PASSED]
test/test_http_parser/test_http_parser.cpp:885: test_query_key_not_found_returns_null [PASSED]
test/test_http_parser/test_http_parser.cpp:886: test_query_empty_value  [PASSED]
test/test_http_parser/test_http_parser.cpp:889: test_single_header_stored [PASSED]
test/test_http_parser/test_http_parser.cpp:890: test_header_lookup_case_insensitive [PASSED]
test/test_http_parser/test_http_parser.cpp:891: test_header_leading_space_stripped [PASSED]
test/test_http_parser/test_http_parser.cpp:892: test_content_length_header_parsed [PASSED]
test/test_http_parser/test_http_parser.cpp:893: test_content_length_in_headers_array [PASSED]
test/test_http_parser/test_http_parser.cpp:894: test_multiple_headers_stored [PASSED]
test/test_http_parser/test_http_parser.cpp:895: test_missing_header_returns_null [PASSED]
test/test_http_parser/test_http_parser.cpp:898: test_get_no_body_completes [PASSED]
test/test_http_parser/test_http_parser.cpp:899: test_post_with_body     [PASSED]
test/test_http_parser/test_http_parser.cpp:900: test_put_with_body      [PASSED]
test/test_http_parser/test_http_parser.cpp:901: test_body_starting_with_newline [PASSED]
test/test_http_parser/test_http_parser.cpp:902: test_post_content_length_zero [PASSED]
test/test_http_parser/test_http_parser.cpp:903: test_body_exactly_at_buffer_limit [PASSED]
test/test_http_parser/test_http_parser.cpp:904: test_body_null_terminated_after_complete [PASSED]
test/test_http_parser/test_http_parser.cpp:907: test_body_one_over_limit_is_413 [PASSED]
test/test_http_parser/test_http_parser.cpp:908: test_body_far_over_limit_is_413 [PASSED]
test/test_http_parser/test_http_parser.cpp:909: test_413_no_body_bytes_fed [PASSED]
test/test_http_parser/test_http_parser.cpp:910: test_413_header_still_stored [PASSED]
test/test_http_parser/test_http_parser.cpp:911: test_body_exactly_at_limit_is_not_413 [PASSED]
test/test_http_parser/test_http_parser.cpp:914: test_path_overflow_stops_feeding [PASSED]
test/test_http_parser/test_http_parser.cpp:915: test_414_path_filled_to_capacity [PASSED]
test/test_http_parser/test_http_parser.cpp:918: test_method_nul_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:919: test_method_control_char_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:920: test_method_del_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:921: test_method_non_tchar_symbol_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:922: test_method_tchar_symbols_accepted [PASSED]
test/test_http_parser/test_http_parser.cpp:925: test_path_nul_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:926: test_path_control_char_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:927: test_path_del_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:928: test_query_nul_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:929: test_query_control_char_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:932: test_header_key_space_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:933: test_header_key_nul_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:934: test_header_key_control_char_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:935: test_header_key_mid_cr_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:936: test_header_key_colon_at_start_skips_header [PASSED]
test/test_http_parser/test_http_parser.cpp:937: test_long_standard_header_key_accepted [PASSED]
test/test_http_parser/test_http_parser.cpp:938: test_overlong_header_key_truncated_not_error [PASSED]
test/test_http_parser/test_http_parser.cpp:941: test_header_val_nul_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:942: test_header_val_control_char_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:943: test_header_val_del_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:944: test_header_val_htab_mid_value_allowed [PASSED]
test/test_http_parser/test_http_parser.cpp:945: test_header_val_leading_htab_stripped [PASSED]
test/test_http_parser/test_http_parser.cpp:946: test_header_val_obs_text_allowed [PASSED]
test/test_http_parser/test_http_parser.cpp:949: test_version_http11_recognized [PASSED]
test/test_http_parser/test_http_parser.cpp:950: test_version_http10_recognized [PASSED]
test/test_http_parser/test_http_parser.cpp:951: test_version_unknown_is_http_unknown [PASSED]
test/test_http_parser/test_http_parser.cpp:952: test_version_reset_to_unknown [PASSED]
test/test_http_parser/test_http_parser.cpp:955: test_bad_expect_lf_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:956: test_blank_line_non_lf_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:959: test_slots_are_independent [PASSED]
test/test_http_parser/test_http_parser.cpp:962: test_incremental_byte_by_byte [PASSED]
test/test_http_parser/test_http_parser.cpp:963: test_incremental_two_chunks [PASSED]
test/test_http_parser/test_http_parser.cpp:966: stress_many_requests_same_slot [PASSED]
test/test_http_parser/test_http_parser.cpp:967: stress_max_headers      [PASSED]
test/test_http_parser/test_http_parser.cpp:968: stress_max_query_params [PASSED]
-------------- native:test_http_parser [PASSED] Took 0.53 seconds --------------

=================================== SUMMARY ===================================
Environment    Test               Status    Duration
-------------  -----------------  --------  ------------
native         test_sse           PASSED    00:00:15.407
native         test_session       PASSED    00:00:00.530
native         test_presentation  PASSED    00:00:00.568
native         test_transport     PASSED    00:00:00.556
native         test_websocket     PASSED    00:00:00.577
native         test_http_parser   PASSED    00:00:00.532
================ 306 test cases: 306 succeeded in 00:00:18.170 ================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_observability in native_observability environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_observability/test_observability.cpp:265: test_transition_fires_hook_with_args [PASSED]
test/test_observability/test_observability.cpp:266: test_each_reason_bumps_its_counter [PASSED]
test/test_observability/test_observability.cpp:267: test_closing_gauge_is_derived_from_pool [PASSED]
test/test_observability/test_observability.cpp:268: test_reset_clears_cumulative_not_derived_gauge [PASSED]
test/test_observability/test_observability.cpp:269: test_no_hook_after_unregister [PASSED]
test/test_observability/test_observability.cpp:270: test_recv_fin_counts_remote_close [PASSED]
test/test_observability/test_observability.cpp:271: test_err_cb_counts_error_close [PASSED]
test/test_observability/test_observability.cpp:272: test_timeout_sweep_counts_timeout [PASSED]
test/test_observability/test_observability.cpp:273: test_local_close_counts_local [PASSED]
test/test_observability/test_observability.cpp:274: test_backpressure_counts_when_ring_full [PASSED]
test/test_observability/test_observability.cpp:276: test_begin_close_dwells_then_drains_on_ack [PASSED]
test/test_observability/test_observability.cpp:277: test_begin_close_finalizes_immediately_when_already_drained [PASSED]
test/test_observability/test_observability.cpp:278: test_begin_close_noop_if_not_active [PASSED]
test/test_observability/test_observability.cpp:279: test_closing_timeout_reaps_stuck_slot [PASSED]
test/test_observability/test_observability.cpp:280: test_recv_during_closing_is_drained_not_processed [PASSED]
------ native_observability:test_observability [PASSED] Took 0.67 seconds ------

=================================== SUMMARY ===================================
Environment           Test                Status    Duration
--------------------  ------------------  --------  ------------
native_observability  test_observability  PASSED    00:00:00.673
================= 15 test cases: 15 succeeded in 00:00:00.673 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_http_ota in native_ota environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_http_ota/test_http_ota.cpp:94: test_large_body_streams_to_completion [PASSED]
test/test_http_ota/test_http_ota.cpp:95: test_no_hooks_large_body_is_413 [PASSED]
test/test_http_ota/test_http_ota.cpp:96: test_nonmatching_path_not_streamed [PASSED]
------------- native_ota:test_http_ota [PASSED] Took 0.63 seconds --------------

=================================== SUMMARY ===================================
Environment    Test           Status    Duration
-------------  -------------  --------  ------------
native_ota     test_http_ota  PASSED    00:00:00.634
================== 3 test cases: 3 succeeded in 00:00:00.634 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
----------- native_prov:test_provisioning [PASSED] Took 0.68 seconds -----------

=================================== SUMMARY ===================================
Environment    Test               Status    Duration
-------------  -----------------  --------  ------------
native_prov    test_provisioning  PASSED    00:00:00.679
================== 5 test cases: 5 succeeded in 00:00:00.679 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_ssh_crypto in native_ssh environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_crypto/test_ssh_crypto.cpp:891: test_sha256_empty         [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:892: test_sha256_abc           [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:893: test_sha256_448bit        [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:894: test_sha256_streaming     [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:897: test_hmac_sha256_tc1      [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:898: test_hmac_sha256_tc2      [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:899: test_hmac_sha256_tc3      [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:900: test_hmac_sha256_streaming [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:903: test_aes256ctr_encrypt    [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:904: test_aes256ctr_decrypt    [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:905: test_aes256ctr_multi_block [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:906: test_aes256ctr_wipe       [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:909: test_bn_roundtrip         [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:910: test_bn_cmp_equal         [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:911: test_bn_cmp_less          [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:912: test_bn_cmp_greater       [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:913: test_bn_is_zero           [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:914: test_bn_dh_validate_rejects_zero [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:915: test_bn_dh_validate_rejects_one [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:916: test_bn_dh_validate_accepts_two [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:919: test_expmod_exp1          [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:920: test_expmod_exp2          [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:921: test_expmod_exp3          [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:922: test_expmod_commutative   [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:925: test_rsa_pkcs1_pad_structure [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:926: test_rsa_sign_verify_roundtrip [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:927: test_rsa_encode_pubkey    [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:928: test_rsa_verify_valid_signature [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:929: test_rsa_verify_rejects_tampered_signature [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:930: test_rsa_verify_rejects_wrong_message [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:933: test_pkt_send_recv_unencrypted [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:934: test_pkt_padding_alignment [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:935: test_pkt_seq_increments   [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:936: test_pkt_disconnect_zeroes_state [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:937: test_pkt_encrypted_roundtrip [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:938: test_pkt_encrypted_fragmented [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:939: test_pkt_encrypted_two_packets [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:940: test_ssh_kdf_canonical_mpint_k [PASSED]
------------ native_ssh:test_ssh_crypto [PASSED] Took 3.83 seconds -------------

Processing test_ssh_auth in native_ssh environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_ssh_auth/test_ssh_auth.cpp:337: test_service_request_accept   [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:338: test_service_request_rejects_unknown [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:339: test_parse_password_request   [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:340: test_parse_none_request       [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:341: test_handle_request_success   [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:342: test_handle_request_wrong_password_fails [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:343: test_handle_none_request_fails_without_auth [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:344: test_handle_request_no_callback_fails [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:345: test_pubkey_probe_returns_pk_ok [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:346: test_pubkey_valid_signature_succeeds [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:347: test_pubkey_tampered_signature_fails [PASSED]
test/test_ssh_auth/test_ssh_auth.cpp:348: test_pubkey_unauthorized_key_fails [PASSED]
------------- native_ssh:test_ssh_auth [PASSED] Took 0.55 seconds --------------

Processing test_ssh_server in native_ssh environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_ssh_server/test_ssh_server.cpp:362: test_full_handshake_to_channel_data [PASSED]
test/test_ssh_server/test_ssh_server.cpp:363: test_channel_open_before_auth_rejected [PASSED]
test/test_ssh_server/test_ssh_server.cpp:364: test_disconnect_closes    [PASSED]
test/test_ssh_server/test_ssh_server.cpp:365: test_ignore_is_noop       [PASSED]
test/test_ssh_server/test_ssh_server.cpp:366: test_auth_bruteforce_disconnect [PASSED]
test/test_ssh_server/test_ssh_server.cpp:367: test_auth_success_after_failures [PASSED]
test/test_ssh_server/test_ssh_server.cpp:368: test_unimplemented_reply_for_unknown_message [PASSED]
------------ native_ssh:test_ssh_server [PASSED] Took 0.65 seconds -------------

Processing test_ssh_transport in native_ssh environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_ssh_transport/test_ssh_transport.cpp:515: test_server_banner_format [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:516: test_recv_banner_complete [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:517: test_recv_banner_bare_lf [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:518: test_recv_banner_split_across_reads [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:519: test_recv_banner_skips_preamble_lines [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:520: test_kexinit_build_starts_with_msg_and_stores_is [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:521: test_kexinit_parse_accepts_supported [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:522: test_kexinit_parse_accepts_when_ours_listed_among_others [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:523: test_kexinit_parse_rejects_missing_kex [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:524: test_kexinit_parse_rejects_missing_cipher [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:525: test_kexinit_parse_rejects_truncated [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:526: test_exchange_hash_matches_independent_assembly [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:527: test_exchange_hash_changes_with_input [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:528: test_kexdh_parse_init_extracts_e_with_padding [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:529: test_kexdh_parse_init_extracts_small_e [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:530: test_kexdh_parse_init_rejects_wrong_type [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:531: test_kexdh_parse_init_rejects_oversized_e [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:532: test_kexdh_build_reply_structure [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:533: test_kexdh_handle_produces_reply_and_installs_keys [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:534: test_kexdh_handle_rejects_invalid_e [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:535: test_derive_keys_session_id_affects_output [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:536: test_rekey_needed_threshold [PASSED]
test/test_ssh_transport/test_ssh_transport.cpp:537: test_begin_rekey_preserves_session_and_auth [PASSED]
----------- native_ssh:test_ssh_transport [PASSED] Took 0.87 seconds -----------

Processing test_ssh_channel in native_ssh environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_ssh_channel/test_ssh_channel.cpp:277: test_open_session_confirms [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:278: test_open_non_session_fails [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:279: test_shell_request_success_with_reply [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:280: test_unknown_request_failure [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:281: test_request_no_reply_produces_nothing [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:282: test_inbound_data_invokes_callback [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:283: test_inbound_data_window_replenish [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:284: test_inbound_data_exceeding_window_rejected [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:285: test_outbound_data_frames_and_decrements_window [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:286: test_outbound_data_exceeding_peer_window_rejected [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:287: test_window_adjust_grows_peer_window [PASSED]
test/test_ssh_channel/test_ssh_channel.cpp:288: test_build_close_emits_eof_and_close [PASSED]
------------ native_ssh:test_ssh_channel [PASSED] Took 0.52 seconds ------------

=================================== SUMMARY ===================================
Environment    Test                Status    Duration
-------------  ------------------  --------  ------------
native_ssh     test_ssh_crypto     PASSED    00:00:03.828
native_ssh     test_ssh_auth       PASSED    00:00:00.548
native_ssh     test_ssh_server     PASSED    00:00:00.655
native_ssh     test_ssh_transport  PASSED    00:00:00.866
native_ssh     test_ssh_channel    PASSED    00:00:00.516
================= 92 test cases: 92 succeeded in 00:00:06.413 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_ssh_hardening in native_ssh_hardened environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_hardening/test_ssh_hardening.cpp:87: test_password_refused_even_with_correct_callback [PASSED]
test/test_ssh_hardening/test_ssh_hardening.cpp:88: test_failure_advertises_publickey_only [PASSED]
------ native_ssh_hardened:test_ssh_hardening [PASSED] Took 0.86 seconds -------

=================================== SUMMARY ===================================
Environment          Test                Status    Duration
-------------------  ------------------  --------  ------------
native_ssh_hardened  test_ssh_hardening  PASSED    00:00:00.864
================== 2 test cases: 2 succeeded in 00:00:00.864 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_ssh_conn in native_ssh_conn environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_conn/test_ssh_conn.cpp:137: test_accept_sends_server_banner [PASSED]
test/test_ssh_conn/test_ssh_conn.cpp:138: test_banner_then_kexinit_advances_and_replies [PASSED]
----------- native_ssh_conn:test_ssh_conn [PASSED] Took 1.22 seconds -----------

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_ssh_conn  test_ssh_conn  PASSED    00:00:01.224
================== 2 test cases: 2 succeeded in 00:00:01.224 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_regex in native_app environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_regex/test_regex.cpp:155: test_numeric_class_plus             [PASSED]
test/test_regex/test_regex.cpp:156: test_dot_star_matches_rest          [PASSED]
test/test_regex/test_regex.cpp:157: test_escaped_dot_extension          [PASSED]
test/test_regex/test_regex.cpp:158: test_optional_quantifier            [PASSED]
test/test_regex/test_regex.cpp:159: test_range_class_only               [PASSED]
test/test_regex/test_regex.cpp:160: test_negated_class                  [PASSED]
test/test_regex/test_regex.cpp:161: test_anchored_full_match            [PASSED]
test/test_regex/test_regex.cpp:162: test_method_still_enforced          [PASSED]
test/test_regex/test_regex.cpp:163: test_pathological_pattern_terminates_no_match [PASSED]
--------------- native_app:test_regex [PASSED] Took 1.08 seconds ---------------

Processing test_template in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_template/test_template.cpp:152: test_basic_substitution       [PASSED]
test/test_template/test_template.cpp:153: test_multiple_placeholders    [PASSED]
test/test_template/test_template.cpp:154: test_unknown_placeholder_is_empty [PASSED]
test/test_template/test_template.cpp:155: test_unterminated_placeholder_is_literal [PASSED]
test/test_template/test_template.cpp:156: test_null_resolver_empties_all [PASSED]
test/test_template/test_template.cpp:157: test_head_suppresses_body_keeps_length [PASSED]
------------- native_app:test_template [PASSED] Took 0.55 seconds --------------

Processing test_path_params in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_path_params/test_path_params.cpp:169: test_single_param_captured [PASSED]
test/test_path_params/test_path_params.cpp:170: test_multiple_params_captured [PASSED]
test/test_path_params/test_path_params.cpp:171: test_missing_param_returns_null [PASSED]
test/test_path_params/test_path_params.cpp:172: test_literal_segment_mismatch_404 [PASSED]
test/test_path_params/test_path_params.cpp:173: test_extra_segment_does_not_match [PASSED]
test/test_path_params/test_path_params.cpp:174: test_empty_param_value_does_not_match [PASSED]
test/test_path_params/test_path_params.cpp:175: test_exact_route_still_matches [PASSED]
test/test_path_params/test_path_params.cpp:176: test_param_route_wrong_method_405 [PASSED]
------------ native_app:test_path_params [PASSED] Took 0.55 seconds ------------

Processing test_digest_vectors in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_digest_vectors/test_digest_vectors.cpp:108: test_sha256_fips_kats [PASSED]
test/test_digest_vectors/test_digest_vectors.cpp:109: test_ha1_matches_openssl [PASSED]
test/test_digest_vectors/test_digest_vectors.cpp:110: test_ha2_matches_openssl [PASSED]
test/test_digest_vectors/test_digest_vectors.cpp:111: test_response_matches_openssl [PASSED]
---------- native_app:test_digest_vectors [PASSED] Took 0.49 seconds -----------

Processing test_form_params in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_form_params/test_form_params.cpp:134: test_form_fields_parsed [PASSED]
test/test_form_params/test_form_params.cpp:135: test_form_missing_key_returns_false [PASSED]
test/test_form_params/test_form_params.cpp:136: test_form_empty_value   [PASSED]
test/test_form_params/test_form_params.cpp:137: test_form_wrong_content_type_ignored [PASSED]
test/test_form_params/test_form_params.cpp:138: test_form_value_truncated_to_buffer [PASSED]
------------ native_app:test_form_params [PASSED] Took 0.55 seconds ------------

Processing test_iface in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_iface/test_iface.cpp:161: test_ap_only_matches_on_ap          [PASSED]
test/test_iface/test_iface.cpp:162: test_ap_only_hidden_on_sta          [PASSED]
test/test_iface/test_iface.cpp:163: test_sta_only_matches_on_sta        [PASSED]
test/test_iface/test_iface.cpp:164: test_sta_only_hidden_on_ap          [PASSED]
test/test_iface/test_iface.cpp:165: test_unfiltered_route_matches_any_interface [PASSED]
test/test_iface/test_iface.cpp:166: test_same_path_two_interfaces_picks_correct [PASSED]
test/test_iface/test_iface.cpp:167: test_set_ap_ip_updates_global       [PASSED]
--------------- native_app:test_iface [PASSED] Took 0.55 seconds ---------------

Processing test_json in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_json/test_json.cpp:207: test_writer_simple_object             [PASSED]
test/test_json/test_json.cpp:208: test_writer_nested_and_array          [PASSED]
test/test_json/test_json.cpp:209: test_writer_value_types               [PASSED]
test/test_json/test_json.cpp:210: test_writer_escapes_strings           [PASSED]
test/test_json/test_json.cpp:211: test_writer_control_char_unicode_escape [PASSED]
test/test_json/test_json.cpp:212: test_writer_overflow_sets_not_ok_and_stays_terminated [PASSED]
test/test_json/test_json.cpp:213: test_writer_depth_overflow_sets_not_ok [PASSED]
test/test_json/test_json.cpp:214: test_reader_get_string                [PASSED]
test/test_json/test_json.cpp:215: test_reader_get_int                   [PASSED]
test/test_json/test_json.cpp:216: test_reader_get_bool                  [PASSED]
test/test_json/test_json.cpp:217: test_reader_only_matches_top_level_key [PASSED]
test/test_json/test_json.cpp:218: test_reader_missing_key               [PASSED]
test/test_json/test_json.cpp:219: test_reader_type_mismatch             [PASSED]
test/test_json/test_json.cpp:220: test_reader_unescapes_value           [PASSED]
test/test_json/test_json.cpp:221: test_reader_unicode_escape_to_byte    [PASSED]
test/test_json/test_json.cpp:222: test_reader_truncates_to_capacity     [PASSED]
test/test_json/test_json.cpp:223: test_reader_negative_int              [PASSED]
--------------- native_app:test_json [PASSED] Took 0.51 seconds ----------------

Processing test_response_headers in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_response_headers/test_response_headers.cpp:225: test_single_custom_header_present [PASSED]
test/test_response_headers/test_response_headers.cpp:226: test_multiple_custom_headers_present [PASSED]
test/test_response_headers/test_response_headers.cpp:227: test_set_cookie_basic [PASSED]
test/test_response_headers/test_response_headers.cpp:228: test_set_cookie_with_attrs [PASSED]
test/test_response_headers/test_response_headers.cpp:229: test_custom_header_on_send_empty [PASSED]
test/test_response_headers/test_response_headers.cpp:230: test_custom_header_on_redirect [PASSED]
test/test_response_headers/test_response_headers.cpp:231: test_headers_do_not_leak_across_requests [PASSED]
test/test_response_headers/test_response_headers.cpp:232: test_clear_response_headers [PASSED]
test/test_response_headers/test_response_headers.cpp:233: test_oversized_header_dropped_whole [PASSED]
--------- native_app:test_response_headers [PASSED] Took 0.55 seconds ----------

Processing test_middleware in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_middleware/test_middleware.cpp:245: test_middleware_runs_then_handler [PASSED]
test/test_middleware/test_middleware.cpp:246: test_middleware_runs_for_unmatched_route [PASSED]
test/test_middleware/test_middleware.cpp:247: test_middleware_can_inject_response_header [PASSED]
test/test_middleware/test_middleware.cpp:248: test_middleware_halt_short_circuits_handler [PASSED]
test/test_middleware/test_middleware.cpp:249: test_middleware_runs_in_registration_order [PASSED]
test/test_middleware/test_middleware.cpp:250: test_use_respects_capacity_cap [PASSED]
test/test_middleware/test_middleware.cpp:251: test_rate_limit_allows_then_rejects [PASSED]
test/test_middleware/test_middleware.cpp:252: test_rate_limit_window_resets [PASSED]
test/test_middleware/test_middleware.cpp:253: test_rate_limit_disabled_by_default [PASSED]
------------ native_app:test_middleware [PASSED] Took 0.56 seconds -------------

Processing test_digest_auth in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_digest_auth/test_digest_auth.cpp:231: test_challenge_is_digest_sha256 [PASSED]
test/test_digest_auth/test_digest_auth.cpp:232: test_valid_digest_authenticates [PASSED]
test/test_digest_auth/test_digest_auth.cpp:233: test_wrong_password_rejected [PASSED]
test/test_digest_auth/test_digest_auth.cpp:234: test_bad_nonce_rejected [PASSED]
test/test_digest_auth/test_digest_auth.cpp:235: test_nonce_is_128bit_hex [PASSED]
------------ native_app:test_digest_auth [PASSED] Took 0.56 seconds ------------

Processing test_web_terminal in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_web_terminal/test_web_terminal.cpp:177: test_serves_terminal_page [PASSED]
test/test_web_terminal/test_web_terminal.cpp:178: test_ws_upgrade_tracks_client [PASSED]
test/test_web_terminal/test_web_terminal.cpp:179: test_command_delivered_to_callback [PASSED]
test/test_web_terminal/test_web_terminal.cpp:180: test_broadcast_reaches_client [PASSED]
test/test_web_terminal/test_web_terminal.cpp:181: test_printf_broadcast [PASSED]
test/test_web_terminal/test_web_terminal.cpp:182: test_no_broadcast_without_clients [PASSED]
test/test_web_terminal/test_web_terminal.cpp:183: test_close_clears_client [PASSED]
----------- native_app:test_web_terminal [PASSED] Took 0.55 seconds ------------

Processing test_defer in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_defer/test_defer.cpp:56: test_defer_runs_inline_on_host       [PASSED]
test/test_defer/test_defer.cpp:57: test_server_defer_routes_by_owner    [PASSED]
test/test_defer/test_defer.cpp:58: test_defer_null_fn_fails             [PASSED]
--------------- native_app:test_defer [PASSED] Took 0.53 seconds ---------------

Processing test_multipart in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_multipart/test_multipart.cpp:494: test_no_content_type_returns_false [PASSED]
test/test_multipart/test_multipart.cpp:495: test_no_boundary_in_content_type_returns_false [PASSED]
test/test_multipart/test_multipart.cpp:496: test_body_missing_delimiter_returns_false [PASSED]
test/test_multipart/test_multipart.cpp:497: test_single_text_field_parsed [PASSED]
test/test_multipart/test_multipart.cpp:498: test_two_text_fields_parsed [PASSED]
test/test_multipart/test_multipart.cpp:499: test_three_text_fields_parsed [PASSED]
test/test_multipart/test_multipart.cpp:500: test_file_upload_part       [PASSED]
test/test_multipart/test_multipart.cpp:501: test_file_upload_with_text_field [PASSED]
test/test_multipart/test_multipart.cpp:502: test_get_field_found        [PASSED]
test/test_multipart/test_multipart.cpp:503: test_get_field_not_found_returns_null [PASSED]
test/test_multipart/test_multipart.cpp:504: test_get_field_multiple_fields [PASSED]
test/test_multipart/test_multipart.cpp:505: test_data_len_is_correct    [PASSED]
test/test_multipart/test_multipart.cpp:506: test_max_parts_captured     [PASSED]
test/test_multipart/test_multipart.cpp:507: test_empty_field_value      [PASSED]
test/test_multipart/test_multipart.cpp:508: test_part_without_filename_has_null_filename [PASSED]
test/test_multipart/test_multipart.cpp:509: test_part_without_content_type_has_null_type [PASSED]
test/test_multipart/test_multipart.cpp:510: test_long_boundary_string   [PASSED]
test/test_multipart/test_multipart.cpp:511: stress_parse_100_requests   [PASSED]
test/test_multipart/test_multipart.cpp:512: stress_get_field_100_lookups [PASSED]
------------- native_app:test_multipart [PASSED] Took 0.57 seconds -------------

Processing test_auth in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_auth/test_auth.cpp:251: test_unprotected_route_fires_handler  [PASSED]
test/test_auth/test_auth.cpp:252: test_protected_route_no_header_returns_401 [PASSED]
test/test_auth/test_auth.cpp:253: test_protected_route_wrong_password_returns_401 [PASSED]
test/test_auth/test_auth.cpp:254: test_protected_route_wrong_username_returns_401 [PASSED]
test/test_auth/test_auth.cpp:255: test_protected_route_valid_credentials_fires_handler [PASSED]
test/test_auth/test_auth.cpp:256: test_401_includes_www_authenticate_header [PASSED]
test/test_auth/test_auth.cpp:257: test_non_basic_scheme_returns_401     [PASSED]
test/test_auth/test_auth.cpp:258: test_credentials_without_colon_returns_401 [PASSED]
test/test_auth/test_auth.cpp:259: test_protected_and_unprotected_routes_coexist [PASSED]
test/test_auth/test_auth.cpp:260: test_auth_route_returns_404_for_wrong_path [PASSED]
test/test_auth/test_auth.cpp:261: test_auth_checked_per_method          [PASSED]
test/test_auth/test_auth.cpp:263: stress_auth_50_valid_requests         [PASSED]
test/test_auth/test_auth.cpp:264: stress_auth_50_invalid_requests       [PASSED]
--------------- native_app:test_auth [PASSED] Took 0.56 seconds ----------------

Processing test_file_serving in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_file_serving/test_file_serving.cpp:324: test_missing_file_returns_404 [PASSED]
test/test_file_serving/test_file_serving.cpp:325: test_existing_file_returns_200 [PASSED]
test/test_file_serving/test_file_serving.cpp:326: test_response_includes_content_type_html [PASSED]
test/test_file_serving/test_file_serving.cpp:327: test_response_includes_content_type_js [PASSED]
test/test_file_serving/test_file_serving.cpp:328: test_content_length_matches_file_size [PASSED]
test/test_file_serving/test_file_serving.cpp:329: test_file_body_is_sent [PASSED]
test/test_file_serving/test_file_serving.cpp:330: test_empty_file_returns_200_with_zero_length [PASSED]
test/test_file_serving/test_file_serving.cpp:331: test_large_file_body_fully_sent [PASSED]
test/test_file_serving/test_file_serving.cpp:332: test_serve_file_does_not_affect_other_routes [PASSED]
test/test_file_serving/test_file_serving.cpp:333: test_multiple_content_types [PASSED]
test/test_file_serving/test_file_serving.cpp:334: stress_serve_file_50_requests [PASSED]
test/test_file_serving/test_file_serving.cpp:335: stress_alternate_missing_and_found [PASSED]
----------- native_app:test_file_serving [PASSED] Took 0.57 seconds ------------

Processing test_dispatch in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_dispatch/test_dispatch.cpp:192: test_method_mismatch_returns_405 [PASSED]
test/test_dispatch/test_dispatch.cpp:193: test_405_includes_allow_header [PASSED]
test/test_dispatch/test_dispatch.cpp:194: test_405_allow_lists_all_methods_for_path [PASSED]
test/test_dispatch/test_dispatch.cpp:195: test_unknown_path_still_404_not_405 [PASSED]
test/test_dispatch/test_dispatch.cpp:196: test_unknown_method_returns_501 [PASSED]
test/test_dispatch/test_dispatch.cpp:197: test_unknown_method_not_treated_as_get [PASSED]
test/test_dispatch/test_dispatch.cpp:198: test_head_runs_get_handler_without_body [PASSED]
test/test_dispatch/test_dispatch.cpp:199: test_get_route_advertises_head_in_allow [PASSED]
test/test_dispatch/test_dispatch.cpp:200: test_head_on_post_only_route_405 [PASSED]
test/test_dispatch/test_dispatch.cpp:202: test_http_parse_skips_ws_upgraded_slot [PASSED]
test/test_dispatch/test_dispatch.cpp:204: test_correct_method_still_dispatches [PASSED]
------------- native_app:test_dispatch [PASSED] Took 0.56 seconds --------------

Processing test_chunked in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_chunked/test_chunked.cpp:209: test_headers_announce_chunked_no_content_length [PASSED]
test/test_chunked/test_chunked.cpp:210: test_single_chunk_framing       [PASSED]
test/test_chunked/test_chunked.cpp:211: test_multiple_chunks_in_order   [PASSED]
test/test_chunked/test_chunked.cpp:212: test_printf_chunk               [PASSED]
test/test_chunked/test_chunked.cpp:213: test_empty_writes_do_not_terminate_early [PASSED]
test/test_chunked/test_chunked.cpp:214: test_head_sends_headers_only    [PASSED]
test/test_chunked/test_chunked.cpp:215: test_custom_header_injected_into_chunked [PASSED]
test/test_chunked/test_chunked.cpp:216: test_log_hook_reports_total_body_length [PASSED]
-------------- native_app:test_chunked [PASSED] Took 0.55 seconds --------------

Processing test_application in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_application/test_application.cpp:903: test_handler_reads_body [PASSED]
test/test_application/test_application.cpp:904: test_handler_reads_query_param [PASSED]
test/test_application/test_application.cpp:905: test_handler_reads_header [PASSED]
test/test_application/test_application.cpp:906: test_wildcard_before_exact_wildcard_wins [PASSED]
test/test_application/test_application.cpp:909: test_fn_on_registers_and_dispatches [PASSED]
test/test_application/test_application.cpp:910: test_fn_on_path_copied_null_terminated [PASSED]
test/test_application/test_application.cpp:911: test_fn_on_table_full_extra_routes_dropped [PASSED]
test/test_application/test_application.cpp:912: test_fn_on_same_path_different_methods_are_distinct [PASSED]
test/test_application/test_application.cpp:915: test_fn_on_not_found_called_when_no_match [PASSED]
test/test_application/test_application.cpp:916: test_fn_on_not_found_not_called_when_match_exists [PASSED]
test/test_application/test_application.cpp:919: test_fn_set_cors_options_preflight_clears_slot [PASSED]
test/test_application/test_application.cpp:920: test_fn_set_cors_empty_string_disables [PASSED]
test/test_application/test_application.cpp:923: test_wrong_method_does_not_match [PASSED]
test/test_application/test_application.cpp:924: test_wrong_path_does_not_match [PASSED]
test/test_application/test_application.cpp:925: test_all_http_methods_dispatched [PASSED]
test/test_application/test_application.cpp:926: test_root_path_matches_exactly [PASSED]
test/test_application/test_application.cpp:927: test_root_path_does_not_match_subpath [PASSED]
test/test_application/test_application.cpp:928: test_wildcard_matches_any_suffix [PASSED]
test/test_application/test_application.cpp:929: test_wildcard_does_not_match_unrelated_prefix [PASSED]
test/test_application/test_application.cpp:930: test_exact_route_wins_when_registered_first [PASSED]
test/test_application/test_application.cpp:931: test_slot_not_stuck_in_complete_after_handle [PASSED]
test/test_application/test_application.cpp:932: test_parse_error_slot_auto_reset [PASSED]
test/test_application/test_application.cpp:935: stress_last_route_dispatched_in_full_table [PASSED]
test/test_application/test_application.cpp:936: stress_sequential_requests_no_state_leak [PASSED]
test/test_application/test_application.cpp:937: stress_all_slots_dispatched_simultaneously [PASSED]
test/test_application/test_application.cpp:938: stress_wildcard_matches_many_paths [PASSED]
test/test_application/test_application.cpp:939: stress_handle_with_no_complete_slots_is_nop [PASSED]
test/test_application/test_application.cpp:942: race_slot_complete_between_handle_calls [PASSED]
test/test_application/test_application.cpp:943: race_conn_freed_after_parse_complete [PASSED]
test/test_application/test_application.cpp:944: race_double_handle_no_double_dispatch [PASSED]
test/test_application/test_application.cpp:945: race_error_and_valid_slot_in_same_handle [PASSED]
test/test_application/test_application.cpp:946: race_callback_manually_resets_slot [PASSED]
test/test_application/test_application.cpp:949: test_uri_too_long_auto_resets_slot [PASSED]
test/test_application/test_application.cpp:952: test_transfer_encoding_chunked_is_501 [PASSED]
test/test_application/test_application.cpp:953: test_transfer_encoding_identity_is_501 [PASSED]
test/test_application/test_application.cpp:955: test_redirect_emits_location_and_status [PASSED]
test/test_application/test_application.cpp:956: test_redirect_invalid_code_defaults_to_302 [PASSED]
test/test_application/test_application.cpp:957: test_mime_type_detection [PASSED]
test/test_application/test_application.cpp:959: test_serve_static_file_and_mime [PASSED]
test/test_application/test_application.cpp:960: test_serve_static_index_fallback [PASSED]
test/test_application/test_application.cpp:961: test_serve_static_gzip_when_accepted [PASSED]
test/test_application/test_application.cpp:962: test_serve_static_no_gzip_when_not_accepted [PASSED]
test/test_application/test_application.cpp:963: test_serve_static_traversal_not_leaked [PASSED]
test/test_application/test_application.cpp:964: test_serve_static_missing_is_404 [PASSED]
test/test_application/test_application.cpp:965: test_serve_static_etag_conditional_get [PASSED]
test/test_application/test_application.cpp:966: test_serve_static_cache_control [PASSED]
test/test_application/test_application.cpp:968: test_request_log_hook_fires [PASSED]
test/test_application/test_application.cpp:969: test_stats_endpoint_emits_json [PASSED]
test/test_application/test_application.cpp:972: test_sse_broadcast_after_upgrade_matches_path [PASSED]
test/test_application/test_application.cpp:975: test_metrics_emits_prometheus [PASSED]
------------ native_app:test_application [PASSED] Took 0.65 seconds ------------

=================================== SUMMARY ===================================
Environment    Test                   Status    Duration
-------------  ---------------------  --------  ------------
native_app     test_regex             PASSED    00:00:01.078
native_app     test_template          PASSED    00:00:00.552
native_app     test_path_params       PASSED    00:00:00.550
native_app     test_digest_vectors    PASSED    00:00:00.492
native_app     test_form_params       PASSED    00:00:00.552
native_app     test_iface             PASSED    00:00:00.552
native_app     test_json              PASSED    00:00:00.508
native_app     test_response_headers  PASSED    00:00:00.554
native_app     test_middleware        PASSED    00:00:00.562
native_app     test_digest_auth       PASSED    00:00:00.561
native_app     test_web_terminal      PASSED    00:00:00.552
native_app     test_defer             PASSED    00:00:00.533
native_app     test_multipart         PASSED    00:00:00.565
native_app     test_auth              PASSED    00:00:00.559
native_app     test_file_serving      PASSED    00:00:00.572
native_app     test_dispatch          PASSED    00:00:00.556
native_app     test_chunked           PASSED    00:00:00.551
native_app     test_application       PASSED    00:00:00.647
================ 202 test cases: 202 succeeded in 00:00:10.497 ================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_snmp_ber in native_snmp environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_snmp_ber/test_snmp_ber.cpp:198: test_integer_vectors          [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:199: test_oid_vector               [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:200: test_octet_string_and_null    [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:201: test_counter32_keeps_unsigned [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:202: test_sequence_roundtrip       [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:203: test_oid_roundtrip            [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:204: test_large_arc_roundtrip      [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:205: test_encoder_overflow_sets_not_ok [PASSED]
test/test_snmp_ber/test_snmp_ber.cpp:206: test_decoder_truncated_length_fails [PASSED]
------------- native_snmp:test_snmp_ber [PASSED] Took 0.67 seconds -------------

Processing test_snmp_agent in native_snmp environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_snmp_agent/test_snmp_agent.cpp:357: test_get_string_v2c       [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:358: test_get_unknown_v2c_exception [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:359: test_get_unknown_v1_error [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:360: test_getnext_walks_to_first [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:361: test_getnext_past_end_endofmibview [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:362: test_set_without_rw_community_denied [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:363: test_set_with_rw_community_invokes_setter [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:364: test_set_readonly_not_writable [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:365: test_getbulk_returns_multiple [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:366: test_dynamic_counter_value [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:367: test_uptime_is_timeticks  [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:368: test_unknown_community_no_response [PASSED]
test/test_snmp_agent/test_snmp_agent.cpp:369: test_v3_message_dropped   [PASSED]
------------ native_snmp:test_snmp_agent [PASSED] Took 0.48 seconds ------------

=================================== SUMMARY ===================================
Environment    Test             Status    Duration
-------------  ---------------  --------  ------------
native_snmp    test_snmp_ber    PASSED    00:00:00.669
native_snmp    test_snmp_agent  PASSED    00:00:00.481
================= 22 test cases: 22 succeeded in 00:00:01.151 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_snmp_v3 in native_snmp_v3 environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_snmp_v3/test_snmp_v3.cpp:452: test_localize_key_sha256_vector [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:453: test_aes128_fips197_vector      [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:454: test_aes_cfb_roundtrip_partial_block [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:455: test_discovery_reports_engine_id [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:456: test_authnopriv_get             [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:457: test_authpriv_get               [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:458: test_wrong_auth_password_reports_wrong_digest [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:459: test_unknown_user_reports       [PASSED]
test/test_snmp_v3/test_snmp_v3.cpp:460: test_not_in_time_window_reports [PASSED]
------------ native_snmp_v3:test_snmp_v3 [PASSED] Took 1.36 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_snmp_v3  test_snmp_v3  PASSED    00:00:01.362
================== 9 test cases: 9 succeeded in 00:00:01.362 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_telnet in native_telnet environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_telnet/test_telnet.cpp:165: test_accept_negotiates_echo_and_sga [PASSED]
test/test_telnet/test_telnet.cpp:166: test_line_echoed_and_dispatched   [PASSED]
test/test_telnet/test_telnet.cpp:167: test_backspace_first_line         [PASSED]
test/test_telnet/test_telnet.cpp:168: test_iac_will_gets_dont           [PASSED]
test/test_telnet/test_telnet.cpp:169: test_iac_do_unsupported_gets_wont [PASSED]
test/test_telnet/test_telnet.cpp:170: test_iac_do_echo_is_silent        [PASSED]
test/test_telnet/test_telnet.cpp:171: test_iac_stripped_from_data       [PASSED]
test/test_telnet/test_telnet.cpp:172: test_print_broadcast              [PASSED]
------------- native_telnet:test_telnet [PASSED] Took 0.74 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_telnet  test_telnet  PASSED    00:00:00.739
================== 8 test cases: 8 succeeded in 00:00:00.739 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_coap in native_coap environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_coap/test_coap.cpp:755: test_block2_explicit_paging           [PASSED]
test/test_coap/test_coap.cpp:756: test_block2_auto_when_large           [PASSED]
test/test_coap/test_coap.cpp:757: test_block2_szx_clamped               [PASSED]
test/test_coap/test_coap.cpp:758: test_block2_absent_for_small          [PASSED]
test/test_coap/test_coap.cpp:759: test_block2_out_of_range              [PASSED]
test/test_coap/test_coap.cpp:760: test_block2_reserved_szx              [PASSED]
test/test_coap/test_coap.cpp:761: test_block1_upload_two_blocks         [PASSED]
test/test_coap/test_coap.cpp:762: test_block1_out_of_order              [PASSED]
test/test_coap/test_coap.cpp:763: test_block1_too_large                 [PASSED]
test/test_coap/test_coap.cpp:764: test_observe_option_in_response       [PASSED]
test/test_coap/test_coap.cpp:765: test_no_observe_option_when_seq_negative [PASSED]
test/test_coap/test_coap.cpp:766: test_get_content                      [PASSED]
test/test_coap/test_coap.cpp:767: test_not_found                        [PASSED]
test/test_coap/test_coap.cpp:768: test_method_not_allowed               [PASSED]
test/test_coap/test_coap.cpp:769: test_non_request_type                 [PASSED]
test/test_coap/test_coap.cpp:770: test_put_with_payload                 [PASSED]
test/test_coap/test_coap.cpp:771: test_multi_segment_path               [PASSED]
test/test_coap/test_coap.cpp:772: test_uri_query                        [PASSED]
test/test_coap/test_coap.cpp:773: test_empty_con_ping_rst               [PASSED]
test/test_coap/test_coap.cpp:774: test_bad_version_rst                  [PASSED]
test/test_coap/test_coap.cpp:775: test_delete                           [PASSED]
test/test_coap/test_coap.cpp:776: test_token_8_bytes                    [PASSED]
test/test_coap/test_coap.cpp:777: test_extended_option_length           [PASSED]
test/test_coap/test_coap.cpp:778: test_ack_ignored                      [PASSED]
test/test_coap/test_coap.cpp:779: test_root_path                        [PASSED]
test/test_coap/test_coap.cpp:780: test_unknown_method_not_implemented   [PASSED]
--------------- native_coap:test_coap [PASSED] Took 0.66 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_coap    test_coap  PASSED    00:00:00.664
================= 26 test cases: 26 succeeded in 00:00:00.664 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_webdav in native_webdav environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_webdav/test_webdav.cpp:160: test_method_classification        [PASSED]
test/test_webdav/test_webdav.cpp:161: test_depth_parsing                [PASSED]
test/test_webdav/test_webdav.cpp:162: test_xml_escape                   [PASSED]
test/test_webdav/test_webdav.cpp:163: test_xml_escape_truncates_safely  [PASSED]
test/test_webdav/test_webdav.cpp:164: test_dest_absolute_uri            [PASSED]
test/test_webdav/test_webdav.cpp:165: test_dest_percent_decoded         [PASSED]
test/test_webdav/test_webdav.cpp:166: test_dest_abs_path                [PASSED]
test/test_webdav/test_webdav.cpp:167: test_dest_rejects_malformed       [PASSED]
test/test_webdav/test_webdav.cpp:168: test_multistatus_file_and_collection [PASSED]
test/test_webdav/test_webdav.cpp:169: test_multistatus_escapes_href     [PASSED]
test/test_webdav/test_webdav.cpp:170: test_multistatus_entry_stops_when_full [PASSED]
------------- native_webdav:test_webdav [PASSED] Took 0.62 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_webdav  test_webdav  PASSED    00:00:00.621
================= 11 test cases: 11 succeeded in 00:00:00.621 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_modbus in native_modbus environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_modbus/test_modbus.cpp:232: test_read_holding_registers       [PASSED]
test/test_modbus/test_modbus.cpp:233: test_read_input_registers         [PASSED]
test/test_modbus/test_modbus.cpp:234: test_read_coils_packs_bits        [PASSED]
test/test_modbus/test_modbus.cpp:235: test_write_single_coil            [PASSED]
test/test_modbus/test_modbus.cpp:236: test_write_single_register        [PASSED]
test/test_modbus/test_modbus.cpp:237: test_write_multiple_registers     [PASSED]
test/test_modbus/test_modbus.cpp:238: test_write_multiple_coils         [PASSED]
test/test_modbus/test_modbus.cpp:239: test_exception_illegal_function   [PASSED]
test/test_modbus/test_modbus.cpp:240: test_exception_illegal_address    [PASSED]
test/test_modbus/test_modbus.cpp:241: test_exception_illegal_value      [PASSED]
test/test_modbus/test_modbus.cpp:242: test_write_single_coil_bad_value  [PASSED]
test/test_modbus/test_modbus.cpp:243: test_non_modbus_protocol_id_ignored [PASSED]
test/test_modbus/test_modbus.cpp:244: test_truncated_frame_ignored      [PASSED]
------------- native_modbus:test_modbus [PASSED] Took 0.64 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_modbus  test_modbus  PASSED    00:00:00.639
================= 13 test cases: 13 succeeded in 00:00:00.639 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
------ native_modbus_master:test_modbus_master [PASSED] Took 0.65 seconds ------

=================================== SUMMARY ===================================
Environment           Test                Status    Duration
--------------------  ------------------  --------  ------------
native_modbus_master  test_modbus_master  PASSED    00:00:00.648
================== 5 test cases: 5 succeeded in 00:00:00.648 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
------- native_ota_rollback:test_ota_rollback [PASSED] Took 0.62 seconds -------

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_ota_rollback  test_ota_rollback  PASSED    00:00:00.624
================== 5 test cases: 5 succeeded in 00:00:00.624 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
--------------- native_totp:test_totp [PASSED] Took 0.65 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_totp    test_totp  PASSED    00:00:00.652
================== 4 test cases: 4 succeeded in 00:00:00.652 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
------------ native_webhook:test_webhook [PASSED] Took 0.63 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_webhook  test_webhook  PASSED    00:00:00.628
================== 5 test cases: 5 succeeded in 00:00:00.628 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_radio_power in native_radio_power environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_radio_power/test_radio_power.cpp:34: test_ps_names            [PASSED]
test/test_radio_power/test_radio_power.cpp:35: test_apply_is_noop_on_host [PASSED]
-------- native_radio_power:test_radio_power [PASSED] Took 0.63 seconds --------

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_radio_power  test_radio_power  PASSED    00:00:00.628
================== 2 test cases: 2 succeeded in 00:00:00.628 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
------- native_dns_resolver:test_dns_resolver [PASSED] Took 0.63 seconds -------

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_dns_resolver  test_dns_resolver  PASSED    00:00:00.626
================== 4 test cases: 4 succeeded in 00:00:00.626 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_audit_log in native_audit_log environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_audit_log/test_audit_log.cpp:179: test_append_assigns_monotonic_seq [PASSED]
test/test_audit_log/test_audit_log.cpp:180: test_chain_verifies_when_untouched [PASSED]
test/test_audit_log/test_audit_log.cpp:181: test_tampered_message_breaks_chain [PASSED]
test/test_audit_log/test_audit_log.cpp:182: test_tampered_hash_breaks_chain [PASSED]
test/test_audit_log/test_audit_log.cpp:183: test_tampered_category_breaks_chain [PASSED]
test/test_audit_log/test_audit_log.cpp:184: test_ring_evicts_oldest_and_still_verifies [PASSED]
test/test_audit_log/test_audit_log.cpp:185: test_tamper_after_wrap_detected_at_oldest [PASSED]
test/test_audit_log/test_audit_log.cpp:186: test_reset_clears_everything [PASSED]
test/test_audit_log/test_audit_log.cpp:187: test_sink_receives_each_record [PASSED]
test/test_audit_log/test_audit_log.cpp:188: test_format_and_dump_json   [PASSED]
test/test_audit_log/test_audit_log.cpp:189: test_dump_json_reports_broken_chain [PASSED]
test/test_audit_log/test_audit_log.cpp:190: test_format_fails_closed_on_small_buffer [PASSED]
---------- native_audit_log:test_audit_log [PASSED] Took 0.64 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_audit_log  test_audit_log  PASSED    00:00:00.644
================= 12 test cases: 12 succeeded in 00:00:00.644 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_oidc in native_oidc environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_oidc/test_oidc.cpp:164: test_token_kid                        [PASSED]
test/test_oidc/test_oidc.cpp:165: test_jwks_find                        [PASSED]
test/test_oidc/test_oidc.cpp:166: test_jwks_find_missing_kid_fails      [PASSED]
test/test_oidc/test_oidc.cpp:167: test_verify_valid_token_and_claims    [PASSED]
test/test_oidc/test_oidc.cpp:168: test_verify_aud_array                 [PASSED]
test/test_oidc/test_oidc.cpp:169: test_reject_expired                   [PASSED]
test/test_oidc/test_oidc.cpp:170: test_reject_wrong_issuer              [PASSED]
test/test_oidc/test_oidc.cpp:171: test_reject_wrong_audience            [PASSED]
test/test_oidc/test_oidc.cpp:172: test_reject_non_rs256_header          [PASSED]
test/test_oidc/test_oidc.cpp:173: test_reject_tampered_payload          [PASSED]
test/test_oidc/test_oidc.cpp:174: test_reject_tampered_signature        [PASSED]
test/test_oidc/test_oidc.cpp:175: test_reject_unknown_key               [PASSED]
test/test_oidc/test_oidc.cpp:176: test_reject_malformed                 [PASSED]
--------------- native_oidc:test_oidc [PASSED] Took 0.80 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_oidc    test_oidc  PASSED    00:00:00.797
================= 13 test cases: 13 succeeded in 00:00:00.797 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
---------------- native_vfs:test_vfs [PASSED] Took 0.63 seconds ----------------

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_vfs     test_vfs  PASSED    00:00:00.633
================= 11 test cases: 11 succeeded in 00:00:00.633 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_graphql in native_graphql environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_graphql/test_graphql.cpp:169: test_flat_selection             [PASSED]
test/test_graphql/test_graphql.cpp:170: test_selection_is_honored       [PASSED]
test/test_graphql/test_graphql.cpp:171: test_nested_object              [PASSED]
test/test_graphql/test_graphql.cpp:172: test_args_collected_along_path  [PASSED]
test/test_graphql/test_graphql.cpp:173: test_scalar_types               [PASSED]
test/test_graphql/test_graphql.cpp:174: test_string_arg_and_escaping    [PASSED]
test/test_graphql/test_graphql.cpp:175: test_unresolved_field_is_null   [PASSED]
test/test_graphql/test_graphql.cpp:176: test_query_keyword_and_name     [PASSED]
test/test_graphql/test_graphql.cpp:177: test_comments_and_commas        [PASSED]
test/test_graphql/test_graphql.cpp:178: test_parse_error_reports_errors [PASSED]
test/test_graphql/test_graphql.cpp:179: test_mutation_rejected          [PASSED]
test/test_graphql/test_graphql.cpp:180: test_depth_limit                [PASSED]
test/test_graphql/test_graphql.cpp:181: test_overflow_fails_closed      [PASSED]
------------ native_graphql:test_graphql [PASSED] Took 0.64 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_graphql  test_graphql  PASSED    00:00:00.636
================= 13 test cases: 13 succeeded in 00:00:00.636 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
------------- native_espnow:test_espnow [PASSED] Took 0.63 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_espnow  test_espnow  PASSED    00:00:00.627
================== 7 test cases: 7 succeeded in 00:00:00.627 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
------------- native_oauth2:test_oauth2 [PASSED] Took 0.66 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_oauth2  test_oauth2  PASSED    00:00:00.662
================== 8 test cases: 8 succeeded in 00:00:00.662 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_opcua in native_opcua environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_opcua/test_opcua.cpp:1002: test_codec_roundtrip               [PASSED]
test/test_opcua/test_opcua.cpp:1003: test_string_null_roundtrip         [PASSED]
test/test_opcua/test_opcua.cpp:1004: test_reader_underrun_latches       [PASSED]
test/test_opcua/test_opcua.cpp:1005: test_writer_overflow_fails_closed  [PASSED]
test/test_opcua/test_opcua.cpp:1006: test_parse_header                  [PASSED]
test/test_opcua/test_opcua.cpp:1007: test_parse_hello                   [PASSED]
test/test_opcua/test_opcua.cpp:1008: test_parse_hello_rejects_short     [PASSED]
test/test_opcua/test_opcua.cpp:1009: test_build_ack_negotiates          [PASSED]
test/test_opcua/test_opcua.cpp:1010: test_nodeid_roundtrip              [PASSED]
test/test_opcua/test_opcua.cpp:1011: test_filetime_from_unix            [PASSED]
test/test_opcua/test_opcua.cpp:1012: test_parse_open                    [PASSED]
test/test_opcua/test_opcua.cpp:1013: test_parse_open_rejects_wrong_type [PASSED]
test/test_opcua/test_opcua.cpp:1014: test_build_open_response           [PASSED]
test/test_opcua/test_opcua.cpp:1015: test_parse_msg                     [PASSED]
test/test_opcua/test_opcua.cpp:1016: test_parse_msg_rejects_non_msg     [PASSED]
test/test_opcua/test_opcua.cpp:1017: test_build_create_session_response [PASSED]
test/test_opcua/test_opcua.cpp:1018: test_build_activate_session_response [PASSED]
test/test_opcua/test_opcua.cpp:1019: test_datavalue_good_int32          [PASSED]
test/test_opcua/test_opcua.cpp:1020: test_datavalue_bad_status          [PASSED]
test/test_opcua/test_opcua.cpp:1021: test_parse_read                    [PASSED]
test/test_opcua/test_opcua.cpp:1022: test_build_read_response           [PASSED]
test/test_opcua/test_opcua.cpp:1023: test_parse_browse                  [PASSED]
test/test_opcua/test_opcua.cpp:1024: test_build_browse_response         [PASSED]
test/test_opcua/test_opcua.cpp:1025: test_build_browse_response_unknown [PASSED]
test/test_opcua/test_opcua.cpp:1026: test_build_close_session_response  [PASSED]
test/test_opcua/test_opcua.cpp:1027: test_build_get_endpoints           [PASSED]
test/test_opcua/test_opcua.cpp:1028: test_build_service_fault           [PASSED]
test/test_opcua/test_opcua.cpp:1029: test_datavalue_roundtrip           [PASSED]
test/test_opcua/test_opcua.cpp:1030: test_parse_and_build_write         [PASSED]
-------------- native_opcua:test_opcua [PASSED] Took 0.67 seconds --------------

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_opcua   test_opcua  PASSED    00:00:00.667
================= 29 test cases: 29 succeeded in 00:00:00.667 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_opcua_client in native_opcua_client environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_opcua_client/test_opcua_client.cpp:315: test_hello_ack_roundtrip [PASSED]
test/test_opcua_client/test_opcua_client.cpp:316: test_open_roundtrip   [PASSED]
test/test_opcua_client/test_opcua_client.cpp:317: test_session_roundtrip [PASSED]
test/test_opcua_client/test_opcua_client.cpp:318: test_get_endpoints_roundtrip [PASSED]
test/test_opcua_client/test_opcua_client.cpp:319: test_service_fault_rejected_by_parsers [PASSED]
test/test_opcua_client/test_opcua_client.cpp:320: test_read_roundtrip   [PASSED]
test/test_opcua_client/test_opcua_client.cpp:321: test_browse_roundtrip [PASSED]
test/test_opcua_client/test_opcua_client.cpp:322: test_write_roundtrip  [PASSED]
test/test_opcua_client/test_opcua_client.cpp:323: test_close_session_roundtrip [PASSED]
test/test_opcua_client/test_opcua_client.cpp:324: test_close_channel_is_clo [PASSED]
test/test_opcua_client/test_opcua_client.cpp:325: test_seq_and_request_id_increment [PASSED]
------- native_opcua_client:test_opcua_client [PASSED] Took 0.68 seconds -------

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_opcua_client  test_opcua_client  PASSED    00:00:00.676
================= 11 test cases: 11 succeeded in 00:00:00.676 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_keepalive in native_keepalive environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_keepalive/test_keepalive.cpp:192: test_http11_default_keeps_alive [PASSED]
test/test_keepalive/test_keepalive.cpp:193: test_http11_explicit_close  [PASSED]
test/test_keepalive/test_keepalive.cpp:194: test_http10_default_closes  [PASSED]
test/test_keepalive/test_keepalive.cpp:195: test_http10_explicit_keepalive [PASSED]
test/test_keepalive/test_keepalive.cpp:196: test_connection_token_list_close [PASSED]
test/test_keepalive/test_keepalive.cpp:197: test_two_sequential_requests_same_slot [PASSED]
test/test_keepalive/test_keepalive.cpp:198: test_pipelined_requests     [PASSED]
test/test_keepalive/test_keepalive.cpp:199: test_404_still_keeps_alive  [PASSED]
test/test_keepalive/test_keepalive.cpp:200: test_max_requests_cap_closes [PASSED]
test/test_keepalive/test_keepalive.cpp:201: test_fresh_connection_resets_count [PASSED]
---------- native_keepalive:test_keepalive [PASSED] Took 1.01 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_keepalive  test_keepalive  PASSED    00:00:01.007
================= 10 test cases: 10 succeeded in 00:00:01.007 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_range in native_range environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_range/test_range.cpp:190: test_no_range_full_200              [PASSED]
test/test_range/test_range.cpp:191: test_range_prefix                   [PASSED]
test/test_range/test_range.cpp:192: test_range_open_ended               [PASSED]
test/test_range/test_range.cpp:193: test_range_suffix                   [PASSED]
test/test_range/test_range.cpp:194: test_range_single_byte              [PASSED]
test/test_range/test_range.cpp:195: test_range_clamped_to_eof           [PASSED]
test/test_range/test_range.cpp:196: test_range_unsatisfiable_416        [PASSED]
test/test_range/test_range.cpp:197: test_malformed_range_ignored        [PASSED]
test/test_range/test_range.cpp:198: test_multirange_falls_back_to_200   [PASSED]
test/test_range/test_range.cpp:199: test_head_with_range_no_body        [PASSED]
-------------- native_range:test_range [PASSED] Took 1.01 seconds --------------

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_range   test_range  PASSED    00:00:01.014
================= 10 test cases: 10 succeeded in 00:00:01.014 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_syslog in native_syslog environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_syslog/test_syslog.cpp:71: test_pri_local0_info               [PASSED]
test/test_syslog/test_syslog.cpp:72: test_pri_computation_varies        [PASSED]
test/test_syslog/test_syslog.cpp:73: test_nilvalue_for_empty_fields     [PASSED]
test/test_syslog/test_syslog.cpp:74: test_empty_message_ok              [PASSED]
test/test_syslog/test_syslog.cpp:75: test_overflow_returns_zero         [PASSED]
test/test_syslog/test_syslog.cpp:76: test_length_matches_strlen         [PASSED]
------------- native_syslog:test_syslog [PASSED] Took 0.64 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_syslog  test_syslog  PASSED    00:00:00.639
================== 6 test cases: 6 succeeded in 00:00:00.639 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_jwt in native_jwt environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_jwt/test_jwt.cpp:125: test_valid_token_accepts                [PASSED]
test/test_jwt/test_jwt.cpp:126: test_wrong_secret_rejects               [PASSED]
test/test_jwt/test_jwt.cpp:127: test_tampered_payload_rejects           [PASSED]
test/test_jwt/test_jwt.cpp:128: test_tampered_signature_rejects         [PASSED]
test/test_jwt/test_jwt.cpp:129: test_malformed_rejected                 [PASSED]
test/test_jwt/test_jwt.cpp:130: test_bearer_header                      [PASSED]
test/test_jwt/test_jwt.cpp:131: test_claim_int                          [PASSED]
test/test_jwt/test_jwt.cpp:132: test_claim_missing                      [PASSED]
test/test_jwt/test_jwt.cpp:133: test_claim_str                          [PASSED]
test/test_jwt/test_jwt.cpp:134: test_scope_allows                       [PASSED]
---------------- native_jwt:test_jwt [PASSED] Took 0.67 seconds ----------------

=================================== SUMMARY ===================================
Environment    Test      Status    Duration
-------------  --------  --------  ------------
native_jwt     test_jwt  PASSED    00:00:00.675
================= 10 test cases: 10 succeeded in 00:00:00.675 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_upload in native_upload environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_upload/test_upload.cpp:110: test_upload_streams_body_to_file  [PASSED]
test/test_upload/test_upload.cpp:111: test_small_body_single_chunk      [PASSED]
test/test_upload/test_upload.cpp:112: test_empty_body_not_streamed      [PASSED]
------------- native_upload:test_upload [PASSED] Took 1.05 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_upload  test_upload  PASSED    00:00:01.053
================== 3 test cases: 3 succeeded in 00:00:01.053 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_http_client in native_http_client environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_http_client/test_http_client.cpp:136: test_url_http_default   [PASSED]
test/test_http_client/test_http_client.cpp:137: test_url_https_port_nopath [PASSED]
test/test_http_client/test_http_client.cpp:138: test_url_bad_scheme     [PASSED]
test/test_http_client/test_http_client.cpp:139: test_build_get          [PASSED]
test/test_http_client/test_http_client.cpp:140: test_build_post_with_body_and_port [PASSED]
test/test_http_client/test_http_client.cpp:141: test_parse_content_length [PASSED]
test/test_http_client/test_http_client.cpp:142: test_parse_status_404   [PASSED]
test/test_http_client/test_http_client.cpp:143: test_parse_chunked      [PASSED]
test/test_http_client/test_http_client.cpp:144: test_parse_connection_close_body [PASSED]
test/test_http_client/test_http_client.cpp:145: test_parse_malformed    [PASSED]
-------- native_http_client:test_http_client [PASSED] Took 0.64 seconds --------

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_http_client  test_http_client  PASSED    00:00:00.637
================= 10 test cases: 10 succeeded in 00:00:00.637 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_compliance in native_compliance environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_compliance/test_compliance.cpp:143: test_http11_missing_host_rejected [PASSED]
test/test_compliance/test_compliance.cpp:144: test_http11_with_host_ok  [PASSED]
test/test_compliance/test_compliance.cpp:145: test_http10_missing_host_ok [PASSED]
test/test_compliance/test_compliance.cpp:146: test_duplicate_host_rejected [PASSED]
test/test_compliance/test_compliance.cpp:147: test_duplicate_host_rejected_http10 [PASSED]
test/test_compliance/test_compliance.cpp:148: test_host_beyond_max_headers_still_counted [PASSED]
test/test_compliance/test_compliance.cpp:149: test_duplicate_host_with_one_beyond_cap_rejected [PASSED]
test/test_compliance/test_compliance.cpp:151: test_content_length_non_digit_rejected [PASSED]
test/test_compliance/test_compliance.cpp:152: test_content_length_empty_rejected [PASSED]
test/test_compliance/test_compliance.cpp:153: test_content_length_conflicting_duplicate_rejected [PASSED]
test/test_compliance/test_compliance.cpp:154: test_content_length_matching_duplicate_ok [PASSED]
test/test_compliance/test_compliance.cpp:155: test_content_length_valid_body [PASSED]
--------- native_compliance:test_compliance [PASSED] Took 0.64 seconds ---------

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_compliance  test_compliance  PASSED    00:00:00.636
================= 12 test cases: 12 succeeded in 00:00:00.636 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_mqtt in native_mqtt environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_mqtt/test_mqtt.cpp:268: test_remlen_boundaries                [PASSED]
test/test_mqtt/test_mqtt.cpp:269: test_remlen_too_big                   [PASSED]
test/test_mqtt/test_mqtt.cpp:270: test_remlen_decode_incomplete         [PASSED]
test/test_mqtt/test_mqtt.cpp:271: test_remlen_decode_malformed          [PASSED]
test/test_mqtt/test_mqtt.cpp:272: test_connect_minimal                  [PASSED]
test/test_mqtt/test_mqtt.cpp:273: test_connect_full                     [PASSED]
test/test_mqtt/test_mqtt.cpp:274: test_publish_qos0_roundtrip           [PASSED]
test/test_mqtt/test_mqtt.cpp:275: test_publish_qos1_flags_and_id        [PASSED]
test/test_mqtt/test_mqtt.cpp:276: test_publish_topic_overflow_rejected  [PASSED]
test/test_mqtt/test_mqtt.cpp:277: test_subscribe                        [PASSED]
test/test_mqtt/test_mqtt.cpp:278: test_unsubscribe                      [PASSED]
test/test_mqtt/test_mqtt.cpp:279: test_ack_packets                      [PASSED]
test/test_mqtt/test_mqtt.cpp:280: test_connack                          [PASSED]
test/test_mqtt/test_mqtt.cpp:281: test_suback                           [PASSED]
test/test_mqtt/test_mqtt.cpp:282: test_ping_disconnect                  [PASSED]
test/test_mqtt/test_mqtt.cpp:283: test_fixed_header_multibyte_remlen    [PASSED]
--------------- native_mqtt:test_mqtt [PASSED] Took 0.64 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_mqtt    test_mqtt  PASSED    00:00:00.638
================= 16 test cases: 16 succeeded in 00:00:00.638 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_ws_client in native_ws_client environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ws_client/test_ws_client.cpp:132: test_accept_rfc_example     [PASSED]
test/test_ws_client/test_ws_client.cpp:133: test_build_handshake        [PASSED]
test/test_ws_client/test_ws_client.cpp:134: test_check_response_ok      [PASSED]
test/test_ws_client/test_ws_client.cpp:135: test_check_response_bad_accept [PASSED]
test/test_ws_client/test_ws_client.cpp:136: test_check_response_not_101 [PASSED]
test/test_ws_client/test_ws_client.cpp:137: test_build_frame_masked     [PASSED]
test/test_ws_client/test_ws_client.cpp:138: test_build_frame_extended_len [PASSED]
test/test_ws_client/test_ws_client.cpp:139: test_parse_frame_server_text [PASSED]
test/test_ws_client/test_ws_client.cpp:140: test_parse_frame_incomplete [PASSED]
test/test_ws_client/test_ws_client.cpp:141: test_parse_frame_extended_len [PASSED]
---------- native_ws_client:test_ws_client [PASSED] Took 0.67 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_ws_client  test_ws_client  PASSED    00:00:00.667
================= 10 test cases: 10 succeeded in 00:00:00.667 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
------------ native_scratch:test_scratch [PASSED] Took 0.67 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_scratch  test_scratch  PASSED    00:00:00.674
================= 14 test cases: 14 succeeded in 00:00:00.674 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_snmp_trap in native_snmp_trap environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_snmp_trap/test_snmp_trap.cpp:125: test_trap_v2c_structure     [PASSED]
test/test_snmp_trap/test_snmp_trap.cpp:126: test_inform_tag             [PASSED]
test/test_snmp_trap/test_snmp_trap.cpp:127: test_buffer_too_small       [PASSED]
---------- native_snmp_trap:test_snmp_trap [PASSED] Took 0.65 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_snmp_trap  test_snmp_trap  PASSED    00:00:00.653
================== 3 test cases: 3 succeeded in 00:00:00.653 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
------------ native_inflate:test_inflate [PASSED] Took 0.63 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_inflate  test_inflate  PASSED    00:00:00.632
================= 12 test cases: 12 succeeded in 00:00:00.632 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
------------ native_deflate:test_deflate [PASSED] Took 0.66 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_deflate  test_deflate  PASSED    00:00:00.665
================= 10 test cases: 10 succeeded in 00:00:00.665 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_websocket in native_ws_deflate environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_websocket/test_websocket.cpp:962: test_sha1_empty_string      [PASSED]
test/test_websocket/test_websocket.cpp:963: test_sha1_abc               [PASSED]
test/test_websocket/test_websocket.cpp:964: test_sha1_rfc6455_handshake_key [PASSED]
test/test_websocket/test_websocket.cpp:965: test_sha1_different_inputs_different_digests [PASSED]
test/test_websocket/test_websocket.cpp:968: test_base64_encode_one_byte [PASSED]
test/test_websocket/test_websocket.cpp:969: test_base64_encode_two_bytes [PASSED]
test/test_websocket/test_websocket.cpp:970: test_base64_encode_three_bytes [PASSED]
test/test_websocket/test_websocket.cpp:971: test_base64_encode_ws_accept_key [PASSED]
test/test_websocket/test_websocket.cpp:972: test_base64_decode_one_byte [PASSED]
test/test_websocket/test_websocket.cpp:973: test_base64_decode_two_bytes [PASSED]
test/test_websocket/test_websocket.cpp:974: test_base64_decode_three_bytes [PASSED]
test/test_websocket/test_websocket.cpp:975: test_base64_decode_ws_accept_key [PASSED]
test/test_websocket/test_websocket.cpp:976: test_base64_decode_rejects_misplaced_padding [PASSED]
test/test_websocket/test_websocket.cpp:977: test_base64_decode_respects_capacity [PASSED]
test/test_websocket/test_websocket.cpp:978: test_base64_round_trip      [PASSED]
test/test_websocket/test_websocket.cpp:981: test_ws_pool_size           [PASSED]
test/test_websocket/test_websocket.cpp:982: test_ws_ids_match_indices_after_init [PASSED]
test/test_websocket/test_websocket.cpp:983: test_ws_all_inactive_after_init [PASSED]
test/test_websocket/test_websocket.cpp:984: test_ws_alloc_returns_non_null [PASSED]
test/test_websocket/test_websocket.cpp:985: test_ws_alloc_sets_active   [PASSED]
test/test_websocket/test_websocket.cpp:986: test_ws_alloc_sets_slot_id  [PASSED]
test/test_websocket/test_websocket.cpp:987: test_ws_alloc_sets_parse_state_header1 [PASSED]
test/test_websocket/test_websocket.cpp:988: test_ws_alloc_pool_full_returns_null [PASSED]
test/test_websocket/test_websocket.cpp:989: test_ws_find_returns_correct_conn [PASSED]
test/test_websocket/test_websocket.cpp:990: test_ws_find_returns_null_when_empty [PASSED]
test/test_websocket/test_websocket.cpp:991: test_ws_find_returns_null_for_different_slot [PASSED]
test/test_websocket/test_websocket.cpp:992: test_ws_find_after_both_slots_allocated [PASSED]
test/test_websocket/test_websocket.cpp:993: test_ws_free_deactivates_slot [PASSED]
test/test_websocket/test_websocket.cpp:994: test_ws_free_restores_ws_id [PASSED]
test/test_websocket/test_websocket.cpp:995: test_ws_free_makes_slot_findable_as_null [PASSED]
test/test_websocket/test_websocket.cpp:996: test_ws_free_nop_on_unallocated [PASSED]
test/test_websocket/test_websocket.cpp:997: test_ws_alloc_after_free_succeeds [PASSED]
test/test_websocket/test_websocket.cpp:1000: test_ws_parse_text_frame_sets_ready [PASSED]
test/test_websocket/test_websocket.cpp:1001: test_ws_parse_payload_stored_correctly [PASSED]
test/test_websocket/test_websocket.cpp:1002: test_ws_parse_binary_frame_sets_ready [PASSED]
test/test_websocket/test_websocket.cpp:1003: test_ws_parse_zero_length_unmasked_frame [PASSED]
test/test_websocket/test_websocket.cpp:1004: test_ws_parse_zero_length_masked_frame [PASSED]
test/test_websocket/test_websocket.cpp:1005: test_ws_reject_unmasked_data_frame [PASSED]
test/test_websocket/test_websocket.cpp:1006: test_ws_reject_reserved_opcode [PASSED]
test/test_websocket/test_websocket.cpp:1007: test_ws_reject_fragmented_control_frame [PASSED]
test/test_websocket/test_websocket.cpp:1008: test_ws_reject_oversized_control_frame [PASSED]
test/test_websocket/test_websocket.cpp:1009: test_ws_parse_16bit_length_frame [PASSED]
test/test_websocket/test_websocket.cpp:1010: test_ws_parse_rsv1_set_closes_protocol [PASSED]
test/test_websocket/test_websocket.cpp:1011: test_ws_parse_rsv2_set_closes_protocol [PASSED]
test/test_websocket/test_websocket.cpp:1012: test_ws_parse_rsv3_set_closes_protocol [PASSED]
test/test_websocket/test_websocket.cpp:1013: test_ws_parse_64bit_length_closes_too_big [PASSED]
test/test_websocket/test_websocket.cpp:1014: test_ws_parse_oversized_16bit_length_closes_too_big [PASSED]
test/test_websocket/test_websocket.cpp:1015: test_ws_fragment_start_waits_for_continuation [PASSED]
test/test_websocket/test_websocket.cpp:1016: test_ws_fragmented_message_reassembled [PASSED]
test/test_websocket/test_websocket.cpp:1017: test_ws_control_frame_interleaved_in_fragments [PASSED]
test/test_websocket/test_websocket.cpp:1018: test_ws_continuation_without_start_rejected [PASSED]
test/test_websocket/test_websocket.cpp:1019: test_ws_new_data_frame_during_fragmentation_rejected [PASSED]
test/test_websocket/test_websocket.cpp:1020: test_ws_parse_ping_auto_pong_resets_frame [PASSED]
test/test_websocket/test_websocket.cpp:1021: test_ws_parse_pong_silently_ignored [PASSED]
test/test_websocket/test_websocket.cpp:1022: test_ws_parse_close_marks_ws_closed [PASSED]
test/test_websocket/test_websocket.cpp:1023: test_ws_parse_stops_at_frame_ready [PASSED]
test/test_websocket/test_websocket.cpp:1024: test_ws_reset_frame_clears_fields [PASSED]
test/test_websocket/test_websocket.cpp:1025: test_ws_parse_mask_applied_correctly [PASSED]
test/test_websocket/test_websocket.cpp:1027: test_ws_permessage_deflate_inbound [PASSED]
test/test_websocket/test_websocket.cpp:1028: test_ws_rsv1_without_negotiation_closes [PASSED]
test/test_websocket/test_websocket.cpp:1029: test_ws_permessage_deflate_outbound [PASSED]
test/test_websocket/test_websocket.cpp:1030: test_ws_outbound_incompressible_not_flagged [PASSED]
test/test_websocket/test_websocket.cpp:1034: stress_ws_parse_reset_100_cycles [PASSED]
test/test_websocket/test_websocket.cpp:1035: stress_ws_alloc_free_pool_cycle [PASSED]
test/test_websocket/test_websocket.cpp:1036: stress_ws_parse_incremental_byte_by_byte [PASSED]
test/test_websocket/test_websocket.cpp:1037: stress_ws_parse_max_payload [PASSED]
test/test_websocket/test_websocket.cpp:1038: stress_ws_parse_two_consecutive_frames [PASSED]
--------- native_ws_deflate:test_websocket [PASSED] Took 1.00 seconds ----------

=================================== SUMMARY ===================================
Environment        Test            Status    Duration
-----------------  --------------  --------  ------------
native_ws_deflate  test_websocket  PASSED    00:00:01.003
================= 67 test cases: 67 succeeded in 00:00:01.003 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
-------- native_time_source:test_time_source [PASSED] Took 0.63 seconds --------

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_time_source  test_time_source  PASSED    00:00:00.629
================== 9 test cases: 9 succeeded in 00:00:00.629 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
------- native_config_store:test_config_store [PASSED] Took 0.63 seconds -------

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_config_store  test_config_store  PASSED    00:00:00.633
================= 14 test cases: 14 succeeded in 00:00:00.633 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
---------- native_device_id:test_device_id [PASSED] Took 0.66 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_device_id  test_device_id  PASSED    00:00:00.659
================== 4 test cases: 4 succeeded in 00:00:00.659 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
------- native_auth_lockout:test_auth_lockout [PASSED] Took 0.64 seconds -------

=================================== SUMMARY ===================================
Environment          Test               Status    Duration
-------------------  -----------------  --------  ------------
native_auth_lockout  test_auth_lockout  PASSED    00:00:00.640
================= 10 test cases: 10 succeeded in 00:00:00.640 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
--------------- native_csrf:test_csrf [PASSED] Took 0.67 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_csrf    test_csrf  PASSED    00:00:00.672
================== 9 test cases: 9 succeeded in 00:00:00.672 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
---------- native_telemetry:test_telemetry [PASSED] Took 0.67 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_telemetry  test_telemetry  PASSED    00:00:00.670
================== 8 test cases: 8 succeeded in 00:00:00.670 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_dashboard in native_dashboard environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_dashboard/test_dashboard.cpp:155: test_layout_json            [PASSED]
test/test_dashboard/test_dashboard.cpp:156: test_values_json_initial_zero [PASSED]
test/test_dashboard/test_dashboard.cpp:157: test_set_and_values         [PASSED]
test/test_dashboard/test_dashboard.cpp:158: test_set_unknown_key        [PASSED]
test/test_dashboard/test_dashboard.cpp:159: test_configure_resets_values [PASSED]
test/test_dashboard/test_dashboard.cpp:160: test_small_buffer_fails_closed [PASSED]
test/test_dashboard/test_dashboard.cpp:161: test_parse_control_ok       [PASSED]
test/test_dashboard/test_dashboard.cpp:162: test_parse_control_float    [PASSED]
test/test_dashboard/test_dashboard.cpp:163: test_parse_control_rejects_malformed [PASSED]
test/test_dashboard/test_dashboard.cpp:164: test_dispatch_control_invokes_cb [PASSED]
test/test_dashboard/test_dashboard.cpp:165: test_layout_control_types   [PASSED]
---------- native_dashboard:test_dashboard [PASSED] Took 0.67 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_dashboard  test_dashboard  PASSED    00:00:00.670
================= 11 test cases: 11 succeeded in 00:00:00.670 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_net_egress in native_net_egress environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_net_egress/test_net_egress.cpp:53: test_classify_sta          [PASSED]
test/test_net_egress/test_net_egress.cpp:54: test_classify_ap           [PASSED]
test/test_net_egress/test_net_egress.cpp:55: test_classify_eth          [PASSED]
test/test_net_egress/test_net_egress.cpp:56: test_classify_none         [PASSED]
test/test_net_egress/test_net_egress.cpp:57: test_egress_host_stub      [PASSED]
--------- native_net_egress:test_net_egress [PASSED] Took 0.65 seconds ---------

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_net_egress  test_net_egress  PASSED    00:00:00.646
================== 5 test cases: 5 succeeded in 00:00:00.646 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
------ native_partition:test_partition_monitor [PASSED] Took 0.62 seconds ------

=================================== SUMMARY ===================================
Environment       Test                    Status    Duration
----------------  ----------------------  --------  ------------
native_partition  test_partition_monitor  PASSED    00:00:00.624
================== 5 test cases: 5 succeeded in 00:00:00.624 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_cbor in native_cbor environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_cbor/test_cbor.cpp:281: test_uint                             [PASSED]
test/test_cbor/test_cbor.cpp:282: test_int                              [PASSED]
test/test_cbor/test_cbor.cpp:283: test_text                             [PASSED]
test/test_cbor/test_cbor.cpp:284: test_bytes                            [PASSED]
test/test_cbor/test_cbor.cpp:285: test_simple                           [PASSED]
test/test_cbor/test_cbor.cpp:286: test_float                            [PASSED]
test/test_cbor/test_cbor.cpp:287: test_array_and_map                    [PASSED]
test/test_cbor/test_cbor.cpp:288: test_overflow_fails_closed            [PASSED]
test/test_cbor/test_cbor.cpp:289: test_decode_uint                      [PASSED]
test/test_cbor/test_cbor.cpp:290: test_decode_int                       [PASSED]
test/test_cbor/test_cbor.cpp:291: test_decode_float_roundtrip           [PASSED]
test/test_cbor/test_cbor.cpp:292: test_decode_roundtrip_map             [PASSED]
test/test_cbor/test_cbor.cpp:293: test_decode_truncated                 [PASSED]
test/test_cbor/test_cbor.cpp:294: test_decode_type_mismatch             [PASSED]
--------------- native_cbor:test_cbor [PASSED] Took 0.63 seconds ---------------

=================================== SUMMARY ===================================
Environment    Test       Status    Duration
-------------  ---------  --------  ------------
native_cbor    test_cbor  PASSED    00:00:00.634
================= 14 test cases: 14 succeeded in 00:00:00.634 =================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_msgpack in native_msgpack environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_msgpack/test_msgpack.cpp:164: test_uint                       [PASSED]
test/test_msgpack/test_msgpack.cpp:165: test_int                        [PASSED]
test/test_msgpack/test_msgpack.cpp:166: test_str                        [PASSED]
test/test_msgpack/test_msgpack.cpp:167: test_bytes                      [PASSED]
test/test_msgpack/test_msgpack.cpp:168: test_simple                     [PASSED]
test/test_msgpack/test_msgpack.cpp:169: test_float                      [PASSED]
test/test_msgpack/test_msgpack.cpp:170: test_array_and_map              [PASSED]
test/test_msgpack/test_msgpack.cpp:171: test_overflow_fails_closed      [PASSED]
------------ native_msgpack:test_msgpack [PASSED] Took 0.64 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_msgpack  test_msgpack  PASSED    00:00:00.643
================== 8 test cases: 8 succeeded in 00:00:00.643 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
----------- native_gpio_map:test_gpio_map [PASSED] Took 0.64 seconds -----------

=================================== SUMMARY ===================================
Environment      Test           Status    Duration
---------------  -------------  --------  ------------
native_gpio_map  test_gpio_map  PASSED    00:00:00.639
================== 8 test cases: 8 succeeded in 00:00:00.639 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_udp_telemetry in native_udp_telemetry environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_udp_telemetry/test_udp_telemetry.cpp:63: test_int_and_uint_fields [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:64: test_float_field     [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:65: test_no_fields_not_ok [PASSED]
test/test_udp_telemetry/test_udp_telemetry.cpp:66: test_overflow_fails_closed [PASSED]
------ native_udp_telemetry:test_udp_telemetry [PASSED] Took 0.63 seconds ------

=================================== SUMMARY ===================================
Environment           Test                Status    Duration
--------------------  ------------------  --------  ------------
native_udp_telemetry  test_udp_telemetry  PASSED    00:00:00.629
================== 4 test cases: 4 succeeded in 00:00:00.629 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
--------- native_guardrails:test_guardrails [PASSED] Took 0.62 seconds ---------

=================================== SUMMARY ===================================
Environment        Test             Status    Duration
-----------------  ---------------  --------  ------------
native_guardrails  test_guardrails  PASSED    00:00:00.623
================== 6 test cases: 6 succeeded in 00:00:00.623 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
------------- native_logbuf:test_logbuf [PASSED] Took 0.63 seconds -------------

=================================== SUMMARY ===================================
Environment    Test         Status    Duration
-------------  -----------  --------  ------------
native_logbuf  test_logbuf  PASSED    00:00:00.628
================== 4 test cases: 4 succeeded in 00:00:00.628 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

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
---------- native_config_io:test_config_io [PASSED] Took 0.65 seconds ----------

=================================== SUMMARY ===================================
Environment       Test            Status    Duration
----------------  --------------  --------  ------------
native_config_io  test_config_io  PASSED    00:00:00.655
================== 4 test cases: 4 succeeded in 00:00:00.655 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_workers in native_workers environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_workers/test_workers.cpp:62: test_worker_count_is_two         [PASSED]
test/test_workers/test_workers.cpp:63: test_check_timeouts_reaps_only_owned_slots [PASSED]
test/test_workers/test_workers.cpp:64: test_pool_init_defaults_owner_zero [PASSED]
------------ native_workers:test_workers [PASSED] Took 0.72 seconds ------------

=================================== SUMMARY ===================================
Environment     Test          Status    Duration
--------------  ------------  --------  ------------
native_workers  test_workers  PASSED    00:00:00.724
================== 3 test cases: 3 succeeded in 00:00:00.724 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_clock in native_clock environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_clock/test_clock.cpp:67: test_default_is_platform_millis      [PASSED]
test/test_clock/test_clock.cpp:68: test_custom_clock_divides_to_1000hz  [PASSED]
test/test_clock/test_clock.cpp:69: test_sub_khz_source_not_divided      [PASSED]
test/test_clock/test_clock.cpp:70: test_revert_to_default               [PASSED]
-------------- native_clock:test_clock [PASSED] Took 0.61 seconds --------------

=================================== SUMMARY ===================================
Environment    Test        Status    Duration
-------------  ----------  --------  ------------
native_clock   test_clock  PASSED    00:00:00.615
================== 4 test cases: 4 succeeded in 00:00:00.615 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_concurrency in native_concurrency environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_concurrency/test_concurrency.cpp:109: test_spsc_ring_no_race  [PASSED]
test/test_concurrency/test_concurrency.cpp:110: test_state_handoff_no_race [PASSED]
-------- native_concurrency:test_concurrency [PASSED] Took 0.86 seconds --------

=================================== SUMMARY ===================================
Environment         Test              Status    Duration
------------------  ----------------  --------  ------------
native_concurrency  test_concurrency  PASSED    00:00:00.862
================== 2 test cases: 2 succeeded in 00:00:00.862 ==================
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 88 tests

Processing test_concurrency in native_tsan environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_concurrency/test_concurrency.cpp:109: test_spsc_ring_no_race  [PASSED]
test/test_concurrency/test_concurrency.cpp:110: test_state_handoff_no_race [PASSED]
----------- native_tsan:test_concurrency [PASSED] Took 1.30 seconds ------------

=================================== SUMMARY ===================================
Environment    Test              Status    Duration
-------------  ----------------  --------  ------------
native_tsan    test_concurrency  PASSED    00:00:01.299
================== 2 test cases: 2 succeeded in 00:00:01.299 ==================
```

</details>
