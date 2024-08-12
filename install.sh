#!/usr/bin/env sh

installed() {
  which "$1" > /dev/null 2>&1
}

MISSING_DEP=false
EXPORT_PATH="export XYL_PATH=\"\$HOME/.xylia\""

if ! installed as; then
  MISSING_DEP=true
  echo "Could not find \`as\` (GNU assembler)"
fi

if ! installed ld; then
  MISSING_DEP=true
  echo "Could not find \`ld\` (GNU linker)"
fi

if ! installed go; then
  MISSING_DEP=true
  echo "Could not find \`go\` (Go compiler)"
fi

if $MISSING_DEP; then
  echo "Could not install because of missing dependencied"
  echo "Please install them and run this script again"
  exit 1
fi

OS=$(uname)
ARCH=$(uname -m)

if [ "$OS" = "Linux" ]; then
  if [ "$ARCH" = "x86_64" ]; then
    # Linux Intel install
    git clone --depth 1 https://github.com/vh8t/xylia.git ~/.xylia
    cd ~/.xylia
    mkdir bin
    go build -o bin/xylia src/main.go
  elif [ "$ARCH" = "arm64" ] || [ "$ARCH" = "aarch64" ]; then
    # Linux ARM install
    echo "This platform is not yet supported"
    exit 1
  else
    echo "Unsupported Linux architecture: $ARCH"
    exit 1
  fi
elif [ "$OS" = "Darwin" ]; then
  if [ "$ARCH" = "x86_64" ]; then
    # macOS Intel install
    echo "This platform is not yet supported"
    exit 1
  elif [ "$ARCH" = "arm64" ] || [ "$ARCH" = "aarch64" ]; then
    # macOS Silicon install
    echo "This platform is not yet supported"
    exit 1
  else
    echo "Unsupported macOS architecture: $ARCH"
    exit 1
  fi
else
  echo "Unsupported operating system: $OS"
  exit 1
fi

SHELL_PATH=$(echo $SHELL)

case "$SHELL_PATH" in
  */bash)
    RC_FILE="$HOME/.bashrc"
    ;;
  */zsh)
    RC_FILE="$HOME/.zshrc"
    ;;
  */fish)
    RC_FILE="$HOME/.config/fish/config.fish"
    ;;
  */ksh)
    RC_FILE="$HOME/.kshrc"
    ;;
  */sh)
    RC_FILE="$HOME/.profile"
    ;;
  *)
    echo "Unknown shell"
    echo "Make sure to put this command in the shell profile"
    echo "$EXPORT_PATH"
    exit
    ;;
esac

echo -n "Do you want to save changes to $RC_FILE? [y/n] "
read -r user_input

user_input=$(echo "$user_input" | tr '[:upper:]' '[:lower:]')

case "$user_input" in
  y|yes)
    if [ -f "$RC_FILE" ]; then
      echo "$EXPORT_PATH" >> "$RC_FILE"
      echo "Exported \`$EXPORT_PATH\`"
    else
      echo "$RC_FILE does not exist."
      echo "Make sure to put this command in the shell profile"
      echo "$EXPORT_PATH"
    fi
    ;;
  n|no)
    echo "Make sure to put this command in the shell profile"
    echo "$EXPORT_PATH"
    ;;
  *)
    echo "Invalid input. Operation aborted."
    echo "Make sure to put this command in the shell profile"
    echo "$EXPORT_PATH"
    exit 1
    ;;
esac
