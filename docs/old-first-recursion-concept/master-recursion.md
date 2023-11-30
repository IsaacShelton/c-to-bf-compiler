
# Layout:

[n] global variables
[n] stack
u32 stack_pointer
u32 frame_pointer
u32 current basicblock id

# Calling Convention:

[n] return_value
u32 return basicblock id
[n] parameters
u32 function basicblock id

# Early Returns:

Same as regular return

# Jumping:

push [n] working_memory
push target_basicblock_id

# Continuation:

working_memory = pop [n] working_memory
<...>

# Continue

push increment_block // For `for` loops
push condition_block // For `while` and `do-while` loops

# Break:

push continuation_block

# Inline Evaluation

<inline expression>

# Block Evaluation

#[CALLER_BLOCK]:
call #[CALLEE_BLOCK] #[CONTINUATION_BLOCK]

#[CONTINUATION_BLOCK]:
pop [n] result_size + working_memory

# Caller Call

#[CALLER_BLOCK]
push [n] working_memory
push [n] return_size
push #[RESUME_BLOCK] // return address
push stack_pointer
frame_pointer = stack_pointer
push [n] arguments
push #[CALLEE_BLOCK]
restore tape pointer

#[CALLEE_BLOCK]
<use stack>
stack[stack_pointer - 4 - return_size] = <RETURN VALUE>
stack_pointer = frame_pointer
frame_pointer = stack[stack_pointer]

#[RESUME_BLOCK]
result = pop [n] result_size
working_memory = pop [n] working_memory

# Callee Call
pop [n] arguments
push frame_pointer
stack_pointer += variables_max_size
<...>
stack[result_pointer] = <result>
stack_pointer = frame_pointer

# Return Convention:

stack[frame_pointer - 8 - return_value_size] = <VALUE> // return value
stack_pointer = frame_pointer
frame_pointer = stack[stack_pointer] // stored_frame_pointer
<end of block>

