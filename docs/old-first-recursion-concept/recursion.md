
# Layout

```
globals
stack
stack pointer
----
basicblock evaluation context
```

Example Basicblocks:

```
if(getX() + getY() < 100){
    doOne();
} else {
    doTwo();
}
```

#1 basicblock
push #3
push #2
ret

#2 basicblock
10
ret

#3 basicblock
push #5
push #4

#4 basicblock
11
ret

#5 basicblock
y = pop
x = pop
x + y < 100
if ---
push #6
else
push #7
end
ret

#6 basicblock
(doOne)
ret

#7 basicblock
(doTwo)
ret

----------------------------

For call:
call_basicblock_id
continuation_block = next_block_id++
push(continuation_block)
push(argument1)
push(argument2)
push(argument3)
push(argument4)
push(call_basicblock_id)

argument4 = pop()
argument3 = pop()
argument2 = pop()
argument1 = pop()

For return:
(do nothing, leave continuation block on top of stack)

For if:
condition = pop() OR inline_eval
then = next_block_id++
else = next_block_id++
continuation = next_block_id++
if condition
push(continuation)
push(then)
else
push(continuation)
push(else)
end

For while:
=== prewhile
eval condition
push(while)
=== while
condition = pop()
do_block = next_block_id++
continuation_block = next_block_id++
if condition
push(prewhile)
push(do_block)
else
push(continuation_block)
end
=== do_block
(do the insides)

For do while:
eval_block = next_block_id++
continuation_block = next_block_id++
(do insides)
push(eval_block)
=== eval_block
eval condition
if condition
push(do_while)
else
push(continuation)
end


NOTE: Call basicblock id might not be known at time of generation,
unless we reserve a certain amount of IDs for functions or something

All recursive functions are preallocated a basicblock id to be used
as an entry point

All recursive functions will be emitted from the context of another
recursive function.























Stack stack;
push<u32>(stack, 1); // Entry point
u32 block_id;

do {
    block_id = peek<u32>(stack);
    pop<u32>(stack);

    if(block_id == 1){

    }
    if(block_id == 2){

    }
    if(block_id == 3){

    }
    ...
} while(block_id != 0);








u8 factorial(u8 count){
    return count <= 1 ? 1 : count + factorial(count - 1);
}

u0 main(){
    printu8(factorial(10));
}







/// ---------- main ----------
block #1:
push u8 10
call #2 return to #3

block #3: // main continuation
result = pop u8
printu8(result)

/// ---------- factorial ----------
block #2:
count = pop u8
count <= 1
jifelse #4 #5

block #4:
push u8 1
j #6

block #5:
frame_pointer = stack_pointer
count = load frame_pointer + "count" offset
push count - 1
call #2 return to #6

block #6:
result = pop u8
set_return result




Each stack frame starts with saved frame pointer

stack_pointer += result_type_size;
stack[stack_pointer] = frame_pointer;
frame_pointer = stack_pointer;
stack_pointer += 4;

To return:
stack_pointer = frame_pointer
frame_pointer = stack[stack_pointer]
stack[stack_pointer] = return_basicblock;
stack_pointer += 4;



Early returns / breaking / continuing:
- Early return indicator can be replaced with just jumping to returner block?
- Breaking can just be replaced with jumping to continuation block?
- Continuing can just be replaced with jumping to increment or test block?

Operators such as &&, ||, and ?: will have to be broken into multiple basicblocks.

Basicblocks can have a "result" type
which viewed from the other persepective,
Basicblocks can take parameters


------------------------------------------------

u0 main(){
    printu32(3 + getValue(2) + 4);
}

u32 getValue(u32 amount){
    if(amount == 0){
        return 0;
    } else {
        return 1 + getValue(amount - 1);
    }
}



/// ---------- main ----------
block #1:
    3
    call #2 return to #3

block #3:
    b = pop u32
    a = pop u32
    add
    4
    add
    printu8


/// ---------- getValue ----------
block #2:
    amount = pop u32
    0
    ==
    jifelse #4 #5

block #4:
    0
    set_return
    ret

block #5:
    1
    amount
    1
    -
    call #2 return to #6

block #6:
    result = pop u32
    set_return
    ret

