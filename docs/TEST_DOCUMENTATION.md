# Test Documentation

![Version](https://img.shields.io/badge/version-v5.60.0-blue)

Welcome to the testing documentation for `DeterministicESPAsyncWebServer`. This repository is designed to be extremely robust, employing **100% hardware-free, deterministic testing**.

Whether you are a beginner looking to understand how C++ testing works or an expert systems engineer designing secure, high-concurrency embedded protocols, this guide explains the architectures, methodologies, and concepts behind our test suite.

---

## 1. Introduction & Core Philosophy

### Why Native Testing?

Traditionally, testing code written for microcontroller frameworks like ESP-IDF or Arduino requires uploading binaries to physical ESP32 chips. This hardware-in-the-loop (HIL) testing has several drawbacks:

- **Slow feedback cycles**: Compiling, flashing, and rebooting microcontrollers takes minutes.
- **Flakiness**: Wireless connections fail, hardware pins float, and components experience wear.
- **Hard-to-reproduce bugs**: Multi-threaded concurrency bugs or network timing jitter cannot be reliably reproduced on physical chips.

`DeterministicESPAsyncWebServer` solves this by executing all test suites **natively** on your development machine (x86/x64 host).

### The Deterministic Asynchronous Model

This server is built on cooperative multitasking. Instead of physical threads, it uses a single-threaded event-driven event loop. Because of this, we can make tests **100% deterministic** through **Time-Travel Mocking**.

Instead of waiting for real-world seconds to elapse to test a connection timeout, the test suite manually increments a virtual clock (`millis()`) and drives the state machine forward manually. This means:

- A 5-second connection timeout can be tested in **less than a millisecond**.
- Execution order is guaranteed to be identical on every single run, eliminating race-condition flakiness.

```mermaid
graph TD
    A[Real Time] -->|Cannot control| B[Physical Hardware]
    C[Virtual Time Mocks] -->|Deterministic Control| D[Native C++ Event Loop]
    D -->|Simulate Events| E[Test Cases]
    E -->|Assert State| F[Pass/Fail]
```

---

## 2. Test Architecture & Mocking Strategies

To isolate our application code from physical hardware and the operating system's IP stack, we use a custom mocking layer.

### Mocks, Stubs, and Spies

- **Stubs**: Provide canned answers to calls made during the test. For example, our **Filesystem Stub** simulates an SPIFFS/LittleFS system by feeding static file contents from memory arrays instead of reading from a physical hard drive.
- **Mocks**: Objects pre-programmed with expectations that form a specification of the calls they are expected to receive.
- **Virtual Network Taps**: We mock the network stack completely. Instead of binding to real network sockets, we hook the server directly into virtual byte-pumps (ring buffers) that simulate incoming TCP packets.

```
       +---------------------------------------------+
       |                  TEST SUITE                 |
       +----------------------++---------------------+
                              || Simulates network packets
                              \/
       +---------------------------------------------+
       |             VIRTUAL TRANSPORT               |
       |  (mocks sockets, ring buffers, timeouts)    |
       +----------------------++---------------------+
                              || Drives HTTP/SSH bytes
                              \/
       +---------------------------------------------+
       |            CORE WEB SERVER ENGINE           |
       |     (HTTP parser, WebSockets, SSH)          |
       +---------------------------------------------+
```

---

## 3. PlatformIO Test Environments

PlatformIO separates configurations into different "environments" using [platformio.ini](file:///C:/Users/Douglas/Desktop/git_project/DeterministicESPAsyncWebserver/platformio.ini). Each environment compiles a specific slice of the codebase with custom compile-time flags to verify behavior in isolation:

| Environment           | Compile-Time Configuration    | Primary Purpose                                                                                         |
| :-------------------- | :---------------------------- | :------------------------------------------------------------------------------------------------------ |
| `native`              | Default flags                 | Core engine verification (HTTP parsing, WebSockets, SSE connection pool).                               |
| `native_app`          | Application logic active      | Layer-7 features: path routing, multipart forms, user authentication, static file serving.              |
| `native_ssh`          | `DETWS_ENABLE_SSH=1`          | The SSH crypto + protocol stack. Runs on software-based crypto paths without hardware acceleration.     |
| `native_ssh_hardened` | `DETWS_SSH_ALLOW_PASSWORD=0`  | Validates secure-only configuration where password-based logins are stripped out at compile-time.       |
| `native_ssh_conn`     | Full SSH + transport link     | Exercises the SSH state machine wired directly through our raw transport/session event-loop.            |
| `native_compliance`   | `DETWS_ENFORCE_HOST_HEADER=1` | Rigid RFC-compliance testing (enforces mandatory HTTP/1.1 `Host` headers and rejects duplicate fields). |
| `native_ota`          | `DETWS_ENABLE_OTA=1`          | The HTTP parser's streaming-body hook (large uploads bypass the buffer cap) against a mock sink.        |
| `native_prov`         | Default flags                 | The captive-portal form-field extractor / URL-decoder (the host-testable part of WiFi provisioning).    |

> [!NOTE]
> The `native` and `native_app` environments build with `DETWS_ENFORCE_HOST_HEADER=0` because their legacy test suites focus strictly on lower-level parser mechanics. The stricter RFC 7230 §5.4 host header validation is tested independently in `native_compliance`.

> [!IMPORTANT]
> **Compilation Isolation & Feature Flags**:
> Under PlatformIO (and standard Arduino/C++ build systems), library source files (in `src/`) are compiled independently of the main application (the sketch's `.ino` file) as separate translation units.
>
> Consequently, `#define` macros specified inside `.ino` sketch files (e.g., `#define DETWS_ENABLE_PROVISIONING 1`) **do not propagate** to the library's compiled source code. If you define a configuration macro or feature flag in your sketch rather than in the build configuration, the library's `.cpp` files will compile with their default configuration, resulting in linker errors (e.g., undefined symbols) or severe runtime/memory layout mismatches.
>
> To configure the library correctly, all override configuration constants and feature flags (such as [`DETWS_ENABLE_PROVISIONING`](@ref DETWS_ENABLE_PROVISIONING), [`DETWS_ENABLE_SSH`](@ref DETWS_ENABLE_SSH), [`MAX_CONNS`](@ref MAX_CONNS), etc.) **must** be set as compiler build flags in your environment (e.g., `build_flags = -DDETWS_ENABLE_PROVISIONING=1` in `platformio.ini`).

---

## 4. Deep Dive: Key Concepts Tested

### 1. HTTP/1.1 Parser & RFC Compliance

HTTP parsing is notoriously difficult to write safely. A single parsing slip can lead to security vulnerabilities like **HTTP Request Smuggling**. Our parser is tested against:

- **RFC 7230 & 7231**: Ensuring correct interpretation of URI paths, query parameters, header keys, and body limits.
- **Buffer Overflows (413 & 414)**: We verify that when client requests send URIs larger than `URI_BUF_SIZE` (414 URI Too Long) or bodies exceeding [`BODY_BUF_SIZE`](@ref BODY_BUF_SIZE) (413 Payload Too Large), the server safely terminates the connection without corrupting memory.
- **Host Header Enforcement**: In compliance builds, the server rejects any HTTP/1.1 request lacking a `Host` header, or containing duplicate `Host` headers.

### 2. WebSocket Protocols

WebSocket communication begins as an HTTP request and upgrades to a binary frame protocol. The suites test:

- **Sec-WebSocket-Accept**: Verifying the server takes the client's key, appends the RFC 6455 GUID (`258EAFA5-E914-47DA-95CA-C5AB0DC85B11`), hashes it using SHA-1, and Base64-encodes it to complete the handshake.
- **Masking Key Validation**: The protocol requires all client-to-server frames to be masked (XOR-encrypted). The tests send both masked and unmasked frames to ensure the server decodes them properly and rejects illegal unmasked frames.
- **Fragmentation**: Large payloads can be split across multiple frames. We simulate fragmented packets to ensure the server correctly buffers and reconstructs them.

### 3. Cryptography & Known-Answer Tests (KAT)

The native SSH server implementation includes an entire cryptography stack. Cryptography code should never be verified with random data. We use **Known-Answer Test Vectors** directly from NIST and RFC specifications:

- **SHA-256 / HMAC-SHA2-256**: Tested against NIST FIPS 180-4 vectors to guarantee message authentication code integrity.
- **AES-256-CTR**: Block cipher decryption/encryption verified against NIST SP 800-38A standard streams.
- **RSA Signature Verification**: Verified using real-world public-private key signatures generated via external `openssl` binaries.

---

## 5. How to Write and Run Tests

All tests are written using the **Unity** testing framework.

### Running Tests Locally

To run all test suites across all environments:

```bash
pio test -e native -e native_app -e native_ssh -e native_ssh_hardened -e native_ssh_conn -e native_compliance
```

To run a single specific environment (which is much faster):

```bash
pio test -e native
```

To regenerate the formatted Markdown test report locally:

```bash
bash test/run_tests.sh
```

---

### Running on Windows (PowerShell) and Linux (WSL)

The native suite is host-only, so on Windows it runs directly for almost every
environment. A few tests use POSIX-only seams (`gmtime_r`, ThreadSanitizer, the
`snmpget` interop) that the Windows MinGW toolchain does not provide, so those
build only on Linux. Continuous integration runs on Linux, so a green run under
**WSL (Ubuntu)** is the one that matches CI.

**On Windows (PowerShell) - the everyday path:**

```powershell
# one environment (fast)
pio test -e native_hostlink

# the formatting / lint gates, identical to CI:
clang-format -i src\services\hostlink\hostlink.cpp          # format C/C++ in place
clang-format --dry-run --Werror (git diff --name-only)     # check only (CI gate)
npx prettier@3.9.1 --write --end-of-line auto docs\*.md     # Markdown; --end-of-line auto avoids CRLF false flags
npx cspell --no-progress docs\ROADMAP.md                    # spellcheck (CI gate)
```

> A `git diff`-based `clang-format` check only sees **tracked** files: a brand
> new file is invisible until you `git add` it, so always run `clang-format` on
> any new file explicitly. (This is exactly what let an unformatted new header
> slip past a local check and fail the Code Formatting job in CI.)

**On Linux (WSL Ubuntu) - the CI-parity path:** PlatformIO lives in a venv at
`~/.pio-venv`, and the repo is visible under `/mnt/c/...`, so no copy is needed.

```bash
cd /mnt/c/Users/<you>/.../DeterministicESPAsyncWebServer
export PATH="$HOME/.pio-venv/bin:$PATH"

pio test -e native_tsan        # a Linux-only environment (ThreadSanitizer)
bash test/run_tests.sh         # full suite + regenerates docs/TEST_REPORT.md
```

**Driving WSL from a Windows shell (Git Bash):** calling `wsl.exe` from Git Bash
mangles arguments in two ways worth knowing:

- Git Bash maps `/tmp` to the Windows temp folder and rewrites POSIX paths on the
  command line. Prefix the call with `MSYS_NO_PATHCONV=1` to stop the rewrite.
- Inline scripts with embedded quotes get re-quoted passing through `wsl.exe` and
  can lose variable assignments. The reliable pattern is to pipe the script in on
  **stdin** (stripping carriage returns first) so no fragile quoting survives:

```bash
# run a script file on WSL, robustly, from Git Bash:
tr -d '\r' < scripts/run_native.sh | MSYS_NO_PATHCONV=1 wsl -d Ubuntu -- bash -l
```

To run the whole native suite in **parallel** on WSL (much faster than one serial
`pio test` invocation that builds every environment back to back):

```bash
envs=$(grep -oE '^\[env:native[A-Za-z0-9_]*\]' platformio.ini \
        | sed -E 's/\[env:(.*)\]/\1/' | grep -vE 'codeql')
printf '%s\n' $envs | xargs -P 6 -I{} pio test -e {}
```

---

### Step-by-Step: Writing a New Test Case

Let's walk through creating a test case to verify that the HTTP parser correctly parses a basic `GET` request.

#### Step 1: Open the Test Suite File

If you are testing parser mechanics, open `test/test_http_parser/test_http_parser.cpp`.

#### Step 2: Write the Test Function

Add a test function. Keep it self-contained and descriptive:

```cpp
void test_http_parser_simple_get_request() {
    // 1. Arrange: Initialize your parser state and sample request bytes
    http_parser_t parser;
    http_parser_init(&parser, 0); // Slot ID 0

    const char* request_bytes = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";

    // 2. Act: Feed bytes incrementally to simulate packet arrivals
    size_t bytes_fed = http_parser_feed(&parser, request_bytes, strlen(request_bytes));

    // 3. Assert: Verify the state is correct
    TEST_ASSERT_EQUAL(strlen(request_bytes), bytes_fed);
    TEST_ASSERT_EQUAL(PARSE_STATE_COMPLETE, parser.state);
    TEST_ASSERT_EQUAL_STRING("/index.html", parser.path);
    TEST_ASSERT_EQUAL_STRING("GET", parser.method);
}
```

> [!TIP]
> Keep your descriptions inside the function body as a single line comment starting with `//`. The reporting scripts automatically parse these comments to generate documentation strings in the final report!

#### Step 3: Register the Test in `main()`

Scroll to the bottom of the test file where `main()` resides, and register your function using `RUN_TEST`:

```cpp
int main() {
    UNITY_BEGIN();

    // ... other registered tests ...
    RUN_TEST(test_http_parser_simple_get_request);

    return UNITY_END();
}
```

---

## 6. Expert-Level Debugging: Memory Safety & Sanitizers

When developing C++ code natively, we can compile our suites with compilers like `gcc` or `clang` and attach advanced debugging sanitizers that would be impossible to run on an actual ESP32 chip.

### AddressSanitizer (ASan)

If you run into segmentation faults or want to ensure your code has no memory leaks, you can enable AddressSanitizer. In your `platformio.ini` file, add:

```ini
[env:native]
platform = native
build_flags =
    -fsanitize=address,undefined
    -g
```

When you execute `pio test`, your host compiler compiles instrumentation checks around every pointer access. If a buffer overflow or use-after-free occurs, the test runner immediately stops and prints a stack trace pointing directly to the offending line of code.

### Simulating Race Conditions

We test session and socket race conditions by interleaved function calling:

1. Initialize the socket buffer.
2. Feed partial packets.
3. Call an intermediate tick handler (simulating thread preemption).
4. Assert that the buffer holds its state and has not entered an invalid transition.
   This is fully reproducible because there are no actual operating system threads involved.

## 7. Comprehensive Test Directory

This section contains a thorough directory of all test cases across all 20 test suites. Click on any test suite to expand its test cases, and click on individual test cases to expand their objectives and assertions.

<details>
<summary><b>test_application (47 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_handler_reads_body</b> &mdash; <i>Handler reads body</i></summary>

    * **Objective**: Handler reads body
    * **Assertions**:
      * <code>Assert equal string ("hello", body_seen)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_handler_reads_query_param</b> &mdash; <i>Handler reads query param</i></summary>

    * **Objective**: Handler reads query param
    * **Assertions**:
      * <code>Assert equal string ("42", q_seen)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_handler_reads_header</b> &mdash; <i>Handler reads header</i></summary>

    * **Objective**: Handler reads header
    * **Assertions**:
      * <code>Assert equal string ("secret", h_seen)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_wildcard_before_exact_wildcard_wins</b> &mdash; <i>Wildcard before exact wildcard wins</i></summary>

    * **Objective**: Wildcard before exact wildcard wins
    * **Assertions**:
      * <code>Assert true (wildcard_called)</code>
      * <code>Assert false (exact_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_on_registers_and_dispatches</b> &mdash; <i>Fn on registers and dispatches</i></summary>

    * **Objective**: Fn on registers and dispatches
    * **Assertions**:
      * <code>Assert true (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_on_path_copied_null_terminated</b> &mdash; <i>Fn on path copied null terminated</i></summary>

    * **Objective**: Fn on path copied null terminated

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_on_table_full_extra_routes_dropped</b> &mdash; <i>Fn on table full extra routes dropped</i></summary>

    * **Objective**: Fn on table full extra routes dropped
    * **Assertions**:
      * <code>Assert true (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_on_same_path_different_methods_are_distinct</b> &mdash; <i>Fn on same path different methods are distinct</i></summary>

    * **Objective**: Fn on same path different methods are distinct
    * **Assertions**:
      * <code>Assert true (get_called)</code>
      * <code>Assert false (post_called)</code>
      * <code>Assert true (post_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_on_not_found_called_when_no_match</b> &mdash; <i>Fn on not found called when no match</i></summary>

    * **Objective**: Fn on not found called when no match
    * **Assertions**:
      * <code>Assert true (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_on_not_found_not_called_when_match_exists</b> &mdash; <i>Fn on not found not called when match exists</i></summary>

    * **Objective**: Fn on not found not called when match exists
    * **Assertions**:
      * <code>Assert true (handler_called)</code>
      * <code>Assert false (nf)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_set_cors_options_preflight_clears_slot</b> &mdash; <i>Fn set cors options preflight clears slot</i></summary>

    * **Objective**: Fn set cors options preflight clears slot
    * **Assertions**:
      * <code>Assert not equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_set_cors_empty_string_disables</b> &mdash; <i>Fn set cors empty string disables</i></summary>

    * **Objective**: Fn set cors empty string disables
    * **Assertions**:
      * <code>Assert true (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_wrong_method_does_not_match</b> &mdash; <i>Wrong method does not match</i></summary>

    * **Objective**: Wrong method does not match
    * **Assertions**:
      * <code>Assert false (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_wrong_path_does_not_match</b> &mdash; <i>Wrong path does not match</i></summary>

    * **Objective**: Wrong path does not match
    * **Assertions**:
      * <code>Assert false (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_all_http_methods_dispatched</b> &mdash; <i>All http methods dispatched</i></summary>

    * **Objective**: All http methods dispatched
    * **Assertions**:
      * <code>Assert equal message (1, counts[i], "method not dispatched")</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_root_path_matches_exactly</b> &mdash; <i>Root path matches exactly</i></summary>

    * **Objective**: Root path matches exactly
    * **Assertions**:
      * <code>Assert true (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_root_path_does_not_match_subpath</b> &mdash; <i>Root path does not match subpath</i></summary>

    * **Objective**: Root path does not match subpath
    * **Assertions**:
      * <code>Assert false (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_wildcard_matches_any_suffix</b> &mdash; <i>Wildcard matches any suffix</i></summary>

    * **Objective**: Wildcard matches any suffix
    * **Assertions**:
      * <code>Assert true (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_wildcard_does_not_match_unrelated_prefix</b> &mdash; <i>Wildcard does not match unrelated prefix</i></summary>

    * **Objective**: Wildcard does not match unrelated prefix
    * **Assertions**:
      * <code>Assert false (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_exact_route_wins_when_registered_first</b> &mdash; <i>Exact route wins when registered first</i></summary>

    * **Objective**: Exact route wins when registered first
    * **Assertions**:
      * <code>Assert true (exact_called)</code>
      * <code>Assert false (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_slot_not_stuck_in_complete_after_handle</b> &mdash; <i>Slot not stuck in complete after handle</i></summary>

    * **Objective**: Slot not stuck in complete after handle
    * **Assertions**:
      * <code>Assert not equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_parse_error_slot_auto_reset</b> &mdash; <i>Parse error slot auto reset</i></summary>

    * **Objective**: Parse error slot auto reset
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>
      * <code>Assert not equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_last_route_dispatched_in_full_table</b> &mdash; <i>Stress - Last route dispatched in full table</i></summary>

    * **Objective**: Stress - Last route dispatched in full table
    * **Assertions**:
      * <code>Assert equal (1, last_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_sequential_requests_no_state_leak</b> &mdash; <i>Stress - Sequential requests no state leak</i></summary>

    * **Objective**: Stress - Sequential requests no state leak
    * **Assertions**:
      * <code>Assert equal (50, seq_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_all_slots_dispatched_simultaneously</b> &mdash; <i>Stress - All slots dispatched simultaneously</i></summary>

    * **Objective**: Stress - All slots dispatched simultaneously
    * **Assertions**:
      * <code>Assert equal message (1, counts[i], "slot not dispatched")</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_wildcard_matches_many_paths</b> &mdash; <i>Stress - Wildcard matches many paths</i></summary>

    * **Objective**: Stress - Wildcard matches many paths
    * **Assertions**:
      * <code>Assert equal (10, wc_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_handle_with_no_complete_slots_is_nop</b> &mdash; <i>Stress - Handle with no complete slots is nop</i></summary>

    * **Objective**: Stress - Handle with no complete slots is nop
    * **Assertions**:
      * <code>Assert false (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_slot_complete_between_handle_calls</b> &mdash; <i>Race - Slot complete between handle calls</i></summary>

    * **Objective**: Race - Slot complete between handle calls
    * **Assertions**:
      * <code>Assert false (dispatched)</code>
      * <code>Assert true (dispatched)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_conn_freed_after_parse_complete</b> &mdash; <i>Race - Conn freed after parse complete</i></summary>

    * **Objective**: Race - Conn freed after parse complete
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert not equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_double_handle_no_double_dispatch</b> &mdash; <i>Race - Double handle no double dispatch</i></summary>

    * **Objective**: Race - Double handle no double dispatch
    * **Assertions**:
      * <code>Assert equal (1, dispatch_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_error_and_valid_slot_in_same_handle</b> &mdash; <i>Race - Error and valid slot in same handle</i></summary>

    * **Objective**: Race - Error and valid slot in same handle
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>
      * <code>Assert not equal (PARSE_ERROR, http_pool[0].parse_state)</code>
      * <code>Assert true (valid_dispatched)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_callback_manually_resets_slot</b> &mdash; <i>Race - Callback manually resets slot</i></summary>

    * **Objective**: Race - Callback manually resets slot
    * **Assertions**:
      * <code>Assert true (manual_reset_called)</code>
      * <code>Assert equal (PARSE_METHOD, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_uri_too_long_auto_resets_slot</b> &mdash; <i>Uri too long auto resets slot</i></summary>

    * **Objective**: Uri too long auto resets slot
    * **Assertions**:
      * <code>Assert equal (PARSE_URI_TOO_LONG, http_pool[0].parse_state)</code>
      * <code>Assert not equal (PARSE_URI_TOO_LONG, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_transfer_encoding_chunked_is_501</b> &mdash; <i>Transfer encoding chunked is 501</i></summary>

    * **Objective**: Transfer encoding chunked is 501
    * **Assertions**:
      * <code>Assert not equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_transfer_encoding_identity_is_501</b> &mdash; <i>Transfer encoding identity is 501</i></summary>

    * **Objective**: Transfer encoding identity is 501
    * **Assertions**:
      * <code>Assert not equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_redirect_emits_location_and_status</b> &mdash; <i>Redirect emits location and status</i></summary>

    * **Objective**: Redirect emits location and status
    * **Assertions**:
      * <code>Assert not null (strstr(out, "HTTP/1.1 301 Moved Permanently"))</code>
      * <code>Assert not null (strstr(out, "Location: /index.html\\r\\n"))</code>
      * <code>Assert not null (strstr(out, "Content-Length: 0\\r\\n"))</code>
      * <code>Assert equal (CONN_FREE, conn_pool[0].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_redirect_invalid_code_defaults_to_302</b> &mdash; <i>Redirect invalid code defaults to 302</i></summary>

    * **Objective**: Redirect invalid code defaults to 302
    * **Assertions**:
      * <code>Assert not null (strstr(out, "HTTP/1.1 302 Found"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_mime_type_detection</b> &mdash; <i>Mime type detection</i></summary>

    * **Objective**: Mime type detection
    * **Assertions**:
      * <code>Assert equal string ("text/html", DetWebServer::mime_type("/index.html"))</code>
      * <code>Assert equal string ("text/css", DetWebServer::mime_type("/css/site.css"))</code>
      * <code>Assert equal string ("application/javascript", DetWebServer::mime_type("/app.JS"))</code>
      * <code>Assert equal string ("application/json", DetWebServer::mime_type("/api/data.json"))</code>
      * <code>Assert equal string ("image/svg+xml", DetWebServer::mime_type("logo.svg"))</code>
      * <code>Assert equal string ("image/png", DetWebServer::mime_type("a.b.c.png"))</code>
      * <code>Assert equal string ("application/octet-stream", DetWebServer::mime_type("/file.unknownext"))</code>
      * <code>Assert equal string ("application/octet-stream", DetWebServer::mime_type("/noext"))</code>
      * <code>Assert equal string ("application/octet-stream", DetWebServer::mime_type("/dir.with.dot/file"))</code>
      * <code>Assert equal string ("application/octet-stream", DetWebServer::mime_type(nullptr))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_serve_static_file_and_mime</b> &mdash; <i>Serve static file and mime</i></summary>

    * **Objective**: Serve static file and mime
    * **Assertions**:
      * <code>Assert not null (strstr(out, "HTTP/1.1 200 OK"))</code>
      * <code>Assert not null (strstr(out, "Content-Type: text/css"))</code>
      * <code>Assert not null (strstr(out, "body{color:red}"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_serve_static_index_fallback</b> &mdash; <i>Serve static index fallback</i></summary>

    * **Objective**: Serve static index fallback
    * **Assertions**:
      * <code>Assert not null (strstr(out, "HTTP/1.1 200 OK"))</code>
      * <code>Assert not null (strstr(out, "Content-Type: text/html"))</code>
      * <code>Assert not null (strstr(out, "&lt;h1&gt;home&lt;/h1&gt;"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_serve_static_gzip_when_accepted</b> &mdash; <i>Serve static gzip when accepted</i></summary>

    * **Objective**: Serve static gzip when accepted
    * **Assertions**:
      * <code>Assert not null (strstr(out, "HTTP/1.1 200 OK"))</code>
      * <code>Assert not null (strstr(out, "Content-Type: application/javascript"))</code>
      * <code>Assert not null (strstr(out, "Content-Encoding: gzip"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_serve_static_no_gzip_when_not_accepted</b> &mdash; <i>Serve static no gzip when not accepted</i></summary>

    * **Objective**: Serve static no gzip when not accepted
    * **Assertions**:
      * <code>Assert null (strstr(out, "Content-Encoding: gzip"))</code>
      * <code>Assert not null (strstr(out, "console.log(1)"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_serve_static_traversal_not_leaked</b> &mdash; <i>Serve static traversal not leaked</i></summary>

    * **Objective**: Serve static traversal not leaked
    * **Assertions**:
      * <code>Assert null (strstr(out, "topsecret"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_serve_static_missing_is_404</b> &mdash; <i>Serve static missing is 404</i></summary>

    * **Objective**: Serve static missing is 404
    * **Assertions**:
      * <code>Assert not null (strstr(out, "404"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_serve_static_etag_conditional_get</b> &mdash; <i>Serve static etag conditional get</i></summary>

    * **Objective**: Serve static etag conditional get
    * **Assertions**:
      * <code>Assert not null (strstr(out1, "HTTP/1.1 200 OK"))</code>
      * <code>Assert not null (etp)</code>
      * <code>Assert not null (strstr(out2, "304 Not Modified"))</code>
      * <code>Assert not null (strstr(out2, etag))</code>
      * <code>Assert null (strstr(out2, "&lt;html&gt;hi&lt;/html&gt;"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_request_log_hook_fires</b> &mdash; <i>Request log hook fires</i></summary>

    * **Objective**: Request log hook fires
    * **Assertions**:
      * <code>Assert equal int (1, g_log_calls)</code>
      * <code>Assert equal string ("GET", g_log_method)</code>
      * <code>Assert equal string ("/hi", g_log_path)</code>
      * <code>Assert equal int (200, g_log_status)</code>
      * <code>Assert equal int (5, g_log_bytes)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_stats_endpoint_emits_json</b> &mdash; <i>Stats endpoint emits json</i></summary>

    * **Objective**: Stats endpoint emits json
    * **Assertions**:
      * <code>Assert not null (strstr(out, "application/json"))</code>
      * <code>Assert not null (strstr(out, "\\"uptime_ms\\""))</code>
      * <code>Assert not null (strstr(out, "\\"requests\\""))</code>
      * <code>Assert not null (strstr(out, "\\"http_2xx\\""))</code>
      * <code>Assert not null (strstr(out, "\\"http_4xx\\""))</code>
      * <code>Assert not null (strstr(out, "\\"active_conns\\""))</code>

  </details>

</details>

<details>
<summary><b>test_auth (13 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_unprotected_route_fires_handler</b> &mdash; <i>Unprotected route fires handler</i></summary>

    * **Objective**: Unprotected route fires handler
    * **Assertions**:
      * <code>Assert true (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_protected_route_no_header_returns_401</b> &mdash; <i>Protected route no header returns 401</i></summary>

    * **Objective**: Protected route no header returns 401
    * **Assertions**:
      * <code>Assert false (handler_called)</code>
      * <code>Assert true (strstr(tcp_captured(), "401 Unauthorized") != nullptr)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_protected_route_wrong_password_returns_401</b> &mdash; <i>Protected route wrong password returns 401</i></summary>

    * **Objective**: Protected route wrong password returns 401
    * **Assertions**:
      * <code>Assert false (handler_called)</code>
      * <code>Assert true (strstr(tcp_captured(), "401") != nullptr)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_protected_route_wrong_username_returns_401</b> &mdash; <i>Protected route wrong username returns 401</i></summary>

    * **Objective**: Protected route wrong username returns 401
    * **Assertions**:
      * <code>Assert false (handler_called)</code>
      * <code>Assert true (strstr(tcp_captured(), "401") != nullptr)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_protected_route_valid_credentials_fires_handler</b> &mdash; <i>Protected route valid credentials fires handler</i></summary>

    * **Objective**: Protected route valid credentials fires handler
    * **Assertions**:
      * <code>Assert true (handler_called)</code>
      * <code>Assert true (strstr(tcp_captured(), "200 OK") != nullptr)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_401_includes_www_authenticate_header</b> &mdash; <i>401 includes www authenticate header</i></summary>

    * **Objective**: 401 includes www authenticate header
    * **Assertions**:
      * <code>Assert not null (strstr(tcp_captured(), "WWW-Authenticate: Basic realm=\\"MyRealm\\""))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_non_basic_scheme_returns_401</b> &mdash; <i>Non basic scheme returns 401</i></summary>

    * **Objective**: Non basic scheme returns 401
    * **Assertions**:
      * <code>Assert false (handler_called)</code>
      * <code>Assert true (strstr(tcp_captured(), "401") != nullptr)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_credentials_without_colon_returns_401</b> &mdash; <i>Credentials without colon returns 401</i></summary>

    * **Objective**: Credentials without colon returns 401
    * **Assertions**:
      * <code>Assert false (handler_called)</code>
      * <code>Assert true (strstr(tcp_captured(), "401") != nullptr)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_protected_and_unprotected_routes_coexist</b> &mdash; <i>Protected and unprotected routes coexist</i></summary>

    * **Objective**: Protected and unprotected routes coexist
    * **Assertions**:
      * <code>Assert true (handler_called)</code>
      * <code>Assert false (handler_called)</code>
      * <code>Assert true (strstr(tcp_captured(), "401") != nullptr)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_auth_route_returns_404_for_wrong_path</b> &mdash; <i>Auth route returns 404 for wrong path</i></summary>

    * **Objective**: Auth route returns 404 for wrong path
    * **Assertions**:
      * <code>Assert false (handler_called)</code>
      * <code>Assert true (strstr(tcp_captured(), "404") != nullptr)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_auth_checked_per_method</b> &mdash; <i>Auth checked per method</i></summary>

    * **Objective**: Auth checked per method
    * **Assertions**:
      * <code>Assert false (handler_called)</code>
      * <code>Assert not null (strstr(tcp_captured(), "405"))</code>
      * <code>Assert null (strstr(tcp_captured(), "401"))</code>
      * <code>Assert not null (strstr(tcp_captured(), "Allow: POST"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_auth_50_valid_requests</b> &mdash; <i>Stress - Auth 50 valid requests</i></summary>

    * **Objective**: Stress - Auth 50 valid requests
    * **Assertions**:
      * <code>Assert true message (handler_called, "handler not called with valid creds")</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_auth_50_invalid_requests</b> &mdash; <i>Stress - Auth 50 invalid requests</i></summary>

    * **Objective**: Stress - Auth 50 invalid requests
    * **Assertions**:
      * <code>Assert false message (handler_called, "handler called with bad creds")</code>

  </details>

</details>

<details>
<summary><b>test_compliance (12 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_http11_missing_host_rejected</b> &mdash; <i>Http11 missing host rejected</i></summary>

    * **Objective**: Http11 missing host rejected
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_http11_with_host_ok</b> &mdash; <i>Http11 with host ok</i></summary>

    * **Objective**: Http11 with host ok
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_http10_missing_host_ok</b> &mdash; <i>Http10 missing host ok</i></summary>

    * **Objective**: Http10 missing host ok
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_duplicate_host_rejected</b> &mdash; <i>Duplicate host rejected</i></summary>

    * **Objective**: Duplicate host rejected
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_duplicate_host_rejected_http10</b> &mdash; <i>Duplicate host rejected http10</i></summary>

    * **Objective**: Duplicate host rejected http10
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_host_beyond_max_headers_still_counted</b> &mdash; <i>Host beyond max headers still counted</i></summary>

    * **Objective**: Host beyond max headers still counted
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_duplicate_host_with_one_beyond_cap_rejected</b> &mdash; <i>Duplicate host with one beyond cap rejected</i></summary>

    * **Objective**: Duplicate host with one beyond cap rejected
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_content_length_non_digit_rejected</b> &mdash; <i>Content length non digit rejected</i></summary>

    * **Objective**: Content length non digit rejected
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_content_length_empty_rejected</b> &mdash; <i>Content length empty rejected</i></summary>

    * **Objective**: Content length empty rejected
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_content_length_conflicting_duplicate_rejected</b> &mdash; <i>Content length conflicting duplicate rejected</i></summary>

    * **Objective**: Content length conflicting duplicate rejected
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_content_length_matching_duplicate_ok</b> &mdash; <i>Content length matching duplicate ok</i></summary>

    * **Objective**: Content length matching duplicate ok
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (3, (int)http_pool[0].content_length)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_content_length_valid_body</b> &mdash; <i>Content length valid body</i></summary>

    * **Objective**: Content length valid body
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (5, (int)http_pool[0].body_len)</code>
      * <code>Assert equal memory ("hello", http_pool[0].body, 5)</code>

  </details>

</details>

<details>
<summary><b>test_dispatch (10 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_method_mismatch_returns_405</b> &mdash; <i>Method mismatch returns 405</i></summary>

    * **Objective**: Method mismatch returns 405
    * **Assertions**:
      * <code>Assert false (handler_called)</code>
      * <code>Assert not null (strstr(tcp_captured(), "405 Method Not Allowed"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_405_includes_allow_header</b> &mdash; <i>405 includes allow header</i></summary>

    * **Objective**: 405 includes allow header
    * **Assertions**:
      * <code>Assert not null (strstr(tcp_captured(), "Allow: POST"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_405_allow_lists_all_methods_for_path</b> &mdash; <i>405 allow lists all methods for path</i></summary>

    * **Objective**: 405 allow lists all methods for path
    * **Assertions**:
      * <code>Assert not null (strstr(resp, "405"))</code>
      * <code>Assert not null (strstr(resp, "POST"))</code>
      * <code>Assert not null (strstr(resp, "DELETE"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_unknown_path_still_404_not_405</b> &mdash; <i>Unknown path still 404 not 405</i></summary>

    * **Objective**: Unknown path still 404 not 405
    * **Assertions**:
      * <code>Assert not null (strstr(tcp_captured(), "404"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_unknown_method_returns_501</b> &mdash; <i>Unknown method returns 501</i></summary>

    * **Objective**: Unknown method returns 501
    * **Assertions**:
      * <code>Assert false (handler_called)</code>
      * <code>Assert not null (strstr(tcp_captured(), "501 Not Implemented"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_unknown_method_not_treated_as_get</b> &mdash; <i>Unknown method not treated as get</i></summary>

    * **Objective**: Unknown method not treated as get
    * **Assertions**:
      * <code>Assert false (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_head_runs_get_handler_without_body</b> &mdash; <i>Head runs get handler without body</i></summary>

    * **Objective**: Head runs get handler without body
    * **Assertions**:
      * <code>Assert true (handler_called)</code>
      * <code>Assert not null (strstr(resp, "200 OK"))</code>
      * <code>Assert not null (strstr(resp, "Content-Length: 2"))</code>
      * <code>Assert not null (sep)</code>
      * <code>Assert equal string ("\\r\\n\\r\\n", sep)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_get_route_advertises_head_in_allow</b> &mdash; <i>Get route advertises head in allow</i></summary>

    * **Objective**: Get route advertises head in allow
    * **Assertions**:
      * <code>Assert not null (strstr(resp, "405"))</code>
      * <code>Assert not null (strstr(resp, "GET"))</code>
      * <code>Assert not null (strstr(resp, "HEAD"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_head_on_post_only_route_405</b> &mdash; <i>Head on post only route 405</i></summary>

    * **Objective**: Head on post only route 405
    * **Assertions**:
      * <code>Assert false (handler_called)</code>
      * <code>Assert not null (strstr(tcp_captured(), "405"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_correct_method_still_dispatches</b> &mdash; <i>Correct method still dispatches</i></summary>

    * **Objective**: Correct method still dispatches
    * **Assertions**:
      * <code>Assert true (handler_called)</code>
      * <code>Assert not null (strstr(tcp_captured(), "200 OK"))</code>

  </details>

</details>

<details>
<summary><b>test_file_serving (12 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_missing_file_returns_404</b> &mdash; <i>Missing file returns 404</i></summary>

    * **Objective**: Missing file returns 404
    * **Assertions**:
      * <code>Assert true (handler_called)</code>
      * <code>Assert not null (strstr(tcp_captured(), "404"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_existing_file_returns_200</b> &mdash; <i>Existing file returns 200</i></summary>

    * **Objective**: Existing file returns 200
    * **Assertions**:
      * <code>Assert true (handler_called)</code>
      * <code>Assert not null (strstr(tcp_captured(), "200 OK"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_response_includes_content_type_html</b> &mdash; <i>Response includes content type html</i></summary>

    * **Objective**: Response includes content type html
    * **Assertions**:
      * <code>Assert not null (strstr(tcp_captured(), "Content-Type: text/html"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_response_includes_content_type_js</b> &mdash; <i>Response includes content type js</i></summary>

    * **Objective**: Response includes content type js
    * **Assertions**:
      * <code>Assert true (handler_called)</code>
      * <code>Assert not null (strstr(tcp_captured(), "Content-Type: application/javascript"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_content_length_matches_file_size</b> &mdash; <i>Content length matches file size</i></summary>

    * **Objective**: Content length matches file size
    * **Assertions**:
      * <code>Assert not null (strstr(tcp_captured(), expected_cl))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_file_body_is_sent</b> &mdash; <i>File body is sent</i></summary>

    * **Objective**: File body is sent
    * **Assertions**:
      * <code>Assert not null (strstr(tcp_captured(), body))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_empty_file_returns_200_with_zero_length</b> &mdash; <i>Empty file returns 200 with zero length</i></summary>

    * **Objective**: Empty file returns 200 with zero length
    * **Assertions**:
      * <code>Assert not null (strstr(tcp_captured(), "200 OK"))</code>
      * <code>Assert not null (strstr(tcp_captured(), "Content-Length: 0"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_large_file_body_fully_sent</b> &mdash; <i>Large file body fully sent</i></summary>

    * **Objective**: Large file body fully sent
    * **Assertions**:
      * <code>Assert not null (strstr(tcp_captured(), "200 OK"))</code>
      * <code>Assert not null (strstr(tcp_captured(), expected_cl))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_serve_file_does_not_affect_other_routes</b> &mdash; <i>Serve file does not affect other routes</i></summary>

    * **Objective**: Serve file does not affect other routes
    * **Assertions**:
      * <code>Assert true (other_called)</code>
      * <code>Assert false (handler_called)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_multiple_content_types</b> &mdash; <i>Multiple content types</i></summary>

    * **Objective**: Multiple content types
    * **Assertions**:
      * <code>Assert not null message (strstr(tcp_captured(), "200 OK"), "expected 200 OK")</code>
      * <code>Assert not null message (strstr(tcp_captured(), cases[i].ctype), "expected content-type in response")</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_serve_file_50_requests</b> &mdash; <i>Stress - Serve file 50 requests</i></summary>

    * **Objective**: Stress - Serve file 50 requests
    * **Assertions**:
      * <code>Assert true message (handler_called, "handler not called")</code>
      * <code>Assert not null message (strstr(tcp_captured(), "200 OK"), "not 200")</code>
      * <code>Assert not null message (strstr(tcp_captured(), body), "body missing")</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_alternate_missing_and_found</b> &mdash; <i>Stress - Alternate missing and found</i></summary>

    * **Objective**: Stress - Alternate missing and found
    * **Assertions**:
      * <code>Assert not null message (strstr(tcp_captured(), "200"), "expected 200")</code>
      * <code>Assert not null message (strstr(tcp_captured(), "404"), "expected 404")</code>

  </details>

</details>

<details>
<summary><b>test_http_ota (3 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_large_body_streams_to_completion</b> &mdash; <i>Large body streams to completion</i></summary>

    * **Objective**: Large body streams to completion
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, r.parse_state)</code>
      * <code>Assert true (r.body_streaming)</code>
      * <code>Assert equal uint (N, (unsigned)g_total)</code>
      * <code>Assert greater than (1, g_chunks)</code>
      * <code>TEST_ASSERT_EQUAL_UINT8('A' + (i % 26), g_capture[i]);</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_no_hooks_large_body_is_413</b> &mdash; <i>No hooks large body is 413</i></summary>

    * **Objective**: No hooks large body is 413
    * **Assertions**:
      * <code>Assert equal (PARSE_ENTITY_TOO_LARGE, r.parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_nonmatching_path_not_streamed</b> &mdash; <i>Nonmatching path not streamed</i></summary>

    * **Objective**: Nonmatching path not streamed
    * **Assertions**:
      * <code>Assert equal (PARSE_ENTITY_TOO_LARGE, r.parse_state)</code>
      * <code>Assert equal uint (0, (unsigned)g_total)</code>

  </details>

</details>

<details>
<summary><b>test_http_parser (80 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_reset_sets_parse_method_state</b> &mdash; <i>Reset sets parse method state</i></summary>

    * **Objective**: Reset sets parse method state
    * **Assertions**:
      * <code>Assert equal (PARSE_METHOD, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_reset_preserves_slot_id</b> &mdash; <i>Reset preserves slot id</i></summary>

    * **Objective**: Reset preserves slot id
    * **Assertions**:
      * <code>Assert equal (2, (int)http_pool[2].slot_id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_reset_clears_method</b> &mdash; <i>Reset clears method</i></summary>

    * **Objective**: Reset clears method
    * **Assertions**:
      * <code>Assert equal ('\\0', http_pool[0].method[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_reset_clears_path</b> &mdash; <i>Reset clears path</i></summary>

    * **Objective**: Reset clears path
    * **Assertions**:
      * <code>Assert equal ('\\0', http_pool[0].path[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_reset_clears_header_count</b> &mdash; <i>Reset clears header count</i></summary>

    * **Objective**: Reset clears header count
    * **Assertions**:
      * <code>Assert equal (0, (int)http_pool[0].header_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_reset_clears_body</b> &mdash; <i>Reset clears body</i></summary>

    * **Objective**: Reset clears body
    * **Assertions**:
      * <code>Assert equal (0, (int)http_pool[0].body_len)</code>
      * <code>Assert equal (0, (int)http_pool[0].content_length)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_reset_clears_query_count</b> &mdash; <i>Reset clears query count</i></summary>

    * **Objective**: Reset clears query count
    * **Assertions**:
      * <code>Assert equal (0, (int)http_pool[0].query_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_feed_after_complete_does_not_change_state</b> &mdash; <i>Feed after complete does not change state</i></summary>

    * **Objective**: Feed after complete does not change state
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_feed_after_error_does_not_change_state</b> &mdash; <i>Feed after error does not change state</i></summary>

    * **Objective**: Feed after error does not change state
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_feed_after_entity_too_large_does_not_change_state</b> &mdash; <i>Feed after entity too large does not change state</i></summary>

    * **Objective**: Feed after entity too large does not change state
    * **Assertions**:
      * <code>Assert equal (PARSE_ENTITY_TOO_LARGE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_method_get</b> &mdash; <i>Method get</i></summary>

    * **Objective**: Method get
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("GET", http_pool[0].method)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_method_post</b> &mdash; <i>Method post</i></summary>

    * **Objective**: Method post
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("POST", http_pool[0].method)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_method_put</b> &mdash; <i>Method put</i></summary>

    * **Objective**: Method put
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("PUT", http_pool[0].method)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_method_delete</b> &mdash; <i>Method delete</i></summary>

    * **Objective**: Method delete
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("DELETE", http_pool[0].method)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_method_patch</b> &mdash; <i>Method patch</i></summary>

    * **Objective**: Method patch
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("PATCH", http_pool[0].method)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_method_head</b> &mdash; <i>Method head</i></summary>

    * **Objective**: Method head
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("HEAD", http_pool[0].method)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_method_options</b> &mdash; <i>Method options</i></summary>

    * **Objective**: Method options
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("OPTIONS", http_pool[0].method)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_method_overflow_is_error</b> &mdash; <i>Method overflow is error</i></summary>

    * **Objective**: Method overflow is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_path_root</b> &mdash; <i>Path root</i></summary>

    * **Objective**: Path root
    * **Assertions**:
      * <code>Assert equal string ("/", http_pool[0].path)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_path_segments</b> &mdash; <i>Path segments</i></summary>

    * **Objective**: Path segments
    * **Assertions**:
      * <code>Assert equal string ("/api/users/42", http_pool[0].path)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_path_without_query</b> &mdash; <i>Path without query</i></summary>

    * **Objective**: Path without query
    * **Assertions**:
      * <code>Assert equal string ("/p", http_pool[0].path)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_path_overflow_is_414</b> &mdash; <i>Path overflow is 414</i></summary>

    * **Objective**: Path overflow is 414
    * **Assertions**:
      * <code>Assert equal (PARSE_URI_TOO_LONG, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_single_query_param</b> &mdash; <i>Single query param</i></summary>

    * **Objective**: Single query param
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (1, (int)http_pool[0].query_count)</code>
      * <code>Assert not null (v)</code>
      * <code>Assert equal string ("42", v)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_two_query_params</b> &mdash; <i>Two query params</i></summary>

    * **Objective**: Two query params
    * **Assertions**:
      * <code>Assert equal (2, (int)http_pool[0].query_count)</code>
      * <code>Assert equal string ("1", http_get_query(&http_pool[0], "a"))</code>
      * <code>Assert equal string ("2", http_get_query(&http_pool[0], "b"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_query_key_not_found_returns_null</b> &mdash; <i>Query key not found returns null</i></summary>

    * **Objective**: Query key not found returns null
    * **Assertions**:
      * <code>Assert null (http_get_query(&http_pool[0], "z"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_query_empty_value</b> &mdash; <i>Query empty value</i></summary>

    * **Objective**: Query empty value
    * **Assertions**:
      * <code>Assert not null (v)</code>
      * <code>Assert equal string ("", v)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_single_header_stored</b> &mdash; <i>Single header stored</i></summary>

    * **Objective**: Single header stored
    * **Assertions**:
      * <code>Assert equal (1, (int)http_pool[0].header_count)</code>
      * <code>Assert not null (v)</code>
      * <code>Assert equal string ("hello", v)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_header_lookup_case_insensitive</b> &mdash; <i>Header lookup case insensitive</i></summary>

    * **Objective**: Header lookup case insensitive
    * **Assertions**:
      * <code>Assert not null (http_get_header(&http_pool[0], "content-type"))</code>
      * <code>Assert not null (http_get_header(&http_pool[0], "CONTENT-TYPE"))</code>
      * <code>Assert equal string ("application/json", http_get_header(&http_pool[0], "Content-Type"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_header_leading_space_stripped</b> &mdash; <i>Header leading space stripped</i></summary>

    * **Objective**: Header leading space stripped
    * **Assertions**:
      * <code>Assert not null (v)</code>
      * <code>Assert equal string ("trimmed", v)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_content_length_header_parsed</b> &mdash; <i>Content length header parsed</i></summary>

    * **Objective**: Content length header parsed
    * **Assertions**:
      * <code>Assert equal (5, (int)http_pool[0].content_length)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_content_length_in_headers_array</b> &mdash; <i>Content length in headers array</i></summary>

    * **Objective**: Content length in headers array
    * **Assertions**:
      * <code>Assert not null (cl)</code>
      * <code>Assert equal string ("3", cl)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_multiple_headers_stored</b> &mdash; <i>Multiple headers stored</i></summary>

    * **Objective**: Multiple headers stored
    * **Assertions**:
      * <code>Assert equal (3, (int)http_pool[0].header_count)</code>
      * <code>Assert equal string ("one", http_get_header(&http_pool[0], "X-A"))</code>
      * <code>Assert equal string ("two", http_get_header(&http_pool[0], "X-B"))</code>
      * <code>Assert equal string ("three", http_get_header(&http_pool[0], "X-C"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_missing_header_returns_null</b> &mdash; <i>Missing header returns null</i></summary>

    * **Objective**: Missing header returns null
    * **Assertions**:
      * <code>Assert null (http_get_header(&http_pool[0], "X-Missing"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_get_no_body_completes</b> &mdash; <i>Get no body completes</i></summary>

    * **Objective**: Get no body completes
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (0, (int)http_pool[0].body_len)</code>
      * <code>Assert equal ('\\0', (char)http_pool[0].body[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_post_with_body</b> &mdash; <i>Post with body</i></summary>

    * **Objective**: Post with body
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (5, (int)http_pool[0].body_len)</code>
      * <code>Assert equal string ("hello", (const char *)http_pool[0].body)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_put_with_body</b> &mdash; <i>Put with body</i></summary>

    * **Objective**: Put with body
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (7, (int)http_pool[0].body_len)</code>
      * <code>Assert equal string ("updated", (const char *)http_pool[0].body)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_body_starting_with_newline</b> &mdash; <i>Body starting with newline</i></summary>

    * **Objective**: Body starting with newline
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (5, (int)http_pool[0].body_len)</code>
      * <code>Assert equal ('\\n', (char)http_pool[0].body[0])</code>
      * <code>Assert equal string ("\\nabcd", (const char *)http_pool[0].body)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_post_content_length_zero</b> &mdash; <i>Post content length zero</i></summary>

    * **Objective**: Post content length zero
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (0, (int)http_pool[0].body_len)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_body_exactly_at_buffer_limit</b> &mdash; <i>Body exactly at buffer limit</i></summary>

    * **Objective**: Body exactly at buffer limit
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (BODY_BUF_SIZE, (int)http_pool[0].body_len)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_body_null_terminated_after_complete</b> &mdash; <i>Body null terminated after complete</i></summary>

    * **Objective**: Body null terminated after complete
    * **Assertions**:
      * <code>Assert equal ('\\0', (char)http_pool[0].body[3])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_body_one_over_limit_is_413</b> &mdash; <i>Body one over limit is 413</i></summary>

    * **Objective**: Body one over limit is 413
    * **Assertions**:
      * <code>Assert equal (PARSE_ENTITY_TOO_LARGE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_body_far_over_limit_is_413</b> &mdash; <i>Body far over limit is 413</i></summary>

    * **Objective**: Body far over limit is 413
    * **Assertions**:
      * <code>Assert equal (PARSE_ENTITY_TOO_LARGE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_413_no_body_bytes_fed</b> &mdash; <i>413 no body bytes fed</i></summary>

    * **Objective**: 413 no body bytes fed
    * **Assertions**:
      * <code>Assert equal (0, (int)http_pool[0].body_len)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_413_header_still_stored</b> &mdash; <i>413 header still stored</i></summary>

    * **Objective**: 413 header still stored
    * **Assertions**:
      * <code>Assert equal (PARSE_ENTITY_TOO_LARGE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("test", http_get_header(&http_pool[0], "X-Tag"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_body_exactly_at_limit_is_not_413</b> &mdash; <i>Body exactly at limit is not 413</i></summary>

    * **Objective**: Body exactly at limit is not 413
    * **Assertions**:
      * <code>Assert not equal (PARSE_ENTITY_TOO_LARGE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_path_overflow_stops_feeding</b> &mdash; <i>Path overflow stops feeding</i></summary>

    * **Objective**: Path overflow stops feeding
    * **Assertions**:
      * <code>Assert equal (PARSE_URI_TOO_LONG, http_pool[0].parse_state)</code>
      * <code>Assert equal (PARSE_URI_TOO_LONG, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_414_path_filled_to_capacity</b> &mdash; <i>414 path filled to capacity</i></summary>

    * **Objective**: 414 path filled to capacity
    * **Assertions**:
      * <code>Assert equal (PARSE_URI_TOO_LONG, http_pool[0].parse_state)</code>
      * <code>Assert equal ('/', http_pool[0].path[0])</code>
      * <code>Assert equal ('a', http_pool[0].path[1])</code>
      * <code>Assert equal (MAX_PATH_LEN - 1, (int)strlen(http_pool[0].path))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_method_nul_byte_is_error</b> &mdash; <i>Method nul byte is error</i></summary>

    * **Objective**: Method nul byte is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_method_control_char_is_error</b> &mdash; <i>Method control char is error</i></summary>

    * **Objective**: Method control char is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_method_del_byte_is_error</b> &mdash; <i>Method del byte is error</i></summary>

    * **Objective**: Method del byte is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_method_non_tchar_symbol_is_error</b> &mdash; <i>Method non tchar symbol is error</i></summary>

    * **Objective**: Method non tchar symbol is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_method_tchar_symbols_accepted</b> &mdash; <i>Method tchar symbols accepted</i></summary>

    * **Objective**: Method tchar symbols accepted
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("X-CMD", http_pool[0].method)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_path_nul_byte_is_error</b> &mdash; <i>Path nul byte is error</i></summary>

    * **Objective**: Path nul byte is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_path_control_char_is_error</b> &mdash; <i>Path control char is error</i></summary>

    * **Objective**: Path control char is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_path_del_byte_is_error</b> &mdash; <i>Path del byte is error</i></summary>

    * **Objective**: Path del byte is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_query_nul_byte_is_error</b> &mdash; <i>Query nul byte is error</i></summary>

    * **Objective**: Query nul byte is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_query_control_char_is_error</b> &mdash; <i>Query control char is error</i></summary>

    * **Objective**: Query control char is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_header_key_space_is_error</b> &mdash; <i>Header key space is error</i></summary>

    * **Objective**: Header key space is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_header_key_nul_byte_is_error</b> &mdash; <i>Header key nul byte is error</i></summary>

    * **Objective**: Header key nul byte is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_header_key_control_char_is_error</b> &mdash; <i>Header key control char is error</i></summary>

    * **Objective**: Header key control char is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_header_key_mid_cr_is_error</b> &mdash; <i>Header key mid cr is error</i></summary>

    * **Objective**: Header key mid cr is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_header_key_colon_at_start_skips_header</b> &mdash; <i>Header key colon at start skips header</i></summary>

    * **Objective**: Header key colon at start skips header
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_header_val_nul_byte_is_error</b> &mdash; <i>Header val nul byte is error</i></summary>

    * **Objective**: Header val nul byte is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_header_val_control_char_is_error</b> &mdash; <i>Header val control char is error</i></summary>

    * **Objective**: Header val control char is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_header_val_del_byte_is_error</b> &mdash; <i>Header val del byte is error</i></summary>

    * **Objective**: Header val del byte is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_header_val_htab_mid_value_allowed</b> &mdash; <i>Header val htab mid value allowed</i></summary>

    * **Objective**: Header val htab mid value allowed
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert not null (v)</code>
      * <code>Assert equal ('f', v[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_header_val_leading_htab_stripped</b> &mdash; <i>Header val leading htab stripped</i></summary>

    * **Objective**: Header val leading htab stripped
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert not null (v)</code>
      * <code>Assert equal string ("value", v)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_header_val_obs_text_allowed</b> &mdash; <i>Header val obs text allowed</i></summary>

    * **Objective**: Header val obs text allowed
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_version_http11_recognized</b> &mdash; <i>Version http11 recognized</i></summary>

    * **Objective**: Version http11 recognized
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (HTTP_11, http_pool[0].version)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_version_http10_recognized</b> &mdash; <i>Version http10 recognized</i></summary>

    * **Objective**: Version http10 recognized
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (HTTP_10, http_pool[0].version)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_version_unknown_is_http_unknown</b> &mdash; <i>Version unknown is http unknown</i></summary>

    * **Objective**: Version unknown is http unknown
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (HTTP_UNKNOWN, http_pool[0].version)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_version_reset_to_unknown</b> &mdash; <i>Version reset to unknown</i></summary>

    * **Objective**: Version reset to unknown
    * **Assertions**:
      * <code>Assert equal (HTTP_11, http_pool[0].version)</code>
      * <code>Assert equal (HTTP_UNKNOWN, http_pool[0].version)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_bad_expect_lf_is_error</b> &mdash; <i>Bad expect lf is error</i></summary>

    * **Objective**: Bad expect lf is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_blank_line_non_lf_is_error</b> &mdash; <i>Blank line non lf is error</i></summary>

    * **Objective**: Blank line non lf is error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_slots_are_independent</b> &mdash; <i>Slots are independent</i></summary>

    * **Objective**: Slots are independent
    * **Assertions**:
      * <code>Assert equal string ("GET", http_pool[0].method)</code>
      * <code>Assert equal string ("/slot0", http_pool[0].path)</code>
      * <code>Assert equal string ("POST", http_pool[1].method)</code>
      * <code>Assert equal string ("/slot1", http_pool[1].path)</code>
      * <code>Assert equal string ("data", (const char *)http_pool[1].body)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_incremental_byte_by_byte</b> &mdash; <i>Incremental byte by byte</i></summary>

    * **Objective**: Incremental byte by byte
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("/inc", http_pool[0].path)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_incremental_two_chunks</b> &mdash; <i>Incremental two chunks</i></summary>

    * **Objective**: Incremental two chunks
    * **Assertions**:
      * <code>Assert not equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("body", (const char *)http_pool[0].body)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_many_requests_same_slot</b> &mdash; <i>Stress - Many requests same slot</i></summary>

    * **Objective**: Stress - Many requests same slot
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_max_headers</b> &mdash; <i>Stress - Max headers</i></summary>

    * **Objective**: Stress - Max headers
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (MAX_HEADERS, (int)http_pool[0].header_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_max_query_params</b> &mdash; <i>Stress - Max query params</i></summary>

    * **Objective**: Stress - Max query params
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (MAX_QUERY_PARAMS, (int)http_pool[0].query_count)</code>

  </details>

</details>

<details>
<summary><b>test_multipart (19 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_no_content_type_returns_false</b> &mdash; <i>No content type returns false</i></summary>

    * **Objective**: No content type returns false
    * **Assertions**:
      * <code>Assert false (ok)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_no_boundary_in_content_type_returns_false</b> &mdash; <i>No boundary in content type returns false</i></summary>

    * **Objective**: No boundary in content type returns false
    * **Assertions**:
      * <code>Assert false (multipart_parse(&http_pool[0], &mp))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_body_missing_delimiter_returns_false</b> &mdash; <i>Body missing delimiter returns false</i></summary>

    * **Objective**: Body missing delimiter returns false
    * **Assertions**:
      * <code>Assert false (multipart_parse(req, &mp))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_single_text_field_parsed</b> &mdash; <i>Single text field parsed</i></summary>

    * **Objective**: Single text field parsed
    * **Assertions**:
      * <code>Assert true (multipart_parse(req, &mp))</code>
      * <code>Assert equal int (1, mp.part_count)</code>
      * <code>Assert not null (mp.parts[0].name)</code>
      * <code>Assert equal string ("field1", mp.parts[0].name)</code>
      * <code>Assert equal string ("value1", mp.parts[0].data)</code>
      * <code>Assert equal uint (6, mp.parts[0].data_len)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_two_text_fields_parsed</b> &mdash; <i>Two text fields parsed</i></summary>

    * **Objective**: Two text fields parsed
    * **Assertions**:
      * <code>Assert true (multipart_parse(req, &mp))</code>
      * <code>Assert equal int (2, mp.part_count)</code>
      * <code>Assert equal string ("username", mp.parts[0].name)</code>
      * <code>Assert equal string ("alice", mp.parts[0].data)</code>
      * <code>Assert equal string ("email", mp.parts[1].name)</code>
      * <code>Assert equal string ("alice@example.com", mp.parts[1].data)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_three_text_fields_parsed</b> &mdash; <i>Three text fields parsed</i></summary>

    * **Objective**: Three text fields parsed
    * **Assertions**:
      * <code>Assert true (multipart_parse(req, &mp))</code>
      * <code>Assert equal int (3, mp.part_count)</code>
      * <code>Assert equal string ("AAA", mp.parts[0].data)</code>
      * <code>Assert equal string ("BBB", mp.parts[1].data)</code>
      * <code>Assert equal string ("CCC", mp.parts[2].data)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_file_upload_part</b> &mdash; <i>File upload part</i></summary>

    * **Objective**: File upload part
    * **Assertions**:
      * <code>Assert true (multipart_parse(req, &mp))</code>
      * <code>Assert equal int (1, mp.part_count)</code>
      * <code>Assert not null (mp.parts[0].name)</code>
      * <code>Assert not null (mp.parts[0].filename)</code>
      * <code>Assert not null (mp.parts[0].type)</code>
      * <code>Assert equal string ("file", mp.parts[0].name)</code>
      * <code>Assert equal string ("test.txt", mp.parts[0].filename)</code>
      * <code>Assert equal string ("text/plain", mp.parts[0].type)</code>
      * <code>Assert equal string ("file contents here", mp.parts[0].data)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_file_upload_with_text_field</b> &mdash; <i>File upload with text field</i></summary>

    * **Objective**: File upload with text field
    * **Assertions**:
      * <code>Assert true (multipart_parse(req, &mp))</code>
      * <code>Assert equal int (2, mp.part_count)</code>
      * <code>Assert equal string ("desc", mp.parts[0].name)</code>
      * <code>Assert equal string ("my description", mp.parts[0].data)</code>
      * <code>Assert null (mp.parts[0].filename)</code>
      * <code>Assert equal string ("upload", mp.parts[1].name)</code>
      * <code>Assert equal string ("pic.jpg", mp.parts[1].filename)</code>
      * <code>Assert equal string ("image/jpeg", mp.parts[1].type)</code>
      * <code>Assert equal string ("JPEG_DATA", mp.parts[1].data)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_get_field_found</b> &mdash; <i>Get field found</i></summary>

    * **Objective**: Get field found
    * **Assertions**:
      * <code>Assert not null (val)</code>
      * <code>Assert equal string ("abc123", val)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_get_field_not_found_returns_null</b> &mdash; <i>Get field not found returns null</i></summary>

    * **Objective**: Get field not found returns null
    * **Assertions**:
      * <code>Assert null (multipart_get_field(&mp, "notexist"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_get_field_multiple_fields</b> &mdash; <i>Get field multiple fields</i></summary>

    * **Objective**: Get field multiple fields
    * **Assertions**:
      * <code>Assert equal string ("one", multipart_get_field(&mp, "first"))</code>
      * <code>Assert equal string ("two", multipart_get_field(&mp, "second"))</code>
      * <code>Assert null (multipart_get_field(&mp, "third"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_data_len_is_correct</b> &mdash; <i>Data len is correct</i></summary>

    * **Objective**: Data len is correct
    * **Assertions**:
      * <code>Assert true (multipart_parse(req, &mp))</code>
      * <code>Assert equal uint (strlen(data_str), mp.parts[0].data_len)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_max_parts_captured</b> &mdash; <i>Max parts captured</i></summary>

    * **Objective**: Max parts captured
    * **Assertions**:
      * <code>Assert true (multipart_parse(req, &mp))</code>
      * <code>Assert equal int (MAX_MULTIPART_PARTS, mp.part_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_empty_field_value</b> &mdash; <i>Empty field value</i></summary>

    * **Objective**: Empty field value
    * **Assertions**:
      * <code>Assert true (multipart_parse(req, &mp))</code>
      * <code>Assert equal int (1, mp.part_count)</code>
      * <code>Assert equal uint (0, mp.parts[0].data_len)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_part_without_filename_has_null_filename</b> &mdash; <i>Part without filename has null filename</i></summary>

    * **Objective**: Part without filename has null filename
    * **Assertions**:
      * <code>Assert null (mp.parts[0].filename)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_part_without_content_type_has_null_type</b> &mdash; <i>Part without content type has null type</i></summary>

    * **Objective**: Part without content type has null type
    * **Assertions**:
      * <code>Assert null (mp.parts[0].type)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_long_boundary_string</b> &mdash; <i>Long boundary string</i></summary>

    * **Objective**: Long boundary string
    * **Assertions**:
      * <code>Assert true (multipart_parse(req, &mp))</code>
      * <code>Assert equal string ("long_boundary_test", mp.parts[0].data)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_parse_100_requests</b> &mdash; <i>Stress - Parse 100 requests</i></summary>

    * **Objective**: Stress - Parse 100 requests
    * **Assertions**:
      * <code>Assert true message (multipart_parse(req, &mp), "parse failed")</code>
      * <code>Assert equal string message (val, mp.parts[0].data, "value mismatch")</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_get_field_100_lookups</b> &mdash; <i>Stress - Get field 100 lookups</i></summary>

    * **Objective**: Stress - Get field 100 lookups
    * **Assertions**:
      * <code>Assert not null message (v, "field not found")</code>
      * <code>Assert equal string message ("found_it", v, "wrong value")</code>
      * <code>Assert null message (multipart_get_field(&mp, "missing"), "expected null")</code>

  </details>

</details>

<details>
<summary><b>test_presentation (63 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_reset_sets_parse_state_to_method</b> &mdash; <i>Fn reset sets parse state to method</i></summary>

    * **Objective**: Fn reset sets parse state to method
    * **Assertions**:
      * <code>Assert equal (PARSE_METHOD, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_reset_sets_slot_id</b> &mdash; <i>Fn reset sets slot id</i></summary>

    * **Objective**: Fn reset sets slot id
    * **Assertions**:
      * <code>Assert equal (2, http_pool[2].slot_id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_reset_clears_method</b> &mdash; <i>Fn reset clears method</i></summary>

    * **Objective**: Fn reset clears method
    * **Assertions**:
      * <code>Assert equal ('\\0', http_pool[0].method[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_reset_clears_path_and_idx</b> &mdash; <i>Fn reset clears path and idx</i></summary>

    * **Objective**: Fn reset clears path and idx
    * **Assertions**:
      * <code>Assert equal ('\\0', http_pool[0].path[0])</code>
      * <code>Assert equal (0, (int)http_pool[0].path_idx)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_reset_clears_query_raw_and_params</b> &mdash; <i>Fn reset clears query raw and params</i></summary>

    * **Objective**: Fn reset clears query raw and params
    * **Assertions**:
      * <code>Assert equal ('\\0', http_pool[0].query[0])</code>
      * <code>Assert equal (0, (int)http_pool[0].query_idx)</code>
      * <code>Assert equal (0, http_pool[0].query_count)</code>
      * <code>Assert equal ('\\0', http_pool[0].query_params[0].key[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_reset_clears_all_header_slots</b> &mdash; <i>Fn reset clears all header slots</i></summary>

    * **Objective**: Fn reset clears all header slots
    * **Assertions**:
      * <code>Assert equal (0, http_pool[0].header_count)</code>
      * <code>Assert equal ('\\0', http_pool[0].headers[0].key[0])</code>
      * <code>Assert equal ('\\0', http_pool[0].headers[2].val[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_reset_clears_body_fields</b> &mdash; <i>Fn reset clears body fields</i></summary>

    * **Objective**: Fn reset clears body fields
    * **Assertions**:
      * <code>Assert equal ('\\0', http_pool[0].body[0])</code>
      * <code>Assert equal (0, (int)http_pool[0].body_len)</code>
      * <code>Assert equal (0, (int)http_pool[0].content_length)</code>
      * <code>Assert equal (0, (int)http_pool[0].body_bytes_read)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_reset_out_of_range_is_nop</b> &mdash; <i>Fn reset out of range is nop</i></summary>

    * **Objective**: Fn reset out of range is nop

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_reset_is_idempotent</b> &mdash; <i>Fn reset is idempotent</i></summary>

    * **Objective**: Fn reset is idempotent
    * **Assertions**:
      * <code>Assert equal (PARSE_METHOD, http_pool[0].parse_state)</code>
      * <code>Assert equal (0, http_pool[0].header_count)</code>
      * <code>Assert equal (0, (int)http_pool[0].body_len)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_header_null_when_no_headers</b> &mdash; <i>Fn get header null when no headers</i></summary>

    * **Objective**: Fn get header null when no headers
    * **Assertions**:
      * <code>Assert null (http_get_header(&http_pool[0], "Host"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_header_finds_single_header</b> &mdash; <i>Fn get header finds single header</i></summary>

    * **Objective**: Fn get header finds single header
    * **Assertions**:
      * <code>Assert not null (v)</code>
      * <code>Assert equal string ("esp32", v)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_header_finds_first_of_many</b> &mdash; <i>Fn get header finds first of many</i></summary>

    * **Objective**: Fn get header finds first of many
    * **Assertions**:
      * <code>Assert equal string ("first", http_get_header(&http_pool[0], "A"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_header_finds_middle_of_many</b> &mdash; <i>Fn get header finds middle of many</i></summary>

    * **Objective**: Fn get header finds middle of many
    * **Assertions**:
      * <code>Assert equal string ("mid", http_get_header(&http_pool[0], "B"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_header_finds_last_of_many</b> &mdash; <i>Fn get header finds last of many</i></summary>

    * **Objective**: Fn get header finds last of many
    * **Assertions**:
      * <code>Assert equal string ("last", http_get_header(&http_pool[0], "C"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_header_case_insensitive_lowercase</b> &mdash; <i>Fn get header case insensitive lowercase</i></summary>

    * **Objective**: Fn get header case insensitive lowercase
    * **Assertions**:
      * <code>Assert not null (http_get_header(&http_pool[0], "content-type"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_header_case_insensitive_uppercase</b> &mdash; <i>Fn get header case insensitive uppercase</i></summary>

    * **Objective**: Fn get header case insensitive uppercase
    * **Assertions**:
      * <code>Assert not null (http_get_header(&http_pool[0], "CONTENT-TYPE"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_header_returns_null_for_absent_key</b> &mdash; <i>Fn get header returns null for absent key</i></summary>

    * **Objective**: Fn get header returns null for absent key
    * **Assertions**:
      * <code>Assert null (http_get_header(&http_pool[0], "Authorization"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_header_does_not_bleed_across_slots</b> &mdash; <i>Fn get header does not bleed across slots</i></summary>

    * **Objective**: Fn get header does not bleed across slots
    * **Assertions**:
      * <code>Assert equal string ("alpha", http_get_header(&http_pool[0], "Host"))</code>
      * <code>Assert equal string ("beta", http_get_header(&http_pool[1], "Host"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_query_null_when_no_params</b> &mdash; <i>Fn get query null when no params</i></summary>

    * **Objective**: Fn get query null when no params
    * **Assertions**:
      * <code>Assert null (http_get_query(&http_pool[0], "key"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_query_finds_single_param</b> &mdash; <i>Fn get query finds single param</i></summary>

    * **Objective**: Fn get query finds single param
    * **Assertions**:
      * <code>Assert not null (v)</code>
      * <code>Assert equal string ("bar", v)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_query_finds_first_param</b> &mdash; <i>Fn get query finds first param</i></summary>

    * **Objective**: Fn get query finds first param
    * **Assertions**:
      * <code>Assert equal string ("1", http_get_query(&http_pool[0], "a"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_query_finds_middle_param</b> &mdash; <i>Fn get query finds middle param</i></summary>

    * **Objective**: Fn get query finds middle param
    * **Assertions**:
      * <code>Assert equal string ("mid", http_get_query(&http_pool[0], "b"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_query_finds_last_param</b> &mdash; <i>Fn get query finds last param</i></summary>

    * **Objective**: Fn get query finds last param
    * **Assertions**:
      * <code>Assert equal string ("end", http_get_query(&http_pool[0], "c"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_query_returns_null_for_absent_key</b> &mdash; <i>Fn get query returns null for absent key</i></summary>

    * **Objective**: Fn get query returns null for absent key
    * **Assertions**:
      * <code>Assert null (http_get_query(&http_pool[0], "z"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_query_empty_value</b> &mdash; <i>Fn get query empty value</i></summary>

    * **Objective**: Fn get query empty value
    * **Assertions**:
      * <code>Assert not null (v)</code>
      * <code>Assert equal string ("", v)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_get_query_does_not_bleed_across_slots</b> &mdash; <i>Fn get query does not bleed across slots</i></summary>

    * **Objective**: Fn get query does not bleed across slots
    * **Assertions**:
      * <code>Assert equal string ("slot0", http_get_query(&http_pool[0], "x"))</code>
      * <code>Assert equal string ("slot1", http_get_query(&http_pool[1], "x"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_get_parses_complete</b> &mdash; <i>Get parses complete</i></summary>

    * **Objective**: Get parses complete
    * **Assertions**:
      * <code>Assert equal string ("GET", http_pool[0].method)</code>
      * <code>Assert equal string ("/api/status", http_pool[0].path)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_post_body_stored</b> &mdash; <i>Post body stored</i></summary>

    * **Objective**: Post body stored
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[1].parse_state)</code>
      * <code>Assert equal string ("hello", (const char *)http_pool[1].body)</code>
      * <code>Assert equal (5, (int)http_pool[1].body_len)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_put_parses_complete</b> &mdash; <i>Put parses complete</i></summary>

    * **Objective**: Put parses complete
    * **Assertions**:
      * <code>Assert equal string ("PUT", http_pool[0].method)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_delete_parses_complete</b> &mdash; <i>Delete parses complete</i></summary>

    * **Objective**: Delete parses complete
    * **Assertions**:
      * <code>Assert equal string ("DELETE", http_pool[0].method)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_patch_parses_complete</b> &mdash; <i>Patch parses complete</i></summary>

    * **Objective**: Patch parses complete
    * **Assertions**:
      * <code>Assert equal string ("PATCH", http_pool[0].method)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_head_parses_complete</b> &mdash; <i>Head parses complete</i></summary>

    * **Objective**: Head parses complete
    * **Assertions**:
      * <code>Assert equal string ("HEAD", http_pool[0].method)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_query_single_param</b> &mdash; <i>Query single param</i></summary>

    * **Objective**: Query single param
    * **Assertions**:
      * <code>Assert equal (1, http_pool[0].query_count)</code>
      * <code>Assert equal string ("key", http_pool[0].query_params[0].key)</code>
      * <code>Assert equal string ("val", http_pool[0].query_params[0].val)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_query_multiple_params</b> &mdash; <i>Query multiple params</i></summary>

    * **Objective**: Query multiple params
    * **Assertions**:
      * <code>Assert equal (3, http_pool[0].query_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_body_null_terminated</b> &mdash; <i>Body null terminated</i></summary>

    * **Objective**: Body null terminated
    * **Assertions**:
      * <code>Assert equal ('\\0', http_pool[0].body[3])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_body_over_buf_size_is_413</b> &mdash; <i>Body over buf size is 413</i></summary>

    * **Objective**: Body over buf size is 413
    * **Assertions**:
      * <code>Assert equal (PARSE_ENTITY_TOO_LARGE, http_pool[0].parse_state)</code>
      * <code>Assert equal (0, (int)http_pool[0].body_len)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_overflow_method_sets_error</b> &mdash; <i>Overflow method sets error</i></summary>

    * **Objective**: Overflow method sets error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[3].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_overflow_path_sets_414</b> &mdash; <i>Overflow path sets 414</i></summary>

    * **Objective**: Overflow path sets 414
    * **Assertions**:
      * <code>Assert equal (PARSE_URI_TOO_LONG, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_bad_lf_after_cr_sets_error</b> &mdash; <i>Bad lf after cr sets error</i></summary>

    * **Objective**: Bad lf after cr sets error
    * **Assertions**:
      * <code>Assert equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_headers_beyond_max_are_dropped</b> &mdash; <i>Headers beyond max are dropped</i></summary>

    * **Objective**: Headers beyond max are dropped
    * **Assertions**:
      * <code>Assert equal (MAX_HEADERS, http_pool[0].header_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_query_params_beyond_max_are_dropped</b> &mdash; <i>Query params beyond max are dropped</i></summary>

    * **Objective**: Query params beyond max are dropped
    * **Assertions**:
      * <code>Assert equal (MAX_QUERY_PARAMS, http_pool[0].query_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_incremental_two_pushes_completes</b> &mdash; <i>Incremental two pushes completes</i></summary>

    * **Objective**: Incremental two pushes completes
    * **Assertions**:
      * <code>Assert not equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_body_starting_with_newline_stored</b> &mdash; <i>Body starting with newline stored</i></summary>

    * **Objective**: Body starting with newline stored
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (5, (int)http_pool[0].body_len)</code>
      * <code>Assert equal ('\\n', (char)http_pool[0].body[0])</code>
      * <code>Assert equal string ("\\nabcd", (const char *)http_pool[0].body)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_put_body_stored</b> &mdash; <i>Put body stored</i></summary>

    * **Objective**: Put body stored
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("PUT", http_pool[0].method)</code>
      * <code>Assert equal (7, (int)http_pool[0].body_len)</code>
      * <code>Assert equal string ("updated", (const char *)http_pool[0].body)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_content_length_header_stored_in_headers_array</b> &mdash; <i>Content length header stored in headers array</i></summary>

    * **Objective**: Content length header stored in headers array
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (3, (int)http_pool[0].content_length)</code>
      * <code>Assert not null (cl)</code>
      * <code>Assert equal string ("3", cl)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_parse_reset_100_cycles</b> &mdash; <i>Stress - Parse reset 100 cycles</i></summary>

    * **Objective**: Stress - Parse reset 100 cycles
    * **Assertions**:
      * <code>Assert equal message (PARSE_COMPLETE, http_pool[0].parse_state, "unexpected parse state mid-cycle")</code>
      * <code>Assert equal message (PARSE_METHOD, http_pool[0].parse_state, "state not reset")</code>
      * <code>Assert equal message (0, http_pool[0].header_count, "headers not reset")</code>
      * <code>Assert equal message ('\\0', http_pool[0].method[0], "method not reset")</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_all_slots_parse_simultaneously</b> &mdash; <i>Stress - All slots parse simultaneously</i></summary>

    * **Objective**: Stress - All slots parse simultaneously
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("GET", http_pool[0].method)</code>
      * <code>Assert equal string ("/zero", http_pool[0].path)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[1].parse_state)</code>
      * <code>Assert equal string ("POST", http_pool[1].method)</code>
      * <code>Assert equal string ("abc", (const char *)http_pool[1].body)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[2].parse_state)</code>
      * <code>Assert equal string ("PUT", http_pool[2].method)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[3].parse_state)</code>
      * <code>Assert equal string ("DELETE", http_pool[3].method)</code>
      * <code>Assert equal string ("/three", http_pool[3].path)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_method_at_max_7_chars_no_error</b> &mdash; <i>Stress - Method at max 7 chars no error</i></summary>

    * **Objective**: Stress - Method at max 7 chars no error
    * **Assertions**:
      * <code>Assert equal string ("OPTIONS", http_pool[0].method)</code>
      * <code>Assert not equal (PARSE_ERROR, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_path_at_exact_limit_no_error</b> &mdash; <i>Stress - Path at exact limit no error</i></summary>

    * **Objective**: Stress - Path at exact limit no error
    * **Assertions**:
      * <code>Assert not equal (PARSE_ERROR, http_pool[0].parse_state)</code>
      * <code>Assert equal (MAX_PATH_LEN - 1, (int)strlen(http_pool[0].path))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_body_exactly_buf_size_all_stored</b> &mdash; <i>Stress - Body exactly buf size all stored</i></summary>

    * **Objective**: Stress - Body exactly buf size all stored
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (BODY_BUF_SIZE, (int)http_pool[0].body_len)</code>
      * <code>Assert equal ('\\0', http_pool[0].body[BODY_BUF_SIZE])</code>
      * <code>Assert equal ('A', http_pool[0].body[0])</code>
      * <code>Assert equal ('Z', http_pool[0].body[25])</code>
      * <code>Assert equal ('A', http_pool[0].body[26])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_exactly_max_headers_all_stored</b> &mdash; <i>Stress - Exactly max headers all stored</i></summary>

    * **Objective**: Stress - Exactly max headers all stored
    * **Assertions**:
      * <code>Assert equal (MAX_HEADERS, http_pool[0].header_count)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("H8", http_pool[0].headers[7].key)</code>
      * <code>Assert equal string ("v8", http_pool[0].headers[7].val)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_exactly_max_query_params_all_stored</b> &mdash; <i>Stress - Exactly max query params all stored</i></summary>

    * **Objective**: Stress - Exactly max query params all stored
    * **Assertions**:
      * <code>Assert equal (MAX_QUERY_PARAMS, http_pool[0].query_count)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("h", http_pool[0].query_params[MAX_QUERY_PARAMS - 1].key)</code>
      * <code>Assert equal string ("8", http_pool[0].query_params[MAX_QUERY_PARAMS - 1].val)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_incremental_byte_by_byte_no_error</b> &mdash; <i>Stress - Incremental byte by byte no error</i></summary>

    * **Objective**: Stress - Incremental byte by byte no error
    * **Assertions**:
      * <code>TEST_ASSERT_NOT_EQUAL_MESSAGE(PARSE_ERROR, http_pool[0].parse_state,</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_sequential_requests_no_state_leak</b> &mdash; <i>Stress - Sequential requests no state leak</i></summary>

    * **Objective**: Stress - Sequential requests no state leak
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("GET", http_pool[0].method)</code>
      * <code>Assert equal (0, http_pool[0].header_count)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("POST", http_pool[0].method)</code>
      * <code>Assert equal string ("hi", (const char *)http_pool[0].body)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_interleaved_producer_consumer_ring_buffer</b> &mdash; <i>Race - Interleaved producer consumer ring buffer</i></summary>

    * **Objective**: Race - Interleaved producer consumer ring buffer
    * **Assertions**:
      * <code>Assert equal ((uint8_t)i, s-&gt;rx_buffer[s-&gt;rx_tail])</code>
      * <code>Assert equal ((uint8_t)i, s-&gt;rx_buffer[s-&gt;rx_tail])</code>
      * <code>Assert equal (s-&gt;rx_head, s-&gt;rx_tail)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_ring_buffer_full_prevents_write</b> &mdash; <i>Race - Ring buffer full prevents write</i></summary>

    * **Objective**: Race - Ring buffer full prevents write
    * **Assertions**:
      * <code>Assert equal (s-&gt;rx_tail, (s-&gt;rx_head + 1) % RX_BUF_SIZE)</code>
      * <code>Assert equal (RX_BUF_SIZE - 1, written)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_aba_slot_reuse_fresh_timestamp</b> &mdash; <i>Race - Aba slot reuse fresh timestamp</i></summary>

    * **Objective**: Race - Aba slot reuse fresh timestamp
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[0].state)</code>
      * <code>Assert equal (CONN_ACTIVE, conn_pool[0].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_double_free_is_nop</b> &mdash; <i>Race - Double free is nop</i></summary>

    * **Objective**: Race - Double free is nop
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[0].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_concurrent_slot_parse_isolation</b> &mdash; <i>Race - Concurrent slot parse isolation</i></summary>

    * **Objective**: Race - Concurrent slot parse isolation
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert not equal (PARSE_COMPLETE, http_pool[1].parse_state)</code>
      * <code>Assert not equal (PARSE_ERROR, http_pool[1].parse_state)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[1].parse_state)</code>
      * <code>Assert equal string ("POST", http_pool[1].method)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_reset_during_parse_header_val</b> &mdash; <i>Race - Reset during parse header val</i></summary>

    * **Objective**: Race - Reset during parse header val
    * **Assertions**:
      * <code>Assert equal (PARSE_HEADER_VAL, http_pool[0].parse_state)</code>
      * <code>Assert equal (PARSE_METHOD, http_pool[0].parse_state)</code>
      * <code>Assert equal (0, http_pool[0].header_count)</code>
      * <code>Assert equal ('\\0', http_pool[0].method[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_reset_during_parse_query</b> &mdash; <i>Race - Reset during parse query</i></summary>

    * **Objective**: Race - Reset during parse query
    * **Assertions**:
      * <code>Assert equal (PARSE_QUERY, http_pool[0].parse_state)</code>
      * <code>Assert equal (PARSE_METHOD, http_pool[0].parse_state)</code>
      * <code>Assert equal (0, (int)http_pool[0].query_idx)</code>
      * <code>Assert equal (0, http_pool[0].query_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_reset_during_parse_body</b> &mdash; <i>Race - Reset during parse body</i></summary>

    * **Objective**: Race - Reset during parse body
    * **Assertions**:
      * <code>Assert equal (PARSE_BODY, http_pool[0].parse_state)</code>
      * <code>Assert equal (PARSE_METHOD, http_pool[0].parse_state)</code>
      * <code>Assert equal (0, (int)http_pool[0].body_len)</code>
      * <code>Assert equal (0, (int)http_pool[0].body_bytes_read)</code>
      * <code>Assert equal ('\\0', http_pool[0].body[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_parse_after_complete_is_nop</b> &mdash; <i>Race - Parse after complete is nop</i></summary>

    * **Objective**: Race - Parse after complete is nop
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("GET", http_pool[0].method)</code>

  </details>

</details>

<details>
<summary><b>test_provisioning (5 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_plain_fields</b> &mdash; <i>Plain fields</i></summary>

    * **Objective**: Plain fields
    * **Assertions**:
      * <code>Assert true (detws_prov_form_field("ssid=MyAP&psk=secret", "ssid", v, sizeof(v)))</code>
      * <code>Assert equal string ("MyAP", v)</code>
      * <code>Assert true (detws_prov_form_field("ssid=MyAP&psk=secret", "psk", v, sizeof(v)))</code>
      * <code>Assert equal string ("secret", v)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_url_decoding</b> &mdash; <i>Url decoding</i></summary>

    * **Objective**: Url decoding
    * **Assertions**:
      * <code>Assert true (detws_prov_form_field("ssid=My+AP&psk=p%40ss%21", "ssid", v, sizeof(v)))</code>
      * <code>Assert equal string ("My AP", v)</code>
      * <code>Assert true (detws_prov_form_field("ssid=My+AP&psk=p%40ss%21", "psk", v, sizeof(v)))</code>
      * <code>Assert equal string ("p@ss!", v)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_missing_field</b> &mdash; <i>Missing field</i></summary>

    * **Objective**: Missing field
    * **Assertions**:
      * <code>Assert false (detws_prov_form_field("ssid=x", "psk", v, sizeof(v)))</code>
      * <code>Assert equal string ("", v)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_no_substring_match</b> &mdash; <i>No substring match</i></summary>

    * **Objective**: No substring match
    * **Assertions**:
      * <code>Assert true (detws_prov_form_field("myssid=wrong&ssid=right", "ssid", v, sizeof(v)))</code>
      * <code>Assert equal string ("right", v)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_capacity_bound</b> &mdash; <i>Capacity bound</i></summary>

    * **Objective**: Capacity bound
    * **Assertions**:
      * <code>Assert true (detws_prov_form_field("ssid=abcdef", "ssid", v, sizeof(v)))</code>
      * <code>Assert equal string ("abc", v)</code>

  </details>

</details>

<details>
<summary><b>test_session (19 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_empty_queue_does_not_crash</b> &mdash; <i>Empty queue does not crash</i></summary>

    * **Objective**: Empty queue does not crash

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_pool_initializes_to_parse_method</b> &mdash; <i>Pool initializes to parse method</i></summary>

    * **Objective**: Pool initializes to parse method
    * **Assertions**:
      * <code>Assert equal (PARSE_METHOD, http_pool[i].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_reset_clears_mid_parse_state</b> &mdash; <i>Reset clears mid parse state</i></summary>

    * **Objective**: Reset clears mid parse state
    * **Assertions**:
      * <code>Assert equal (PARSE_METHOD, http_pool[0].parse_state)</code>
      * <code>Assert equal (0, http_pool[0].header_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_tick_fires_check_timeouts_stale_slot_freed</b> &mdash; <i>Tick fires check timeouts stale slot freed</i></summary>

    * **Objective**: Tick fires check timeouts stale slot freed
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[0].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_tick_does_not_free_fresh_connection</b> &mdash; <i>Tick does not free fresh connection</i></summary>

    * **Objective**: Tick does not free fresh connection
    * **Assertions**:
      * <code>Assert equal (CONN_ACTIVE, conn_pool[0].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_tick_timeout_before_event_drain_ordering</b> &mdash; <i>Fn tick timeout before event drain ordering</i></summary>

    * **Objective**: Fn tick timeout before event drain ordering
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[1].state)</code>
      * <code>Assert equal (CONN_FREE, conn_pool[1].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_fn_tick_only_active_slots_expire</b> &mdash; <i>Fn tick only active slots expire</i></summary>

    * **Objective**: Fn tick only active slots expire
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[0].state)</code>
      * <code>Assert equal (CONN_FREE, conn_pool[1].state)</code>
      * <code>Assert equal (CONN_FREE, conn_pool[2].state)</code>
      * <code>Assert equal (CONN_ACTIVE, conn_pool[3].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_1000_idle_ticks_stable</b> &mdash; <i>Stress - 1000 idle ticks stable</i></summary>

    * **Objective**: Stress - 1000 idle ticks stable
    * **Assertions**:
      * <code>Assert equal (CONN_ACTIVE, conn_pool[i].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_timeout_all_slots_10_cycles</b> &mdash; <i>Stress - Timeout all slots 10 cycles</i></summary>

    * **Objective**: Stress - Timeout all slots 10 cycles
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[i].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_mixed_fresh_stale_slots_many_ticks</b> &mdash; <i>Stress - Mixed fresh stale slots many ticks</i></summary>

    * **Objective**: Stress - Mixed fresh stale slots many ticks
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[0].state)</code>
      * <code>Assert equal (CONN_FREE, conn_pool[1].state)</code>
      * <code>Assert equal (CONN_ACTIVE, conn_pool[2].state)</code>
      * <code>Assert equal (CONN_ACTIVE, conn_pool[3].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_evt_connect_calls_http_reset</b> &mdash; <i>Evt connect calls http reset</i></summary>

    * **Objective**: Evt connect calls http reset
    * **Assertions**:
      * <code>Assert equal (PARSE_METHOD, http_pool[1].parse_state)</code>
      * <code>Assert equal (0, http_pool[1].header_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_evt_disconnect_calls_http_reset</b> &mdash; <i>Evt disconnect calls http reset</i></summary>

    * **Objective**: Evt disconnect calls http reset
    * **Assertions**:
      * <code>Assert equal (PARSE_METHOD, http_pool[0].parse_state)</code>
      * <code>Assert equal (0, http_pool[0].header_count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_evt_error_calls_http_reset</b> &mdash; <i>Evt error calls http reset</i></summary>

    * **Objective**: Evt error calls http reset
    * **Assertions**:
      * <code>Assert equal (PARSE_METHOD, http_pool[2].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_evt_data_calls_http_parse</b> &mdash; <i>Evt data calls http parse</i></summary>

    * **Objective**: Evt data calls http parse
    * **Assertions**:
      * <code>Assert equal (PARSE_COMPLETE, http_pool[0].parse_state)</code>
      * <code>Assert equal string ("GET", http_pool[0].method)</code>
      * <code>Assert equal string ("/evt", http_pool[0].path)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_multiple_events_drained_in_one_tick</b> &mdash; <i>Multiple events drained in one tick</i></summary>

    * **Objective**: Multiple events drained in one tick
    * **Assertions**:
      * <code>Assert equal (PARSE_METHOD, http_pool[0].parse_state)</code>
      * <code>Assert equal (PARSE_COMPLETE, http_pool[1].parse_state)</code>
      * <code>Assert equal (PARSE_METHOD, http_pool[2].parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_external_free_between_ticks</b> &mdash; <i>Race - External free between ticks</i></summary>

    * **Objective**: Race - External free between ticks
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[0].state)</code>
      * <code>Assert equal (CONN_FREE, conn_pool[0].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_activity_update_saves_slot_from_timeout</b> &mdash; <i>Race - Activity update saves slot from timeout</i></summary>

    * **Objective**: Race - Activity update saves slot from timeout
    * **Assertions**:
      * <code>Assert equal (CONN_ACTIVE, conn_pool[0].state)</code>
      * <code>Assert equal (CONN_ACTIVE, conn_pool[0].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_all_expire_then_idle_tick</b> &mdash; <i>Race - All expire then idle tick</i></summary>

    * **Objective**: Race - All expire then idle tick
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[i].state)</code>
      * <code>Assert equal (CONN_FREE, conn_pool[i].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>race_millis_wraparound_no_spurious_timeout</b> &mdash; <i>Race - Millis wraparound no spurious timeout</i></summary>

    * **Objective**: Race - Millis wraparound no spurious timeout
    * **Assertions**:
      * <code>Assert equal (CONN_ACTIVE, conn_pool[0].state)</code>

  </details>

</details>

<details>
<summary><b>test_sse (37 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_pool_size</b> &mdash; <i>Sse pool size</i></summary>

    * **Objective**: Sse pool size
    * **Assertions**:
      * <code>Assert equal (2, MAX_SSE_CONNS)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_ids_match_indices_after_init</b> &mdash; <i>Sse ids match indices after init</i></summary>

    * **Objective**: Sse ids match indices after init
    * **Assertions**:
      * <code>Assert equal (i, (int)sse_pool[i].sse_id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_all_inactive_after_init</b> &mdash; <i>Sse all inactive after init</i></summary>

    * **Objective**: Sse all inactive after init
    * **Assertions**:
      * <code>Assert false (sse_pool[i].active)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_path_empty_after_init</b> &mdash; <i>Sse path empty after init</i></summary>

    * **Objective**: Sse path empty after init
    * **Assertions**:
      * <code>Assert equal ('\\0', sse_pool[i].path[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_alloc_returns_non_null</b> &mdash; <i>Sse alloc returns non null</i></summary>

    * **Objective**: Sse alloc returns non null
    * **Assertions**:
      * <code>Assert not null (sse_alloc(0, "/events"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_alloc_sets_active</b> &mdash; <i>Sse alloc sets active</i></summary>

    * **Objective**: Sse alloc sets active
    * **Assertions**:
      * <code>Assert true (sse-&gt;active)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_alloc_sets_slot_id</b> &mdash; <i>Sse alloc sets slot id</i></summary>

    * **Objective**: Sse alloc sets slot id
    * **Assertions**:
      * <code>Assert equal (0, (int)sse-&gt;slot_id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_alloc_stores_path</b> &mdash; <i>Sse alloc stores path</i></summary>

    * **Objective**: Sse alloc stores path
    * **Assertions**:
      * <code>Assert equal string ("/sensors", sse-&gt;path)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_alloc_stores_different_paths_per_slot</b> &mdash; <i>Sse alloc stores different paths per slot</i></summary>

    * **Objective**: Sse alloc stores different paths per slot
    * **Assertions**:
      * <code>Assert equal string ("/events", s0-&gt;path)</code>
      * <code>Assert equal string ("/metrics", s1-&gt;path)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_alloc_path_truncated_to_max</b> &mdash; <i>Sse alloc path truncated to max</i></summary>

    * **Objective**: Sse alloc path truncated to max
    * **Assertions**:
      * <code>Assert not null (sse)</code>
      * <code>Assert equal (MAX_PATH_LEN - 1, (int)strlen(sse-&gt;path))</code>
      * <code>Assert equal ('\\0', sse-&gt;path[MAX_PATH_LEN - 1])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_alloc_pool_full_returns_null</b> &mdash; <i>Sse alloc pool full returns null</i></summary>

    * **Objective**: Sse alloc pool full returns null
    * **Assertions**:
      * <code>Assert not null (sse_alloc(0, "/a"))</code>
      * <code>Assert not null (sse_alloc(1, "/b"))</code>
      * <code>Assert null (sse_alloc(2, "/c"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_alloc_sse_id_is_pool_index</b> &mdash; <i>Sse alloc sse id is pool index</i></summary>

    * **Objective**: Sse alloc sse id is pool index
    * **Assertions**:
      * <code>Assert equal (0, (int)s0-&gt;sse_id)</code>
      * <code>Assert equal (1, (int)s1-&gt;sse_id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_find_returns_correct_conn</b> &mdash; <i>Sse find returns correct conn</i></summary>

    * **Objective**: Sse find returns correct conn
    * **Assertions**:
      * <code>Assert not null (found)</code>
      * <code>Assert equal ptr (allocated, found)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_find_returns_null_when_empty</b> &mdash; <i>Sse find returns null when empty</i></summary>

    * **Objective**: Sse find returns null when empty
    * **Assertions**:
      * <code>Assert null (sse_find(0))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_find_returns_null_for_different_slot</b> &mdash; <i>Sse find returns null for different slot</i></summary>

    * **Objective**: Sse find returns null for different slot
    * **Assertions**:
      * <code>Assert null (sse_find(1))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_find_after_both_slots_allocated</b> &mdash; <i>Sse find after both slots allocated</i></summary>

    * **Objective**: Sse find after both slots allocated
    * **Assertions**:
      * <code>Assert not null (sse_find(0))</code>
      * <code>Assert not null (sse_find(1))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_find_checks_slot_id_not_sse_id</b> &mdash; <i>Sse find checks slot id not sse id</i></summary>

    * **Objective**: Sse find checks slot id not sse id
    * **Assertions**:
      * <code>Assert null (sse_find(0))</code>
      * <code>Assert not null (sse_find(3))</code>
      * <code>Assert equal ptr (sse, sse_find(3))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_free_deactivates_slot</b> &mdash; <i>Sse free deactivates slot</i></summary>

    * **Objective**: Sse free deactivates slot
    * **Assertions**:
      * <code>Assert false (sse_pool[0].active)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_free_restores_sse_id</b> &mdash; <i>Sse free restores sse id</i></summary>

    * **Objective**: Sse free restores sse id
    * **Assertions**:
      * <code>Assert equal (0, (int)sse_pool[0].sse_id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_free_makes_slot_findable_as_null</b> &mdash; <i>Sse free makes slot findable as null</i></summary>

    * **Objective**: Sse free makes slot findable as null
    * **Assertions**:
      * <code>Assert null (sse_find(0))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_free_clears_path</b> &mdash; <i>Sse free clears path</i></summary>

    * **Objective**: Sse free clears path
    * **Assertions**:
      * <code>Assert equal ('\\0', sse_pool[0].path[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_free_nop_on_unallocated</b> &mdash; <i>Sse free nop on unallocated</i></summary>

    * **Objective**: Sse free nop on unallocated
    * **Assertions**:
      * <code>Assert false (sse_pool[0].active)</code>
      * <code>Assert false (sse_pool[1].active)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_alloc_after_free_succeeds</b> &mdash; <i>Sse alloc after free succeeds</i></summary>

    * **Objective**: Sse alloc after free succeeds
    * **Assertions**:
      * <code>Assert not null (sse)</code>
      * <code>Assert true (sse-&gt;active)</code>
      * <code>Assert equal string ("/new", sse-&gt;path)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_free_only_frees_matching_slot</b> &mdash; <i>Sse free only frees matching slot</i></summary>

    * **Objective**: Sse free only frees matching slot
    * **Assertions**:
      * <code>Assert false (sse_pool[0].active)</code>
      * <code>Assert true (sse_pool[1].active)</code>
      * <code>Assert equal string ("/b", sse_pool[1].path)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_write_null_data_returns_false</b> &mdash; <i>Sse write null data returns false</i></summary>

    * **Objective**: Sse write null data returns false
    * **Assertions**:
      * <code>Assert false (sse_write(sse, nullptr, nullptr, nullptr))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_write_returns_false_when_conn_not_active</b> &mdash; <i>Sse write returns false when conn not active</i></summary>

    * **Objective**: Sse write returns false when conn not active
    * **Assertions**:
      * <code>Assert false (sse_write(sse, "hello", nullptr, nullptr))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_write_returns_false_when_pcb_null</b> &mdash; <i>Sse write returns false when pcb null</i></summary>

    * **Objective**: Sse write returns false when pcb null
    * **Assertions**:
      * <code>Assert false (sse_write(sse, "data", nullptr, nullptr))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_write_data_only_returns_true</b> &mdash; <i>Sse write data only returns true</i></summary>

    * **Objective**: Sse write data only returns true
    * **Assertions**:
      * <code>Assert true (sse_write(sse, "hello", nullptr, nullptr))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_write_with_event_returns_true</b> &mdash; <i>Sse write with event returns true</i></summary>

    * **Objective**: Sse write with event returns true
    * **Assertions**:
      * <code>Assert true (sse_write(sse, "payload", "update", nullptr))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_write_with_id_returns_true</b> &mdash; <i>Sse write with id returns true</i></summary>

    * **Objective**: Sse write with id returns true
    * **Assertions**:
      * <code>Assert true (sse_write(sse, "payload", nullptr, "42"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_write_with_all_fields_returns_true</b> &mdash; <i>Sse write with all fields returns true</i></summary>

    * **Objective**: Sse write with all fields returns true
    * **Assertions**:
      * <code>Assert true (sse_write(sse, "body", "status", "1"))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sse_write_does_not_affect_other_slots</b> &mdash; <i>Sse write does not affect other slots</i></summary>

    * **Objective**: Sse write does not affect other slots
    * **Assertions**:
      * <code>Assert true (s1-&gt;active)</code>
      * <code>Assert equal string ("/b", s1-&gt;path)</code>
      * <code>Assert equal (1, (int)s1-&gt;slot_id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_sse_alloc_free_100_cycles</b> &mdash; <i>Stress - Sse alloc free 100 cycles</i></summary>

    * **Objective**: Stress - Sse alloc free 100 cycles
    * **Assertions**:
      * <code>Assert not null message (sse, "alloc failed")</code>
      * <code>Assert true message (sse-&gt;active, "not active")</code>
      * <code>Assert equal string message ("/events", sse-&gt;path, "path wrong")</code>
      * <code>Assert false message (sse_pool[0].active, "still active after free")</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_sse_alloc_free_both_slots_alternating</b> &mdash; <i>Stress - Sse alloc free both slots alternating</i></summary>

    * **Objective**: Stress - Sse alloc free both slots alternating
    * **Assertions**:
      * <code>Assert not null (s0)</code>
      * <code>Assert not null (s1)</code>
      * <code>Assert null (sse_alloc(2, "/c"))</code>
      * <code>Assert not null (s1b)</code>
      * <code>Assert equal string ("/new", s1b-&gt;path)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_sse_write_100_calls</b> &mdash; <i>Stress - Sse write 100 calls</i></summary>

    * **Objective**: Stress - Sse write 100 calls
    * **Assertions**:
      * <code>Assert true message (ok, "write failed")</code>
      * <code>Assert true (sse-&gt;active)</code>
      * <code>Assert equal (0, (int)sse-&gt;slot_id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_sse_find_with_full_pool</b> &mdash; <i>Stress - Sse find with full pool</i></summary>

    * **Objective**: Stress - Sse find with full pool
    * **Assertions**:
      * <code>Assert equal ptr (s0, sse_find(0))</code>
      * <code>Assert equal ptr (s1, sse_find(1))</code>
      * <code>Assert null (sse_find(2))</code>
      * <code>Assert null (sse_find(3))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_sse_write_slot_isolation</b> &mdash; <i>Stress - Sse write slot isolation</i></summary>

    * **Objective**: Stress - Sse write slot isolation
    * **Assertions**:
      * <code>Assert equal string ("/metrics", s1-&gt;path)</code>
      * <code>Assert true (s1-&gt;active)</code>
      * <code>Assert equal (1, (int)s1-&gt;slot_id)</code>
      * <code>Assert equal (1, (int)s1-&gt;sse_id)</code>

  </details>

</details>

<details>
<summary><b>test_ssh_auth (12 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_service_request_accept</b> &mdash; <i>Service request accept</i></summary>

    * **Objective**: Service request accept
    * **Assertions**:
      * <code>Assert equal int (0, ssh_auth_handle_service_request(pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_SERVICE_ACCEPT, out[0])</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(12, sl);</code>
      * <code>Assert equal memory ("ssh-userauth", out + 5, 12)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_service_request_rejects_unknown</b> &mdash; <i>Service request rejects unknown</i></summary>

    * **Objective**: Service request rejects unknown
    * **Assertions**:
      * <code>Assert equal int (-1, ssh_auth_handle_service_request(pkt, n, out, &olen, sizeof(out)))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_parse_password_request</b> &mdash; <i>Parse password request</i></summary>

    * **Objective**: Parse password request
    * **Assertions**:
      * <code>Assert equal int (0, ssh_auth_parse_request(pkt, n, &req))</code>
      * <code>Assert true (req.is_password)</code>
      * <code>Assert equal string ("alice", req.user)</code>
      * <code>Assert equal string ("ssh-connection", req.service)</code>
      * <code>Assert equal string ("password", req.method)</code>
      * <code>Assert equal string ("s3cret", req.password)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_parse_none_request</b> &mdash; <i>Parse none request</i></summary>

    * **Objective**: Parse none request
    * **Assertions**:
      * <code>Assert equal int (0, ssh_auth_parse_request(pkt, n, &req))</code>
      * <code>Assert false (req.is_password)</code>
      * <code>Assert equal string ("bob", req.user)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_handle_request_success</b> &mdash; <i>Handle request success</i></summary>

    * **Objective**: Handle request success
    * **Assertions**:
      * <code>Assert equal int (0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_USERAUTH_SUCCESS, out[0])</code>
      * <code>Assert true (ssh_sess[0].authed)</code>
      * <code>Assert equal (SSH_PHASE_OPEN, ssh_sess[0].phase)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_handle_request_wrong_password_fails</b> &mdash; <i>Handle request wrong password fails</i></summary>

    * **Objective**: Handle request wrong password fails
    * **Assertions**:
      * <code>Assert equal int (0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_USERAUTH_FAILURE, out[0])</code>
      * <code>Assert false (ssh_sess[0].authed)</code>
      * <code>Assert true (adv)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_handle_none_request_fails_without_auth</b> &mdash; <i>Handle none request fails without auth</i></summary>

    * **Objective**: Handle none request fails without auth
    * **Assertions**:
      * <code>Assert equal int (0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_USERAUTH_FAILURE, out[0])</code>
      * <code>Assert false (ssh_sess[0].authed)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_handle_request_no_callback_fails</b> &mdash; <i>Handle request no callback fails</i></summary>

    * **Objective**: Handle request no callback fails
    * **Assertions**:
      * <code>Assert equal int (0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_USERAUTH_FAILURE, out[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_pubkey_probe_returns_pk_ok</b> &mdash; <i>Pubkey probe returns pk ok</i></summary>

    * **Objective**: Pubkey probe returns pk ok
    * **Assertions**:
      * <code>Assert equal int (0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_USERAUTH_PK_OK, out[0])</code>
      * <code>Assert false (ssh_sess[0].authed)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_pubkey_valid_signature_succeeds</b> &mdash; <i>Pubkey valid signature succeeds</i></summary>

    * **Objective**: Pubkey valid signature succeeds
    * **Assertions**:
      * <code>Assert equal int (0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_USERAUTH_SUCCESS, out[0])</code>
      * <code>Assert true (ssh_sess[0].authed)</code>
      * <code>Assert equal (SSH_PHASE_OPEN, ssh_sess[0].phase)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_pubkey_tampered_signature_fails</b> &mdash; <i>Pubkey tampered signature fails</i></summary>

    * **Objective**: Pubkey tampered signature fails
    * **Assertions**:
      * <code>Assert equal int (0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_USERAUTH_FAILURE, out[0])</code>
      * <code>Assert false (ssh_sess[0].authed)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_pubkey_unauthorized_key_fails</b> &mdash; <i>Pubkey unauthorized key fails</i></summary>

    * **Objective**: Pubkey unauthorized key fails
    * **Assertions**:
      * <code>Assert equal int (0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_USERAUTH_FAILURE, out[0])</code>

  </details>

</details>

<details>
<summary><b>test_ssh_channel (12 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_open_session_confirms</b> &mdash; <i>Open session confirms</i></summary>

    * **Objective**: Open session confirms
    * **Assertions**:
      * <code>Assert equal int (0, ssh_channel_handle_open(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_CHANNEL_OPEN_CONFIRM, out[0])</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(42, rd_u32(out + 1)); // recipient = client's sender</code>
      * <code>Assert true (ssh_chan[0].open)</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(42, ssh_chan[0].peer_id);</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(1000, ssh_chan[0].peer_window);</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_open_non_session_fails</b> &mdash; <i>Open non session fails</i></summary>

    * **Objective**: Open non session fails
    * **Assertions**:
      * <code>Assert equal int (0, ssh_channel_handle_open(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_CHANNEL_OPEN_FAILURE, out[0])</code>
      * <code>Assert false (ssh_chan[0].open)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_shell_request_success_with_reply</b> &mdash; <i>Shell request success with reply</i></summary>

    * **Objective**: Shell request success with reply
    * **Assertions**:
      * <code>Assert equal int (0, ssh_channel_handle_request(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_CHANNEL_SUCCESS, out[0])</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(5, rd_u32(out + 1));</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_unknown_request_failure</b> &mdash; <i>Unknown request failure</i></summary>

    * **Objective**: Unknown request failure
    * **Assertions**:
      * <code>Assert equal int (0, ssh_channel_handle_request(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_CHANNEL_FAILURE, out[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_request_no_reply_produces_nothing</b> &mdash; <i>Request no reply produces nothing</i></summary>

    * **Objective**: Request no reply produces nothing
    * **Assertions**:
      * <code>Assert equal int (0, ssh_channel_handle_request(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(0, (uint32_t)olen);</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_inbound_data_invokes_callback</b> &mdash; <i>Inbound data invokes callback</i></summary>

    * **Objective**: Inbound data invokes callback
    * **Assertions**:
      * <code>Assert equal int (0, ssh_channel_handle_data(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal int (1, data_cb_count)</code>
      * <code>Assert equal int (5, (int)last_data_len)</code>
      * <code>Assert equal memory ("hello", last_data, 5)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_inbound_data_window_replenish</b> &mdash; <i>Inbound data window replenish</i></summary>

    * **Objective**: Inbound data window replenish
    * **Assertions**:
      * <code>Assert equal int (0, ssh_channel_handle_data(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_CHANNEL_WINDOW_ADJUST, out[0])</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(5, rd_u32(out + 1)); // peer channel</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(SSH_CHAN_WINDOW, ssh_chan[0].local_window);</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_inbound_data_exceeding_window_rejected</b> &mdash; <i>Inbound data exceeding window rejected</i></summary>

    * **Objective**: Inbound data exceeding window rejected
    * **Assertions**:
      * <code>Assert equal int (-1, ssh_channel_handle_data(0, pkt, n, out, &olen, sizeof(out)))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_outbound_data_frames_and_decrements_window</b> &mdash; <i>Outbound data frames and decrements window</i></summary>

    * **Objective**: Outbound data frames and decrements window
    * **Assertions**:
      * <code>Assert equal int (0, ssh_channel_build_data(0, (const uint8_t *)"abc", 3, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_CHANNEL_DATA, out[0])</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(5, rd_u32(out + 1)); // peer channel</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(3, rd_u32(out + 5)); // data length</code>
      * <code>Assert equal memory ("abc", out + 9, 3)</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(997, ssh_chan[0].peer_window);</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_outbound_data_exceeding_peer_window_rejected</b> &mdash; <i>Outbound data exceeding peer window rejected</i></summary>

    * **Objective**: Outbound data exceeding peer window rejected
    * **Assertions**:
      * <code>Assert equal int (-1, ssh_channel_build_data(0, (const uint8_t *)"abc", 3, out, &olen, sizeof(out)))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_window_adjust_grows_peer_window</b> &mdash; <i>Window adjust grows peer window</i></summary>

    * **Objective**: Window adjust grows peer window
    * **Assertions**:
      * <code>Assert equal int (0, ssh_channel_handle_window_adjust(0, pkt, sizeof(pkt)))</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(600, ssh_chan[0].peer_window);</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_build_close_emits_eof_and_close</b> &mdash; <i>Build close emits eof and close</i></summary>

    * **Objective**: Build close emits eof and close
    * **Assertions**:
      * <code>Assert equal int (0, ssh_channel_build_close(0, out, &olen, sizeof(out)))</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(10, (uint32_t)olen);</code>
      * <code>Assert equal (SSH_MSG_CHANNEL_EOF, out[0])</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(5, rd_u32(out + 1));</code>
      * <code>Assert equal (SSH_MSG_CHANNEL_CLOSE, out[5])</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(5, rd_u32(out + 6));</code>
      * <code>Assert false (ssh_chan[0].open)</code>

  </details>

</details>

<details>
<summary><b>test_ssh_conn (2 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_accept_sends_server_banner</b> &mdash; <i>Accept sends server banner</i></summary>

    * **Objective**: Accept sends server banner
    * **Assertions**:
      * <code>Assert true (tcp_captured_len() &gt;= 8)</code>
      * <code>Assert equal memory ("SSH-2.0-", resp, 8)</code>
      * <code>Assert not equal (SSH_ID_NONE, conn_pool[0].ssh_id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_banner_then_kexinit_advances_and_replies</b> &mdash; <i>Banner then kexinit advances and replies</i></summary>

    * **Objective**: Banner then kexinit advances and replies
    * **Assertions**:
      * <code>Assert equal string ("SSH-2.0-TestClient", ssh_sess[j].v_c)</code>
      * <code>Assert equal (SSH_PHASE_DH_INIT, ssh_sess[j].phase)</code>
      * <code>Assert true (tcp_captured_len() &gt; 0)</code>

  </details>

</details>

<details>
<summary><b>test_ssh_hardening (2 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_password_refused_even_with_correct_callback</b> &mdash; <i>Password refused even with correct callback</i></summary>

    * **Objective**: Password refused even with correct callback
    * **Assertions**:
      * <code>Assert equal int (0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_USERAUTH_FAILURE, out[0])</code>
      * <code>Assert false (ssh_sess[0].authed)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_failure_advertises_publickey_only</b> &mdash; <i>Failure advertises publickey only</i></summary>

    * **Objective**: Failure advertises publickey only
    * **Assertions**:
      * <code>Assert equal int (0, ssh_auth_build_failure(out, &olen, sizeof(out), false))</code>
      * <code>Assert true (has_pk)</code>
      * <code>Assert false (has_pw)</code>

  </details>

</details>

<details>
<summary><b>test_ssh_server (7 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_full_handshake_to_channel_data</b> &mdash; <i>Full handshake to channel data</i></summary>

    * **Objective**: Full handshake to channel data
    * **Assertions**:
      * <code>Assert equal int (0, ssh_server_dispatch(0, pkt[0], pkt, n))</code>
      * <code>Assert equal int (1, emt_n)</code>
      * <code>Assert equal (SSH_MSG_KEXINIT, emt_type[0])</code>
      * <code>Assert equal (SSH_PHASE_DH_INIT, s-&gt;phase)</code>
      * <code>Assert equal int (0, ssh_server_dispatch(0, pkt[0], pkt, n))</code>
      * <code>Assert equal int (2, emt_n)</code>
      * <code>Assert equal (SSH_MSG_KEXDH_REPLY, emt_type[0])</code>
      * <code>Assert equal (SSH_MSG_NEWKEYS, emt_type[1])</code>
      * <code>Assert true (ssh_keys[0].active)</code>
      * <code>Assert equal int (0, ssh_server_dispatch(0, nk, &nk, 1))</code>
      * <code>Assert true (ssh_pkt[0].encrypted)</code>
      * <code>Assert equal (SSH_PHASE_SERVICE, s-&gt;phase)</code>
      * <code>Assert equal int (0, ssh_server_dispatch(0, pkt[0], pkt, n))</code>
      * <code>Assert equal (SSH_MSG_SERVICE_ACCEPT, emt_type[0])</code>
      * <code>Assert equal (SSH_PHASE_AUTH, s-&gt;phase)</code>
      * <code>Assert equal int (0, ssh_server_dispatch(0, pkt[0], pkt, n))</code>
      * <code>Assert equal (SSH_MSG_USERAUTH_SUCCESS, emt_type[0])</code>
      * <code>Assert true (s-&gt;authed)</code>
      * <code>Assert equal (SSH_PHASE_OPEN, s-&gt;phase)</code>
      * <code>Assert equal int (0, ssh_server_dispatch(0, pkt[0], pkt, n))</code>
      * <code>Assert equal (SSH_MSG_CHANNEL_OPEN_CONFIRM, emt_type[0])</code>
      * <code>Assert true (ssh_chan[0].open)</code>
      * <code>Assert equal int (0, ssh_server_dispatch(0, pkt[0], pkt, n))</code>
      * <code>Assert equal (SSH_MSG_CHANNEL_SUCCESS, emt_type[0])</code>
      * <code>Assert equal int (0, ssh_server_dispatch(0, pkt[0], pkt, n))</code>
      * <code>Assert equal int (1, chan_data_count)</code>
      * <code>Assert equal int (2, (int)chan_data_len)</code>
      * <code>Assert equal memory ("hi", chan_data, 2)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_channel_open_before_auth_rejected</b> &mdash; <i>Channel open before auth rejected</i></summary>

    * **Objective**: Channel open before auth rejected
    * **Assertions**:
      * <code>Assert equal int (-1, ssh_server_dispatch(0, pkt[0], pkt, n))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_disconnect_closes</b> &mdash; <i>Disconnect closes</i></summary>

    * **Objective**: Disconnect closes
    * **Assertions**:
      * <code>Assert equal int (-1, ssh_server_dispatch(0, pkt[0], pkt, 1))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ignore_is_noop</b> &mdash; <i>Ignore is noop</i></summary>

    * **Objective**: Ignore is noop
    * **Assertions**:
      * <code>Assert equal int (0, ssh_server_dispatch(0, pkt[0], pkt, 1))</code>
      * <code>Assert equal int (0, emt_n)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_auth_bruteforce_disconnect</b> &mdash; <i>Auth bruteforce disconnect</i></summary>

    * **Objective**: Auth bruteforce disconnect
    * **Assertions**:
      * <code>Assert equal int (0, ssh_server_dispatch(0, pkt[0], pkt, n))</code>
      * <code>Assert equal (SSH_MSG_USERAUTH_FAILURE, emt_type[0])</code>
      * <code>Assert false (s-&gt;authed)</code>
      * <code>Assert equal int (SSH_MAX_AUTH_ATTEMPTS - 1, s-&gt;auth_failures)</code>
      * <code>Assert equal int (-1, ssh_server_dispatch(0, pkt[0], pkt, n))</code>
      * <code>Assert equal int (2, emt_n)</code>
      * <code>Assert equal (SSH_MSG_USERAUTH_FAILURE, emt_type[0])</code>
      * <code>Assert equal (SSH_MSG_DISCONNECT, emt_type[1])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_auth_success_after_failures</b> &mdash; <i>Auth success after failures</i></summary>

    * **Objective**: Auth success after failures
    * **Assertions**:
      * <code>Assert equal int (0, ssh_server_dispatch(0, pkt[0], pkt, n))</code>
      * <code>Assert equal (SSH_MSG_USERAUTH_FAILURE, emt_type[0])</code>
      * <code>Assert equal int (0, ssh_server_dispatch(0, pkt[0], pkt, n))</code>
      * <code>Assert equal (SSH_MSG_USERAUTH_SUCCESS, emt_type[0])</code>
      * <code>Assert true (s-&gt;authed)</code>
      * <code>Assert equal int (1, s-&gt;auth_failures)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_unimplemented_reply_for_unknown_message</b> &mdash; <i>Unimplemented reply for unknown message</i></summary>

    * **Objective**: Unimplemented reply for unknown message
    * **Assertions**:
      * <code>Assert equal int (0, ssh_server_dispatch(0, pkt[0], pkt, 1))</code>
      * <code>Assert equal (SSH_MSG_UNIMPLEMENTED, emt_type[0])</code>
      * <code>Assert equal int (5, (int)emt_last_len)</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(6, seq); // rejected packet = seq_no_recv - 1</code>

  </details>

</details>

<details>
<summary><b>test_ssh_transport (23 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_server_banner_format</b> &mdash; <i>Server banner format</i></summary>

    * **Objective**: Server banner format
    * **Assertions**:
      * <code>Assert equal int (0, ssh_transport_server_banner(buf, &n, sizeof(buf)))</code>
      * <code>Assert true (n &gt; 8)</code>
      * <code>Assert equal memory ("SSH-2.0-", buf, 8)</code>
      * <code>Assert equal ('\\r', buf[n - 2])</code>
      * <code>Assert equal ('\\n', buf[n - 1])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_recv_banner_complete</b> &mdash; <i>Recv banner complete</i></summary>

    * **Objective**: Recv banner complete
    * **Assertions**:
      * <code>Assert equal int (1, rc)</code>
      * <code>TEST_ASSERT_EQUAL_size_t(strlen(banner), consumed);</code>
      * <code>Assert equal string ("SSH-2.0-OpenSSH_9.6", ssh_sess[0].v_c)</code>
      * <code>Assert equal (SSH_PHASE_KEXINIT, ssh_sess[0].phase)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_recv_banner_bare_lf</b> &mdash; <i>Recv banner bare lf</i></summary>

    * **Objective**: Recv banner bare lf
    * **Assertions**:
      * <code>Assert equal int (1, ssh_transport_recv_banner(0, (const uint8_t *)banner, strlen(banner), &consumed))</code>
      * <code>Assert equal string ("SSH-2.0-x", ssh_sess[0].v_c)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_recv_banner_split_across_reads</b> &mdash; <i>Recv banner split across reads</i></summary>

    * **Objective**: Recv banner split across reads
    * **Assertions**:
      * <code>Assert equal int (0, ssh_transport_recv_banner(0, (const uint8_t *)p1, strlen(p1), &consumed))</code>
      * <code>Assert equal int (1, ssh_transport_recv_banner(0, (const uint8_t *)p2, strlen(p2), &consumed))</code>
      * <code>Assert equal string ("SSH-2.0-Client_1", ssh_sess[0].v_c)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_recv_banner_skips_preamble_lines</b> &mdash; <i>Recv banner skips preamble lines</i></summary>

    * **Objective**: Recv banner skips preamble lines
    * **Assertions**:
      * <code>Assert equal int (1, ssh_transport_recv_banner(0, (const uint8_t *)data, strlen(data), &consumed))</code>
      * <code>Assert equal string ("SSH-2.0-Real", ssh_sess[0].v_c)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_kexinit_build_starts_with_msg_and_stores_is</b> &mdash; <i>Kexinit build starts with msg and stores is</i></summary>

    * **Objective**: Kexinit build starts with msg and stores is
    * **Assertions**:
      * <code>Assert equal int (0, ssh_kexinit_build(0, buf, &n, sizeof(buf)))</code>
      * <code>Assert equal (SSH_MSG_KEXINIT, buf[0])</code>
      * <code>TEST_ASSERT_EQUAL_size_t(n, ssh_sess[0].i_s_len);</code>
      * <code>Assert equal memory (buf, ssh_sess[0].i_s, n)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_kexinit_parse_accepts_supported</b> &mdash; <i>Kexinit parse accepts supported</i></summary>

    * **Objective**: Kexinit parse accepts supported
    * **Assertions**:
      * <code>Assert equal int (0, ssh_kexinit_parse(0, buf, n))</code>
      * <code>Assert equal (SSH_PHASE_DH_INIT, ssh_sess[0].phase)</code>
      * <code>TEST_ASSERT_EQUAL_size_t(n, ssh_sess[0].i_c_len);</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_kexinit_parse_accepts_when_ours_listed_among_others</b> &mdash; <i>Kexinit parse accepts when ours listed among others</i></summary>

    * **Objective**: Kexinit parse accepts when ours listed among others
    * **Assertions**:
      * <code>Assert equal int (0, ssh_kexinit_parse(0, buf, n))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_kexinit_parse_rejects_missing_kex</b> &mdash; <i>Kexinit parse rejects missing kex</i></summary>

    * **Objective**: Kexinit parse rejects missing kex
    * **Assertions**:
      * <code>Assert equal int (-1, ssh_kexinit_parse(0, buf, n))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_kexinit_parse_rejects_missing_cipher</b> &mdash; <i>Kexinit parse rejects missing cipher</i></summary>

    * **Objective**: Kexinit parse rejects missing cipher
    * **Assertions**:
      * <code>Assert equal int (-1, ssh_kexinit_parse(0, buf, n))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_kexinit_parse_rejects_truncated</b> &mdash; <i>Kexinit parse rejects truncated</i></summary>

    * **Objective**: Kexinit parse rejects truncated
    * **Assertions**:
      * <code>Assert equal int (-1, ssh_kexinit_parse(0, buf, sizeof(buf)))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_exchange_hash_matches_independent_assembly</b> &mdash; <i>Exchange hash matches independent assembly</i></summary>

    * **Objective**: Exchange hash matches independent assembly
    * **Assertions**:
      * <code>Assert equal int (0, ssh_kex_exchange_hash(0, e_be, f_be, k_be, ks, sizeof(ks), got))</code>
      * <code>Assert equal memory (expected, got, SSH_SHA256_DIGEST_LEN)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_exchange_hash_changes_with_input</b> &mdash; <i>Exchange hash changes with input</i></summary>

    * **Objective**: Exchange hash changes with input
    * **Assertions**:
      * <code>Assert not equal (0, memcmp(h1, h2, SSH_SHA256_DIGEST_LEN))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_kexdh_parse_init_extracts_e_with_padding</b> &mdash; <i>Kexdh parse init extracts e with padding</i></summary>

    * **Objective**: Kexdh parse init extracts e with padding
    * **Assertions**:
      * <code>Assert equal int (0, ssh_kexdh_parse_init(pkt, n, got))</code>
      * <code>Assert equal memory (e_be, got, 256)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_kexdh_parse_init_extracts_small_e</b> &mdash; <i>Kexdh parse init extracts small e</i></summary>

    * **Objective**: Kexdh parse init extracts small e
    * **Assertions**:
      * <code>Assert equal int (0, ssh_kexdh_parse_init(pkt, n, got))</code>
      * <code>Assert equal memory (e_be, got, 256)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_kexdh_parse_init_rejects_wrong_type</b> &mdash; <i>Kexdh parse init rejects wrong type</i></summary>

    * **Objective**: Kexdh parse init rejects wrong type
    * **Assertions**:
      * <code>Assert equal int (-1, ssh_kexdh_parse_init(pkt, sizeof(pkt), got))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_kexdh_parse_init_rejects_oversized_e</b> &mdash; <i>Kexdh parse init rejects oversized e</i></summary>

    * **Objective**: Kexdh parse init rejects oversized e
    * **Assertions**:
      * <code>Assert equal int (-1, ssh_kexdh_parse_init(pkt, 5 + 300, got))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_kexdh_build_reply_structure</b> &mdash; <i>Kexdh build reply structure</i></summary>

    * **Objective**: Kexdh build reply structure
    * **Assertions**:
      * <code>Assert equal int (0, ssh_kexdh_build_reply(ks, sizeof(ks), f_be, sig, sizeof(sig), out, &n, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_KEXDH_REPLY, out[0])</code>
      * <code>TEST_ASSERT_EQUAL_UINT32(12, kslen);</code>
      * <code>Assert equal memory (ks, out + 5, 12)</code>
      * <code>Assert true (alg)</code>
      * <code>Assert true (found_sig)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_kexdh_handle_produces_reply_and_installs_keys</b> &mdash; <i>Kexdh handle produces reply and installs keys</i></summary>

    * **Objective**: Kexdh handle produces reply and installs keys
    * **Assertions**:
      * <code>Assert equal int (0, ssh_dh_generate(0))</code>
      * <code>Assert equal int (0, ssh_kexdh_handle(0, pkt, n, reply, &rlen, sizeof(reply)))</code>
      * <code>Assert equal (SSH_MSG_KEXDH_REPLY, reply[0])</code>
      * <code>Assert true (ssh_sess[0].have_session_id)</code>
      * <code>Assert equal (SSH_PHASE_NEWKEYS, ssh_sess[0].phase)</code>
      * <code>Assert true (ssh_keys[0].active)</code>
      * <code>Assert true (alg)</code>
      * <code>Assert true (ssh_pkt[0].encrypted)</code>
      * <code>Assert equal (SSH_PHASE_SERVICE, ssh_sess[0].phase)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_kexdh_handle_rejects_invalid_e</b> &mdash; <i>Kexdh handle rejects invalid e</i></summary>

    * **Objective**: Kexdh handle rejects invalid e
    * **Assertions**:
      * <code>Assert equal int (0, ssh_dh_generate(0))</code>
      * <code>Assert equal int (-1, ssh_kexdh_handle(0, pkt, n, reply, &rlen, sizeof(reply)))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_derive_keys_session_id_affects_output</b> &mdash; <i>Derive keys session id affects output</i></summary>

    * **Objective**: Derive keys session id affects output
    * **Assertions**:
      * <code>Assert not equal (0, memcmp(a, ssh_keys[0].mac_key_c2s, 32))</code>
      * <code>Assert equal memory (a, ssh_keys[0].mac_key_c2s, 32)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_rekey_needed_threshold</b> &mdash; <i>Rekey needed threshold</i></summary>

    * **Objective**: Rekey needed threshold
    * **Assertions**:
      * <code>Assert false (ssh_rekey_needed(0))</code>
      * <code>Assert true (ssh_rekey_needed(0))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_begin_rekey_preserves_session_and_auth</b> &mdash; <i>Begin rekey preserves session and auth</i></summary>

    * **Objective**: Begin rekey preserves session and auth
    * **Assertions**:
      * <code>Assert equal int (0, ssh_dh_generate(0))</code>
      * <code>Assert equal int (0, ssh_transport_begin_rekey(0, out, &n, sizeof(out)))</code>
      * <code>Assert equal (SSH_MSG_KEXINIT, out[0])</code>
      * <code>Assert equal (SSH_PHASE_KEXINIT, ssh_sess[0].phase)</code>
      * <code>Assert true (ssh_sess[0].have_session_id)</code>
      * <code>Assert true (ssh_sess[0].authed)</code>
      * <code>Assert equal (SSH_PHASE_OPEN, ssh_sess[0].phase)</code>

  </details>

</details>

<details>
<summary><b>test_transport (28 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_pool_capacity_is_four</b> &mdash; <i>Pool capacity is four</i></summary>

    * **Objective**: Pool capacity is four
    * **Assertions**:
      * <code>Assert equal (4, MAX_CONNS)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_rx_buffer_size_is_one_kb</b> &mdash; <i>Rx buffer size is one kb</i></summary>

    * **Objective**: Rx buffer size is one kb
    * **Assertions**:
      * <code>Assert equal (1024, RX_BUF_SIZE)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_timeout_constant_is_5000ms</b> &mdash; <i>Timeout constant is 5000ms</i></summary>

    * **Objective**: Timeout constant is 5000ms
    * **Assertions**:
      * <code>Assert equal (5000, CONN_TIMEOUT_MS)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_all_slots_free_after_init</b> &mdash; <i>All slots free after init</i></summary>

    * **Objective**: All slots free after init
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[i].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_all_pcbs_null_after_init</b> &mdash; <i>All pcbs null after init</i></summary>

    * **Objective**: All pcbs null after init
    * **Assertions**:
      * <code>Assert null (conn_pool[i].pcb)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_all_ring_buffers_empty_after_init</b> &mdash; <i>All ring buffers empty after init</i></summary>

    * **Objective**: All ring buffers empty after init
    * **Assertions**:
      * <code>Assert equal (conn_pool[i].rx_head, conn_pool[i].rx_tail)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_slot_ids_match_indices</b> &mdash; <i>Slot ids match indices</i></summary>

    * **Objective**: Slot ids match indices
    * **Assertions**:
      * <code>Assert equal (i, conn_pool[i].id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ring_empty_when_head_equals_tail</b> &mdash; <i>Ring empty when head equals tail</i></summary>

    * **Objective**: Ring empty when head equals tail
    * **Assertions**:
      * <code>Assert equal (s.rx_head, s.rx_tail)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ring_wrap_at_boundary</b> &mdash; <i>Ring wrap at boundary</i></summary>

    * **Objective**: Ring wrap at boundary
    * **Assertions**:
      * <code>Assert equal (0, (int)next)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ring_full_sentinel_one_slot_reserved</b> &mdash; <i>Ring full sentinel one slot reserved</i></summary>

    * **Objective**: Ring full sentinel one slot reserved
    * **Assertions**:
      * <code>Assert equal (tail, (head + 1) % RX_BUF_SIZE)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ring_can_store_size_minus_one_bytes</b> &mdash; <i>Ring can store size minus one bytes</i></summary>

    * **Objective**: Ring can store size minus one bytes
    * **Assertions**:
      * <code>Assert equal (RX_BUF_SIZE - 1, (int)count)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_event_types_are_distinct</b> &mdash; <i>Event types are distinct</i></summary>

    * **Objective**: Event types are distinct
    * **Assertions**:
      * <code>Assert not equal ((int)EVT_CONNECT, (int)EVT_DATA)</code>
      * <code>Assert not equal ((int)EVT_DATA, (int)EVT_DISCONNECT)</code>
      * <code>Assert not equal ((int)EVT_DISCONNECT, (int)EVT_ERROR)</code>
      * <code>Assert not equal ((int)EVT_CONNECT, (int)EVT_ERROR)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_timeout_does_not_fire_on_free_slot</b> &mdash; <i>Timeout does not fire on free slot</i></summary>

    * **Objective**: Timeout does not fire on free slot
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[0].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_timeout_does_not_fire_before_deadline</b> &mdash; <i>Timeout does not fire before deadline</i></summary>

    * **Objective**: Timeout does not fire before deadline
    * **Assertions**:
      * <code>Assert equal (CONN_ACTIVE, conn_pool[0].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_timeout_fires_at_deadline</b> &mdash; <i>Timeout fires at deadline</i></summary>

    * **Objective**: Timeout fires at deadline
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[0].state)</code>
      * <code>Assert null (conn_pool[0].pcb)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_timeout_fires_only_on_stale_slots</b> &mdash; <i>Timeout fires only on stale slots</i></summary>

    * **Objective**: Timeout fires only on stale slots
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[0].state)</code>
      * <code>Assert equal (CONN_ACTIVE, conn_pool[1].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_init_succeeds_on_native</b> &mdash; <i>Init succeeds on native</i></summary>

    * **Objective**: Init succeeds on native
    * **Assertions**:
      * <code>Assert equal (1, ok)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_all_last_activity_ms_zero_after_init</b> &mdash; <i>All last activity ms zero after init</i></summary>

    * **Objective**: All last activity ms zero after init
    * **Assertions**:
      * <code>Assert equal (0, (int)conn_pool[i].last_activity_ms)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_queue_not_null_after_init</b> &mdash; <i>Queue not null after init</i></summary>

    * **Objective**: Queue not null after init
    * **Assertions**:
      * <code>Assert not null (listener_pool[0].queue)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_ring_buffer_fill_drain_integrity</b> &mdash; <i>Stress - Ring buffer fill drain integrity</i></summary>

    * **Objective**: Stress - Ring buffer fill drain integrity
    * **Assertions**:
      * <code>Assert equal (RX_BUF_SIZE - 1, (int)((s-&gt;rx_head - s-&gt;rx_tail + RX_BUF_SIZE) % RX_BUF_SIZE))</code>
      * <code>Assert equal message (expected, actual, "ring buffer byte mismatch")</code>
      * <code>Assert equal (s-&gt;rx_head, s-&gt;rx_tail)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_ring_buffer_multi_cycle_no_corruption</b> &mdash; <i>Stress - Ring buffer multi cycle no corruption</i></summary>

    * **Objective**: Stress - Ring buffer multi cycle no corruption
    * **Assertions**:
      * <code>Assert not equal message (next, s-&gt;rx_tail, "ring full during stress write")</code>
      * <code>Assert equal message (read_val, s-&gt;rx_buffer[s-&gt;rx_tail], "ring corrupt on drain")</code>
      * <code>Assert equal (s-&gt;rx_head, s-&gt;rx_tail)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_all_slots_timeout_simultaneously</b> &mdash; <i>Stress - All slots timeout simultaneously</i></summary>

    * **Objective**: Stress - All slots timeout simultaneously
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[i].state)</code>
      * <code>Assert null (conn_pool[i].pcb)</code>
      * <code>Assert equal (i, conn_pool[i].id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_timeout_arm_recover_cycle</b> &mdash; <i>Stress - Timeout arm recover cycle</i></summary>

    * **Objective**: Stress - Timeout arm recover cycle
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[i].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_check_timeouts_high_call_rate</b> &mdash; <i>Stress - Check timeouts high call rate</i></summary>

    * **Objective**: Stress - Check timeouts high call rate
    * **Assertions**:
      * <code>Assert equal (CONN_FREE, conn_pool[0].state)</code>
      * <code>Assert equal (CONN_FREE, conn_pool[1].state)</code>
      * <code>Assert equal (CONN_ACTIVE, conn_pool[2].state)</code>
      * <code>Assert equal (CONN_FREE, conn_pool[3].state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_ring_buffer_byte_by_byte_fill_and_drain</b> &mdash; <i>Stress - Ring buffer byte by byte fill and drain</i></summary>

    * **Objective**: Stress - Ring buffer byte by byte fill and drain
    * **Assertions**:
      * <code>Assert equal (RX_BUF_SIZE - 1, written)</code>
      * <code>Assert equal ((uint8_t)(read & 0xFF), s-&gt;rx_buffer[s-&gt;rx_tail])</code>
      * <code>Assert equal (written, read)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_accept_throttle_blocks_over_budget</b> &mdash; <i>Accept throttle blocks over budget</i></summary>

    * **Objective**: Accept throttle blocks over budget
    * **Assertions**:
      * <code>Assert true (listener_accept_allowed(0))</code>
      * <code>Assert false (listener_accept_allowed(0))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_accept_throttle_window_refills</b> &mdash; <i>Accept throttle window refills</i></summary>

    * **Objective**: Accept throttle window refills
    * **Assertions**:
      * <code>Assert true (listener_accept_allowed(10))</code>
      * <code>Assert false (listener_accept_allowed(10))</code>
      * <code>Assert true (listener_accept_allowed(10 + DETWS_ACCEPT_THROTTLE_WINDOW_MS))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_accept_throttle_handles_rollover</b> &mdash; <i>Accept throttle handles rollover</i></summary>

    * **Objective**: Accept throttle handles rollover
    * **Assertions**:
      * <code>Assert true (listener_accept_allowed(near_max))</code>
      * <code>Assert true (listener_accept_allowed(near_max + DETWS_ACCEPT_THROTTLE_WINDOW_MS))</code>

  </details>

</details>

<details>
<summary><b>test_websocket (63 tests)</b></summary>

  <details style="margin-left: 20px;">
    <summary><b>test_sha1_empty_string</b> &mdash; <i>Sha1 empty string</i></summary>

    * **Objective**: Sha1 empty string
    * **Assertions**:
      * <code>Assert equal memory (expected, digest, SHA1_DIGEST_LEN)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sha1_abc</b> &mdash; <i>Sha1 abc</i></summary>

    * **Objective**: Sha1 abc
    * **Assertions**:
      * <code>Assert equal memory (expected, digest, SHA1_DIGEST_LEN)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sha1_rfc6455_handshake_key</b> &mdash; <i>Sha1 rfc6455 handshake key</i></summary>

    * **Objective**: Sha1 rfc6455 handshake key
    * **Assertions**:
      * <code>Assert equal memory (expected, digest, SHA1_DIGEST_LEN)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_sha1_different_inputs_different_digests</b> &mdash; <i>Sha1 different inputs different digests</i></summary>

    * **Objective**: Sha1 different inputs different digests
    * **Assertions**:
      * <code>Assert not equal (0, memcmp(d1, d2, SHA1_DIGEST_LEN))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_base64_encode_one_byte</b> &mdash; <i>Base64 encode one byte</i></summary>

    * **Objective**: Base64 encode one byte
    * **Assertions**:
      * <code>Assert equal string ("TQ==", out)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_base64_encode_two_bytes</b> &mdash; <i>Base64 encode two bytes</i></summary>

    * **Objective**: Base64 encode two bytes
    * **Assertions**:
      * <code>Assert equal string ("TWE=", out)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_base64_encode_three_bytes</b> &mdash; <i>Base64 encode three bytes</i></summary>

    * **Objective**: Base64 encode three bytes
    * **Assertions**:
      * <code>Assert equal string ("TWFu", out)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_base64_encode_ws_accept_key</b> &mdash; <i>Base64 encode ws accept key</i></summary>

    * **Objective**: Base64 encode ws accept key
    * **Assertions**:
      * <code>Assert equal string ("s3pPLMBiTxaQ9kYGzzhZRbK+xOo=", out)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_base64_decode_one_byte</b> &mdash; <i>Base64 decode one byte</i></summary>

    * **Objective**: Base64 decode one byte
    * **Assertions**:
      * <code>Assert equal (1, (int)n)</code>
      * <code>Assert equal (0x4D, (int)dst[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_base64_decode_two_bytes</b> &mdash; <i>Base64 decode two bytes</i></summary>

    * **Objective**: Base64 decode two bytes
    * **Assertions**:
      * <code>Assert equal (2, (int)n)</code>
      * <code>Assert equal (0x4D, (int)dst[0])</code>
      * <code>Assert equal (0x61, (int)dst[1])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_base64_decode_three_bytes</b> &mdash; <i>Base64 decode three bytes</i></summary>

    * **Objective**: Base64 decode three bytes
    * **Assertions**:
      * <code>Assert equal (3, (int)n)</code>
      * <code>Assert equal (0x4D, (int)dst[0])</code>
      * <code>Assert equal (0x61, (int)dst[1])</code>
      * <code>Assert equal (0x6E, (int)dst[2])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_base64_decode_ws_accept_key</b> &mdash; <i>Base64 decode ws accept key</i></summary>

    * **Objective**: Base64 decode ws accept key
    * **Assertions**:
      * <code>Assert equal (SHA1_DIGEST_LEN, (int)n)</code>
      * <code>Assert equal memory (expected, dst, SHA1_DIGEST_LEN)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_base64_decode_rejects_misplaced_padding</b> &mdash; <i>Base64 decode rejects misplaced padding</i></summary>

    * **Objective**: Base64 decode rejects misplaced padding
    * **Assertions**:
      * <code>Assert equal (0, (int)base64_decode("A=BC", dst, sizeof(dst)))</code>
      * <code>Assert equal (0, (int)base64_decode("AB=C", dst, sizeof(dst)))</code>
      * <code>Assert equal (0, (int)base64_decode("=BCD", dst, sizeof(dst)))</code>
      * <code>Assert equal (0, (int)base64_decode("TWE=TWFu", dst, sizeof(dst)))</code>
      * <code>Assert equal (0, (int)base64_decode("TWF", dst, sizeof(dst)))</code>
      * <code>Assert equal (1, (int)base64_decode("TQ==", dst, sizeof(dst)))</code>
      * <code>Assert equal (2, (int)base64_decode("TWE=", dst, sizeof(dst)))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_base64_decode_respects_capacity</b> &mdash; <i>Base64 decode respects capacity</i></summary>

    * **Objective**: Base64 decode respects capacity
    * **Assertions**:
      * <code>Assert equal (0, (int)n)</code>
      * <code>Assert equal (3, (int)base64_decode("TWFu", dst3, sizeof(dst3)))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_base64_round_trip</b> &mdash; <i>Base64 round trip</i></summary>

    * **Objective**: Base64 round trip
    * **Assertions**:
      * <code>Assert equal ((int)sizeof(src), (int)n)</code>
      * <code>Assert equal memory (src, decoded, sizeof(src))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_pool_size</b> &mdash; <i>Ws pool size</i></summary>

    * **Objective**: Ws pool size
    * **Assertions**:
      * <code>Assert equal (2, MAX_WS_CONNS)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_ids_match_indices_after_init</b> &mdash; <i>Ws ids match indices after init</i></summary>

    * **Objective**: Ws ids match indices after init
    * **Assertions**:
      * <code>Assert equal (i, (int)ws_pool[i].ws_id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_all_inactive_after_init</b> &mdash; <i>Ws all inactive after init</i></summary>

    * **Objective**: Ws all inactive after init
    * **Assertions**:
      * <code>Assert false (ws_pool[i].active)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_alloc_returns_non_null</b> &mdash; <i>Ws alloc returns non null</i></summary>

    * **Objective**: Ws alloc returns non null
    * **Assertions**:
      * <code>Assert not null (ws_alloc(0))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_alloc_sets_active</b> &mdash; <i>Ws alloc sets active</i></summary>

    * **Objective**: Ws alloc sets active
    * **Assertions**:
      * <code>Assert true (ws-&gt;active)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_alloc_sets_slot_id</b> &mdash; <i>Ws alloc sets slot id</i></summary>

    * **Objective**: Ws alloc sets slot id
    * **Assertions**:
      * <code>Assert equal (0, (int)ws-&gt;slot_id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_alloc_sets_parse_state_header1</b> &mdash; <i>Ws alloc sets parse state header1</i></summary>

    * **Objective**: Ws alloc sets parse state header1
    * **Assertions**:
      * <code>Assert equal (WS_HEADER1, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_alloc_pool_full_returns_null</b> &mdash; <i>Ws alloc pool full returns null</i></summary>

    * **Objective**: Ws alloc pool full returns null
    * **Assertions**:
      * <code>Assert not null (ws_alloc(0))</code>
      * <code>Assert not null (ws_alloc(1))</code>
      * <code>Assert null (ws_alloc(2))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_find_returns_correct_conn</b> &mdash; <i>Ws find returns correct conn</i></summary>

    * **Objective**: Ws find returns correct conn
    * **Assertions**:
      * <code>Assert not null (found)</code>
      * <code>Assert equal ptr (allocated, found)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_find_returns_null_when_empty</b> &mdash; <i>Ws find returns null when empty</i></summary>

    * **Objective**: Ws find returns null when empty
    * **Assertions**:
      * <code>Assert null (ws_find(0))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_find_returns_null_for_different_slot</b> &mdash; <i>Ws find returns null for different slot</i></summary>

    * **Objective**: Ws find returns null for different slot
    * **Assertions**:
      * <code>Assert null (ws_find(1))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_find_after_both_slots_allocated</b> &mdash; <i>Ws find after both slots allocated</i></summary>

    * **Objective**: Ws find after both slots allocated
    * **Assertions**:
      * <code>Assert not null (ws_find(0))</code>
      * <code>Assert not null (ws_find(1))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_free_deactivates_slot</b> &mdash; <i>Ws free deactivates slot</i></summary>

    * **Objective**: Ws free deactivates slot
    * **Assertions**:
      * <code>Assert false (ws_pool[0].active)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_free_restores_ws_id</b> &mdash; <i>Ws free restores ws id</i></summary>

    * **Objective**: Ws free restores ws id
    * **Assertions**:
      * <code>Assert equal (0, (int)ws_pool[0].ws_id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_free_makes_slot_findable_as_null</b> &mdash; <i>Ws free makes slot findable as null</i></summary>

    * **Objective**: Ws free makes slot findable as null
    * **Assertions**:
      * <code>Assert null (ws_find(0))</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_free_nop_on_unallocated</b> &mdash; <i>Ws free nop on unallocated</i></summary>

    * **Objective**: Ws free nop on unallocated
    * **Assertions**:
      * <code>Assert false (ws_pool[0].active)</code>
      * <code>Assert false (ws_pool[1].active)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_alloc_after_free_succeeds</b> &mdash; <i>Ws alloc after free succeeds</i></summary>

    * **Objective**: Ws alloc after free succeeds
    * **Assertions**:
      * <code>Assert not null (ws)</code>
      * <code>Assert true (ws-&gt;active)</code>
      * <code>Assert equal (0, (int)ws-&gt;slot_id)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_text_frame_sets_ready</b> &mdash; <i>Ws parse text frame sets ready</i></summary>

    * **Objective**: Ws parse text frame sets ready
    * **Assertions**:
      * <code>Assert equal (WS_FRAME_READY, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_payload_stored_correctly</b> &mdash; <i>Ws parse payload stored correctly</i></summary>

    * **Objective**: Ws parse payload stored correctly
    * **Assertions**:
      * <code>Assert equal (WS_FRAME_READY, ws-&gt;parse_state)</code>
      * <code>Assert equal (5, (int)ws-&gt;payload_len)</code>
      * <code>Assert equal string ("Hello", (const char *)ws-&gt;buf)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_binary_frame_sets_ready</b> &mdash; <i>Ws parse binary frame sets ready</i></summary>

    * **Objective**: Ws parse binary frame sets ready
    * **Assertions**:
      * <code>Assert equal (WS_FRAME_READY, ws-&gt;parse_state)</code>
      * <code>Assert equal (WS_OP_BINARY, ws-&gt;opcode)</code>
      * <code>Assert equal (3, (int)ws-&gt;payload_len)</code>
      * <code>Assert equal (0x01, (int)ws-&gt;buf[0])</code>
      * <code>Assert equal (0x02, (int)ws-&gt;buf[1])</code>
      * <code>Assert equal (0x03, (int)ws-&gt;buf[2])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_zero_length_unmasked_frame</b> &mdash; <i>Ws parse zero length unmasked frame</i></summary>

    * **Objective**: Ws parse zero length unmasked frame
    * **Assertions**:
      * <code>Assert equal (WS_ERROR, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_zero_length_masked_frame</b> &mdash; <i>Ws parse zero length masked frame</i></summary>

    * **Objective**: Ws parse zero length masked frame
    * **Assertions**:
      * <code>Assert equal (WS_FRAME_READY, ws-&gt;parse_state)</code>
      * <code>Assert equal (0, (int)ws-&gt;payload_len)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_reject_unmasked_data_frame</b> &mdash; <i>Ws reject unmasked data frame</i></summary>

    * **Objective**: Ws reject unmasked data frame
    * **Assertions**:
      * <code>Assert equal (WS_ERROR, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_reject_reserved_opcode</b> &mdash; <i>Ws reject reserved opcode</i></summary>

    * **Objective**: Ws reject reserved opcode
    * **Assertions**:
      * <code>Assert equal (WS_ERROR, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_reject_fragmented_control_frame</b> &mdash; <i>Ws reject fragmented control frame</i></summary>

    * **Objective**: Ws reject fragmented control frame
    * **Assertions**:
      * <code>Assert equal (WS_ERROR, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_reject_oversized_control_frame</b> &mdash; <i>Ws reject oversized control frame</i></summary>

    * **Objective**: Ws reject oversized control frame
    * **Assertions**:
      * <code>Assert equal (WS_ERROR, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_16bit_length_frame</b> &mdash; <i>Ws parse 16bit length frame</i></summary>

    * **Objective**: Ws parse 16bit length frame
    * **Assertions**:
      * <code>Assert equal (WS_FRAME_READY, ws-&gt;parse_state)</code>
      * <code>Assert equal (130, (int)ws-&gt;payload_len)</code>
      * <code>Assert equal (0, (int)ws-&gt;buf[0])</code>
      * <code>Assert equal (129 & 0xFF, (int)ws-&gt;buf[129])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_rsv1_set_closes_protocol</b> &mdash; <i>Ws parse rsv1 set closes protocol</i></summary>

    * **Objective**: Ws parse rsv1 set closes protocol
    * **Assertions**:
      * <code>Assert equal (WS_ERROR, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_rsv2_set_closes_protocol</b> &mdash; <i>Ws parse rsv2 set closes protocol</i></summary>

    * **Objective**: Ws parse rsv2 set closes protocol
    * **Assertions**:
      * <code>Assert equal (WS_ERROR, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_rsv3_set_closes_protocol</b> &mdash; <i>Ws parse rsv3 set closes protocol</i></summary>

    * **Objective**: Ws parse rsv3 set closes protocol
    * **Assertions**:
      * <code>Assert equal (WS_ERROR, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_64bit_length_closes_too_big</b> &mdash; <i>Ws parse 64bit length closes too big</i></summary>

    * **Objective**: Ws parse 64bit length closes too big
    * **Assertions**:
      * <code>Assert equal (WS_ERROR, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_oversized_16bit_length_closes_too_big</b> &mdash; <i>Ws parse oversized 16bit length closes too big</i></summary>

    * **Objective**: Ws parse oversized 16bit length closes too big
    * **Assertions**:
      * <code>Assert equal (WS_ERROR, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_fragment_start_waits_for_continuation</b> &mdash; <i>Ws fragment start waits for continuation</i></summary>

    * **Objective**: Ws fragment start waits for continuation
    * **Assertions**:
      * <code>Assert not equal (WS_FRAME_READY, ws-&gt;parse_state)</code>
      * <code>Assert not equal (WS_ERROR, ws-&gt;parse_state)</code>
      * <code>Assert true (ws-&gt;fragmenting)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_fragmented_message_reassembled</b> &mdash; <i>Ws fragmented message reassembled</i></summary>

    * **Objective**: Ws fragmented message reassembled
    * **Assertions**:
      * <code>Assert true (ws-&gt;fragmenting)</code>
      * <code>Assert equal (WS_FRAME_READY, ws-&gt;parse_state)</code>
      * <code>Assert equal (WS_OP_TEXT, ws-&gt;opcode)</code>
      * <code>Assert equal (5, (int)ws-&gt;payload_len)</code>
      * <code>Assert equal memory ("Hello", ws-&gt;buf, 5)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_control_frame_interleaved_in_fragments</b> &mdash; <i>Ws control frame interleaved in fragments</i></summary>

    * **Objective**: Ws control frame interleaved in fragments
    * **Assertions**:
      * <code>Assert equal (WS_FRAME_READY, ws-&gt;parse_state)</code>
      * <code>Assert equal (WS_OP_TEXT, ws-&gt;opcode)</code>
      * <code>Assert equal (5, (int)ws-&gt;payload_len)</code>
      * <code>Assert equal memory ("Hello", ws-&gt;buf, 5)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_continuation_without_start_rejected</b> &mdash; <i>Ws continuation without start rejected</i></summary>

    * **Objective**: Ws continuation without start rejected
    * **Assertions**:
      * <code>Assert equal (WS_ERROR, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_new_data_frame_during_fragmentation_rejected</b> &mdash; <i>Ws new data frame during fragmentation rejected</i></summary>

    * **Objective**: Ws new data frame during fragmentation rejected
    * **Assertions**:
      * <code>Assert equal (WS_ERROR, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_ping_auto_pong_resets_frame</b> &mdash; <i>Ws parse ping auto pong resets frame</i></summary>

    * **Objective**: Ws parse ping auto pong resets frame
    * **Assertions**:
      * <code>Assert equal (WS_HEADER1, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_pong_silently_ignored</b> &mdash; <i>Ws parse pong silently ignored</i></summary>

    * **Objective**: Ws parse pong silently ignored
    * **Assertions**:
      * <code>Assert equal (WS_HEADER1, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_close_marks_ws_closed</b> &mdash; <i>Ws parse close marks ws closed</i></summary>

    * **Objective**: Ws parse close marks ws closed
    * **Assertions**:
      * <code>Assert equal (WS_CLOSED, ws-&gt;parse_state)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_stops_at_frame_ready</b> &mdash; <i>Ws parse stops at frame ready</i></summary>

    * **Objective**: Ws parse stops at frame ready
    * **Assertions**:
      * <code>Assert equal (WS_FRAME_READY, ws-&gt;parse_state)</code>
      * <code>Assert not equal (conn_pool[0].rx_head, conn_pool[0].rx_tail)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_reset_frame_clears_fields</b> &mdash; <i>Ws reset frame clears fields</i></summary>

    * **Objective**: Ws reset frame clears fields
    * **Assertions**:
      * <code>Assert equal (WS_HEADER1, ws-&gt;parse_state)</code>
      * <code>Assert equal (0, (int)ws-&gt;payload_len)</code>
      * <code>Assert equal (0, (int)ws-&gt;payload_idx)</code>
      * <code>Assert false (ws-&gt;fin)</code>
      * <code>Assert false (ws-&gt;masked)</code>
      * <code>Assert equal (0, (int)ws-&gt;mask_key[0])</code>
      * <code>Assert equal ('\\0', (char)ws-&gt;buf[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>test_ws_parse_mask_applied_correctly</b> &mdash; <i>Ws parse mask applied correctly</i></summary>

    * **Objective**: Ws parse mask applied correctly
    * **Assertions**:
      * <code>Assert equal (WS_FRAME_READY, ws-&gt;parse_state)</code>
      * <code>Assert equal ('H', (char)ws-&gt;buf[0])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_ws_parse_reset_100_cycles</b> &mdash; <i>Stress - Ws parse reset 100 cycles</i></summary>

    * **Objective**: Stress - Ws parse reset 100 cycles
    * **Assertions**:
      * <code>Assert not null message (ws, "alloc failed")</code>
      * <code>Assert equal message (WS_FRAME_READY, ws-&gt;parse_state, "not FRAME_READY")</code>
      * <code>Assert equal string message (text, (const char *)ws-&gt;buf, "payload mismatch")</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_ws_alloc_free_pool_cycle</b> &mdash; <i>Stress - Ws alloc free pool cycle</i></summary>

    * **Objective**: Stress - Ws alloc free pool cycle
    * **Assertions**:
      * <code>Assert not null (w0)</code>
      * <code>Assert not null (w1)</code>
      * <code>Assert null (ws_alloc(2))</code>
      * <code>Assert not null (w0b)</code>
      * <code>Assert equal (0, (int)w0b-&gt;slot_id)</code>
      * <code>Assert false (ws_pool[0].active)</code>
      * <code>Assert false (ws_pool[1].active)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_ws_parse_incremental_byte_by_byte</b> &mdash; <i>Stress - Ws parse incremental byte by byte</i></summary>

    * **Objective**: Stress - Ws parse incremental byte by byte
    * **Assertions**:
      * <code>Assert not equal message (WS_ERROR, ws-&gt;parse_state, "WS_ERROR during valid incremental parse")</code>
      * <code>Assert equal (WS_FRAME_READY, ws-&gt;parse_state)</code>
      * <code>Assert equal string (text, (const char *)ws-&gt;buf)</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_ws_parse_max_payload</b> &mdash; <i>Stress - Ws parse max payload</i></summary>

    * **Objective**: Stress - Ws parse max payload
    * **Assertions**:
      * <code>Assert equal (WS_FRAME_READY, ws-&gt;parse_state)</code>
      * <code>Assert equal ((int)WS_FRAME_SIZE, (int)ws-&gt;payload_len)</code>
      * <code>Assert equal (0, (int)ws-&gt;buf[0])</code>
      * <code>Assert equal ((int)((WS_FRAME_SIZE - 1) & 0xFF), (int)ws-&gt;buf[WS_FRAME_SIZE - 1])</code>
      * <code>Assert equal ('\\0', (char)ws-&gt;buf[WS_FRAME_SIZE])</code>

  </details>

  <details style="margin-left: 20px;">
    <summary><b>stress_ws_parse_two_consecutive_frames</b> &mdash; <i>Stress - Ws parse two consecutive frames</i></summary>

    * **Objective**: Stress - Ws parse two consecutive frames
    * **Assertions**:
      * <code>Assert equal (WS_FRAME_READY, ws-&gt;parse_state)</code>
      * <code>Assert equal string (t1, (const char *)ws-&gt;buf)</code>
      * <code>Assert equal (WS_FRAME_READY, ws-&gt;parse_state)</code>
      * <code>Assert equal string (t2, (const char *)ws-&gt;buf)</code>

  </details>

</details>
