import os
import re
import glob

# Calculate project root dynamically (two levels up from docs/utilities)
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
TEST_DIR = os.path.join(PROJECT_ROOT, 'test')
DOCS_FILE = os.path.join(PROJECT_ROOT, 'docs', 'TEST_DOCUMENTATION.md')

def clean_name(name):
    prefix = ""
    if name.startswith("stress_"):
        prefix = "Stress - "
        name = name[7:]
    elif name.startswith("race_"):
        prefix = "Race - "
        name = name[5:]
    elif name.startswith("test_"):
        name = name[5:]
    
    words = name.split('_')
    words[0] = words[0].capitalize()
    return prefix + " ".join(words)

def extract_assertions(lines):
    assertions = []
    for line in lines:
        line = line.strip()
        if 'TEST_ASSERT' in line:
            match = re.search(r'TEST_ASSERT_([A-Z_]+)\((.*)\)', line)
            if match:
                assert_type = match.group(1).replace('_', ' ').lower()
                args = match.group(2)
                assertions.append(f"Assert {assert_type} ({args})")
            else:
                assertions.append(line)
    return assertions

def parse_test_file(filepath):
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    run_tests = re.findall(r'RUN_TEST\(([a-zA-Z0-9_]+)\)', content)
    if not run_tests:
        run_tests = re.findall(r'void\s+([a-zA-Z0-9_]+)\s*\(', content)
        run_tests = [t for t in run_tests if t.startswith('test_') or t.startswith('stress_') or t.startswith('race_')]
    
    seen = set()
    run_tests = [x for x in run_tests if not (x in seen or seen.add(x))]
    
    lines = content.splitlines()
    
    test_cases = []
    for test_fn in run_tests:
        fn_line_idx = -1
        for idx, line in enumerate(lines):
            if re.match(r'^\s*void\s+' + re.escape(test_fn) + r'\s*\(', line):
                fn_line_idx = idx
                break
        
        if fn_line_idx == -1:
            continue
        
        brace_count = 0
        fn_lines = []
        found_start = False
        for idx in range(fn_line_idx, len(lines)):
            line = lines[idx]
            fn_lines.append(line)
            if '{' in line:
                brace_count += line.count('{')
                found_start = True
            if '}' in line and found_start:
                brace_count -= line.count('}')
                if brace_count <= 0:
                    break
        
        comment = ""
        for line in fn_lines:
            t = line.strip()
            if t.startswith('//'):
                cmt = re.sub(r'^//\s*', '', t)
                if not any(x in cmt.lower() for x in ['copyright', 'spdx', 'license']):
                    comment = cmt
                    break
            elif found_start and t and not t.startswith('{') and not t.startswith('//'):
                if t.startswith('//'):
                    comment = re.sub(r'^//\s*', '', t)
                break
        
        if not comment:
            comment = clean_name(test_fn)
            
        assertions = extract_assertions(fn_lines)
        
        test_cases.append({
            'fn_name': test_fn,
            'title': clean_name(test_fn),
            'description': comment,
            'assertions': assertions
        })
        
    return test_cases

# Scan all C++ files under test/
suites = {}
test_files = glob.glob(os.path.join(TEST_DIR, 'test_*', 'test_*.cpp'))
for filepath in sorted(test_files):
    suite_name = os.path.basename(os.path.dirname(filepath))
    test_cases = parse_test_file(filepath)
    if test_cases:
        suites[suite_name] = test_cases

# Load existing documentation
with open(DOCS_FILE, 'r', encoding='utf-8') as f:
    orig_docs = f.read()

# Strip any existing deep-dive section
if "## 7. Comprehensive Test Directory" in orig_docs:
    orig_docs = orig_docs.split("## 7. Comprehensive Test Directory")[0].rstrip()

# Generate the deep dive markdown with collapsible sections
markdown = []
markdown.append("\n\n## 7. Comprehensive Test Directory\n")
markdown.append("This section contains a thorough directory of all test cases across all 18 test suites. Click on any test suite to expand its test cases, and click on individual test cases to expand their objectives and assertions.\n")

for suite_name, tests in sorted(suites.items()):
    markdown.append(f"<details>")
    markdown.append(f"<summary><b>{suite_name} ({len(tests)} tests)</b></summary>\n")
    
    for test in tests:
        markdown.append(f"  <details style=\"margin-left: 20px;\">")
        markdown.append(f"    <summary><code>{test['fn_name']}</code> &mdash; <i>{test['description']}</i></summary>\n")
        markdown.append(f"    * **Objective**: {test['description']}")
        
        if test['assertions']:
            markdown.append("    * **Assertions**:")
            for assertion in test['assertions']:
                safe_assert = assertion.replace('<', '&lt;').replace('>', '&gt;')
                markdown.append(f"      * `{safe_assert}`")
        markdown.append("  </details>")
        
    markdown.append(f"</details>\n")

# Write back to documentation file
with open(DOCS_FILE, 'w', encoding='utf-8') as f:
    f.write(orig_docs + "\n" + "\n".join(markdown) + "\n")

print(f"Successfully generated collapsible deep dive for {len(suites)} suites and appended to {DOCS_FILE}.")
