#!/usr/bin/env sh

self="$(dirname -- "$0")"
set -ex

"$self/../peck" "$self/addition_subtraction.peck"
"$self/../peck" "$self/and_or.peck"
"$self/../peck" "$self/array_access_simple.peck"
"$self/../peck" "$self/array_assignment.peck"
"$self/../peck" "$self/array_assignment_extend.peck"
"$self/../peck" "$self/array_initializer.peck"
"$self/../peck" "$self/array_pass_extend.peck"
"$self/../peck" "$self/array_u16_index.peck"
"$self/../peck" "$self/bitwise_and_or_xor.peck"
"$self/../peck" "$self/bitwise_complement.peck"
"$self/../peck" "$self/c_types.peck"
"$self/../peck" "$self/call_self.peck" "$self/call_self.rlebf"
"$self/../peck" "$self/call_self_argument.peck" "$self/call_self_argument.rlebf"
"$self/../peck" "$self/call_self_if.peck" "$self/call_self_if.rlebf"
"$self/../peck" "$self/casts.peck"
"$self/../peck" "$self/divmod.peck"
"$self/../peck" "$self/do_while.peck"
"$self/../peck" "$self/do_while_break.peck"
"$self/../peck" "$self/do_while_break_nested.peck"
"$self/../peck" "$self/do_while_continue.peck"
"$self/../peck" "$self/dynamic_copy.peck"
"$self/../peck" "$self/early_return.peck"
"$self/../peck" "$self/early_return_break.peck"
"$self/../peck" "$self/early_return_break_simple.peck"
"$self/../peck" "$self/early_return_compound.peck"
"$self/../peck" "$self/early_return_deep_nested_loop.peck"
"$self/../peck" "$self/early_return_in_loop.peck"
"$self/../peck" "$self/empty_global.peck"
"$self/../peck" "$self/enums.peck"
"$self/../peck" "$self/equals_not_equals.peck"
"$self/../peck" "$self/for.peck"
"$self/../peck" "$self/function_call_simple.peck"
"$self/../peck" "$self/get.peck"
"$self/../peck" "$self/global_variables.peck"
"$self/../peck" "$self/if_else.peck"
"$self/../peck" "$self/if_else_if.peck"
"$self/../peck" "$self/if_variable_lookup.peck"
"$self/../peck" "$self/less_greater.peck"
"$self/../peck" "$self/less_greater_equal.peck"
"$self/../peck" "$self/line_comments.peck"
"$self/../peck" "$self/multiline_comments.peck"
"$self/../peck" "$self/multiplication.peck"
"$self/../peck" "$self/nd_arrays.peck"
"$self/../peck" "$self/negate.peck"
"$self/../peck" "$self/nested_expressions.peck"
"$self/../peck" "$self/nested_for.peck"
"$self/../peck" "$self/nested_structs.peck"
"$self/../peck" "$self/nested_while.peck"
"$self/../peck" "$self/no_result_decrement.peck"
"$self/../peck" "$self/no_result_increment.peck"
"$self/../peck" "$self/not.peck"
"$self/../peck" "$self/operator_assign.peck"
"$self/../peck" "$self/post_increment_decrement.peck"
"$self/../peck" "$self/pre_increment_decrement.peck"
"$self/../peck" "$self/print_array.peck"
"$self/../peck" "$self/print_reference.peck"
"$self/../peck" "$self/print_string.peck"
"$self/../peck" "$self/printu1.peck"
"$self/../peck" "$self/put.peck"
"$self/../peck" "$self/recursion_detection_simple.peck"
"$self/../peck" "$self/return.peck"
"$self/../peck" "$self/return_culling.peck"
"$self/../peck" "$self/shifts.peck"
"$self/../peck" "$self/string.peck"
"$self/../peck" "$self/sizeof_type.peck"
"$self/../peck" "$self/sizeof_value.peck"
"$self/../peck" "$self/struct.peck"
"$self/../peck" "$self/struct_initializer.peck"
"$self/../peck" "$self/switch_fallthrough.peck"
"$self/../peck" "$self/switch_simple.peck"
"$self/../peck" "$self/switch_variables.peck"
"$self/../peck" "$self/ternary.peck"
"$self/../peck" "$self/tic_tac_toe.peck"
"$self/../peck" "$self/u16_access.peck"
"$self/../peck" "$self/u16_add.peck"
"$self/../peck" "$self/u16_and.peck"
"$self/../peck" "$self/u16_bit_neg.peck"
"$self/../peck" "$self/u16_div.peck"
"$self/../peck" "$self/u16_eq.peck"
"$self/../peck" "$self/u16_from_literal.peck"
"$self/../peck" "$self/u16_gt.peck"
"$self/../peck" "$self/u16_gte.peck"
"$self/../peck" "$self/u16_lt.peck"
"$self/../peck" "$self/u16_lte.peck"
"$self/../peck" "$self/u16_mod.peck"
"$self/../peck" "$self/u16_mul.peck"
"$self/../peck" "$self/u16_negate.peck"
"$self/../peck" "$self/u16_neq.peck"
"$self/../peck" "$self/u16_no_result_increment.peck"
"$self/../peck" "$self/u16_or.peck"
"$self/../peck" "$self/u16_post_increment.peck"
"$self/../peck" "$self/u16_pre_increment.peck"
"$self/../peck" "$self/u16_print.peck"
"$self/../peck" "$self/u16_sll.peck"
"$self/../peck" "$self/u16_srl.peck"
"$self/../peck" "$self/u16_sub.peck"
"$self/../peck" "$self/u16_xor.peck"
"$self/../peck" "$self/u32_access.peck"
"$self/../peck" "$self/u32_access_u16.peck"
"$self/../peck" "$self/u32_add.peck"
"$self/../peck" "$self/u32_and.peck"
"$self/../peck" "$self/u32_bit_neg.peck"
"$self/../peck" "$self/u32_div.peck"
"$self/../peck" "$self/u32_eq.peck"
"$self/../peck" "$self/u32_from_literal.peck"
"$self/../peck" "$self/u32_gt.peck"
"$self/../peck" "$self/u32_gte.peck"
"$self/../peck" "$self/u32_lt.peck"
"$self/../peck" "$self/u32_lte.peck"
"$self/../peck" "$self/u32_mod.peck"
"$self/../peck" "$self/u32_mul.peck"
"$self/../peck" "$self/u32_negate.peck"
"$self/../peck" "$self/u32_neq.peck"
"$self/../peck" "$self/u32_no_result_increment.peck"
"$self/../peck" "$self/u32_or.peck"
"$self/../peck" "$self/u32_post_increment.peck"
"$self/../peck" "$self/u32_pre_increment.peck"
"$self/../peck" "$self/u32_print.peck"
"$self/../peck" "$self/u32_sll.peck"
"$self/../peck" "$self/u32_srl.peck"
"$self/../peck" "$self/u32_sub.peck"
"$self/../peck" "$self/u32_xor.peck"
"$self/../peck" "$self/variable_usage_simple.peck"
"$self/../peck" "$self/while.peck"
"$self/../peck" "$self/while_break.peck"
"$self/../peck" "$self/while_break_nested.peck"
"$self/../peck" "$self/while_continue.peck"

