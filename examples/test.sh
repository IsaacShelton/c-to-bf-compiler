#!/usr/bin/env sh

self="$(dirname -- "$0")"
set -ex

"$self/../peck" "$self/addition_subtraction.peck"
"$self/../peck" "$self/and_or.peck"
"$self/../peck" "$self/array_access_simple.peck"
"$self/../peck" "$self/array_assignment.peck"
"$self/../peck" "$self/array_assignment_extend.peck"
"$self/../peck" "$self/array_pass_extend.peck"
"$self/../peck" "$self/bitwise_and_or_xor.peck"
"$self/../peck" "$self/bitwise_complement.peck"
"$self/../peck" "$self/casts.peck"
"$self/../peck" "$self/divmod.peck"
"$self/../peck" "$self/do_while.peck"
"$self/../peck" "$self/do_while_break.peck"
"$self/../peck" "$self/do_while_break_nested.peck"
"$self/../peck" "$self/early_return.peck"
"$self/../peck" "$self/early_return_in_loop.peck"
"$self/../peck" "$self/empty_global.peck"
"$self/../peck" "$self/equals_not_equals.peck"
"$self/../peck" "$self/function_call_simple.peck"
"$self/../peck" "$self/get.peck"
"$self/../peck" "$self/if_else.peck"
"$self/../peck" "$self/if_else_if.peck"
"$self/../peck" "$self/if_variable_lookup.peck"
"$self/../peck" "$self/less_greater.peck"
"$self/../peck" "$self/less_greater_equal.peck"
"$self/../peck" "$self/line_comments.peck"
"$self/../peck" "$self/multiline_comments.peck"
"$self/../peck" "$self/multiplication.peck"
"$self/../peck" "$self/negate.peck"
"$self/../peck" "$self/nested_expressions.peck"
"$self/../peck" "$self/nested_while.peck"
"$self/../peck" "$self/not.peck"
"$self/../peck" "$self/operator_assign.peck"
"$self/../peck" "$self/post_increment_decrement.peck"
"$self/../peck" "$self/pre_increment_decrement.peck"
"$self/../peck" "$self/print_array.peck"
"$self/../peck" "$self/print_string.peck"
"$self/../peck" "$self/printu1.peck"
"$self/../peck" "$self/put.peck"
"$self/../peck" "$self/recursion_detection_simple.peck"
"$self/../peck" "$self/return.peck"
"$self/../peck" "$self/return_culling.peck"
"$self/../peck" "$self/shifts.peck"
"$self/../peck" "$self/string.peck"
"$self/../peck" "$self/struct.peck"
"$self/../peck" "$self/ternary.peck"
"$self/../peck" "$self/variable_usage_simple.peck"
"$self/../peck" "$self/while.peck"
"$self/../peck" "$self/while_break.peck"
"$self/../peck" "$self/while_break_nested.peck"

