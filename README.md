# Xylia

Xylia is a hobby stack based programming language, all the operations and values are stored on the stack.
The compiler is still missing many features and the programming language is pretty unstable as well.


# Prerequisities

Xylia only supports Linux x86_64 for now but I am working on arm64 version as well.
In the future I would like to expand it to work on OSX and Apple Silicon.

You will need the following tools
- `as`: GNU assembler, should be installed on all Linux machines
- `ld`: GNU linker, should be installed on all Linux machines
- `go`: Go compiler, https://go.dev/


# Install

To install Xylia, you can run a one-liner command that will download and execute the installation script:

```sh
curl -sSL https://raw.githubusercontent.com/vh8t/xylia/main/install.sh | sh
```

# Install from scratch

To install Xylia from source, follow these steps:

1. **Clone the repository**: First, copy the Xylia repository to a directory of your choice. This will download the source code to your local machine.

```sh
git clone --depth 1 https://github.com/vh8t/xylia.git ~/.xylia
```

2. **Navigate to the directory**: Change into the directory where the repository was cloned.

```sh
cd ~/.xylia
```

3. **Compile the Source Code**: Build the Xylia compiler from the source using the Go compiler.

```sh
mkdir bin
go build -o bin/xylia src/main.go
```

4. **Set Up Environment Variables**: To use Xylia from any terminal session, add the Xylia binary directory to your system's `PATH`.

Add the following lines to your shell profile configuration file:

```sh
export XYL_HOME="$HOME/.xylia"
export PATH="$HOME/.xylia/bin:$PATH"
```

This configuration sets `XYL_HOME` to the directory where Xylia is installed and adds the bin directory to your `PATH`.

5. **Apply the Changes**: To make the changes effective immediately, source the profile file or restart your terminal.

# Documentation

Xylia has 4 basic types
- `int`: 64 bit integer
- `char`: 8 bit integer
- `bool`: `true` or `false`
- `ptr`: pointer to memory address

## Hello, World!

```xyl
import std

proc main in
    "Hello, World!\n" println
    0 return
end
```

Xylia is stack based, meaning all values are a expression that are pushed onto the stack.
This program first imports the standard library and then defines `main` procedure which is the entry point of the program.
Xylia does not rely on any indentation so the whole program could be written in one line.
We first push the `Hello, World\n` onto the stack and then call the `println` function which expects 1 argument, all arguments are just top most values on the stack, then we push 0 and call `return` which returns the top of the stack value from the procedure.

## Operations

```xyl
proc main in
    1 1 +
    dump        # print the top stack value and pop it
    0 return
end
```

This code will push 1 and 1 onto the stack and then we use the `+` to add the top 2 values on stack together. For now the only operations that are supported are `+` `-` and `*`.

## Procedures

```xyl
proc add int num1 int num2 in       # C equivalent `int add(int num1, int num2)`
    num1 num2 +
    return
end

proc main in
    10 12 add
    dump
    0 return
end
```

This program will add 10 and 12 and print it out

## Code branching

```xyl
import linux.io

proc main in
    1 2 =       # Compare 1 and 2 for equality
    if
        "Equal" println
    else
        "Not equal" println
    end
    "Result: " print
    dump
    0 return
end
```

## While loops

```xyl
proc main in
    # Loop from 0 to 9
    0 while dup 10 < do
        dup dump
        1 +
    end
end
```

## Syscalls

It is not recommended to use syscalls directly but it is meant to make libraries and procedures that are not implemented yet

```xyl
proc main in
    1 1 "Hello, World!\n" 14
    syscall 4
    0 return
end
```

## Buffers

Buffer is like a fixed size variable, you can create a fixed size buffer and use it as a variable for procedures

```xyl
import linux.os
import linux.io

buffer cwd 256      # Allocates buffer with 256 bytes of uninitialized memory

proc main in
    cwd 256 getcwd   # Call getcwd function `proc getcwd ptr buf int size in`
    cwd println
    0 return
end
```

This procedure puts `1` (sys_write), `1` (stdout), `"Hello, World!\n"` (const char *buffer) and `14` (size_t length) onto the stack and then calls syscall with `4` arguments, this prints the `Hello, World!` text to the terminal

## Keywords

- `dup` duplicate the top value on stack
- `drop` delete the top value on stack
- `swap` swap to 2 values on stack
- `inc` increment top value on stack
- `dec` decrement top value on stack
- `dump` print out the top stack value (as int)
- `return` return top value on stack
- `syscall` execute syscall
- `derefc` dereference pointer on stack to char
- `derefi` dereference pointer on stack to int
- `proc` define process
- `in` end of process arguments, start of process body
- `true` push `1` on stack
- `false` push `0` on stack
- `buffer` create new buffer
