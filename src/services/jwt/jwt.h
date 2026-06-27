// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file jwt.h
 * @brief Zero-heap JWT (JSON Web Token) bearer-auth verification, HS256.
 *
 * Stateless request authentication: a client presents `Authorization: Bearer
 * <jwt>` and the server verifies the token's HMAC-SHA-256 signature against a
 * shared secret (reusing the SSH crypto layer's HMAC). No sessions, no per-client
 * state, no heap - all work happens in fixed stack/BSS buffers and the whole core
 * is host-testable (env:native_jwt).
 *
 * Only HS256 (HMAC-SHA-256) is supported - the deterministic, allocation-free
 * choice for a constrained device sharing a secret with its issuer. RS256/ES256
 * (asymmetric) are out of scope. Signature verification is constant-time.
 *
 * The verifier does NOT itself enforce time-based claims (the device may have no
 * clock); use jwt_claim_int() to read `exp`/`nbf`/`iat` and compare against your
 * own time source when you have one.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_JWT_H
#define DETERMINISTICESPASYNCWEBSERVER_JWT_H

#include "DetWebServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_JWT

/**
 * @brief Verify the HS256 signature of a JWT.
 *
 * Checks that the token has exactly the `header.payload.signature` shape and that
 * base64url(HMAC-SHA-256(secret, "header.payload")) equals the signature segment
 * (constant-time). Does not inspect claims.
 *
 * @param token       the compact JWT string.
 * @param token_len   length of @p token (e.g. strlen).
 * @param secret      HMAC key bytes.
 * @param secret_len  key length.
 * @return true if the signature is valid.
 */
bool jwt_verify_hs256(const char *token, size_t token_len, const uint8_t *secret, size_t secret_len);

/**
 * @brief Validate an `Authorization` header value carrying a Bearer JWT.
 *
 * Accepts a value beginning with `Bearer ` (case-insensitive scheme), then
 * verifies the token via jwt_verify_hs256().
 *
 * @param auth_header the full Authorization header value (may be nullptr).
 * @param secret      HMAC key bytes.
 * @param secret_len  key length.
 * @return true if a well-formed Bearer token validates.
 */
bool jwt_bearer_valid(const char *auth_header, const uint8_t *secret, size_t secret_len);

/**
 * @brief Read an integer claim (e.g. "exp", "iat", "nbf") from a JWT payload.
 *
 * base64url-decodes the payload segment and scans the JSON for a top-level
 * numeric member @p name. Does not verify the signature - call
 * jwt_verify_hs256() first.
 *
 * @param token      the compact JWT string.
 * @param token_len  length of @p token.
 * @param name       claim name (without quotes).
 * @param out        receives the parsed value on success.
 * @return true if the claim is present and parses as an integer.
 */
bool jwt_claim_int(const char *token, size_t token_len, const char *name, long *out);

/**
 * @brief Read a string claim (e.g. "sub", "role", "scope") from a JWT payload.
 *
 * base64url-decodes the payload and copies the top-level string member @p name
 * into @p out (null-terminated, bounded by @p out_cap). Does not verify the
 * signature - call jwt_verify_hs256() first. Minimal unescaping (handles `\"` and
 * `\\`; other escapes are copied without their backslash), which suits
 * scope / role / sub values.
 *
 * @return true if the claim is present and is a string that fit in @p out.
 */
bool jwt_claim_str(const char *token, size_t token_len, const char *name, char *out, size_t out_cap);

/**
 * @brief Test whether a space-separated OAuth2 scope claim grants @p required.
 *
 * @param scope_claim the `scope` claim value (space-delimited scopes, RFC 6749 3.3).
 * @param required    the scope to look for.
 * @return true if @p required is one of the whole space-separated tokens.
 */
bool jwt_scope_allows(const char *scope_claim, const char *required);

#endif // DETWS_ENABLE_JWT

#endif // DETERMINISTICESPASYNCWEBSERVER_JWT_H
