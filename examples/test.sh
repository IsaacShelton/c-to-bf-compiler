#!/usr/bin/env sh

self="$(dirname -- "$0")"
set -ex

"$self/../peck" "$self/addition_subtraction.peck"
"$self/../peck" "$self/array_access_simple.peck"
"$self/../peck" "$self/casts.peck"
"$self/../peck" "$self/divmod.peck"
"$self/../peck" "$self/empty_global.peck"
"$self/../peck" "$self/equals_not_equals.peck"
"$self/../peck" "$self/function_call_simple.peck"
"$self/../peck" "$self/less_greater.peck"
"$self/../peck" "$self/less_greater_equal.peck"
"$self/../peck" "$self/line_comments.peck"
"$self/../peck" "$self/multiline_comments.peck"
"$self/../peck" "$self/multiplication.peck"
"$self/../peck" "$self/nested_expressions.peck"
"$self/../peck" "$self/print_array.peck"
"$self/../peck" "$self/print_string.peck"
"$self/../peck" "$self/printu1.peck"
"$self/../peck" "$self/put.peck"
"$self/../peck" "$self/recursion_detection_simple.peck"
"$self/../peck" "$self/shifts.peck"
"$self/../peck" "$self/variable_usage_simple.peck"

