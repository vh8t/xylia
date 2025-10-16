#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"

// Print version information
void cli_show_version(void) {
  printf("Xylia %s\n", XYLIA_VERSION);
  printf("A dynamically-typed programming language with optional type hints\n");
}

// Print help information
void cli_show_help(const char *program_name) {
  printf("Xylia %s - A dynamically-typed programming language\n\n",
         XYLIA_VERSION);

  printf("USAGE:\n");
  printf("    %s [OPTIONS] [SUBCOMMAND] [ARGS...]\n", program_name);
  printf("    %s [OPTIONS] <file> [ARGS...]     Run a Xylia script (default)\n",
         program_name);
  printf("\n");

  printf("OPTIONS:\n");
  printf("    -h, --help       Show this help message\n");
  printf("    -V, --version    Show version information\n");
  printf("    -v, --verbose    Enable verbose output\n");
  printf("    --zsh            Print script to set up zsh shell integration\n");
  printf(
      "    --bash           Print script to set up bash shell integration\n");
  printf("\n");

  printf("SUBCOMMANDS:\n");
  printf("    run <file>       Run a Xylia script file\n");
  printf("    test <file>      Runs the tests from a Xylia script file\n");
  printf("    repl             Start interactive REPL session\n");
  printf("    docs <input>     Generate documentation from source files\n");
  printf("    help             Show this help message\n");
  printf("    version          Show version information\n");
  printf("\n");

  printf("ARGUMENTS:\n");
  printf("    <file>           Path to Xylia script file to execute\n");
  printf("    [ARGS...]        Arguments passed to the script\n");
  printf("\n");

  printf("EXAMPLES:\n");
  printf("    %s hello.xyl                    # Run hello.xyl\n", program_name);
  printf("    %s run hello.xyl arg1 arg2      # Run hello.xyl with arguments\n",
         program_name);
  printf("    %s repl                         # Start interactive session\n",
         program_name);
  printf("    %s docs src/                    # Generate docs for directory\n",
         program_name);
  printf("    %s --version                    # Show version\n", program_name);
  printf("    %s --help                       # Show this help\n",
         program_name);
  printf("\n");

  printf("ENVIRONMENT:\n");
  printf("    XYL_HOME         Path to Xylia standard library (required)\n");
  printf("\n");

  printf("For more information, visit: https://github.com/vh8t/xylia\n");
}

// Check if string looks like a file path (contains .xyl extension or path
// separators)
bool cli_looks_like_file(const char *str) {
  if (!str)
    return false;

  // Check for .xyl extension
  size_t len = strlen(str);
  if (len > 4 && strcmp(str + len - 4, ".xyl") == 0) {
    return true;
  }

  // Check for path separators
  if (strchr(str, '/') != NULL || strchr(str, '\\') != NULL) {
    return true;
  }

  // Check if it's not a known subcommand
  if (strcmp(str, "run") == 0 || strcmp(str, "repl") == 0 ||
      strcmp(str, "docs") == 0 || strcmp(str, "help") == 0 ||
      strcmp(str, "version") == 0 || strcmp(str, "test") == 0) {
    return false;
  }

  return true;
}

// Check if file exists and is readable
bool cli_file_exists(const char *path) {
  FILE *f = fopen(path, "r");
  if (f) {
    fclose(f);
    return true;
  }
  return false;
}
