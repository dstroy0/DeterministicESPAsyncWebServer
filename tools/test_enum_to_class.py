#!/usr/bin/env python3
"""Check that enum_to_class's use-site rewrite steps over comments and literals."""
import importlib.util
import os
import re
import sys

spec = importlib.util.spec_from_file_location(
    "e2c", os.path.join(os.path.dirname(os.path.abspath(__file__)), "enum_to_class.py")
)
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)

subs = [(re.compile(r"(?<![\w:])IKE_SA_INIT\b"), "IkeExchange::IKE_SA_INIT")]

DOC_IN = "/**\n * the IKE_SA_INIT -> IKE_AUTH state machine\n */\nq = IKE_SA_INIT;"
DOC_WANT = "/**\n * the IKE_SA_INIT -> IKE_AUTH state machine\n */\nq = IkeExchange::IKE_SA_INIT;"
BLOCK_IN = "/*\n * IKE_SA_INIT here\n */\nv = IKE_SA_INIT;"
BLOCK_WANT = "/*\n * IKE_SA_INIT here\n */\nv = IkeExchange::IKE_SA_INIT;"

cases = [
    ("x = IKE_SA_INIT;", "x = IkeExchange::IKE_SA_INIT;"),
    ("// the IKE_SA_INIT state machine", "// the IKE_SA_INIT state machine"),
    # the real ServerConfig.h shape: a doc block whose prose names the constants
    (DOC_IN, DOC_WANT),
    ("/* IKE_SA_INIT */ y = IKE_SA_INIT;", "/* IKE_SA_INIT */ y = IkeExchange::IKE_SA_INIT;"),
    ('s = "IKE_SA_INIT";', 's = "IKE_SA_INIT";'),
    ('log("IKE_SA_INIT", IKE_SA_INIT);', 'log("IKE_SA_INIT", IkeExchange::IKE_SA_INIT);'),
    # a // inside a string must not start a comment and swallow the rest
    ('url = "http://x"; z = IKE_SA_INIT;', 'url = "http://x"; z = IkeExchange::IKE_SA_INIT;'),
    # already scoped -> idempotent
    ("IkeExchange::IKE_SA_INIT", "IkeExchange::IKE_SA_INIT"),
    (BLOCK_IN, BLOCK_WANT),
]

fails = 0
for src, want in cases:
    got = m.sub_code_only(subs, src)
    ok = got == want
    fails += not ok
    label = src.replace("\n", "\\n")
    print(("  ok   " if ok else "  FAIL ") + label[:70])
    if not ok:
        print("        want:", repr(want))
        print("        got :", repr(got))

print("ALL PASS" if not fails else f"{fails} FAILED")
sys.exit(1 if fails else 0)
