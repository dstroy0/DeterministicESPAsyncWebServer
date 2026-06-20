# Test Report — DeterministicESPAsyncWebServer

**Generated:** 2026-06-19 22:07:56  
**Command:** `pio test -e native -e native_app`  
**Result:** ✅ 222 passed — 9.18s  

---

## Summary

| Suite | Environment | Tests | Status | Duration |
|---|---|---|---|---|
| `test_http_parser` | `native` | 80 | ✅ | 00:00:01.776 |
| `test_presentation` | `native` | 63 | ✅ | 00:00:01.775 |
| `test_session` | `native` | 19 | ✅ | 00:00:01.670 |
| `test_transport` | `native` | 25 | ✅ | 00:00:01.777 |
| `test_application` | `native_app` | 35 | ✅ | 00:00:01.466 |

---

## test_http_parser — ✅ 80 passed

*Comprehensive unit tests for the standalone HTTP/1.1 parser.*

| # | Test | Status | Description |
|---|------|--------|-------------|
| 1 | `test_reset_sets_parse_method_state` | ✅ | Reset sets parse method state |
| 2 | `test_reset_preserves_slot_id` | ✅ | Reset preserves slot id |
| 3 | `test_reset_clears_method` | ✅ | Reset clears method |
| 4 | `test_reset_clears_path` | ✅ | Reset clears path |
| 5 | `test_reset_clears_header_count` | ✅ | Reset clears header count |
| 6 | `test_reset_clears_body` | ✅ | Reset clears body |
| 7 | `test_reset_clears_query_count` | ✅ | Reset clears query count |
| 8 | `test_feed_after_complete_does_not_change_state` | ✅ | Feed after complete does not change state |
| 9 | `test_feed_after_error_does_not_change_state` | ✅ | Feed after error does not change state |
| 10 | `test_feed_after_entity_too_large_does_not_change_state` | ✅ | Feed after entity too large does not change state |
| 11 | `test_method_get` | ✅ | Method get |
| 12 | `test_method_post` | ✅ | Method post |
| 13 | `test_method_put` | ✅ | Method put |
| 14 | `test_method_delete` | ✅ | Method delete |
| 15 | `test_method_patch` | ✅ | Method patch |
| 16 | `test_method_head` | ✅ | Method head |
| 17 | `test_method_options` | ✅ | Method options |
| 18 | `test_method_overflow_is_error` | ✅ | More than 7 chars (sizeof method - 1) before a space → PARSE_ERROR |
| 19 | `test_path_root` | ✅ | Path root |
| 20 | `test_path_segments` | ✅ | Path segments |
| 21 | `test_path_without_query` | ✅ | Path without query |
| 22 | `test_path_overflow_is_414` | ✅ | Build a path longer than MAX_PATH_LEN |
| 23 | `test_single_query_param` | ✅ | Single query param |
| 24 | `test_two_query_params` | ✅ | Two query params |
| 25 | `test_query_key_not_found_returns_null` | ✅ | Query key not found returns null |
| 26 | `test_query_empty_value` | ✅ | Query empty value |
| 27 | `test_single_header_stored` | ✅ | Single header stored |
| 28 | `test_header_lookup_case_insensitive` | ✅ | Header lookup case insensitive |
| 29 | `test_header_leading_space_stripped` | ✅ | Header leading space stripped |
| 30 | `test_content_length_header_parsed` | ✅ | Content length header parsed |
| 31 | `test_content_length_in_headers_array` | ✅ | Content length in headers array |
| 32 | `test_multiple_headers_stored` | ✅ | Multiple headers stored |
| 33 | `test_missing_header_returns_null` | ✅ | Missing header returns null |
| 34 | `test_get_no_body_completes` | ✅ | Get no body completes |
| 35 | `test_post_with_body` | ✅ | Post with body |
| 36 | `test_put_with_body` | ✅ | Put with body |
| 37 | `test_body_starting_with_newline` | ✅ | Body starting with newline |
| 38 | `test_post_content_length_zero` | ✅ | Post content length zero |
| 39 | `test_body_exactly_at_buffer_limit` | ✅ | Body of exactly BODY_BUF_SIZE bytes — should succeed |
| 40 | `test_body_null_terminated_after_complete` | ✅ | Body null terminated after complete |
| 41 | `test_body_one_over_limit_is_413` | ✅ | Content-Length == BODY_BUF_SIZE + 1 → PARSE_ENTITY_TOO_LARGE |
| 42 | `test_body_far_over_limit_is_413` | ✅ | Body far over limit is 413 |
| 43 | `test_413_no_body_bytes_fed` | ✅ | Even though we detected 413, no body bytes should have been stored |
| 44 | `test_413_header_still_stored` | ✅ | Headers before the blank line must be accessible even when 413 |
| 45 | `test_body_exactly_at_limit_is_not_413` | ✅ | BODY_BUF_SIZE is the max that fits — should NOT trigger 413 |
| 46 | `test_path_overflow_stops_feeding` | ✅ | Bytes fed after URI_TOO_LONG are ignored — state must not change |
| 47 | `test_414_path_filled_to_capacity` | ✅ | Buffer fills to MAX_PATH_LEN-1 chars before overflow is detected |
| 48 | `test_method_nul_byte_is_error` | ✅ | Method nul byte is error |
| 49 | `test_method_control_char_is_error` | ✅ | Method control char is error |
| 50 | `test_method_del_byte_is_error` | ✅ | Method del byte is error |
| 51 | `test_method_non_tchar_symbol_is_error` | ✅ | '(' is VCHAR but not tchar |
| 52 | `test_method_tchar_symbols_accepted` | ✅ | '-' is a valid tchar; a custom method like "X-CMD" is valid per RFC 7230 |
| 53 | `test_path_nul_byte_is_error` | ✅ | Path nul byte is error |
| 54 | `test_path_control_char_is_error` | ✅ | Path control char is error |
| 55 | `test_path_del_byte_is_error` | ✅ | Path del byte is error |
| 56 | `test_query_nul_byte_is_error` | ✅ | Query nul byte is error |
| 57 | `test_query_control_char_is_error` | ✅ | Query control char is error |
| 58 | `test_header_key_space_is_error` | ✅ | Space in a field-name is not a valid tchar |
| 59 | `test_header_key_nul_byte_is_error` | ✅ | Header key nul byte is error |
| 60 | `test_header_key_control_char_is_error` | ✅ | Header key control char is error |
| 61 | `test_header_key_mid_cr_is_error` | ✅ | CR in the middle of a key name must be PARSE_ERROR, not blank-line detection |
| 62 | `test_header_key_colon_at_start_skips_header` | ✅ | Empty key name (colon immediately after CRLF): transition to val with empty key |
| 63 | `test_header_val_nul_byte_is_error` | ✅ | Header val nul byte is error |
| 64 | `test_header_val_control_char_is_error` | ✅ | Header val control char is error |
| 65 | `test_header_val_del_byte_is_error` | ✅ | Header val del byte is error |
| 66 | `test_header_val_htab_mid_value_allowed` | ✅ | HTAB is valid mid-value (RFC 7230 §3.2) |
| 67 | `test_header_val_leading_htab_stripped` | ✅ | Leading HTAB (OWS) is stripped just like leading SP |
| 68 | `test_header_val_obs_text_allowed` | ✅ | obs-text bytes (%x80-FF) are allowed for legacy compatibility (RFC 7230 §3.2.6) |
| 69 | `test_version_http11_recognized` | ✅ | Version http11 recognized |
| 70 | `test_version_http10_recognized` | ✅ | Version http10 recognized |
| 71 | `test_version_unknown_is_http_unknown` | ✅ | Version unknown is http unknown |
| 72 | `test_version_reset_to_unknown` | ✅ | Version reset to unknown |
| 73 | `test_bad_expect_lf_is_error` | ✅ | CRLF in version line replaced by CR + X (no LF) |
| 74 | `test_blank_line_non_lf_is_error` | ✅ | Header block ends with CR + non-LF in the blank line |
| 75 | `test_slots_are_independent` | ✅ | Slots are independent |
| 76 | `test_incremental_byte_by_byte` | ✅ | Incremental byte by byte |
| 77 | `test_incremental_two_chunks` | ✅ | Incremental two chunks |
| 78 | `stress_many_requests_same_slot` | ✅ | Stress — Many requests same slot |
| 79 | `stress_max_headers` | ✅ | Build a request with MAX_HEADERS header lines |
| 80 | `stress_max_query_params` | ✅ | Build a query string with MAX_QUERY_PARAMS parameters |

