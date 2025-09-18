#!/bin/bash

echo "=== Performance Comparison: g++ vs clang ==="
echo "Binary sizes:"
echo "g++ build:   $(du -h build/mx-packageinstaller | cut -f1)"
echo "clang build: $(du -h build-clang/mx-packageinstaller | cut -f1)"
echo ""

echo "=== Running g++ build (3 times) ==="
for i in {1..3}; do
    echo "--- Run $i ---"
    timeout 10s build/mx-packageinstaller --help 2>&1 | grep -E "(Timer|ms)" || echo "No timing output in help"
done

echo ""
echo "=== Running clang build (3 times) ==="
for i in {1..3}; do
    echo "--- Run $i ---"
    timeout 10s build-clang/mx-packageinstaller --help 2>&1 | grep -E "(Timer|ms)" || echo "No timing output in help"
done

echo ""
echo "=== Binary analysis ==="
echo "g++ build symbols:"
nm -D build/mx-packageinstaller | wc -l
echo "clang build symbols:"
nm -D build-clang/mx-packageinstaller | wc -l

echo ""
echo "=== Shared library dependencies ==="
echo "g++ build:"
ldd build/mx-packageinstaller | wc -l
echo "clang build:"
ldd build-clang/mx-packageinstaller | wc -l