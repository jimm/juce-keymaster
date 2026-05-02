#!/bin/bash

HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE/Builds/MacOSX/build/Debug"
exec ./UnitTestRunner "$@"