---

## test_presentation — ✅ 63 passed

*Unit, stress, and race-condition tests for Layer 6 (Presentation).*

| # | Test | Status | Description |
|---|------|--------|-------------|
| 1 | `test_fn_reset_sets_parse_state_to_method` | ✅ | Fn reset sets parse state to method |
| 2 | `test_fn_reset_sets_slot_id` | ✅ | Fn reset sets slot id |
| 3 | `test_fn_reset_clears_method` | ✅ | Fn reset clears method |
| 4 | `test_fn_reset_clears_path_and_idx` | ✅ | Fn reset clears path and idx |
| 5 | `test_fn_reset_clears_query_raw_and_params` | ✅ | Fn reset clears query raw and params |
| 6 | `test_fn_reset_clears_all_header_slots` | ✅ | Fn reset clears all header slots |
| 7 | `test_fn_reset_clears_body_fields` | ✅ | Fn reset clears body fields |
| 8 | `test_fn_reset_out_of_range_is_nop` | ✅ | Fn reset out of range is nop |
| 9 | `test_fn_reset_is_idempotent` | ✅ | Fn reset is idempotent |
| 10 | `test_fn_get_header_null_when_no_headers` | ✅ | setUp already reset all slots — header_count is 0 |
| 11 | `test_fn_get_header_finds_single_header` | ✅ | Fn get header finds single header |
| 12 | `test_fn_get_header_finds_first_of_many` | ✅ | Fn get header finds first of many |
| 13 | `test_fn_get_header_finds_middle_of_many` | ✅ | Fn get header finds middle of many |
| 14 | `test_fn_get_header_finds_last_of_many` | ✅ | Fn get header finds last of many |
| 15 | `test_fn_get_header_case_insensitive_lowercase` | ✅ | Fn get header case insensitive lowercase |
| 16 | `test_fn_get_header_case_insensitive_uppercase` | ✅ | Fn get header case insensitive uppercase |
| 17 | `test_fn_get_header_returns_null_for_absent_key` | ✅ | Fn get header returns null for absent key |
| 18 | `test_fn_get_header_does_not_bleed_across_slots` | ✅ | Fn get header does not bleed across slots |
| 19 | `test_fn_get_query_null_when_no_params` | ✅ | Fn get query null when no params |
| 20 | `test_fn_get_query_finds_single_param` | ✅ | Fn get query finds single param |
| 21 | `test_fn_get_query_finds_first_param` | ✅ | Fn get query finds first param |
| 22 | `test_fn_get_query_finds_middle_param` | ✅ | Fn get query finds middle param |
| 23 | `test_fn_get_query_finds_last_param` | ✅ | Fn get query finds last param |
| 24 | `test_fn_get_query_returns_null_for_absent_key` | ✅ | Fn get query returns null for absent key |
| 25 | `test_fn_get_query_empty_value` | ✅ | Fn get query empty value |
| 26 | `test_fn_get_query_does_not_bleed_across_slots` | ✅ | Fn get query does not bleed across slots |
| 27 | `test_get_parses_complete` | ✅ | Get parses complete |
| 28 | `test_post_body_stored` | ✅ | Post body stored |
| 29 | `test_put_parses_complete` | ✅ | Put parses complete |
| 30 | `test_delete_parses_complete` | ✅ | Delete parses complete |
| 31 | `test_patch_parses_complete` | ✅ | Patch parses complete |
| 32 | `test_head_parses_complete` | ✅ | Head parses complete |
| 33 | `test_query_single_param` | ✅ | Query single param |
| 34 | `test_query_multiple_params` | ✅ | Query multiple params |
| 35 | `test_body_null_terminated` | ✅ | Body null terminated |
| 36 | `test_body_over_buf_size_is_413` | ✅ | Content-Length > BODY_BUF_SIZE → PARSE_ENTITY_TOO_LARGE before any body is read. |
| 37 | `test_overflow_method_sets_error` | ✅ | Overflow method sets error |
| 38 | `test_overflow_path_sets_414` | ✅ | Overflow path sets 414 |
| 39 | `test_bad_lf_after_cr_sets_error` | ✅ | '\x00' would terminate the C-string in push(), so use a visible non-LF byte. |
| 40 | `test_headers_beyond_max_are_dropped` | ✅ | Headers beyond max are dropped |
| 41 | `test_query_params_beyond_max_are_dropped` | ✅ | Query params beyond max are dropped |
| 42 | `test_incremental_two_pushes_completes` | ✅ | Incremental two pushes completes |
| 43 | `test_body_starting_with_newline_stored` | ✅ | Body starting with newline stored |
| 44 | `test_put_body_stored` | ✅ | Put body stored |
| 45 | `test_content_length_header_stored_in_headers_array` | ✅ | Content length header stored in headers array |
| 46 | `stress_parse_reset_100_cycles` | ✅ | Stress — Parse reset 100 cycles |
| 47 | `stress_all_slots_parse_simultaneously` | ✅ | Stress — All slots parse simultaneously |
| 48 | `stress_method_at_max_7_chars_no_error` | ✅ | Stress — Method at max 7 chars no error |
| 49 | `stress_path_at_exact_limit_no_error` | ✅ | Stress — Path at exact limit no error |
| 50 | `stress_body_exactly_buf_size_all_stored` | ✅ | Stress — Body exactly buf size all stored |
| 51 | `stress_exactly_max_headers_all_stored` | ✅ | Stress — Exactly max headers all stored |
| 52 | `stress_exactly_max_query_params_all_stored` | ✅ | Stress — Exactly max query params all stored |
| 53 | `stress_incremental_byte_by_byte_no_error` | ✅ | Stress — Incremental byte by byte no error |
| 54 | `stress_sequential_requests_no_state_leak` | ✅ | Stress — Sequential requests no state leak |
| 55 | `race_interleaved_producer_consumer_ring_buffer` | ✅ | Producer writes first 100 bytes |
| 56 | `race_ring_buffer_full_prevents_write` | ✅ | Race — Ring buffer full prevents write |
| 57 | `race_aba_slot_reuse_fresh_timestamp` | ✅ | Race — Aba slot reuse fresh timestamp |
| 58 | `race_double_free_is_nop` | ✅ | Race — Double free is nop |
| 59 | `race_concurrent_slot_parse_isolation` | ✅ | Slot 0: push a full request |
| 60 | `race_reset_during_parse_header_val` | ✅ | Race — Reset during parse header val |
| 61 | `race_reset_during_parse_query` | ✅ | Race — Reset during parse query |
| 62 | `race_reset_during_parse_body` | ✅ | Race — Reset during parse body |
| 63 | `race_parse_after_complete_is_nop` | ✅ | Race — Parse after complete is nop |

