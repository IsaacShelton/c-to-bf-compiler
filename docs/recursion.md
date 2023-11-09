
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

