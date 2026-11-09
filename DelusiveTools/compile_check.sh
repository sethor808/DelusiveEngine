#!/usr/bin/env bash
#Headless compile check: builds every target and prints only whether it compiled.
#Usage: DelusiveTools/compile_check.sh   (exit code 0 = compiles, 1 = does not)
root="$(cd "$(dirname "$0")/.." && pwd)"
build="$root/build/compile-check"
log="$build/compile_check.log"
mkdir -p "$build"

generator=()
command -v ninja >/dev/null && generator=(-G Ninja)

if cmake -S "$root" -B "$build" "${generator[@]}" -DCMAKE_BUILD_TYPE=Debug >"$log" 2>&1 \
	&& cmake --build "$build" --parallel >>"$log" 2>&1; then
	echo "COMPILE OK"
	exit 0
fi

grep -E "error|Error" "$log" | head -n 40
echo "COMPILE FAILED (full log: $log)"
exit 1