---

## test_session — ✅ 19 passed

*Unit, stress, and race-condition tests for Layer 5 (Session).*

| # | Test | Status | Description |
|---|------|--------|-------------|
| 1 | `test_empty_queue_does_not_crash` | ✅ | Empty queue does not crash |
| 2 | `test_pool_initialises_to_parse_method` | ✅ | Pool initialises to parse method |
| 3 | `test_reset_clears_mid_parse_state` | ✅ | Reset clears mid parse state |
| 4 | `test_tick_fires_check_timeouts_stale_slot_freed` | ✅ | Tick fires check timeouts stale slot freed |
| 5 | `test_tick_does_not_free_fresh_connection` | ✅ | Tick does not free fresh connection |
| 6 | `test_fn_tick_timeout_before_event_drain_ordering` | ✅ | Fn tick timeout before event drain ordering |
| 7 | `test_fn_tick_only_active_slots_expire` | ✅ | Fn tick only active slots expire |
| 8 | `stress_1000_idle_ticks_stable` | ✅ | Stress — 1000 idle ticks stable |
| 9 | `stress_timeout_all_slots_10_cycles` | ✅ | Stress — Timeout all slots 10 cycles |
| 10 | `stress_mixed_fresh_stale_slots_many_ticks` | ✅ | Stress — Mixed fresh stale slots many ticks |
| 11 | `test_evt_connect_calls_http_reset` | ✅ | Evt connect calls http reset |
| 12 | `test_evt_disconnect_calls_http_reset` | ✅ | Evt disconnect calls http reset |
| 13 | `test_evt_error_calls_http_reset` | ✅ | Evt error calls http reset |
| 14 | `test_evt_data_calls_http_parse` | ✅ | Evt data calls http parse |
| 15 | `test_multiple_events_drained_in_one_tick` | ✅ | Slot 0: dirty state → EVT_CONNECT → reset |
| 16 | `race_external_free_between_ticks` | ✅ | First tick: slot expires inside check_timeouts |
| 17 | `race_activity_update_saves_slot_from_timeout` | ✅ | Race — Activity update saves slot from timeout |
| 18 | `race_all_expire_then_idle_tick` | ✅ | Race — All expire then idle tick |
| 19 | `race_millis_wraparound_no_spurious_timeout` | ✅ | last_activity close to UINT32_MAX, now just past wrap |

---

## test_transport — ✅ 25 passed

*Unit and stress tests for Layer 4 (Transport) — constants, pool invariants,*

