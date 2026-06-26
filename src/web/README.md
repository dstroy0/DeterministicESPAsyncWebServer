# Library HTML Assets

The `input` directory contains all pages and components served to users by the web server, separated by file.

These files serve as the source templates before they are processed (beautified, decorated, minified) and compressed (only if large enough, gzipped) into C++ binary array buffers (`uint8_t[]`) using the utilities in `web/wizard/`.
