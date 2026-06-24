# Test Report

**Generated:** 2026-06-24 03:15:34
**Command:** `pio test -e native -e native_app -e native_ssh -e native_ssh_hardened -e native_ssh_conn -e native_compliance`
**Result:** ✅ 592 passed - 30s

---

## Summary

| Suite                   | Environment           | Tests | Status |     Duration |
| :---------------------- | :-------------------- | ----: | :----: | -----------: |
| `test_sse`              | `native`              |    37 |   ✅   | 00:00:06.067 |
| `test_session`          | `native`              |    19 |   ✅   | 00:00:00.484 |
| `test_presentation`     | `native`              |    63 |   ✅   | 00:00:00.520 |
| `test_transport`        | `native`              |    28 |   ✅   | 00:00:00.490 |
| `test_websocket`        | `native`              |    63 |   ✅   | 00:00:00.533 |
| `test_http_parser`      | `native`              |    80 |   ✅   | 00:00:00.513 |
| `test_ssh_crypto`       | `native_ssh`          |    37 |   ✅   | 00:00:03.811 |
| `test_ssh_auth`         | `native_ssh`          |    12 |   ✅   | 00:00:00.516 |
| `test_ssh_server`       | `native_ssh`          |     7 |   ✅   | 00:00:00.634 |
| `test_ssh_transport`    | `native_ssh`          |    23 |   ✅   | 00:00:00.839 |
| `test_ssh_channel`      | `native_ssh`          |    12 |   ✅   | 00:00:00.484 |
| `test_ssh_hardening`    | `native_ssh_hardened` |     2 |   ✅   | 00:00:00.785 |
| `test_ssh_conn`         | `native_ssh_conn`     |     2 |   ✅   | 00:00:01.061 |
| `test_regex`            | `native_app`          |     9 |   ✅   | 00:00:00.905 |
| `test_template`         | `native_app`          |     6 |   ✅   | 00:00:00.503 |
| `test_path_params`      | `native_app`          |     8 |   ✅   | 00:00:00.502 |
| `test_digest_vectors`   | `native_app`          |     4 |   ✅   | 00:00:00.473 |
| `test_form_params`      | `native_app`          |     5 |   ✅   | 00:00:00.489 |
| `test_iface`            | `native_app`          |     7 |   ✅   | 00:00:00.493 |
| `test_json`             | `native_app`          |    17 |   ✅   | 00:00:00.488 |
| `test_response_headers` | `native_app`          |     9 |   ✅   | 00:00:00.499 |
| `test_middleware`       | `native_app`          |     9 |   ✅   | 00:00:00.518 |
| `test_digest_auth`      | `native_app`          |     5 |   ✅   | 00:00:00.512 |
| `test_web_terminal`     | `native_app`          |     7 |   ✅   | 00:00:00.499 |
| `test_multipart`        | `native_app`          |    19 |   ✅   | 00:00:00.508 |
| `test_auth`             | `native_app`          |    13 |   ✅   | 00:00:00.507 |
| `test_file_serving`     | `native_app`          |    12 |   ✅   | 00:00:00.513 |
| `test_dispatch`         | `native_app`          |    10 |   ✅   | 00:00:00.498 |
| `test_chunked`          | `native_app`          |     8 |   ✅   | 00:00:00.504 |
| `test_application`      | `native_app`          |    47 |   ✅   | 00:00:00.595 |
| `test_compliance`       | `native_compliance`   |    12 |   ✅   | 00:00:00.621 |

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

## test_transport - ✅ 28 passed

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

</details>

---

## test_websocket - ✅ 63 passed

<details>
<summary><b>Expand Suite Details</b></summary>

_Unit and stress tests for SHA-1, Base64, and the WebSocket frame parser._

|   # | Test                                                   | Status | Description                                                               |
| --: | :----------------------------------------------------- | :----: | :------------------------------------------------------------------------ | ---------------------------------------------------------- | ------------------- |
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