| # | Test | Status | Description |
|---|------|--------|-------------|
| 1 | `test_pool_capacity_is_four` | ✅ | ---- Pool defaults after init() ------------------------------------ |
| 2 | `test_rx_buffer_size_is_one_kb` | ✅ | ---- Pool defaults after init() ------------------------------------ |
| 3 | `test_timeout_constant_is_5000ms` | ✅ | ---- Pool defaults after init() ------------------------------------ |
| 4 | `test_all_slots_free_after_init` | ✅ | All slots free after init |
| 5 | `test_all_pcbs_null_after_init` | ✅ | All pcbs null after init |
| 6 | `test_all_ring_buffers_empty_after_init` | ✅ | All ring buffers empty after init |
| 7 | `test_slot_ids_match_indices` | ✅ | Slot ids match indices |
| 8 | `test_ring_empty_when_head_equals_tail` | ✅ | Ring empty when head equals tail |
| 9 | `test_ring_wrap_at_boundary` | ✅ | Ring wrap at boundary |
| 10 | `test_ring_full_sentinel_one_slot_reserved` | ✅ | Ring full sentinel one slot reserved |
| 11 | `test_ring_can_store_size_minus_one_bytes` | ✅ | Ring can store size minus one bytes |
| 12 | `test_event_types_are_distinct` | ✅ | Event types are distinct |
| 13 | `test_timeout_does_not_fire_on_free_slot` | ✅ | Timeout does not fire on free slot |
| 14 | `test_timeout_does_not_fire_before_deadline` | ✅ | Timeout does not fire before deadline |
| 15 | `test_timeout_fires_at_deadline` | ✅ | Timeout fires at deadline |
| 16 | `test_timeout_fires_only_on_stale_slots` | ✅ | Timeout fires only on stale slots |
| 17 | `test_init_succeeds_on_native` | ✅ | Init succeeds on native |
| 18 | `test_all_last_activity_ms_zero_after_init` | ✅ | All last activity ms zero after init |
| 19 | `test_queue_not_null_after_init` | ✅ | Queue not null after init |
| 20 | `stress_ring_buffer_fill_drain_integrity` | ✅ | Write known pattern |
| 21 | `stress_ring_buffer_multi_cycle_no_corruption` | ✅ | Stress — Ring buffer multi cycle no corruption |
| 22 | `stress_all_slots_timeout_simultaneously` | ✅ | Stress — All slots timeout simultaneously |
| 23 | `stress_timeout_arm_recover_cycle` | ✅ | Stress — Timeout arm recover cycle |
| 24 | `stress_check_timeouts_high_call_rate` | ✅ | Stress — Check timeouts high call rate |
| 25 | `stress_ring_buffer_byte_by_byte_fill_and_drain` | ✅ | Stress — Ring buffer byte by byte fill and drain |

---

## test_application — ✅ 35 passed

*Unit, stress, and race-condition tests for Layer 7 (Application).*

| # | Test | Status | Description |
|---|------|--------|-------------|
| 1 | `test_handler_reads_body` | ✅ | Handler reads body |
| 2 | `test_handler_reads_query_param` | ✅ | Handler reads query param |
| 3 | `test_handler_reads_header` | ✅ | Handler reads header |
| 4 | `test_wildcard_before_exact_wildcard_wins` | ✅ | Wildcard before exact wildcard wins |
| 5 | `test_fn_on_registers_and_dispatches` | ✅ | Fn on registers and dispatches |
| 6 | `test_fn_on_path_copied_null_terminated` | ✅ | A path of exactly MAX_PATH_LEN-1 chars must not overflow the route buffer. |
| 7 | `test_fn_on_table_full_extra_routes_dropped` | ✅ | Fill the table; on() beyond MAX_ROUTES must silently drop |
| 8 | `test_fn_on_same_path_different_methods_are_distinct` | ✅ | Fn on same path different methods are distinct |
| 9 | `test_fn_on_not_found_called_when_no_match` | ✅ | Fn on not found called when no match |
| 10 | `test_fn_on_not_found_not_called_when_match_exists` | ✅ | Fn on not found not called when match exists |
| 11 | `test_fn_set_cors_options_preflight_clears_slot` | ✅ | Fn set cors options preflight clears slot |
| 12 | `test_fn_set_cors_empty_string_disables` | ✅ | Fn set cors empty string disables |
| 13 | `test_wrong_method_does_not_match` | ✅ | Wrong method does not match |
| 14 | `test_wrong_path_does_not_match` | ✅ | Wrong path does not match |
| 15 | `test_all_http_methods_dispatched` | ✅ | All http methods dispatched |
| 16 | `test_root_path_matches_exactly` | ✅ | Root path matches exactly |
| 17 | `test_root_path_does_not_match_subpath` | ✅ | Root path does not match subpath |
| 18 | `test_wildcard_matches_any_suffix` | ✅ | Wildcard matches any suffix |
| 19 | `test_wildcard_does_not_match_unrelated_prefix` | ✅ | Wildcard does not match unrelated prefix |
| 20 | `test_exact_route_wins_when_registered_first` | ✅ | Exact route wins when registered first |
| 21 | `test_slot_not_stuck_in_complete_after_handle` | ✅ | Slot not stuck in complete after handle |
| 22 | `test_parse_error_slot_auto_reset` | ✅ | Parse error slot auto reset |
| 23 | `stress_last_route_dispatched_in_full_table` | ✅ | Stress — Last route dispatched in full table |
| 24 | `stress_sequential_requests_no_state_leak` | ✅ | Stress — Sequential requests no state leak |
| 25 | `stress_all_slots_dispatched_simultaneously` | ✅ | Stress — All slots dispatched simultaneously |
| 26 | `stress_wildcard_matches_many_paths` | ✅ | Stress — Wildcard matches many paths |
| 27 | `stress_handle_with_no_complete_slots_is_nop` | ✅ | All slots in PARSE_METHOD (setUp resets them) — nothing to dispatch |
| 28 | `race_slot_complete_between_handle_calls` | ✅ | Race — Slot complete between handle calls |
| 29 | `race_conn_freed_after_parse_complete` | ✅ | Race — Conn freed after parse complete |
| 30 | `race_double_handle_no_double_dispatch` | ✅ | Race — Double handle no double dispatch |
| 31 | `race_error_and_valid_slot_in_same_handle` | ✅ | Slot 0: inject a parse error |
| 32 | `race_callback_manually_resets_slot` | ✅ | Race — Callback manually resets slot |
| 33 | `test_uri_too_long_auto_resets_slot` | ✅ | Overflow the path buffer — handle() should send 414 and free the slot |
| 34 | `test_transfer_encoding_chunked_is_501` | ✅ | A request advertising Transfer-Encoding must be rejected with 501 |
| 35 | `test_transfer_encoding_identity_is_501` | ✅ | Even "identity" is rejected — we advertise no TE support at all |

