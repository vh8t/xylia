#ifndef XYL_CLI_H
#define XYL_CLI_H

#include <stdbool.h>

// CLI operation result codes
typedef enum {
  CLI_SUCCESS = 0,
  CLI_ERROR = 1,
  CLI_INVALID_ARGS = 2
} cli_result_t;

// CLI context for shared state
typedef struct {
  bool verbose;
  bool failed;
} cli_context_t;

// Subcommand function signatures
typedef cli_result_t (*subcommand_fn_t)(int argc, char **argv,
                                        cli_context_t *ctx);

// Core subcommand operations
cli_result_t cli_run(int argc, char **argv, cli_context_t *ctx);
cli_result_t cli_run_test(int argc, char **argv, cli_context_t *ctx);
cli_result_t cli_repl(int argc, char **argv, cli_context_t *ctx);
cli_result_t cli_docs(int argc, char **argv, cli_context_t *ctx);

// Utility functions
void cli_show_version(void);
void cli_show_help(const char *program_name);
bool cli_looks_like_file(const char *str);
bool cli_file_exists(const char *path);

#endif