## test_http_parser - ✅ 80 passed

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
|  63 | `test_header_val_nul_byte_is_error`                      |   ✅   | Header val nul byte is error                                                    |
|  64 | `test_header_val_control_char_is_error`                  |   ✅   | Header val control char is error                                                |
|  65 | `test_header_val_del_byte_is_error`                      |   ✅   | Header val del byte is error                                                    |
|  66 | `test_header_val_htab_mid_value_allowed`                 |   ✅   | HTAB is valid mid-value (RFC 7230 §3.2)                                         |
|  67 | `test_header_val_leading_htab_stripped`                  |   ✅   | Leading HTAB (OWS) is stripped just like leading SP                             |
|  68 | `test_header_val_obs_text_allowed`                       |   ✅   | obs-text bytes (%x80-FF) are allowed for legacy compatibility (RFC 7230 §3.2.6) |
|  69 | `test_version_http11_recognized`                         |   ✅   | Version http11 recognized                                                       |
|  70 | `test_version_http10_recognized`                         |   ✅   | Version http10 recognized                                                       |
|  71 | `test_version_unknown_is_http_unknown`                   |   ✅   | Version unknown is http unknown                                                 |
|  72 | `test_version_reset_to_unknown`                          |   ✅   | Version reset to unknown                                                        |
|  73 | `test_bad_expect_lf_is_error`                            |   ✅   | CRLF in version line replaced by CR + X (no LF)                                 |
|  74 | `test_blank_line_non_lf_is_error`                        |   ✅   | Header block ends with CR + non-LF in the blank line                            |
|  75 | `test_slots_are_independent`                             |   ✅   | Slots are independent                                                           |
|  76 | `test_incremental_byte_by_byte`                          |   ✅   | Incremental byte by byte                                                        |
|  77 | `test_incremental_two_chunks`                            |   ✅   | Incremental two chunks                                                          |
|  78 | `stress_many_requests_same_slot`                         |   ✅   | Stress - Many requests same slot                                                |
|  79 | `stress_max_headers`                                     |   ✅   | Build a request with MAX_HEADERS header lines                                   |
|  80 | `stress_max_query_params`                                |   ✅   | Build a query string with MAX_QUERY_PARAMS parameters                           |

</details>

---

## test_ssh_crypto - ✅ 37 passed

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

_Unit tests for http_get_form() — application/x-www-form-urlencoded body_

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

## test_dispatch - ✅ 10 passed

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
|  10 | `test_correct_method_still_dispatches`      |   ✅   | Correct method still dispatches                                             |

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

## test_application - ✅ 47 passed

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
|  46 | `test_request_log_hook_fires`                         |   ✅   | Request log hook fires                                                     |
|  47 | `test_stats_endpoint_emits_json`                      |   ✅   | Stats endpoint emits json                                                  |

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
Collected 37 tests
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
------------------ native:test_sse [PASSED] Took 6.07 seconds ------------------

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
---------------- native:test_session [PASSED] Took 0.48 seconds ----------------

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
------------- native:test_presentation [PASSED] Took 0.52 seconds --------------

Processing test_transport in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_transport/test_transport.cpp:394: test_pool_capacity_is_four  [PASSED]
test/test_transport/test_transport.cpp:395: test_rx_buffer_size_is_one_kb [PASSED]
test/test_transport/test_transport.cpp:396: test_timeout_constant_is_5000ms [PASSED]
test/test_transport/test_transport.cpp:397: test_all_slots_free_after_init [PASSED]
test/test_transport/test_transport.cpp:398: test_all_pcbs_null_after_init [PASSED]
test/test_transport/test_transport.cpp:399: test_all_ring_buffers_empty_after_init [PASSED]
test/test_transport/test_transport.cpp:400: test_slot_ids_match_indices [PASSED]
test/test_transport/test_transport.cpp:401: test_ring_empty_when_head_equals_tail [PASSED]
test/test_transport/test_transport.cpp:402: test_ring_wrap_at_boundary  [PASSED]
test/test_transport/test_transport.cpp:403: test_ring_full_sentinel_one_slot_reserved [PASSED]
test/test_transport/test_transport.cpp:404: test_ring_can_store_size_minus_one_bytes [PASSED]
test/test_transport/test_transport.cpp:405: test_event_types_are_distinct [PASSED]
test/test_transport/test_transport.cpp:406: test_timeout_does_not_fire_on_free_slot [PASSED]
test/test_transport/test_transport.cpp:407: test_timeout_does_not_fire_before_deadline [PASSED]
test/test_transport/test_transport.cpp:408: test_timeout_fires_at_deadline [PASSED]
test/test_transport/test_transport.cpp:409: test_timeout_fires_only_on_stale_slots [PASSED]
test/test_transport/test_transport.cpp:410: test_init_succeeds_on_native [PASSED]
test/test_transport/test_transport.cpp:411: test_all_last_activity_ms_zero_after_init [PASSED]
test/test_transport/test_transport.cpp:412: test_queue_not_null_after_init [PASSED]
test/test_transport/test_transport.cpp:415: stress_ring_buffer_fill_drain_integrity [PASSED]
test/test_transport/test_transport.cpp:416: stress_ring_buffer_multi_cycle_no_corruption [PASSED]
test/test_transport/test_transport.cpp:417: stress_all_slots_timeout_simultaneously [PASSED]
test/test_transport/test_transport.cpp:418: stress_timeout_arm_recover_cycle [PASSED]
test/test_transport/test_transport.cpp:419: stress_check_timeouts_high_call_rate [PASSED]
test/test_transport/test_transport.cpp:420: stress_ring_buffer_byte_by_byte_fill_and_drain [PASSED]
test/test_transport/test_transport.cpp:423: test_accept_throttle_blocks_over_budget [PASSED]
test/test_transport/test_transport.cpp:424: test_accept_throttle_window_refills [PASSED]
test/test_transport/test_transport.cpp:425: test_accept_throttle_handles_rollover [PASSED]
--------------- native:test_transport [PASSED] Took 0.49 seconds ---------------