---

## Raw Output

<details>
<summary>Expand full pio output</summary>

```
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 5 tests

Processing test_http_parser in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test\test_http_parser\test_http_parser.cpp:829: test_reset_sets_parse_method_state	[PASSED]
test\test_http_parser\test_http_parser.cpp:830: test_reset_preserves_slot_id	[PASSED]
test\test_http_parser\test_http_parser.cpp:831: test_reset_clears_method	[PASSED]
test\test_http_parser\test_http_parser.cpp:832: test_reset_clears_path	[PASSED]
test\test_http_parser\test_http_parser.cpp:833: test_reset_clears_header_count	[PASSED]
test\test_http_parser\test_http_parser.cpp:834: test_reset_clears_body	[PASSED]
test\test_http_parser\test_http_parser.cpp:835: test_reset_clears_query_count	[PASSED]
test\test_http_parser\test_http_parser.cpp:838: test_feed_after_complete_does_not_change_state	[PASSED]
test\test_http_parser\test_http_parser.cpp:839: test_feed_after_error_does_not_change_state	[PASSED]
test\test_http_parser\test_http_parser.cpp:840: test_feed_after_entity_too_large_does_not_change_state	[PASSED]
test\test_http_parser\test_http_parser.cpp:843: test_method_get	[PASSED]
test\test_http_parser\test_http_parser.cpp:844: test_method_post	[PASSED]
test\test_http_parser\test_http_parser.cpp:845: test_method_put	[PASSED]
test\test_http_parser\test_http_parser.cpp:846: test_method_delete	[PASSED]
test\test_http_parser\test_http_parser.cpp:847: test_method_patch	[PASSED]
test\test_http_parser\test_http_parser.cpp:848: test_method_head	[PASSED]
test\test_http_parser\test_http_parser.cpp:849: test_method_options	[PASSED]
test\test_http_parser\test_http_parser.cpp:850: test_method_overflow_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:853: test_path_root	[PASSED]
test\test_http_parser\test_http_parser.cpp:854: test_path_segments	[PASSED]
test\test_http_parser\test_http_parser.cpp:855: test_path_without_query	[PASSED]
test\test_http_parser\test_http_parser.cpp:856: test_path_overflow_is_414	[PASSED]
test\test_http_parser\test_http_parser.cpp:859: test_single_query_param	[PASSED]
test\test_http_parser\test_http_parser.cpp:860: test_two_query_params	[PASSED]
test\test_http_parser\test_http_parser.cpp:861: test_query_key_not_found_returns_null	[PASSED]
test\test_http_parser\test_http_parser.cpp:862: test_query_empty_value	[PASSED]
test\test_http_parser\test_http_parser.cpp:865: test_single_header_stored	[PASSED]
test\test_http_parser\test_http_parser.cpp:866: test_header_lookup_case_insensitive	[PASSED]
test\test_http_parser\test_http_parser.cpp:867: test_header_leading_space_stripped	[PASSED]
test\test_http_parser\test_http_parser.cpp:868: test_content_length_header_parsed	[PASSED]
test\test_http_parser\test_http_parser.cpp:869: test_content_length_in_headers_array	[PASSED]
test\test_http_parser\test_http_parser.cpp:870: test_multiple_headers_stored	[PASSED]
test\test_http_parser\test_http_parser.cpp:871: test_missing_header_returns_null	[PASSED]
test\test_http_parser\test_http_parser.cpp:874: test_get_no_body_completes	[PASSED]
test\test_http_parser\test_http_parser.cpp:875: test_post_with_body	[PASSED]
test\test_http_parser\test_http_parser.cpp:876: test_put_with_body	[PASSED]
test\test_http_parser\test_http_parser.cpp:877: test_body_starting_with_newline	[PASSED]
test\test_http_parser\test_http_parser.cpp:878: test_post_content_length_zero	[PASSED]
test\test_http_parser\test_http_parser.cpp:879: test_body_exactly_at_buffer_limit	[PASSED]
test\test_http_parser\test_http_parser.cpp:880: test_body_null_terminated_after_complete	[PASSED]
test\test_http_parser\test_http_parser.cpp:883: test_body_one_over_limit_is_413	[PASSED]
test\test_http_parser\test_http_parser.cpp:884: test_body_far_over_limit_is_413	[PASSED]
test\test_http_parser\test_http_parser.cpp:885: test_413_no_body_bytes_fed	[PASSED]
test\test_http_parser\test_http_parser.cpp:886: test_413_header_still_stored	[PASSED]
test\test_http_parser\test_http_parser.cpp:887: test_body_exactly_at_limit_is_not_413	[PASSED]
test\test_http_parser\test_http_parser.cpp:890: test_path_overflow_stops_feeding	[PASSED]
test\test_http_parser\test_http_parser.cpp:891: test_414_path_filled_to_capacity	[PASSED]
test\test_http_parser\test_http_parser.cpp:894: test_method_nul_byte_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:895: test_method_control_char_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:896: test_method_del_byte_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:897: test_method_non_tchar_symbol_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:898: test_method_tchar_symbols_accepted	[PASSED]
test\test_http_parser\test_http_parser.cpp:901: test_path_nul_byte_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:902: test_path_control_char_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:903: test_path_del_byte_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:904: test_query_nul_byte_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:905: test_query_control_char_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:908: test_header_key_space_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:909: test_header_key_nul_byte_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:910: test_header_key_control_char_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:911: test_header_key_mid_cr_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:912: test_header_key_colon_at_start_skips_header	[PASSED]
test\test_http_parser\test_http_parser.cpp:915: test_header_val_nul_byte_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:916: test_header_val_control_char_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:917: test_header_val_del_byte_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:918: test_header_val_htab_mid_value_allowed	[PASSED]
test\test_http_parser\test_http_parser.cpp:919: test_header_val_leading_htab_stripped	[PASSED]
test\test_http_parser\test_http_parser.cpp:920: test_header_val_obs_text_allowed	[PASSED]
test\test_http_parser\test_http_parser.cpp:923: test_version_http11_recognized	[PASSED]
test\test_http_parser\test_http_parser.cpp:924: test_version_http10_recognized	[PASSED]
test\test_http_parser\test_http_parser.cpp:925: test_version_unknown_is_http_unknown	[PASSED]
test\test_http_parser\test_http_parser.cpp:926: test_version_reset_to_unknown	[PASSED]
test\test_http_parser\test_http_parser.cpp:929: test_bad_expect_lf_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:930: test_blank_line_non_lf_is_error	[PASSED]
test\test_http_parser\test_http_parser.cpp:933: test_slots_are_independent	[PASSED]
test\test_http_parser\test_http_parser.cpp:936: test_incremental_byte_by_byte	[PASSED]
test\test_http_parser\test_http_parser.cpp:937: test_incremental_two_chunks	[PASSED]
test\test_http_parser\test_http_parser.cpp:940: stress_many_requests_same_slot	[PASSED]
test\test_http_parser\test_http_parser.cpp:941: stress_max_headers	[PASSED]
test\test_http_parser\test_http_parser.cpp:942: stress_max_query_params	[PASSED]
-------------- native:test_http_parser [PASSED] Took 1.78 seconds --------------

Processing test_presentation in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test\test_presentation\test_presentation.cpp:829: test_fn_reset_sets_parse_state_to_method	[PASSED]
test\test_presentation\test_presentation.cpp:830: test_fn_reset_sets_slot_id	[PASSED]
test\test_presentation\test_presentation.cpp:831: test_fn_reset_clears_method	[PASSED]
test\test_presentation\test_presentation.cpp:832: test_fn_reset_clears_path_and_idx	[PASSED]
test\test_presentation\test_presentation.cpp:833: test_fn_reset_clears_query_raw_and_params	[PASSED]
test\test_presentation\test_presentation.cpp:834: test_fn_reset_clears_all_header_slots	[PASSED]
test\test_presentation\test_presentation.cpp:835: test_fn_reset_clears_body_fields	[PASSED]
test\test_presentation\test_presentation.cpp:836: test_fn_reset_out_of_range_is_nop	[PASSED]
test\test_presentation\test_presentation.cpp:837: test_fn_reset_is_idempotent	[PASSED]
test\test_presentation\test_presentation.cpp:840: test_fn_get_header_null_when_no_headers	[PASSED]
test\test_presentation\test_presentation.cpp:841: test_fn_get_header_finds_single_header	[PASSED]
test\test_presentation\test_presentation.cpp:842: test_fn_get_header_finds_first_of_many	[PASSED]
test\test_presentation\test_presentation.cpp:843: test_fn_get_header_finds_middle_of_many	[PASSED]
test\test_presentation\test_presentation.cpp:844: test_fn_get_header_finds_last_of_many	[PASSED]
test\test_presentation\test_presentation.cpp:845: test_fn_get_header_case_insensitive_lowercase	[PASSED]
test\test_presentation\test_presentation.cpp:846: test_fn_get_header_case_insensitive_uppercase	[PASSED]
test\test_presentation\test_presentation.cpp:847: test_fn_get_header_returns_null_for_absent_key	[PASSED]
test\test_presentation\test_presentation.cpp:848: test_fn_get_header_does_not_bleed_across_slots	[PASSED]
test\test_presentation\test_presentation.cpp:851: test_fn_get_query_null_when_no_params	[PASSED]
test\test_presentation\test_presentation.cpp:852: test_fn_get_query_finds_single_param	[PASSED]
test\test_presentation\test_presentation.cpp:853: test_fn_get_query_finds_first_param	[PASSED]
test\test_presentation\test_presentation.cpp:854: test_fn_get_query_finds_middle_param	[PASSED]
test\test_presentation\test_presentation.cpp:855: test_fn_get_query_finds_last_param	[PASSED]
test\test_presentation\test_presentation.cpp:856: test_fn_get_query_returns_null_for_absent_key	[PASSED]
test\test_presentation\test_presentation.cpp:857: test_fn_get_query_empty_value	[PASSED]
test\test_presentation\test_presentation.cpp:858: test_fn_get_query_does_not_bleed_across_slots	[PASSED]
test\test_presentation\test_presentation.cpp:861: test_get_parses_complete	[PASSED]
test\test_presentation\test_presentation.cpp:862: test_post_body_stored	[PASSED]
test\test_presentation\test_presentation.cpp:863: test_put_parses_complete	[PASSED]
test\test_presentation\test_presentation.cpp:864: test_delete_parses_complete	[PASSED]
test\test_presentation\test_presentation.cpp:865: test_patch_parses_complete	[PASSED]
test\test_presentation\test_presentation.cpp:866: test_head_parses_complete	[PASSED]
test\test_presentation\test_presentation.cpp:867: test_query_single_param	[PASSED]
test\test_presentation\test_presentation.cpp:868: test_query_multiple_params	[PASSED]
test\test_presentation\test_presentation.cpp:869: test_body_null_terminated	[PASSED]
test\test_presentation\test_presentation.cpp:870: test_body_over_buf_size_is_413	[PASSED]
test\test_presentation\test_presentation.cpp:871: test_overflow_method_sets_error	[PASSED]
test\test_presentation\test_presentation.cpp:872: test_overflow_path_sets_414	[PASSED]
test\test_presentation\test_presentation.cpp:873: test_bad_lf_after_cr_sets_error	[PASSED]
test\test_presentation\test_presentation.cpp:874: test_headers_beyond_max_are_dropped	[PASSED]
test\test_presentation\test_presentation.cpp:875: test_query_params_beyond_max_are_dropped	[PASSED]
test\test_presentation\test_presentation.cpp:876: test_incremental_two_pushes_completes	[PASSED]
test\test_presentation\test_presentation.cpp:877: test_body_starting_with_newline_stored	[PASSED]
test\test_presentation\test_presentation.cpp:878: test_put_body_stored	[PASSED]
test\test_presentation\test_presentation.cpp:879: test_content_length_header_stored_in_headers_array	[PASSED]
test\test_presentation\test_presentation.cpp:882: stress_parse_reset_100_cycles	[PASSED]
test\test_presentation\test_presentation.cpp:883: stress_all_slots_parse_simultaneously	[PASSED]
test\test_presentation\test_presentation.cpp:884: stress_method_at_max_7_chars_no_error	[PASSED]
test\test_presentation\test_presentation.cpp:885: stress_path_at_exact_limit_no_error	[PASSED]
test\test_presentation\test_presentation.cpp:886: stress_body_exactly_buf_size_all_stored	[PASSED]
test\test_presentation\test_presentation.cpp:887: stress_exactly_max_headers_all_stored	[PASSED]
test\test_presentation\test_presentation.cpp:888: stress_exactly_max_query_params_all_stored	[PASSED]
test\test_presentation\test_presentation.cpp:889: stress_incremental_byte_by_byte_no_error	[PASSED]
test\test_presentation\test_presentation.cpp:890: stress_sequential_requests_no_state_leak	[PASSED]
test\test_presentation\test_presentation.cpp:893: race_interleaved_producer_consumer_ring_buffer	[PASSED]
test\test_presentation\test_presentation.cpp:894: race_ring_buffer_full_prevents_write	[PASSED]
test\test_presentation\test_presentation.cpp:895: race_aba_slot_reuse_fresh_timestamp	[PASSED]
test\test_presentation\test_presentation.cpp:896: race_double_free_is_nop	[PASSED]
test\test_presentation\test_presentation.cpp:897: race_concurrent_slot_parse_isolation	[PASSED]
test\test_presentation\test_presentation.cpp:898: race_reset_during_parse_header_val	[PASSED]
test\test_presentation\test_presentation.cpp:899: race_reset_during_parse_query	[PASSED]
test\test_presentation\test_presentation.cpp:900: race_reset_during_parse_body	[PASSED]
test\test_presentation\test_presentation.cpp:901: race_parse_after_complete_is_nop	[PASSED]
------------- native:test_presentation [PASSED] Took 1.77 seconds -------------

Processing test_session in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test\test_session\test_session.cpp:337: test_empty_queue_does_not_crash	[PASSED]
test\test_session\test_session.cpp:338: test_pool_initialises_to_parse_method	[PASSED]
test\test_session\test_session.cpp:339: test_reset_clears_mid_parse_state	[PASSED]
test\test_session\test_session.cpp:340: test_tick_fires_check_timeouts_stale_slot_freed	[PASSED]
test\test_session\test_session.cpp:341: test_tick_does_not_free_fresh_connection	[PASSED]
test\test_session\test_session.cpp:344: test_fn_tick_timeout_before_event_drain_ordering	[PASSED]
test\test_session\test_session.cpp:345: test_fn_tick_only_active_slots_expire	[PASSED]
test\test_session\test_session.cpp:348: stress_1000_idle_ticks_stable	[PASSED]
test\test_session\test_session.cpp:349: stress_timeout_all_slots_10_cycles	[PASSED]
test\test_session\test_session.cpp:350: stress_mixed_fresh_stale_slots_many_ticks	[PASSED]
test\test_session\test_session.cpp:353: test_evt_connect_calls_http_reset	[PASSED]
test\test_session\test_session.cpp:354: test_evt_disconnect_calls_http_reset	[PASSED]
test\test_session\test_session.cpp:355: test_evt_error_calls_http_reset	[PASSED]
test\test_session\test_session.cpp:356: test_evt_data_calls_http_parse	[PASSED]
test\test_session\test_session.cpp:357: test_multiple_events_drained_in_one_tick	[PASSED]
test\test_session\test_session.cpp:360: race_external_free_between_ticks	[PASSED]
test\test_session\test_session.cpp:361: race_activity_update_saves_slot_from_timeout	[PASSED]
test\test_session\test_session.cpp:362: race_all_expire_then_idle_tick	[PASSED]
test\test_session\test_session.cpp:363: race_millis_wraparound_no_spurious_timeout	[PASSED]
---------------- native:test_session [PASSED] Took 1.67 seconds ----------------

Processing test_transport in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test\test_transport\test_transport.cpp:343: test_pool_capacity_is_four	[PASSED]
test\test_transport\test_transport.cpp:344: test_rx_buffer_size_is_one_kb	[PASSED]
test\test_transport\test_transport.cpp:345: test_timeout_constant_is_5000ms	[PASSED]
test\test_transport\test_transport.cpp:346: test_all_slots_free_after_init	[PASSED]
test\test_transport\test_transport.cpp:347: test_all_pcbs_null_after_init	[PASSED]
test\test_transport\test_transport.cpp:348: test_all_ring_buffers_empty_after_init	[PASSED]
test\test_transport\test_transport.cpp:349: test_slot_ids_match_indices	[PASSED]
test\test_transport\test_transport.cpp:350: test_ring_empty_when_head_equals_tail	[PASSED]
test\test_transport\test_transport.cpp:351: test_ring_wrap_at_boundary	[PASSED]
test\test_transport\test_transport.cpp:352: test_ring_full_sentinel_one_slot_reserved	[PASSED]
test\test_transport\test_transport.cpp:353: test_ring_can_store_size_minus_one_bytes	[PASSED]
test\test_transport\test_transport.cpp:354: test_event_types_are_distinct	[PASSED]
test\test_transport\test_transport.cpp:355: test_timeout_does_not_fire_on_free_slot	[PASSED]
test\test_transport\test_transport.cpp:356: test_timeout_does_not_fire_before_deadline	[PASSED]
test\test_transport\test_transport.cpp:357: test_timeout_fires_at_deadline	[PASSED]
test\test_transport\test_transport.cpp:358: test_timeout_fires_only_on_stale_slots	[PASSED]
test\test_transport\test_transport.cpp:359: test_init_succeeds_on_native	[PASSED]
test\test_transport\test_transport.cpp:360: test_all_last_activity_ms_zero_after_init	[PASSED]
test\test_transport\test_transport.cpp:361: test_queue_not_null_after_init	[PASSED]
test\test_transport\test_transport.cpp:364: stress_ring_buffer_fill_drain_integrity	[PASSED]
test\test_transport\test_transport.cpp:365: stress_ring_buffer_multi_cycle_no_corruption	[PASSED]
test\test_transport\test_transport.cpp:366: stress_all_slots_timeout_simultaneously	[PASSED]
test\test_transport\test_transport.cpp:367: stress_timeout_arm_recover_cycle	[PASSED]
test\test_transport\test_transport.cpp:368: stress_check_timeouts_high_call_rate	[PASSED]
test\test_transport\test_transport.cpp:369: stress_ring_buffer_byte_by_byte_fill_and_drain	[PASSED]
--------------- native:test_transport [PASSED] Took 1.78 seconds ---------------

Processing test_application in native_app environment
--------------------------------------------------------------------------------
Building...
Testing...
test\test_application\test_application.cpp:575: test_handler_reads_body	[PASSED]
test\test_application\test_application.cpp:576: test_handler_reads_query_param	[PASSED]
test\test_application\test_application.cpp:577: test_handler_reads_header	[PASSED]
test\test_application\test_application.cpp:578: test_wildcard_before_exact_wildcard_wins	[PASSED]
test\test_application\test_application.cpp:581: test_fn_on_registers_and_dispatches	[PASSED]
test\test_application\test_application.cpp:582: test_fn_on_path_copied_null_terminated	[PASSED]
test\test_application\test_application.cpp:583: test_fn_on_table_full_extra_routes_dropped	[PASSED]
test\test_application\test_application.cpp:584: test_fn_on_same_path_different_methods_are_distinct	[PASSED]
test\test_application\test_application.cpp:587: test_fn_on_not_found_called_when_no_match	[PASSED]
test\test_application\test_application.cpp:588: test_fn_on_not_found_not_called_when_match_exists	[PASSED]
test\test_application\test_application.cpp:591: test_fn_set_cors_options_preflight_clears_slot	[PASSED]
test\test_application\test_application.cpp:592: test_fn_set_cors_empty_string_disables	[PASSED]
test\test_application\test_application.cpp:595: test_wrong_method_does_not_match	[PASSED]
test\test_application\test_application.cpp:596: test_wrong_path_does_not_match	[PASSED]
test\test_application\test_application.cpp:597: test_all_http_methods_dispatched	[PASSED]
test\test_application\test_application.cpp:598: test_root_path_matches_exactly	[PASSED]
test\test_application\test_application.cpp:599: test_root_path_does_not_match_subpath	[PASSED]
test\test_application\test_application.cpp:600: test_wildcard_matches_any_suffix	[PASSED]
test\test_application\test_application.cpp:601: test_wildcard_does_not_match_unrelated_prefix	[PASSED]
test\test_application\test_application.cpp:602: test_exact_route_wins_when_registered_first	[PASSED]
test\test_application\test_application.cpp:603: test_slot_not_stuck_in_complete_after_handle	[PASSED]
test\test_application\test_application.cpp:604: test_parse_error_slot_auto_reset	[PASSED]
test\test_application\test_application.cpp:607: stress_last_route_dispatched_in_full_table	[PASSED]
test\test_application\test_application.cpp:608: stress_sequential_requests_no_state_leak	[PASSED]
test\test_application\test_application.cpp:609: stress_all_slots_dispatched_simultaneously	[PASSED]
test\test_application\test_application.cpp:610: stress_wildcard_matches_many_paths	[PASSED]
test\test_application\test_application.cpp:611: stress_handle_with_no_complete_slots_is_nop	[PASSED]
test\test_application\test_application.cpp:614: race_slot_complete_between_handle_calls	[PASSED]
test\test_application\test_application.cpp:615: race_conn_freed_after_parse_complete	[PASSED]
test\test_application\test_application.cpp:616: race_double_handle_no_double_dispatch	[PASSED]
test\test_application\test_application.cpp:617: race_error_and_valid_slot_in_same_handle	[PASSED]
test\test_application\test_application.cpp:618: race_callback_manually_resets_slot	[PASSED]
test\test_application\test_application.cpp:621: test_uri_too_long_auto_resets_slot	[PASSED]
test\test_application\test_application.cpp:624: test_transfer_encoding_chunked_is_501	[PASSED]
test\test_application\test_application.cpp:625: test_transfer_encoding_identity_is_501	[PASSED]
------------ native_app:test_application [PASSED] Took 1.47 seconds ------------

=================================== SUMMARY ===================================
Environment    Test               Status    Duration
-------------  -----------------  --------  ------------
native         test_http_parser   PASSED    00:00:01.776
native         test_presentation  PASSED    00:00:01.775
native         test_session       PASSED    00:00:01.670
native         test_transport     PASSED    00:00:01.777
native_app     test_application   PASSED    00:00:01.466
================ 222 test cases: 222 succeeded in 00:00:08.464 ================
```

</details>
