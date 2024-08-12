# Xylia

Xylia is a hobby stack based programming language, all the operations and values are stored on the stack.
The compiler is still missing many features and the programming language is pretty unstable as well.


# Prerequisities

Xylia only supports Lunux x86_64 for now but I am working on arm64 version as well.
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