Processing test_websocket in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_websocket/test_websocket.cpp:846: test_sha1_empty_string      [PASSED]
test/test_websocket/test_websocket.cpp:847: test_sha1_abc               [PASSED]
test/test_websocket/test_websocket.cpp:848: test_sha1_rfc6455_handshake_key [PASSED]
test/test_websocket/test_websocket.cpp:849: test_sha1_different_inputs_different_digests [PASSED]
test/test_websocket/test_websocket.cpp:852: test_base64_encode_one_byte [PASSED]
test/test_websocket/test_websocket.cpp:853: test_base64_encode_two_bytes [PASSED]
test/test_websocket/test_websocket.cpp:854: test_base64_encode_three_bytes [PASSED]
test/test_websocket/test_websocket.cpp:855: test_base64_encode_ws_accept_key [PASSED]
test/test_websocket/test_websocket.cpp:856: test_base64_decode_one_byte [PASSED]
test/test_websocket/test_websocket.cpp:857: test_base64_decode_two_bytes [PASSED]
test/test_websocket/test_websocket.cpp:858: test_base64_decode_three_bytes [PASSED]
test/test_websocket/test_websocket.cpp:859: test_base64_decode_ws_accept_key [PASSED]
test/test_websocket/test_websocket.cpp:860: test_base64_decode_rejects_misplaced_padding [PASSED]
test/test_websocket/test_websocket.cpp:861: test_base64_decode_respects_capacity [PASSED]
test/test_websocket/test_websocket.cpp:862: test_base64_round_trip      [PASSED]
test/test_websocket/test_websocket.cpp:865: test_ws_pool_size           [PASSED]
test/test_websocket/test_websocket.cpp:866: test_ws_ids_match_indices_after_init [PASSED]
test/test_websocket/test_websocket.cpp:867: test_ws_all_inactive_after_init [PASSED]
test/test_websocket/test_websocket.cpp:868: test_ws_alloc_returns_non_null [PASSED]
test/test_websocket/test_websocket.cpp:869: test_ws_alloc_sets_active   [PASSED]
test/test_websocket/test_websocket.cpp:870: test_ws_alloc_sets_slot_id  [PASSED]
test/test_websocket/test_websocket.cpp:871: test_ws_alloc_sets_parse_state_header1 [PASSED]
test/test_websocket/test_websocket.cpp:872: test_ws_alloc_pool_full_returns_null [PASSED]
test/test_websocket/test_websocket.cpp:873: test_ws_find_returns_correct_conn [PASSED]
test/test_websocket/test_websocket.cpp:874: test_ws_find_returns_null_when_empty [PASSED]
test/test_websocket/test_websocket.cpp:875: test_ws_find_returns_null_for_different_slot [PASSED]
test/test_websocket/test_websocket.cpp:876: test_ws_find_after_both_slots_allocated [PASSED]
test/test_websocket/test_websocket.cpp:877: test_ws_free_deactivates_slot [PASSED]
test/test_websocket/test_websocket.cpp:878: test_ws_free_restores_ws_id [PASSED]
test/test_websocket/test_websocket.cpp:879: test_ws_free_makes_slot_findable_as_null [PASSED]
test/test_websocket/test_websocket.cpp:880: test_ws_free_nop_on_unallocated [PASSED]
test/test_websocket/test_websocket.cpp:881: test_ws_alloc_after_free_succeeds [PASSED]
test/test_websocket/test_websocket.cpp:884: test_ws_parse_text_frame_sets_ready [PASSED]
test/test_websocket/test_websocket.cpp:885: test_ws_parse_payload_stored_correctly [PASSED]
test/test_websocket/test_websocket.cpp:886: test_ws_parse_binary_frame_sets_ready [PASSED]
test/test_websocket/test_websocket.cpp:887: test_ws_parse_zero_length_unmasked_frame [PASSED]
test/test_websocket/test_websocket.cpp:888: test_ws_parse_zero_length_masked_frame [PASSED]
test/test_websocket/test_websocket.cpp:889: test_ws_reject_unmasked_data_frame [PASSED]
test/test_websocket/test_websocket.cpp:890: test_ws_reject_reserved_opcode [PASSED]
test/test_websocket/test_websocket.cpp:891: test_ws_reject_fragmented_control_frame [PASSED]
test/test_websocket/test_websocket.cpp:892: test_ws_reject_oversized_control_frame [PASSED]
test/test_websocket/test_websocket.cpp:893: test_ws_parse_16bit_length_frame [PASSED]
test/test_websocket/test_websocket.cpp:894: test_ws_parse_rsv1_set_closes_protocol [PASSED]
test/test_websocket/test_websocket.cpp:895: test_ws_parse_rsv2_set_closes_protocol [PASSED]
test/test_websocket/test_websocket.cpp:896: test_ws_parse_rsv3_set_closes_protocol [PASSED]
test/test_websocket/test_websocket.cpp:897: test_ws_parse_64bit_length_closes_too_big [PASSED]
test/test_websocket/test_websocket.cpp:898: test_ws_parse_oversized_16bit_length_closes_too_big [PASSED]
test/test_websocket/test_websocket.cpp:899: test_ws_fragment_start_waits_for_continuation [PASSED]
test/test_websocket/test_websocket.cpp:900: test_ws_fragmented_message_reassembled [PASSED]
test/test_websocket/test_websocket.cpp:901: test_ws_control_frame_interleaved_in_fragments [PASSED]
test/test_websocket/test_websocket.cpp:902: test_ws_continuation_without_start_rejected [PASSED]
test/test_websocket/test_websocket.cpp:903: test_ws_new_data_frame_during_fragmentation_rejected [PASSED]
test/test_websocket/test_websocket.cpp:904: test_ws_parse_ping_auto_pong_resets_frame [PASSED]
test/test_websocket/test_websocket.cpp:905: test_ws_parse_pong_silently_ignored [PASSED]
test/test_websocket/test_websocket.cpp:906: test_ws_parse_close_marks_ws_closed [PASSED]
test/test_websocket/test_websocket.cpp:907: test_ws_parse_stops_at_frame_ready [PASSED]
test/test_websocket/test_websocket.cpp:908: test_ws_reset_frame_clears_fields [PASSED]
test/test_websocket/test_websocket.cpp:909: test_ws_parse_mask_applied_correctly [PASSED]
test/test_websocket/test_websocket.cpp:912: stress_ws_parse_reset_100_cycles [PASSED]
test/test_websocket/test_websocket.cpp:913: stress_ws_alloc_free_pool_cycle [PASSED]
test/test_websocket/test_websocket.cpp:914: stress_ws_parse_incremental_byte_by_byte [PASSED]
test/test_websocket/test_websocket.cpp:915: stress_ws_parse_max_payload [PASSED]
test/test_websocket/test_websocket.cpp:916: stress_ws_parse_two_consecutive_frames [PASSED]
--------------- native:test_websocket [PASSED] Took 0.53 seconds ---------------

