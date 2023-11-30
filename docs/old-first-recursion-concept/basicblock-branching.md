
<function caller>:
stack_pointer += return_size;
emit_u32(function_continuation_block);
emit_arguments();
push_working_memory_offset(args_size);
emit_u32(function_start);
emit_push(4);
push_working_memory();
<end>

<function caller continuation block>:
pop_working_memory(count + return_size);
<end>

<function start>:
pop_arguments();
...
cell_offset = emit_context.current_cell_offset;
if(condition){
    push_working_memory();
    emit_u32(then_block);
    emit_push(4);

    go to cell offset
}
if(!condition){
    push_working_memory();
    emit_u32(else_block);
    emit_push(4);

    go to cell offset
}
go to stack driver offset
<end>

<then_block>:
pop_working_memory();
...
push_working_memory();
emit_u32(continuation_block);
emit_push(4);
<end>

<else_block>:
pop_working_memory();
...
push_working_memory();
emit_u32(continuation_block);
emit_push(4);
<end>

<continuation_block>:
pop_working_memory();

