import re
import os

# Calculate project root dynamically (two levels up from docs/utilities)
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
changelog_path = os.path.join(PROJECT_ROOT, "docs", "CHANGELOG.md")

def wrap_all_between(content, heading_regex, next_heading_regex, summary_text_builder):
    """Finds all sections matching a heading and wraps their body until the next heading."""
    matches = list(re.finditer(heading_regex, content))
    # Process in reverse order to keep offsets valid
    for i in range(len(matches) - 1, -1, -1):
        match = matches[i]
        start_idx = match.end()
        # Find the next heading (or end of file)
        next_match = re.search(next_heading_regex, content[start_idx:])
        if next_match:
            end_idx = start_idx + next_match.start()
        else:
            end_idx = len(content)
        
        heading_text = match.group(1) if match.groups() else match.group(0)
        summary_text = summary_text_builder(heading_text)
        
        body = content[start_idx:end_idx].strip()
        
        # Avoid double-wrapping
        if "<details>" in body:
            continue
            
        wrapped = f"\n\n<details>\n<summary><b>{summary_text}</b></summary>\n\n{body}\n\n</details>\n\n"
        content = content[:start_idx] + wrapped + content[end_idx:]
    return content

if __name__ == "__main__":
    if os.path.exists(changelog_path):
        with open(changelog_path, "r", encoding="utf-8") as f:
            content = f.read()
        
        # Wrap older releases, e.g. ## [1.2.0]... but not ## [Unreleased]
        updated = wrap_all_between(
            content,
            r'(?m)^## (\[\d+\.\d+\.\d+\][^#\n]*)$',
            r'(?m)^## ',
            lambda heading: f"Show Changelog for version {heading.strip()}"
        )
        
        with open(changelog_path, "w", encoding="utf-8") as f:
            f.write(updated)
        print(f"Successfully decorated {changelog_path}")
    else:
        print(f"Error: {changelog_path} not found")
