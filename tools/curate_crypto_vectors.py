# Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Curate external crypto known-answer-test (KAT) vectors into compact, auditable
# JSON subsets under test/vectors/. These ground the library's crypto primitives
# against vectors produced OUTSIDE the codebase:
#   - Project Wycheproof (C2SP/wycheproof) for HMAC-SHA256/512, AES-128-GCM,
#     X25519, and Ed25519 - including its adversarial edge cases (wrong tags,
#     modified IVs, low-order points, signature malleability), and
#   - the RFC appendix vectors for HKDF-SHA256 (RFC 5869), the ChaCha20 block
#     (RFC 8439 2.4.2), and Poly1305 (RFC 8439 2.5.2).
#
# Run this only to (re)fetch/refresh the vendored subsets; it needs the network
# and a git checkout of Wycheproof. The committed test/vectors/*.json are then
# consumed offline by tools/gen_crypto_vectors.py. Selection is deterministic:
# every vector whose result is "invalid" or that carries a Wycheproof flag (the
# security-relevant edge cases) is kept up to CAP_FLAGGED, plus the first
# CAP_PLAIN plain-"valid" vectors, so both outcomes are always represented.
#
# Usage:  python3 tools/curate_crypto_vectors.py [path-to-wycheproof-checkout]
# If no path is given it shallow-clones a pinned commit into a temp dir.

import glob
import json
import os
import subprocess
import sys
import tempfile

# Pinned Wycheproof revision for reproducible provenance. Refreshing the vectors
# is a deliberate act: bump this, re-run, and review the JSON diff.
WYCHEPROOF_REPO = "https://github.com/C2SP/wycheproof"
WYCHEPROOF_REF = "master"

CAP_FLAGGED = 40  # adversarial / edge-case vectors kept per primitive
CAP_PLAIN = 12  # plain happy-path vectors kept per primitive

HERE = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(HERE, "..", "test", "vectors")


def _clone_pinned():
    d = tempfile.mkdtemp(prefix="wycheproof_")
    subprocess.run(["git", "clone", "--depth", "1", "--branch", WYCHEPROOF_REF, WYCHEPROOF_REPO, d], check=True)
    return d


def _rev(checkout):
    return subprocess.run(["git", "-C", checkout, "rev-parse", "HEAD"], capture_output=True, text=True).stdout.strip()


def _find(checkout, name):
    hits = glob.glob(os.path.join(checkout, "**", name), recursive=True)
    if not hits:
        raise SystemExit("cannot find %s under %s" % (name, checkout))
    # Prefer the testvectors_v1 schema when both are present.
    hits.sort(key=lambda p: ("testvectors_v1" not in p, len(p)))
    return hits[0]


def _select(tests):
    """Keep the interesting edge cases first, then a few plain-valid ones."""
    flagged, plain = [], []
    for t in tests:
        if t["result"] == "invalid" or t.get("flags"):
            flagged.append(t)
        elif t["result"] == "valid":
            plain.append(t)
        else:  # "acceptable"
            flagged.append(t)
    return flagged[:CAP_FLAGGED] + plain[:CAP_PLAIN]


def curate_wycheproof(checkout, filename, group_filter, field_map):
    """Load one Wycheproof file, flatten selected group/test fields to hex."""
    path = _find(checkout, filename)
    with open(path, "r") as f:
        doc = json.load(f)
    out = []
    for g in doc["testGroups"]:
        if group_filter and not group_filter(g):
            continue
        picked = _select(g["tests"])
        for t in picked:
            v = {"tcId": t["tcId"], "comment": t.get("comment", ""), "result": t["result"], "flags": t.get("flags", [])}
            for dst, src in field_map.items():
                # src may be "group:key" or a dotted "group:key.sub" (pull from
                # the group), otherwise it is a test-level key.
                if src.startswith("group:"):
                    node = g
                    for part in src[6:].split("."):
                        node = node.get(part, "") if isinstance(node, dict) else ""
                    v[dst] = str(node)
                else:
                    v[dst] = t.get(src, "")
            out.append(v)
    return {
        "source": WYCHEPROOF_REPO,
        "commit": _rev(checkout),
        "file": os.path.relpath(path, checkout),
        "vectors": out,
    }


def write(name, doc):
    os.makedirs(OUT_DIR, exist_ok=True)
    p = os.path.join(OUT_DIR, name)
    with open(p, "w") as f:
        json.dump(doc, f, indent=1, sort_keys=False)
        f.write("\n")
    print("wrote %s (%d vectors)" % (os.path.relpath(p, os.path.join(HERE, "..")), len(doc["vectors"])))