Processing test_http_parser in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_http_parser/test_http_parser.cpp:827: test_reset_sets_parse_method_state [PASSED]
test/test_http_parser/test_http_parser.cpp:828: test_reset_preserves_slot_id [PASSED]
test/test_http_parser/test_http_parser.cpp:829: test_reset_clears_method [PASSED]
test/test_http_parser/test_http_parser.cpp:830: test_reset_clears_path  [PASSED]
test/test_http_parser/test_http_parser.cpp:831: test_reset_clears_header_count [PASSED]
test/test_http_parser/test_http_parser.cpp:832: test_reset_clears_body  [PASSED]
test/test_http_parser/test_http_parser.cpp:833: test_reset_clears_query_count [PASSED]
test/test_http_parser/test_http_parser.cpp:836: test_feed_after_complete_does_not_change_state [PASSED]
test/test_http_parser/test_http_parser.cpp:837: test_feed_after_error_does_not_change_state [PASSED]
test/test_http_parser/test_http_parser.cpp:838: test_feed_after_entity_too_large_does_not_change_state [PASSED]
test/test_http_parser/test_http_parser.cpp:841: test_method_get         [PASSED]
test/test_http_parser/test_http_parser.cpp:842: test_method_post        [PASSED]
test/test_http_parser/test_http_parser.cpp:843: test_method_put         [PASSED]
test/test_http_parser/test_http_parser.cpp:844: test_method_delete      [PASSED]
test/test_http_parser/test_http_parser.cpp:845: test_method_patch       [PASSED]
test/test_http_parser/test_http_parser.cpp:846: test_method_head        [PASSED]
test/test_http_parser/test_http_parser.cpp:847: test_method_options     [PASSED]
test/test_http_parser/test_http_parser.cpp:848: test_method_overflow_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:851: test_path_root          [PASSED]
test/test_http_parser/test_http_parser.cpp:852: test_path_segments      [PASSED]
test/test_http_parser/test_http_parser.cpp:853: test_path_without_query [PASSED]
test/test_http_parser/test_http_parser.cpp:854: test_path_overflow_is_414 [PASSED]
test/test_http_parser/test_http_parser.cpp:857: test_single_query_param [PASSED]
test/test_http_parser/test_http_parser.cpp:858: test_two_query_params   [PASSED]
test/test_http_parser/test_http_parser.cpp:859: test_query_key_not_found_returns_null [PASSED]
test/test_http_parser/test_http_parser.cpp:860: test_query_empty_value  [PASSED]
test/test_http_parser/test_http_parser.cpp:863: test_single_header_stored [PASSED]
test/test_http_parser/test_http_parser.cpp:864: test_header_lookup_case_insensitive [PASSED]
test/test_http_parser/test_http_parser.cpp:865: test_header_leading_space_stripped [PASSED]
test/test_http_parser/test_http_parser.cpp:866: test_content_length_header_parsed [PASSED]
test/test_http_parser/test_http_parser.cpp:867: test_content_length_in_headers_array [PASSED]
test/test_http_parser/test_http_parser.cpp:868: test_multiple_headers_stored [PASSED]
test/test_http_parser/test_http_parser.cpp:869: test_missing_header_returns_null [PASSED]
test/test_http_parser/test_http_parser.cpp:872: test_get_no_body_completes [PASSED]
test/test_http_parser/test_http_parser.cpp:873: test_post_with_body     [PASSED]
test/test_http_parser/test_http_parser.cpp:874: test_put_with_body      [PASSED]
test/test_http_parser/test_http_parser.cpp:875: test_body_starting_with_newline [PASSED]
test/test_http_parser/test_http_parser.cpp:876: test_post_content_length_zero [PASSED]
test/test_http_parser/test_http_parser.cpp:877: test_body_exactly_at_buffer_limit [PASSED]
test/test_http_parser/test_http_parser.cpp:878: test_body_null_terminated_after_complete [PASSED]
test/test_http_parser/test_http_parser.cpp:881: test_body_one_over_limit_is_413 [PASSED]
test/test_http_parser/test_http_parser.cpp:882: test_body_far_over_limit_is_413 [PASSED]
test/test_http_parser/test_http_parser.cpp:883: test_413_no_body_bytes_fed [PASSED]
test/test_http_parser/test_http_parser.cpp:884: test_413_header_still_stored [PASSED]
test/test_http_parser/test_http_parser.cpp:885: test_body_exactly_at_limit_is_not_413 [PASSED]
test/test_http_parser/test_http_parser.cpp:888: test_path_overflow_stops_feeding [PASSED]
test/test_http_parser/test_http_parser.cpp:889: test_414_path_filled_to_capacity [PASSED]
test/test_http_parser/test_http_parser.cpp:892: test_method_nul_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:893: test_method_control_char_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:894: test_method_del_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:895: test_method_non_tchar_symbol_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:896: test_method_tchar_symbols_accepted [PASSED]
test/test_http_parser/test_http_parser.cpp:899: test_path_nul_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:900: test_path_control_char_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:901: test_path_del_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:902: test_query_nul_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:903: test_query_control_char_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:906: test_header_key_space_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:907: test_header_key_nul_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:908: test_header_key_control_char_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:909: test_header_key_mid_cr_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:910: test_header_key_colon_at_start_skips_header [PASSED]
test/test_http_parser/test_http_parser.cpp:913: test_header_val_nul_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:914: test_header_val_control_char_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:915: test_header_val_del_byte_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:916: test_header_val_htab_mid_value_allowed [PASSED]
test/test_http_parser/test_http_parser.cpp:917: test_header_val_leading_htab_stripped [PASSED]
test/test_http_parser/test_http_parser.cpp:918: test_header_val_obs_text_allowed [PASSED]
test/test_http_parser/test_http_parser.cpp:921: test_version_http11_recognized [PASSED]
test/test_http_parser/test_http_parser.cpp:922: test_version_http10_recognized [PASSED]
test/test_http_parser/test_http_parser.cpp:923: test_version_unknown_is_http_unknown [PASSED]
test/test_http_parser/test_http_parser.cpp:924: test_version_reset_to_unknown [PASSED]
test/test_http_parser/test_http_parser.cpp:927: test_bad_expect_lf_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:928: test_blank_line_non_lf_is_error [PASSED]
test/test_http_parser/test_http_parser.cpp:931: test_slots_are_independent [PASSED]
test/test_http_parser/test_http_parser.cpp:934: test_incremental_byte_by_byte [PASSED]
test/test_http_parser/test_http_parser.cpp:935: test_incremental_two_chunks [PASSED]
test/test_http_parser/test_http_parser.cpp:938: stress_many_requests_same_slot [PASSED]
test/test_http_parser/test_http_parser.cpp:939: stress_max_headers      [PASSED]
test/test_http_parser/test_http_parser.cpp:940: stress_max_query_params [PASSED]
-------------- native:test_http_parser [PASSED] Took 0.51 seconds --------------

