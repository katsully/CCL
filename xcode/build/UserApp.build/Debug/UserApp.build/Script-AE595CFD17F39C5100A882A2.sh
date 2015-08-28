#!/bin/sh
find "$TARGET_BUILD_DIR" -name '*.dll' -print0 | xargs -0 rm
