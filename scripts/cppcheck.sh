#!/bin/sh

cd "${MESON_SOURCE_ROOT}"

cppcheck src/ --enable=all --inconclusive --quiet --verbose \
	--std=c11 --library=posix --force \
	--output-file="${MESON_BUILD_ROOT}/cppcheck_result.txt"