Processing test_ssh_crypto in native_ssh environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_crypto/test_ssh_crypto.cpp:848: test_sha256_empty         [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:849: test_sha256_abc           [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:850: test_sha256_448bit        [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:851: test_sha256_streaming     [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:854: test_hmac_sha256_tc1      [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:855: test_hmac_sha256_tc2      [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:856: test_hmac_sha256_tc3      [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:857: test_hmac_sha256_streaming [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:860: test_aes256ctr_encrypt    [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:861: test_aes256ctr_decrypt    [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:862: test_aes256ctr_multi_block [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:863: test_aes256ctr_wipe       [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:866: test_bn_roundtrip         [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:867: test_bn_cmp_equal         [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:868: test_bn_cmp_less          [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:869: test_bn_cmp_greater       [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:870: test_bn_is_zero           [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:871: test_bn_dh_validate_rejects_zero [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:872: test_bn_dh_validate_rejects_one [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:873: test_bn_dh_validate_accepts_two [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:876: test_expmod_exp1          [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:877: test_expmod_exp2          [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:878: test_expmod_exp3          [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:879: test_expmod_commutative   [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:882: test_rsa_pkcs1_pad_structure [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:883: test_rsa_sign_verify_roundtrip [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:884: test_rsa_encode_pubkey    [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:885: test_rsa_verify_valid_signature [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:886: test_rsa_verify_rejects_tampered_signature [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:887: test_rsa_verify_rejects_wrong_message [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:890: test_pkt_send_recv_unencrypted [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:891: test_pkt_padding_alignment [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:892: test_pkt_seq_increments   [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:893: test_pkt_disconnect_zeroes_state [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:894: test_pkt_encrypted_roundtrip [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:895: test_pkt_encrypted_fragmented [PASSED]
test/test_ssh_crypto/test_ssh_crypto.cpp:896: test_pkt_encrypted_two_packets [PASSED]
------------ native_ssh:test_ssh_crypto [PASSED] Took 3.81 seconds -------------

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
------------- native_ssh:test_ssh_auth [PASSED] Took 0.52 seconds --------------

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
------------ native_ssh:test_ssh_server [PASSED] Took 0.63 seconds -------------

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
----------- native_ssh:test_ssh_transport [PASSED] Took 0.84 seconds -----------

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
------------ native_ssh:test_ssh_channel [PASSED] Took 0.48 seconds ------------

Processing test_ssh_hardening in native_ssh_hardened environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_hardening/test_ssh_hardening.cpp:87: test_password_refused_even_with_correct_callback [PASSED]
test/test_ssh_hardening/test_ssh_hardening.cpp:88: test_failure_advertises_publickey_only [PASSED]
------ native_ssh_hardened:test_ssh_hardening [PASSED] Took 0.78 seconds -------

Processing test_ssh_conn in native_ssh_conn environment
--------------------------------------------------------------------------------
Building...
Library Manager: Installing throwtheswitch/Unity @ ^2.6.1
Unpacking 0% 10% 20% 30% 40% 50% 60% 70% 80% 90% 100%
Library Manager: Unity@2.6.1 has been installed!
Testing...
test/test_ssh_conn/test_ssh_conn.cpp:137: test_accept_sends_server_banner [PASSED]
test/test_ssh_conn/test_ssh_conn.cpp:138: test_banner_then_kexinit_advances_and_replies [PASSED]
----------- native_ssh_conn:test_ssh_conn [PASSED] Took 1.06 seconds -----------

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
--------------- native_app:test_regex [PASSED] Took 0.91 seconds ---------------

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
------------- native_app:test_template [PASSED] Took 0.50 seconds --------------

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
------------ native_app:test_path_params [PASSED] Took 0.50 seconds ------------

Processing test_digest_vectors in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_digest_vectors/test_digest_vectors.cpp:108: test_sha256_fips_kats [PASSED]
test/test_digest_vectors/test_digest_vectors.cpp:109: test_ha1_matches_openssl [PASSED]
test/test_digest_vectors/test_digest_vectors.cpp:110: test_ha2_matches_openssl [PASSED]
test/test_digest_vectors/test_digest_vectors.cpp:111: test_response_matches_openssl [PASSED]
---------- native_app:test_digest_vectors [PASSED] Took 0.47 seconds -----------

Processing test_form_params in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_form_params/test_form_params.cpp:134: test_form_fields_parsed [PASSED]
test/test_form_params/test_form_params.cpp:135: test_form_missing_key_returns_false [PASSED]
test/test_form_params/test_form_params.cpp:136: test_form_empty_value   [PASSED]
test/test_form_params/test_form_params.cpp:137: test_form_wrong_content_type_ignored [PASSED]
test/test_form_params/test_form_params.cpp:138: test_form_value_truncated_to_buffer [PASSED]
------------ native_app:test_form_params [PASSED] Took 0.49 seconds ------------

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
--------------- native_app:test_iface [PASSED] Took 0.49 seconds ---------------

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
--------------- native_app:test_json [PASSED] Took 0.49 seconds ----------------

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
--------- native_app:test_response_headers [PASSED] Took 0.50 seconds ----------

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
------------ native_app:test_middleware [PASSED] Took 0.52 seconds -------------

Processing test_digest_auth in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_digest_auth/test_digest_auth.cpp:231: test_challenge_is_digest_sha256 [PASSED]
test/test_digest_auth/test_digest_auth.cpp:232: test_valid_digest_authenticates [PASSED]
test/test_digest_auth/test_digest_auth.cpp:233: test_wrong_password_rejected [PASSED]
test/test_digest_auth/test_digest_auth.cpp:234: test_bad_nonce_rejected [PASSED]
test/test_digest_auth/test_digest_auth.cpp:235: test_nonce_is_128bit_hex [PASSED]
------------ native_app:test_digest_auth [PASSED] Took 0.51 seconds ------------

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
----------- native_app:test_web_terminal [PASSED] Took 0.50 seconds ------------

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
------------- native_app:test_multipart [PASSED] Took 0.51 seconds -------------

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
--------------- native_app:test_auth [PASSED] Took 0.51 seconds ----------------

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
----------- native_app:test_file_serving [PASSED] Took 0.51 seconds ------------

Processing test_dispatch in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_dispatch/test_dispatch.cpp:162: test_method_mismatch_returns_405 [PASSED]
test/test_dispatch/test_dispatch.cpp:163: test_405_includes_allow_header [PASSED]
test/test_dispatch/test_dispatch.cpp:164: test_405_allow_lists_all_methods_for_path [PASSED]
test/test_dispatch/test_dispatch.cpp:165: test_unknown_path_still_404_not_405 [PASSED]
test/test_dispatch/test_dispatch.cpp:166: test_unknown_method_returns_501 [PASSED]
test/test_dispatch/test_dispatch.cpp:167: test_unknown_method_not_treated_as_get [PASSED]
test/test_dispatch/test_dispatch.cpp:168: test_head_runs_get_handler_without_body [PASSED]
test/test_dispatch/test_dispatch.cpp:169: test_get_route_advertises_head_in_allow [PASSED]
test/test_dispatch/test_dispatch.cpp:170: test_head_on_post_only_route_405 [PASSED]
test/test_dispatch/test_dispatch.cpp:171: test_correct_method_still_dispatches [PASSED]
------------- native_app:test_dispatch [PASSED] Took 0.50 seconds --------------

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
-------------- native_app:test_chunked [PASSED] Took 0.50 seconds --------------

Processing test_application in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test/test_application/test_application.cpp:828: test_handler_reads_body [PASSED]
test/test_application/test_application.cpp:829: test_handler_reads_query_param [PASSED]
test/test_application/test_application.cpp:830: test_handler_reads_header [PASSED]
test/test_application/test_application.cpp:831: test_wildcard_before_exact_wildcard_wins [PASSED]
test/test_application/test_application.cpp:834: test_fn_on_registers_and_dispatches [PASSED]
test/test_application/test_application.cpp:835: test_fn_on_path_copied_null_terminated [PASSED]
test/test_application/test_application.cpp:836: test_fn_on_table_full_extra_routes_dropped [PASSED]
test/test_application/test_application.cpp:837: test_fn_on_same_path_different_methods_are_distinct [PASSED]
test/test_application/test_application.cpp:840: test_fn_on_not_found_called_when_no_match [PASSED]
test/test_application/test_application.cpp:841: test_fn_on_not_found_not_called_when_match_exists [PASSED]
test/test_application/test_application.cpp:844: test_fn_set_cors_options_preflight_clears_slot [PASSED]
test/test_application/test_application.cpp:845: test_fn_set_cors_empty_string_disables [PASSED]
test/test_application/test_application.cpp:848: test_wrong_method_does_not_match [PASSED]
test/test_application/test_application.cpp:849: test_wrong_path_does_not_match [PASSED]
test/test_application/test_application.cpp:850: test_all_http_methods_dispatched [PASSED]
test/test_application/test_application.cpp:851: test_root_path_matches_exactly [PASSED]
test/test_application/test_application.cpp:852: test_root_path_does_not_match_subpath [PASSED]
test/test_application/test_application.cpp:853: test_wildcard_matches_any_suffix [PASSED]
test/test_application/test_application.cpp:854: test_wildcard_does_not_match_unrelated_prefix [PASSED]
test/test_application/test_application.cpp:855: test_exact_route_wins_when_registered_first [PASSED]
test/test_application/test_application.cpp:856: test_slot_not_stuck_in_complete_after_handle [PASSED]
test/test_application/test_application.cpp:857: test_parse_error_slot_auto_reset [PASSED]
test/test_application/test_application.cpp:860: stress_last_route_dispatched_in_full_table [PASSED]
test/test_application/test_application.cpp:861: stress_sequential_requests_no_state_leak [PASSED]
test/test_application/test_application.cpp:862: stress_all_slots_dispatched_simultaneously [PASSED]
test/test_application/test_application.cpp:863: stress_wildcard_matches_many_paths [PASSED]
test/test_application/test_application.cpp:864: stress_handle_with_no_complete_slots_is_nop [PASSED]
test/test_application/test_application.cpp:867: race_slot_complete_between_handle_calls [PASSED]
test/test_application/test_application.cpp:868: race_conn_freed_after_parse_complete [PASSED]
test/test_application/test_application.cpp:869: race_double_handle_no_double_dispatch [PASSED]
test/test_application/test_application.cpp:870: race_error_and_valid_slot_in_same_handle [PASSED]
test/test_application/test_application.cpp:871: race_callback_manually_resets_slot [PASSED]
test/test_application/test_application.cpp:874: test_uri_too_long_auto_resets_slot [PASSED]
test/test_application/test_application.cpp:877: test_transfer_encoding_chunked_is_501 [PASSED]
test/test_application/test_application.cpp:878: test_transfer_encoding_identity_is_501 [PASSED]
test/test_application/test_application.cpp:880: test_redirect_emits_location_and_status [PASSED]
test/test_application/test_application.cpp:881: test_redirect_invalid_code_defaults_to_302 [PASSED]
test/test_application/test_application.cpp:882: test_mime_type_detection [PASSED]
test/test_application/test_application.cpp:884: test_serve_static_file_and_mime [PASSED]
test/test_application/test_application.cpp:885: test_serve_static_index_fallback [PASSED]
test/test_application/test_application.cpp:886: test_serve_static_gzip_when_accepted [PASSED]
test/test_application/test_application.cpp:887: test_serve_static_no_gzip_when_not_accepted [PASSED]
test/test_application/test_application.cpp:888: test_serve_static_traversal_not_leaked [PASSED]
test/test_application/test_application.cpp:889: test_serve_static_missing_is_404 [PASSED]
test/test_application/test_application.cpp:890: test_serve_static_etag_conditional_get [PASSED]
test/test_application/test_application.cpp:892: test_request_log_hook_fires [PASSED]
test/test_application/test_application.cpp:893: test_stats_endpoint_emits_json [PASSED]
------------ native_app:test_application [PASSED] Took 0.60 seconds ------------

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
--------- native_compliance:test_compliance [PASSED] Took 0.62 seconds ---------

=================================== SUMMARY ===================================
Environment          Test                   Status    Duration
-------------------  ---------------------  --------  ------------
native               test_sse               PASSED    00:00:06.067
native               test_session           PASSED    00:00:00.484
native               test_presentation      PASSED    00:00:00.520
native               test_transport         PASSED    00:00:00.490
native               test_websocket         PASSED    00:00:00.533
native               test_http_parser       PASSED    00:00:00.513
native_ssh           test_ssh_crypto        PASSED    00:00:03.811
native_ssh           test_ssh_auth          PASSED    00:00:00.516
native_ssh           test_ssh_server        PASSED    00:00:00.634
native_ssh           test_ssh_transport     PASSED    00:00:00.839
native_ssh           test_ssh_channel       PASSED    00:00:00.484
native_ssh_hardened  test_ssh_hardening     PASSED    00:00:00.785
native_ssh_conn      test_ssh_conn          PASSED    00:00:01.061
native_app           test_regex             PASSED    00:00:00.905
native_app           test_template          PASSED    00:00:00.503
native_app           test_path_params       PASSED    00:00:00.502
native_app           test_digest_vectors    PASSED    00:00:00.473
native_app           test_form_params       PASSED    00:00:00.489
native_app           test_iface             PASSED    00:00:00.493
native_app           test_json              PASSED    00:00:00.488
native_app           test_response_headers  PASSED    00:00:00.499
native_app           test_middleware        PASSED    00:00:00.518
native_app           test_digest_auth       PASSED    00:00:00.512
native_app           test_web_terminal      PASSED    00:00:00.499
native_app           test_multipart         PASSED    00:00:00.508
native_app           test_auth              PASSED    00:00:00.507
native_app           test_file_serving      PASSED    00:00:00.513
native_app           test_dispatch          PASSED    00:00:00.498
native_app           test_chunked           PASSED    00:00:00.504
native_app           test_application       PASSED    00:00:00.595
native_compliance    test_compliance        PASSED    00:00:00.621
================ 592 test cases: 592 succeeded in 00:00:26.365 ================
```

</details>
