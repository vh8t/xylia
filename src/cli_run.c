#include <stdio.h>
#include <stdlib.h>

#include "cli.h"
#include "memory.h"
#include "vm.h"

// Run a script file
static void run_file(const char *path, cli_context_t *ctx) {
  if (ctx->verbose) {
    printf("Running file: %s\n", path);
  }

  char *source = read_file(path);
  if (!source) {
    ctx->failed = true;
    return;
  }

  result_t result = interpret(source, path);
  free(source);

  if (result != RESULT_OK) {
    ctx->failed = true;
  }
}

// Implementation of the 'run' subcommand
cli_result_t cli_run(int argc, char **argv, cli_context_t *ctx) {
  if (argc < 1) {
    fprintf(stderr, "Error: 'run' command requires a file argument\n");
    fprintf(stderr, "Usage: xylia run <file> [args...]\n");
    return CLI_INVALID_ARGS;
  }

  const char *file = argv[0];

  // Check if file exists
  if (!cli_file_exists(file)) {
    fprintf(stderr, "Error: File '%s' not found\n", file);
    return CLI_ERROR;
  }

  // Set script arguments (everything after the filename)
  set_args(argc - 1, argv + 1);

  // Run the file
  run_file(file, ctx);

  return ctx->failed ? CLI_ERROR : CLI_SUCCESS;
}

// Implementation of the 'test' subcommand
cli_result_t cli_run_test(int argc, char **argv, cli_context_t *ctx) {
  if (argc < 1) {
    fprintf(stderr, "Error: 'test' command requires a file argument\n");
    fprintf(stderr, "Usage: xylia test <file> [args...]\n");
    return CLI_INVALID_ARGS;
  }

  const char *file = argv[0];

  // Check if file exists
  if (!cli_file_exists(file)) {
    fprintf(stderr, "Error: File '%s' not found\n", file);
    return CLI_ERROR;
  }

  load_test_functions();

  // Set script arguments (everything after the filename)
  set_args(argc - 1, argv + 1);

  // Run the file
  run_file(file, ctx);

  return ctx->failed ? CLI_ERROR : CLI_SUCCESS;
}
