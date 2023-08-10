# BrainKiss Tooling

*Brainkiss is a suite of tools for creating actual software with BrainF\*ck*

[Try it out online]()

[Interactive Showcase]()

[Downloads for every platform imaginable]()

[Subscribe to Blog]()

### BrainKiss Workflow
The `brainkiss` command is the heart of the brainkiss tooling suite.

It is used to compile input code to any platform, including: Windows, macOS, Linux, Web, Node, Python, and many more...

Input formats:

- `c-like code`
- `brainf*ck`,
- `compressed brainf*ck` (referred to as kissed brainf*ck)

Output targets:

- `browser`
- `node`
- `x86_64-windows`
- `arm64-windows`
- `x86_64-macos`
- `arm64-macos`
- `x86_64-linux`
- `arm64-linux`
- `source-c`
- `source-javascript`
- `source-python`

```
 ┌──────────────────┐
 │    input file    │ brainf*ck code, kissed brainf*ck code, or c-like code
 └──────────────────┘
          │
          │
          │
 ┌────────▼─────────┐
 │    brainkiss     │ general purpose packager
 └──────────────────┘
          │
          │
          │
 ┌────────▼─────────┐
 │    executable    │ includes platform-specific APIs
 └──────────────────┘
```


### Subprograms
- `brainkiss/peck` - Compiles C-like language into normal BrainF\*ck code
- `brainkiss/kiss` - Compresses normal brainf\*ck code into kissed brainf\*ck with LZ77 based compression
- `brainkiss/dekiss` - Decompresses kissed brainf\*ck code into normal brainf\*ck
- `brainkiss/munch` - Intelligently compiles brainf\*ck into different languages and bitcodes
- `brainkiss/adapter` - General purpose modular adapter for interacting with target system

Most of these tools are themselves written in BrainF\*ck.

# =======================================

### The Compressor/Decompressor (kiss/dekiss)
The kiss and dekiss tools allow for compressing and decompressing brainf\*ck. Kissed brainf\*ck is isomorphic with regular brainf\*ck, but is nicer to work with. It uses the LZ77 compression algorithm to create high quality, streamable, and human readable compressed brainf\*ck programs.

- `kiss main.bf`
- `dekiss main.kbf`

### The Programming Language (peck)
A C-like language for writing BrainF*ck programs. Built from the ground up with enterprise in mind, it's capable of producing "normal" software and can be learned in less than a minute.

Normal usage:
`peck main.peck main.bf`

Naked usage: `/bin/cat main.peck | peck > main.bf`

```
u0 main(){
	print("Hello World\n");
}
```


### The Optimizer (suck)
The suck optimizer is a best-in-its-class BrainF\*ck optimizer, capable of working with normal or kissed BrainF\*ck.

- `suck main.bf`

### The Interpreter (makeout)

- `makeout main.bf`

### The Multi-Transpiler (munch)

- `munch-c main.bf`
- `munch-js main.bf`
- `munch-go main.bf`
- `munch-java main.bf`
- and many more...

### The Interface Collection (spoon)
The spoon interfaces allow BrainF\*ck to do more than just stdin/stdout. BrainF\*ck programs can send and receive standardized JSON messages to do anything ranging from 2D platformer games to team management software. The best part is you get extreme portability for free. Each target is so simple, that anybody can create their own in an hour or two.

With BrainF\*ck, your programs are **immortal**. Simply create a small (~500 lines) interface program to translate with the target system.

If we're missing a target, make a PR!

- `spoon-2d-cglfw`
- `spoon-2d-js`
- `spoon-2d-gogg`
- `spoon-2d-javaswing`
- and many more...

### The Build System (skull)
skull.json

```
{
    "targets": {
        "": {
            "platform": "local",
            "autorun": true,
        },
        "web": {
            "platform": "browser",
        },
        "pc": {
            "platform": "windows",
        },
        "mac": {
            "platform": "macos",
        },
        "linux": {
            "platform": "linux",
            "architecture": "x86_64",
        }
    },
    "dependencies": [
        "bkaudio1",
        "bkgraphics1"
    ]
}

```

```
#!/usr/bin/env sh
# skull
./multi_way_pipe "./main" "bkfs1" "bkhostos1" "bkosshell1" "bkterminal1" "bkcc1"

```
## F.A.Q.s
Q: Is the software itself written in BrainF\*ck?

A: Yes.

Q: But why?

A: BrainF\*ck allows you to create radically portable software.

Q: Why not some flavor of assembly?

A: Assembly is overkill and unnecessary complex. BrainF\*ck is universal and can be implemented by an idiot.