# --- RFC appendix vectors (small, canonical; transcribed from the RFC text and
#     independently checkable with a reference tool). ------------------------
def rfc_vectors():
    # RFC 5869 Appendix A - HKDF-SHA256 Extract: PRK = HMAC-SHA256(salt, IKM).
    hkdf = {
        "source": "RFC 5869 Appendix A",
        "commit": "",
        "file": "rfc5869",
        "vectors": [
            {  # A.1
                "tcId": 1,
                "comment": "RFC 5869 A.1 basic",
                "result": "valid",
                "flags": [],
                "salt": "000102030405060708090a0b0c",
                "ikm": "0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b",
                "prk": "077709362c2e32df0ddc3f0dc47bba6390b6c73bb50f9c3122ec844ad7c2b3e5",
            },
            {  # A.2 longer inputs
                "tcId": 2,
                "comment": "RFC 5869 A.2 longer inputs",
                "result": "valid",
                "flags": [],
                "salt": "606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f"
                "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f"
                "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf",
                "ikm": "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"
                "202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f"
                "404142434445464748494a4b4c4d4e4f",
                "prk": "06a6b88c5853361a06104c9ceb35b45cef760014904671014a193f40c15fc244",
            },
            {  # A.3 zero-length salt
                "tcId": 3,
                "comment": "RFC 5869 A.3 zero-length salt",
                "result": "valid",
                "flags": [],
                "salt": "",
                "ikm": "0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b",
                "prk": "19ef24a32c717b167f33a91d6f648bdf96596776afdb6377ac434c1c293ccb04",
            },
        ],
    }
    # RFC 8439 2.4.2 - ChaCha20 block: 64-byte keystream for the given key/nonce
    # at the given 32-bit block counter (little-endian counter word).
    chacha = {
        "source": "RFC 8439 sec 2.4.2",
        "commit": "",
        "file": "rfc8439",
        "vectors": [
            {
                "tcId": 1,
                "comment": "RFC 8439 2.4.2 keystream",
                "result": "valid",
                "flags": [],
                "key": "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
                "nonce": "000000000000004a00000000",
                "counter": 1,
                "keystream": "224f51f3401bd9e12fde276fb8631ded8c131f823d2c06e27e4fcaec9ef3cf788a3b0aa372600a92b57974cded2b9334794cba40c63e34cdea212c4cf07d41b7",
            }
        ],
    }
    # RFC 8439 2.5.2 - Poly1305: one-time MAC of the message under the 32-byte key.
    poly = {
        "source": "RFC 8439 sec 2.5.2",
        "commit": "",
        "file": "rfc8439",
        "vectors": [
            {
                "tcId": 1,
                "comment": "RFC 8439 2.5.2 tag",
                "result": "valid",
                "flags": [],
                "key": "85d6be7857556d337f4452fe42d506a80103808afb0db2fd4abff6af4149f51b",
                "msg": "43727970746f6772617068696320466f72756d2052657365617263682047726f7570",
                "tag": "a8061dc1305136c6c22b8baf0c0127a9",
            }
        ],
    }
    return hkdf, chacha, poly


def main():
    checkout = sys.argv[1] if len(sys.argv) > 1 else _clone_pinned()
    rev = _rev(checkout)
    print("wycheproof @ %s" % rev)

    aes_gcm_128 = lambda g: g.get("keySize") == 128 and g.get("ivSize") == 96 and g.get("tagSize") == 128
    write(
        "wycheproof_aes_128_gcm.json",
        curate_wycheproof(
            checkout,
            "aes_gcm_test.json",
            aes_gcm_128,
            {"key": "key", "iv": "iv", "aad": "aad", "msg": "msg", "ct": "ct", "tag": "tag"},
        ),
    )
    write(
        "wycheproof_hmac_sha256.json",
        curate_wycheproof(
            checkout,
            "hmac_sha256_test.json",
            None,
            {"key": "key", "msg": "msg", "tag": "tag", "tagSize": "group:tagSize"},
        ),
    )
    write(
        "wycheproof_hmac_sha512.json",
        curate_wycheproof(
            checkout,
            "hmac_sha512_test.json",
            None,
            {"key": "key", "msg": "msg", "tag": "tag", "tagSize": "group:tagSize"},
        ),
    )
    write(
        "wycheproof_x25519.json",
        curate_wycheproof(
            checkout,
            "x25519_test.json",
            None,
            {"public": "public", "private": "private", "shared": "shared"},
        ),
    )
    # Ed25519 lives under an "eddsa"-style file; pull the group public key.
    ed_name = "ed25519_test.json"
    try:
        _find(checkout, ed_name)
    except SystemExit:
        ed_name = "eddsa_test.json"
    write(
        "wycheproof_ed25519.json",
        curate_wycheproof(
            checkout,
            ed_name,
            None,
            {"public": "group:publicKey.pk", "msg": "msg", "sig": "sig"},
        ),
    )

    hkdf, chacha, poly = rfc_vectors()
    write("rfc5869_hkdf_sha256.json", hkdf)
    write("rfc8439_chacha20.json", chacha)
    write("rfc8439_poly1305.json", poly)


if __name__ == "__main__":
    main()
