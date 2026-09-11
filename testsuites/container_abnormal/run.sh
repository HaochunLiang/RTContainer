#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Run all five QEMU images and judge RTEMS test markers, not QEMU exit alone.
set -u

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 1
exe_dir=${1:-"$repo_dir/build-container-abnormal/aarch64/a53_lp64_qemu/testsuites/container_abnormal"}
qemu_bin=${QEMU:-qemu-system-aarch64}
log_dir="$exe_dir/logs"

for tool in "$qemu_bin" timeout; do
  command -v "$tool" >/dev/null 2>&1 || {
    echo "Missing command: $tool" >&2
    exit 1
  }
done
for number in 01 02 03 04 05; do
  test -f "$exe_dir/container_abnormal$number.exe" || {
    echo "Missing executable: $exe_dir/container_abnormal$number.exe" >&2
    exit 1
  }
done
mkdir -p "$log_dir" || exit 1

failures=0
for number in 01 02 03 04 05; do
  test_name="CONTAINER ABNORMAL $number"
  log_file="$log_dir/container_abnormal$number.log"
  echo "Running $test_name (host timeout: 45 seconds)"
  qemu_status=0
  timeout --foreground -k 5s 45s "$qemu_bin" \
    -M virt,gic-version=3 -cpu cortex-a53 -smp 1 -m 512M \
    -nographic -no-reboot -monitor none \
    -kernel "$exe_dir/container_abnormal$number.exe" \
    >"$log_file" 2>&1 || qemu_status=$?
  cat "$log_file"
  # Some BSPs stay halted after TEST_END instead of exiting QEMU (status 124).
  if { test "$qemu_status" -eq 0 || test "$qemu_status" -eq 124; } &&
     grep -Fq "*** END OF TEST $test_name ***" "$log_file" &&
     ! grep -Fq '[FAIL]' "$log_file"; then
    echo "PASS: $test_name"
  else
    echo "FAIL: $test_name (QEMU/timeout status: $qemu_status)"
    failures=$((failures + 1))
  fi
done
echo "Failed tests: $failures / 5. Logs: $log_dir"
test "$failures" -eq 0
