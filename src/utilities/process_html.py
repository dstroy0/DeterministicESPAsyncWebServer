##
# @file process_html.py
# @brief Beautifies, minifies, and decorates HTML documents with premium embedded styling.
# @details This script provides utility functions to compress HTML payloads (useful for embedded web servers)
#          and inject customized CSS styling elements dynamically.
# @author Douglas Quigg (dstroy0, dquigg123@gmail.com)
# @date June 2026
#

import argparse
import re
import os

PREMIUM_CSS = """/* Premium Embedded CSS Theme */
:root {
    --bg-primary: #0a0e17;
    --bg-secondary: rgba(255, 255, 255, 0.03);
    --border-color: rgba(255, 255, 255, 0.08);
    --text-primary: #f3f4f6;
    --text-secondary: #9ca3af;
    --accent: #3b82f6;
    --accent-hover: #2563eb;
    --accent-glow: rgba(59, 130, 246, 0.3);
    --success: #10b981;
    --error: #ef4444;
}

body {
    background-color: var(--bg-primary);
    color: var(--text-primary);
    font-family: 'Inter', system-ui, -apple-system, sans-serif;
    margin: 0;
    padding: 2rem;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    min-height: 100vh;
    box-sizing: border-box;
}

.card {
    background: var(--bg-secondary);
    border: 1px solid var(--border-color);
    backdrop-filter: blur(12px);
    -webkit-backdrop-filter: blur(12px);
    border-radius: 16px;
    padding: 2.5rem;
    max-width: 480px;
    width: 100%;
    box-shadow: 0 10px 30px rgba(0, 0, 0, 0.5);
    transition: transform 0.3s ease, box-shadow 0.3s ease;
}

.card:hover {
    transform: translateY(-4px);
    box-shadow: 0 15px 35px rgba(0, 0, 0, 0.6), 0 0 15px var(--accent-glow);
}

h1, h2, h3 {
    margin-top: 0;
    color: #ffffff;
    font-weight: 700;
    letter-spacing: -0.025em;
}

p {
    color: var(--text-secondary);
    line-height: 1.6;
}

button {
    background: var(--accent);
    color: #ffffff;
    border: none;
    padding: 0.75rem 1.5rem;
    font-size: 0.95rem;
    font-weight: 600;
    border-radius: 8px;
    cursor: pointer;
    transition: all 0.2s ease;
    box-shadow: 0 4px 12px var(--accent-glow);
}

button:hover {
    background: var(--accent-hover);
    transform: translateY(-1px);
    box-shadow: 0 6px 16px var(--accent-glow);
}

button:active {
    transform: translateY(1px);
}

input, select, textarea {
    background: rgba(0, 0, 0, 0.2);
    border: 1px solid var(--border-color);
    color: var(--text-primary);
    padding: 0.75rem 1rem;
    border-radius: 8px;
    font-size: 0.95rem;
    width: 100%;
    box-sizing: border-box;
    transition: all 0.2s ease;
}

input:focus, select:focus, textarea:focus {
    outline: none;
    border-color: var(--accent);
    box-shadow: 0 0 0 3px var(--accent-glow);
}
"""

##
# @brief Strip comments and collapse whitespaces to reduce file size.
# @param html The input raw HTML content.
# @return The minified HTML string.
#
def minify_html(html):
    # Remove HTML comments
    html = re.sub(r'<!--(?!\s*#)(?:(?!-->).)*-->', '', html, flags=re.DOTALL)
    # Collapse multiple spaces and newlines
    html = re.sub(r'\s+', ' ', html)
    # Remove space between tags
    html = re.sub(r'>\s+<', '><', html)
    return html.strip()

##
# @brief Structure the HTML tag structure with proper indentations.
# @param html The input raw HTML content.
# @return The formatted/beautified HTML string.
#
def beautify_html(html):
    tokens = re.split(r'(<[^>]+>)', html)
    indent = 0
    indented_html = []
    
    no_indent_tags = {'img', 'input', 'br', 'hr', 'meta', 'link'}
    
    for token in tokens:
        token = token.strip()
        if not token:
            continue
        
        if token.startswith('</'):
            indent = max(0, indent - 1)
            indented_html.append('  ' * indent + token)
        elif token.startswith('<') and not token.endswith('/>') and not token.startswith('<!'):
            tag_match = re.match(r'<([a-zA-Z0-9:-]+)', token)
            tag_name = tag_match.group(1).lower() if tag_match else ''
            
            if tag_name in no_indent_tags or token.endswith('/>'):
                indented_html.append('  ' * indent + token)
            else:
                indented_html.append('  ' * indent + token)
                indent += 1
        else:
            indented_html.append('  ' * indent + token)
            
    return '\n'.join(indented_html)

##
# @brief Inject a CSS stylesheet block inside the HTML head.
# @param html The raw HTML string.
# @param css_content The CSS code to inject.
# @return The updated HTML string with the injected style block.
#
def inject_css(html, css_content):
    style_block = f"\n<style>\n{css_content}\n</style>\n"
    if "</head>" in html:
        return html.replace("</head>", f"{style_block}</head>")
    else:
        return style_block + html

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Beautify, decorate, and minify library HTML files.")
    parser.add_argument("--input", required=True, help="Input HTML file path")
    parser.add_argument("--output", required=True, help="Output HTML file path")
    parser.add_argument("--minify", action="store_true", help="Minify the HTML")
    parser.add_argument("--beautify", action="store_true", help="Beautify the HTML")
    parser.add_argument("--decorate-css", action="store_true", help="Inject premium CSS style block into HTML")
    parser.add_argument("--css-out", help="Path to write the premium CSS file standalone")
    
    args = parser.parse_args()
    
    if not os.path.exists(args.input):
        print(f"Error: input file {args.input} does not exist.")
        exit(1)
        
    with open(args.input, "r", encoding="utf-8") as f:
        html = f.read()
        
    # Inject CSS if requested
    if args.decorate_css:
        html = inject_css(html, PREMIUM_CSS)
        print("Premium CSS theme injected.")
        
    # Process html formatting
    if args.minify:
        html = minify_html(html)
        print("HTML minification complete.")
    elif args.beautify:
        html = beautify_html(html)
        print("HTML beautification complete.")
        
    # Write standalone CSS if requested
    if args.css_out:
        os.makedirs(os.path.dirname(os.path.abspath(args.css_out)) or ".", exist_ok=True)
        with open(args.css_out, "w", encoding="utf-8") as f:
            f.write(PREMIUM_CSS)
        print(f"CSS stylesheet generated at {args.css_out}")
        
    # Save output
    os.makedirs(os.path.dirname(os.path.abspath(args.output)) or ".", exist_ok=True)
    with open(args.output, "w", encoding="utf-8") as f:
        f.write(html)
    print(f"Processed HTML written to {args.output}")
