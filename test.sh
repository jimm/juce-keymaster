#!/bin/bash

HERE="$(cd "$(dirname "$0")" && pwd)"
exec "$HERE"/Builds/MacOSX/build/Debug/UnitTestRunner "$@"
