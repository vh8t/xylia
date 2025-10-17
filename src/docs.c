#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "docs.h"
#include "object.h"
#include "scanner.h"

// Utility function for portability (strndup might not be available on all
// systems)
static char *safe_strndup(const char *s, size_t n) {
  if (!s)
    return NULL;

  size_t len = strlen(s);
  if (len > n)
    len = n;

  char *result = (char *)malloc(len + 1);
  if (!result)
    return NULL;

  memcpy(result, s, len);
  result[len] = '\0';
  return result;
}

// Documentation comment functions
doc_comment_t *doc_comment_new(const char *content, size_t length, int line) {
  doc_comment_t *comment = (doc_comment_t *)malloc(sizeof(doc_comment_t));
  if (!comment)
    return NULL;

  comment->content = (char *)malloc(length + 1);
  if (!comment->content) {
    free(comment);
    return NULL;
  }

  memcpy(comment->content, content, length);
  comment->content[length] = '\0';
  comment->length = length;
  comment->line = line;
  comment->next = NULL;

  return comment;
}

void doc_comment_free(doc_comment_t *comment) {
  while (comment) {
    doc_comment_t *next = comment->next;
    free(comment->content);
    free(comment);
    comment = next;
  }
}

doc_comment_t *doc_comment_append(doc_comment_t *head,
                                  doc_comment_t *new_comment) {
  if (!head)
    return new_comment;

  doc_comment_t *current = head;
  while (current->next) {
    current = current->next;
  }
  current->next = new_comment;

  return head;
}

// Parameter documentation functions
doc_param_t *doc_param_new(const char *name, type_hint_t type,
                           doc_comment_t *doc) {
  doc_param_t *param = (doc_param_t *)malloc(sizeof(doc_param_t));
  if (!param)
    return NULL;

  size_t name_len = strlen(name);
  param->name = (char *)malloc(name_len + 1);
  if (!param->name) {
    free(param);
    return NULL;
  }

  strcpy(param->name, name);
  param->type = type;
  param->doc = doc;
  param->next = NULL;

  return param;
}

void doc_param_free(doc_param_t *param) {
  while (param) {
    doc_param_t *next = param->next;
    free(param->name);
    doc_comment_free(param->doc);
    free(param);
    param = next;
  }
}

// Function documentation functions
doc_function_t *doc_function_new(const char *name, int line) {
  doc_function_t *func = (doc_function_t *)malloc(sizeof(doc_function_t));
  if (!func)
    return NULL;

  size_t name_len = strlen(name);
  func->name = (char *)malloc(name_len + 1);
  if (!func->name) {
    free(func);
    return NULL;
  }

  strcpy(func->name, name);
  func->doc = NULL;
  func->params = NULL;
  func->return_type.has_hint = false;
  func->return_type.base_type = NULL;
  func->return_type.type_params = NULL;
  func->return_type.is_generic = false;
  func->return_doc = NULL;
  func->is_method = false;
  func->class_name = NULL;
  func->line = line;
  func->next = NULL;

  return func;
}

void doc_function_free(doc_function_t *func) {
  while (func) {
    doc_function_t *next = func->next;
    free(func->name);
    free(func->class_name);
    doc_comment_free(func->doc);
    doc_comment_free(func->return_doc);
    doc_param_free(func->params);
    free(func);
    func = next;
  }
}

// Variable documentation functions
doc_variable_t *doc_variable_new(const char *name, type_hint_t type, int line) {
  doc_variable_t *var = (doc_variable_t *)malloc(sizeof(doc_variable_t));
  if (!var)
    return NULL;

  size_t name_len = strlen(name);
  var->name = (char *)malloc(name_len + 1);
  if (!var->name) {
    free(var);
    return NULL;
  }

  strcpy(var->name, name);
  var->type = type;
  var->doc = NULL;
  var->line = line;
  var->next = NULL;

  return var;
}

void doc_variable_free(doc_variable_t *var) {
  while (var) {
    doc_variable_t *next = var->next;
    free(var->name);
    doc_comment_free(var->doc);
    free(var);
    var = next;
  }
}

// Class documentation functions
doc_class_t *doc_class_new(const char *name, int line) {
  doc_class_t *cls = (doc_class_t *)malloc(sizeof(doc_class_t));
  if (!cls)
    return NULL;

  size_t name_len = strlen(name);
  cls->name = (char *)malloc(name_len + 1);
  if (!cls->name) {
    free(cls);
    return NULL;
  }

  strcpy(cls->name, name);
  cls->doc = NULL;
  cls->methods = NULL;
  cls->fields = NULL;
  cls->line = line;
  cls->next = NULL;

  return cls;
}

void doc_class_free(doc_class_t *cls) {
  while (cls) {
    doc_class_t *next = cls->next;
    free(cls->name);
    doc_comment_free(cls->doc);
    doc_function_free(cls->methods);
    doc_variable_free(cls->fields);
    free(cls);
    cls = next;
  }
}

// Module documentation functions
doc_module_t *doc_module_new(const char *name, const char *path) {
  doc_module_t *module = (doc_module_t *)malloc(sizeof(doc_module_t));
  if (!module)
    return NULL;

  size_t name_len = strlen(name);
  size_t path_len = strlen(path);

  module->name = (char *)malloc(name_len + 1);
  module->path = (char *)malloc(path_len + 1);

  if (!module->name || !module->path) {
    free(module->name);
    free(module->path);
    free(module);
    return NULL;
  }

  strcpy(module->name, name);
  strcpy(module->path, path);
  module->doc = NULL;
  module->functions = NULL;
  module->variables = NULL;
  module->classes = NULL;

  return module;
}

void doc_module_free(doc_module_t *module) {
  if (module) {
    free(module->name);
    free(module->path);
    doc_comment_free(module->doc);
    doc_function_free(module->functions);
    doc_variable_free(module->variables);
    doc_class_free(module->classes);
    free(module);
  }
}

// Utility functions
char *doc_format_type_hint(const type_hint_t *hint) {
  if (!hint->has_hint) {
    return strdup("unknown");
  }

  if (!hint->is_generic) {
    return strdup(hint->base_type->chars);
  }

  // Format generic types like Result<T, E>
  size_t total_len = strlen(hint->base_type->chars) + 3; // base + "<>"
  type_param_t *param = hint->type_params;

  // Calculate total length needed
  while (param) {
    total_len += strlen(param->type_name->chars);
    if (param->next)
      total_len += 2; // ", "
    param = param->next;
  }

  char *result = (char *)malloc(total_len + 1);
  if (!result)
    return NULL;

  strcpy(result, hint->base_type->chars);
  strcat(result, "<");

  param = hint->type_params;
  bool first = true;
  while (param) {
    if (!first)
      strcat(result, ", ");
    strcat(result, param->type_name->chars);
    first = false;
    param = param->next;
  }

  strcat(result, ">");
  return result;
}

char *doc_escape_html(const char *text) {
  if (!text)
    return NULL;

  size_t len = strlen(text);
  size_t escaped_len = len * 6; // Worst case: all chars need escaping
  char *escaped = (char *)malloc(escaped_len + 1);
  if (!escaped)
    return NULL;

  char *out = escaped;
  for (const char *in = text; *in; in++) {
    switch (*in) {
    case '<':
      strcpy(out, "&lt;");
      out += 4;
      break;
    case '>':
      strcpy(out, "&gt;");
      out += 4;
      break;
    case '&':
      strcpy(out, "&amp;");
      out += 5;
      break;
    case '"':
      strcpy(out, "&quot;");
      out += 6;
      break;
    case '\'':
      strcpy(out, "&#39;");
      out += 5;
      break;
    default:
      *out++ = *in;
      break;
    }
  }
  *out = '\0';

  return escaped;
}

char *doc_escape_markdown(const char *text) {
  if (!text)
    return NULL;

  size_t len = strlen(text);
  size_t escaped_len = len * 2; // Worst case: all chars need escaping
  char *escaped = (char *)malloc(escaped_len + 1);
  if (!escaped)
    return NULL;

  char *out = escaped;
  for (const char *in = text; *in; in++) {
    switch (*in) {
    case '*':
    case '_':
    case '`':
    case '[':
    case ']':
    case '(':
    case ')':
    case '#':
    case '+':
    case '-':
    case '.':
    case '!':
    case '\\':
      *out++ = '\\';
      *out++ = *in;
      break;
    default:
      *out++ = *in;
      break;
    }
  }
  *out = '\0';

  return escaped;
}

// Scope tracking for better documentation extraction
typedef enum { SCOPE_MODULE, SCOPE_CLASS, SCOPE_FUNCTION } scope_type_t;

typedef struct scope_info {
  scope_type_t type;
  char *name;
  doc_class_t *current_class;
  int brace_depth;
  bool expecting_brace;
  bool seen_declaration;
  struct scope_info *parent;
} scope_info_t;

static scope_info_t *push_scope(scope_info_t *current, scope_type_t type,
                                const char *name, doc_class_t *cls) {
  scope_info_t *new_scope = (scope_info_t *)malloc(sizeof(scope_info_t));
  new_scope->type = type;
  new_scope->name = name ? strdup(name) : NULL;
  new_scope->current_class = cls;
  new_scope->brace_depth = 0;
  new_scope->expecting_brace = (type == SCOPE_CLASS || type == SCOPE_FUNCTION);
  new_scope->seen_declaration = false;
  new_scope->parent = current;
  return new_scope;
}

static scope_info_t *pop_scope(scope_info_t *current) {
  if (!current)
    return NULL;
  scope_info_t *parent = current->parent;
  free(current->name);
  free(current);
  return parent;
}

// Documentation extraction from source code
doc_module_t *extract_docs_from_source(const char *source, const char *path) {
  // Extract module name from path
  const char *module_name = strrchr(path, '/');
  if (module_name) {
    module_name++; // Skip the '/'
  } else {
    module_name = path;
  }

  // Remove .xyl extension if present
  char *name_copy = strdup(module_name);
  char *dot = strrchr(name_copy, '.');
  if (dot && strcmp(dot, ".xyl") == 0) {
    *dot = '\0';
  }

  doc_module_t *module = doc_module_new(name_copy, path);
  free(name_copy);

  if (!module)
    return NULL;

  // Initialize scanner for documentation extraction with doc comment mode
  // enabled
  init_scanner_with_doc_mode(source, true);

  doc_comment_t *pending_comments = NULL;
  doc_comment_t *module_comments = NULL;
  scope_info_t *current_scope = push_scope(NULL, SCOPE_MODULE, "module", NULL);
  current_scope->expecting_brace = false;
  token_t token;

  bool seen_first_declaration = false;
  int first_declaration_line = -1;

  do {
    token = scan_token();

    switch (token.type) {
    case TOK_DOC_COMMENT: {
      // Create new doc comment
      doc_comment_t *comment =
          doc_comment_new(token.start, token.length, token.row);
      if (comment) {
        // Check if there's a gap from previous comments
        if (pending_comments) {
          doc_comment_t *last_comment = pending_comments;
          while (last_comment->next)
            last_comment = last_comment->next;

          // If there's a gap of more than 1 line, and we haven't seen a
          // declaration, treat previous comments as module comments
          if (!seen_first_declaration &&
              comment->line - last_comment->line > 1) {
            module_comments = pending_comments;
            pending_comments = NULL;
          }
        }
        pending_comments = doc_comment_append(pending_comments, comment);
      }
      break;
    }

    case TOK_LBRACE: {
      if (current_scope) {
        if (current_scope->expecting_brace) {
          current_scope->expecting_brace = false;
          current_scope->brace_depth = 1;
        } else {
          current_scope->brace_depth++;
        }
      }
      break;
    }

    case TOK_RBRACE: {
      if (current_scope) {
        current_scope->brace_depth--;
        // If we've closed all braces for this scope, pop back to parent
        if (current_scope->brace_depth <= 0 &&
            current_scope->type != SCOPE_MODULE) {
          current_scope = pop_scope(current_scope);
        }
      }
      break;
    }

    case TOK_OPERATOR: {
      // Parse operator overload
      char temp_name[256] = "operator ";
      token_t paren_token = {
          0}; // Will hold the opening paren if we find it early
      bool have_paren = false;

      token_t op_token = scan_token();
      if (op_token.type == TOK_LBRACKET) {
        strcat(temp_name, "[");
        token_t rbracket = scan_token();
        if (rbracket.type == TOK_RBRACKET) {
          strcat(temp_name, "]");
          // Check if next token is assignment
          token_t next = scan_token();
          if (next.type == TOK_ASSIGN) {
            strcat(temp_name, "=");
          } else if (next.type == TOK_LPAREN) {
            // This is the opening paren for parameters
            paren_token = next;
            have_paren = true;
          }
          // Note: if it's neither = nor (, we'll scan for the paren later
        }
      } else if (op_token.type == TOK_ASSIGN) {
        strcat(temp_name, "=");
      } else if (op_token.type == TOK_PLUS) {
        strcat(temp_name, "+");
      } else if (op_token.type == TOK_MINUS) {
        strcat(temp_name, "-");
      } else {
        // Copy the token text directly
        size_t len = op_token.length;
        if (len < 240) {
          strncat(temp_name, op_token.start, len);
        }
      }

      char *func_name = strdup(temp_name);
      doc_function_t *func = doc_function_new(func_name, op_token.row);

      // Handle comment assignment for operators
      if (func && pending_comments) {
        func->doc = pending_comments;
        pending_comments = NULL;
      }

      current_scope->seen_declaration = true;

      // Parse function parameters and type hints
      token_t paren;
      if (have_paren) {
        paren = paren_token;
      } else {
        // Scan for opening parenthesis
        do {
          paren = scan_token();
        } while (paren.type != TOK_LPAREN && paren.type != TOK_EOF &&
                 paren.type != TOK_LBRACE);
      }

      if (paren.type == TOK_LPAREN) {
        // Parse parameters until we hit TOK_RPAREN
        token_t param_token = scan_token();
        int paren_depth = 1; // Track nested parentheses

        while (param_token.type != TOK_EOF && paren_depth > 0) {
          if (param_token.type == TOK_RPAREN) {
            paren_depth--;
            if (paren_depth == 0)
              break;
          } else if (param_token.type == TOK_LPAREN) {
            paren_depth++;
          } else if (param_token.type == TOK_IDENT && paren_depth == 1) {
            char *param_name =
                safe_strndup(param_token.start, param_token.length);

            // Look ahead for type hint
            token_t next_token = scan_token();
            type_hint_t param_type = {false, NULL, NULL, false};

            if (next_token.type == TOK_COLON) {
              token_t type_token = scan_token();
              if (type_token.type == TOK_IDENT) {
                param_type.has_hint = true;
                param_type.base_type =
                    copy_string(type_token.start, type_token.length, true);
              }
              // Skip to next parameter (comma or closing paren)
              next_token = scan_token();
            }

            // next_token should now be comma, closing paren, or something else
            doc_param_t *param = doc_param_new(param_name, param_type, NULL);
            if (param) {
              // Add parameter to end of list to maintain order
              if (func->params == NULL) {
                func->params = param;
              } else {
                doc_param_t *last = func->params;
                while (last->next)
                  last = last->next;
                last->next = param;
              }
            }
            free(param_name);

            // Handle the next token
            if (next_token.type == TOK_RPAREN) {
              paren_depth--;
              if (paren_depth == 0)
                break;
            } else if (next_token.type == TOK_COMMA) {
              param_token = scan_token();
              continue;
            } else if (next_token.type == TOK_LPAREN) {
              paren_depth++;
              param_token = scan_token();
              continue;
            } else {
              param_token = next_token;
              continue;
            }
          }
          param_token = scan_token();
        }

        // Check for return type - be careful not to consume unrelated tokens
        token_t arrow_token = scan_token();
        if (arrow_token.type == TOK_ARROW) {
          token_t return_type_token = scan_token();
          if (return_type_token.type == TOK_IDENT) {
            func->return_type.has_hint = true;
            func->return_type.base_type = copy_string(
                return_type_token.start, return_type_token.length, true);
          }
        }
        // If not arrow, we've consumed a token that might be important
        // Continue processing it in the main loop
      }

      // Determine where to add the function based on current scope
      if (current_scope && current_scope->type == SCOPE_CLASS &&
          current_scope->current_class && !current_scope->expecting_brace) {
        // Add as method to current class
        func->is_method = true;
        func->class_name = strdup(current_scope->name);
        if (current_scope->current_class->methods == NULL) {
          current_scope->current_class->methods = func;
        } else {
          doc_function_t *last = current_scope->current_class->methods;
          while (last->next)
            last = last->next;
          last->next = func;
        }
      } else {
        // Add as module-level function
        if (module->functions == NULL) {
          module->functions = func;
        } else {
          doc_function_t *last = module->functions;
          while (last->next)
            last = last->next;
          last->next = func;
        }
      }

      // Push function scope
      current_scope =
          push_scope(current_scope, SCOPE_FUNCTION, func_name,
                     current_scope ? current_scope->current_class : NULL);
      free(func_name);
      break;
    }

    case TOK_FUNC: {
      // Parse regular function declaration
      token_t name_token = scan_token();
      if (name_token.type == TOK_IDENT) {
        char *func_name = safe_strndup(name_token.start, name_token.length);

        doc_function_t *func = doc_function_new(func_name, name_token.row);

        // Handle comment assignment
        if (!seen_first_declaration) {
          seen_first_declaration = true;
          first_declaration_line = name_token.row;

          // Check if pending comments are separated from this declaration
          if (pending_comments) {
            doc_comment_t *last_comment = pending_comments;
            while (last_comment->next)
              last_comment = last_comment->next;

            // If there's a gap between comments and declaration, treat as
            // module comments
            if (name_token.row - last_comment->line > 1) {
              module_comments = pending_comments;
              pending_comments = NULL;
            }
          }
        }

        if (func && pending_comments) {
          func->doc = pending_comments;
          pending_comments = NULL;
        }

        current_scope->seen_declaration = true;

        // Parse function parameters and type hints
        token_t paren = scan_token();
        if (paren.type == TOK_LPAREN) {
          // Parse parameters until we hit TOK_RPAREN
          token_t param_token = scan_token();
          while (param_token.type != TOK_RPAREN &&
                 param_token.type != TOK_EOF) {
            if (param_token.type == TOK_IDENT) {
              char *param_name =
                  safe_strndup(param_token.start, param_token.length);

              // Look ahead for type hint
              token_t next_token = scan_token();
              type_hint_t param_type = {false, NULL, NULL, false};

              if (next_token.type == TOK_COLON) {
                token_t type_token = scan_token();
                if (type_token.type == TOK_IDENT) {
                  param_type.has_hint = true;
                  param_type.base_type =
                      copy_string(type_token.start, type_token.length, true);
                }
                // Skip to next parameter (comma or closing paren)
                next_token = scan_token();
              }

              // next_token should now be comma, closing paren, or something
              // else
              doc_param_t *param = doc_param_new(param_name, param_type, NULL);
              if (param) {
                // Add parameter to end of list to maintain order
                if (func->params == NULL) {
                  func->params = param;
                } else {
                  doc_param_t *last = func->params;
                  while (last->next)
                    last = last->next;
                  last->next = param;
                }
              }
              free(param_name);

              // If we hit closing paren, we're done
              if (next_token.type == TOK_RPAREN) {
                break;
              }
              // If we hit comma, continue to next parameter
              if (next_token.type == TOK_COMMA) {
                param_token = scan_token();
                continue;
              }
              // Otherwise, move to next token
              param_token = next_token;
            } else {
              // Skip non-identifier tokens
              param_token = scan_token();
            }
          }

          // Check for return type - peek ahead without consuming tokens if not
          // arrow
          token_t arrow_token = scan_token();
          if (arrow_token.type == TOK_ARROW) {
            token_t return_type_token = scan_token();
            if (return_type_token.type == TOK_IDENT) {
              func->return_type.has_hint = true;
              func->return_type.base_type = copy_string(
                  return_type_token.start, return_type_token.length, true);
            }
          }
          // Note: If not arrow, we've already consumed that token, which is
          // fine as it will be processed by the main scanner loop
        }

        // Determine where to add the function based on current scope
        if (current_scope && current_scope->type == SCOPE_CLASS &&
            current_scope->current_class && !current_scope->expecting_brace) {
          // Add as method to current class
          func->is_method = true;
          func->class_name = strdup(current_scope->name);
          if (current_scope->current_class->methods == NULL) {
            current_scope->current_class->methods = func;
          } else {
            doc_function_t *last = current_scope->current_class->methods;
            while (last->next)
              last = last->next;
            last->next = func;
          }
        } else {
          // Add as module-level function
          if (module->functions == NULL) {
            module->functions = func;
          } else {
            doc_function_t *last = module->functions;
            while (last->next)
              last = last->next;
            last->next = func;
          }
        }

        // Push function scope
        current_scope =
            push_scope(current_scope, SCOPE_FUNCTION, func_name,
                       current_scope ? current_scope->current_class : NULL);
        free(func_name);
      }
      break;
    }

    case TOK_LET: {
      // Only process module-level variables
      if (current_scope && current_scope->type == SCOPE_MODULE) {
        token_t name_token = scan_token();
        if (name_token.type == TOK_IDENT) {
          char *var_name = safe_strndup(name_token.start, name_token.length);

          // Check for type hint
          token_t colon_token = scan_token();
          type_hint_t var_type = {false, NULL, NULL, false};
          if (colon_token.type == TOK_COLON) {
            token_t type_token = scan_token();
            if (type_token.type == TOK_IDENT) {
              var_type.has_hint = true;
              var_type.base_type =
                  copy_string(type_token.start, type_token.length, true);
            }
          }

          doc_variable_t *var =
              doc_variable_new(var_name, var_type, name_token.row);
          free(var_name);

          // Handle comment assignment
          if (!seen_first_declaration) {
            seen_first_declaration = true;
            first_declaration_line = name_token.row;

            // Check if pending comments are separated from this declaration
            if (pending_comments) {
              doc_comment_t *last_comment = pending_comments;
              while (last_comment->next)
                last_comment = last_comment->next;

              // If there's a gap between comments and declaration, treat as
              // module comments
              if (name_token.row - last_comment->line > 1) {
                module_comments = pending_comments;
                pending_comments = NULL;
              }
            }
          }

          if (var && pending_comments) {
            var->doc = pending_comments;
            pending_comments = NULL;
          }

          current_scope->seen_declaration = true;

          // Add to module variables list
          if (var) {
            if (module->variables == NULL) {
              module->variables = var;
            } else {
              doc_variable_t *last = module->variables;
              while (last->next)
                last = last->next;
              last->next = var;
            }
          }
        }
      } else {
        // Skip local variables - just clear pending comments
        doc_comment_free(pending_comments);
        pending_comments = NULL;
      }
      break;
    }

    case TOK_CLASS: {
      // Parse class declaration
      token_t name_token = scan_token();
      if (name_token.type == TOK_IDENT) {
        char *class_name = safe_strndup(name_token.start, name_token.length);
        doc_class_t *cls = doc_class_new(class_name, name_token.row);

        // Handle comment assignment
        if (!seen_first_declaration) {
          seen_first_declaration = true;
          first_declaration_line = name_token.row;

          // Check if pending comments are separated from this declaration
          if (pending_comments) {
            doc_comment_t *last_comment = pending_comments;
            while (last_comment->next)
              last_comment = last_comment->next;

            // If there's a gap between comments and declaration, treat as
            // module comments
            if (name_token.row - last_comment->line > 1) {
              module_comments = pending_comments;
              pending_comments = NULL;
            }
          }
        }

        if (cls && pending_comments) {
          cls->doc = pending_comments;
          pending_comments = NULL;
        }

        current_scope->seen_declaration = true;

        // Add to module classes list
        if (cls) {
          if (module->classes == NULL) {
            module->classes = cls;
          } else {
            doc_class_t *last = module->classes;
            while (last->next)
              last = last->next;
            last->next = cls;
          }
        }

        // Push class scope
        current_scope = push_scope(current_scope, SCOPE_CLASS, class_name, cls);
        free(class_name);
      }
      break;
    }

    default:
      // For other tokens, clear pending comments if they're not followed by a
      // declaration
      if (token.type != TOK_EOF && token.type != TOK_DOC_COMMENT &&
          token.type != TOK_FUNC && token.type != TOK_LET &&
          token.type != TOK_CLASS && token.type != TOK_LBRACE &&
          token.type != TOK_RBRACE) {
        // Only clear if we hit a statement/expression that's not
        // whitespace/structural
        if (token.type != TOK_LPAREN && token.type != TOK_RPAREN &&
            token.type != TOK_SEMICOLON && token.type != TOK_COLON &&
            token.type != TOK_ARROW && token.type != TOK_IDENT) {
          doc_comment_free(pending_comments);
          pending_comments = NULL;
        }
      }
      break;
    }
  } while (token.type != TOK_EOF);

  // Assign module comments if we collected any
  if (module_comments) {
    module->doc = module_comments;
  }

  // Clean up any remaining comments and scopes
  doc_comment_free(pending_comments);
  while (current_scope) {
    current_scope = pop_scope(current_scope);
  }

  return module;
}

// Markdown generation functions
static void write_comment_md(FILE *f, doc_comment_t *comment,
                             const char *prefix) {
  while (comment) {
    if (comment->content && strlen(comment->content) > 0) {
      fprintf(f, "%s%s\n", prefix ? prefix : "", comment->content);
    }
    comment = comment->next;
  }
}

static void write_type_hint_md(FILE *f, const type_hint_t *hint) {
  if (!hint->has_hint) {
    fprintf(f, "`unknown`");
    return;
  }

  fprintf(f, "`%s", hint->base_type->chars);

  if (hint->is_generic && hint->type_params) {
    fprintf(f, "<");
    type_param_t *param = hint->type_params;
    bool first = true;
    while (param) {
      if (!first)
        fprintf(f, ", ");
      fprintf(f, "%s", param->type_name->chars);
      first = false;
      param = param->next;
    }
    fprintf(f, ">");
  }

  fprintf(f, "`");
}

static void write_function_md(FILE *f, doc_function_t *func,
                              const char *class_name) {
  if (class_name == NULL)
    fprintf(f, "### `%s`\n\n", func->name);
  else
    fprintf(f, "### `%s::%s`\n\n", class_name, func->name);

  // Function signature
  fprintf(f, "```xylia\n");
  if (class_name == NULL)
    fprintf(f, "func %s(", func->name);
  else
    fprintf(f, "func %s::%s(", class_name, func->name);

  doc_param_t *param = func->params;
  bool first = true;
  while (param) {
    if (!first)
      fprintf(f, ", ");
    fprintf(f, "%s", param->name);
    if (param->type.has_hint) {
      fprintf(f, ": ");
      char *type_str = doc_format_type_hint(&param->type);
      fprintf(f, "%s", type_str);
      free(type_str);
    }
    first = false;
    param = param->next;
  }

  fprintf(f, ")");

  if (func->return_type.has_hint) {
    fprintf(f, " -> ");
    char *type_str = doc_format_type_hint(&func->return_type);
    fprintf(f, "%s", type_str);
    free(type_str);
  }

  fprintf(f, "\n```\n\n");

  // Documentation
  if (func->doc) {
    write_comment_md(f, func->doc, "");
    fprintf(f, "\n");
  }

  // Parameters
  if (func->params) {
    fprintf(f, "**Parameters:**\n\n");
    param = func->params;
    while (param) {
      fprintf(f, "- `%s`", param->name);
      if (param->type.has_hint) {
        fprintf(f, " (");
        write_type_hint_md(f, &param->type);
        fprintf(f, ")");
      }
      if (param->doc) {
        fprintf(f, ": ");
        write_comment_md(f, param->doc, "");
      }
      fprintf(f, "\n");
      param = param->next;
    }
    fprintf(f, "\n");
  }

  // Return value
  if (func->return_type.has_hint || func->return_doc) {
    fprintf(f, "**Returns:**");
    if (func->return_type.has_hint) {
      fprintf(f, " ");
      write_type_hint_md(f, &func->return_type);
      fprintf(f, " ");
    }
    if (func->return_doc) {
      fprintf(f, "\n\n");
      write_comment_md(f, func->return_doc, "");
    }
    fprintf(f, "\n\n");
  }
}

static void write_variable_md(FILE *f, doc_variable_t *var) {
  fprintf(f, "### %s\n\n", var->name);

  if (var->type.has_hint) {
    fprintf(f, "**Type:** ");
    write_type_hint_md(f, &var->type);
    fprintf(f, "\n\n");
  }

  if (var->doc) {
    write_comment_md(f, var->doc, "");
    fprintf(f, "\n");
  }
}

static void write_class_md(FILE *f, doc_class_t *cls) {
  fprintf(f, "## %s\n\n", cls->name);

  if (cls->doc) {
    write_comment_md(f, cls->doc, "");
    fprintf(f, "\n");
  }

  // Class methods
  if (cls->methods) {
    fprintf(f, "### Methods\n\n");
    doc_function_t *method = cls->methods;
    while (method) {
      write_function_md(f, method, cls->name);
      method = method->next;
    }
  }

  // Class fields
  if (cls->fields) {
    fprintf(f, "### Fields\n\n");
    doc_variable_t *field = cls->fields;
    while (field) {
      write_variable_md(f, field);
      field = field->next;
    }
  }
}

bool generate_docs_markdown(doc_module_t *module, const char *output_path) {
  if (!module || !output_path)
    return false;

  FILE *f = fopen(output_path, "w");
  if (!f)
    return false;

  // Module header
  fprintf(f, "# %s\n\n", module->name);

  if (module->doc) {
    write_comment_md(f, module->doc, "");
    fprintf(f, "\n");
  }

  // Table of contents
  bool has_content = module->functions || module->variables || module->classes;
  if (has_content) {
    fprintf(f, "## Table of Contents\n\n");

    if (module->functions) {
      fprintf(f, "- [Functions](#functions)\n");
      doc_function_t *func = module->functions;
      while (func) {
        fprintf(f, "  - [%s](#%s)\n", func->name, func->name);
        func = func->next;
      }
    }

    if (module->variables) {
      fprintf(f, "- [Variables](#variables)\n");
      doc_variable_t *var = module->variables;
      while (var) {
        fprintf(f, "  - [%s](#%s)\n", var->name, var->name);
        var = var->next;
      }
    }

    if (module->classes) {
      fprintf(f, "- [Classes](#classes)\n");
      doc_class_t *cls = module->classes;
      while (cls) {
        fprintf(f, "  - [%s](#%s)\n", cls->name, cls->name);
        cls = cls->next;
      }
    }

    fprintf(f, "\n");
  }

  // Functions section
  if (module->functions) {
    fprintf(f, "## Functions\n\n");
    doc_function_t *func = module->functions;
    while (func) {
      if (!func->is_method) { // Only include non-method functions
        write_function_md(f, func, NULL);
      }
      func = func->next;
    }
  }

  // Variables section
  if (module->variables) {
    fprintf(f, "## Variables\n\n");
    doc_variable_t *var = module->variables;
    while (var) {
      write_variable_md(f, var);
      var = var->next;
    }
  }

  // Classes section
  if (module->classes) {
    fprintf(f, "## Classes\n\n");
    doc_class_t *cls = module->classes;
    while (cls) {
      write_class_md(f, cls);
      cls = cls->next;
    }
  }

  fclose(f);
  return true;
}

// Main documentation generation function
bool generate_docs(doc_module_t *module, const doc_options_t *options) {
  if (!module || !options)
    return false;

  switch (options->format) {
  case DOC_FORMAT_MARKDOWN: {
    char output_path[1024];
    snprintf(output_path, sizeof(output_path), "%s/%s.md", options->output_dir,
             module->name);
    return generate_docs_markdown(module, output_path);
  }

  case DOC_FORMAT_HTML:
    return generate_docs_html(module, options);

  case DOC_FORMAT_JSON:
    // TODO: Implement JSON output
    return false;

  default:
    return false;
  }
}

// HTML generation functions
static void write_html_header(FILE *f, const char *title, const char *theme) {
  fprintf(f, "<!DOCTYPE html>\n");
  fprintf(f, "<html lang=\"en\">\n");
  fprintf(f, "<head>\n");
  fprintf(f, "    <meta charset=\"UTF-8\">\n");
  fprintf(f, "    <meta name=\"viewport\" content=\"width=device-width, "
             "initial-scale=1.0\">\n");
  fprintf(f, "    <title>%s</title>\n", title ? title : "Documentation");
  fprintf(f, "    <style>\n");
  fprintf(f, "        body { font-family: -apple-system, BlinkMacSystemFont, "
             "'Segoe UI', Roboto, sans-serif; }\n");
  fprintf(f, "        .container { max-width: 1200px; margin: 0 auto; padding: "
             "20px; }\n");
  fprintf(f, "        .header { border-bottom: 1px solid #e1e5e9; "
             "margin-bottom: 30px; padding-bottom: 20px; }\n");
  fprintf(f, "        .nav { background: #f6f8fa; padding: 15px; "
             "border-radius: 6px; margin-bottom: 30px; }\n");
  fprintf(f, "        .nav ul { list-style: none; margin: 0; padding: 0; }\n");
  fprintf(f,
          "        .nav li { display: inline-block; margin-right: 20px; }\n");
  fprintf(f, "        .nav a { text-decoration: none; color: #586069; "
             "font-weight: 500; }\n");
  fprintf(f, "        .nav a:hover { color: #0366d6; }\n");
  fprintf(f, "        .section { margin-bottom: 40px; }\n");
  fprintf(f, "        .function, .variable, .class { border: 1px solid "
             "#e1e5e9; border-radius: 6px; margin-bottom: 20px; }\n");
  fprintf(
      f,
      "        .function-header, .variable-header, .class-header { background: "
      "#f6f8fa; padding: 15px; border-bottom: 1px solid #e1e5e9; }\n");
  fprintf(f, "        .function-body, .variable-body, .class-body { padding: "
             "15px; }\n");
  fprintf(f, "        .signature { background: #f6f8fa; border: 1px solid "
             "#e1e5e9; border-radius: 3px; padding: 10px; font-family: "
             "monospace; margin: 10px 0; }\n");
  fprintf(f, "        .type { color: #d73a49; font-weight: bold; }\n");
  fprintf(f, "        .param-list { margin: 10px 0; }\n");
  fprintf(f, "        .param { margin: 5px 0; }\n");
  fprintf(f, "        .returns { margin: 10px 0; font-weight: bold; }\n");
  fprintf(f, "        h1 { color: #24292e; }\n");
  fprintf(f, "        h2 { color: #24292e; border-bottom: 1px solid #e1e5e9; "
             "padding-bottom: 10px; }\n");
  fprintf(f, "        h3 { color: #586069; }\n");
  fprintf(f, "        code { background: #f6f8fa; padding: 2px 4px; "
             "border-radius: 3px; font-size: 0.9em; }\n");
  fprintf(f,
          "        .footer { margin-top: 50px; padding-top: 20px; border-top: "
          "1px solid #e1e5e9; color: #586069; text-align: center; }\n");
  fprintf(f, "    </style>\n");
  fprintf(f, "</head>\n");
  fprintf(f, "<body>\n");
  fprintf(f, "    <div class=\"container\">\n");
}

static void write_html_footer(FILE *f) {
  fprintf(f, "        <div class=\"footer\">\n");
  fprintf(f, "            <p>Generated by Xylia docs v%s</p>\n", XYLIA_VERSION);
  fprintf(f, "        </div>\n");
  fprintf(f, "    </div>\n");
  fprintf(f, "</body>\n");
  fprintf(f, "</html>\n");
}

static void write_comment_html(FILE *f, doc_comment_t *comment) {
  while (comment) {
    if (comment->content && strlen(comment->content) > 0) {
      char *escaped = doc_escape_html(comment->content);
      fprintf(f, "<p>%s</p>", escaped ? escaped : comment->content);
      free(escaped);
    }
    comment = comment->next;
  }
}

static void write_type_hint_html(FILE *f, const type_hint_t *hint) {
  if (!hint->has_hint) {
    fprintf(f, "<span class=\"type\">unknown</span>");
    return;
  }

  fprintf(f, "<span class=\"type\">%s", hint->base_type->chars);

  if (hint->is_generic && hint->type_params) {
    fprintf(f, "&lt;");
    type_param_t *param = hint->type_params;
    bool first = true;
    while (param) {
      if (!first)
        fprintf(f, ", ");
      fprintf(f, "%s", param->type_name->chars);
      first = false;
      param = param->next;
    }
    fprintf(f, "&gt;");
  }

  fprintf(f, "</span>");
}

static void write_function_html(FILE *f, doc_function_t *func,
                                const char *class_name) {
  fprintf(f, "        <div class=\"function\" id=\"%s\">\n", func->name);
  fprintf(f, "            <div class=\"function-header\">\n");
  if (class_name == NULL)
    fprintf(f, "                <h3>%s</h3>\n", func->name);
  else
    fprintf(f, "                <h3>%s::%s</h3>\n", class_name, func->name);
  fprintf(f, "            </div>\n");
  fprintf(f, "            <div class=\"function-body\">\n");

  // Function signature
  fprintf(f, "                <div class=\"signature\">\n");
  if (class_name == NULL)
    fprintf(f, "                    <code>func %s(", func->name);
  else
    fprintf(f, "                    <code>func %s::%s(", class_name,
            func->name);

  doc_param_t *param = func->params;
  bool first = true;
  while (param) {
    if (!first)
      fprintf(f, ", ");
    fprintf(f, "%s", param->name);
    if (param->type.has_hint) {
      fprintf(f, ": ");
      write_type_hint_html(f, &param->type);
    }
    first = false;
    param = param->next;
  }

  fprintf(f, ")");

  if (func->return_type.has_hint) {
    fprintf(f, " -&gt; ");
    write_type_hint_html(f, &func->return_type);
  }

  fprintf(f, "</code>\n");
  fprintf(f, "                </div>\n");

  // Documentation
  if (func->doc) {
    write_comment_html(f, func->doc);
  }

  // Parameters
  if (func->params) {
    fprintf(f, "                <h4>Parameters:</h4>\n");
    fprintf(f, "                <div class=\"param-list\">\n");
    param = func->params;
    while (param) {
      fprintf(f, "                    <div class=\"param\">\n");
      fprintf(f, "                        <code>%s</code>", param->name);
      if (param->type.has_hint) {
        fprintf(f, " (");
        write_type_hint_html(f, &param->type);
        fprintf(f, ")");
      }
      if (param->doc) {
        fprintf(f, ": ");
        write_comment_html(f, param->doc);
      }
      fprintf(f, "                    </div>\n");
      param = param->next;
    }
    fprintf(f, "                </div>\n");
  }

  // Return value
  if (func->return_type.has_hint || func->return_doc) {
    fprintf(f, "                <div class=\"returns\">\n");
    fprintf(f, "                    <h4>Returns:</h4>\n");
    if (func->return_type.has_hint) {
      write_type_hint_html(f, &func->return_type);
      fprintf(f, " ");
    }
    if (func->return_doc) {
      write_comment_html(f, func->return_doc);
    }
    fprintf(f, "                </div>\n");
  }

  fprintf(f, "            </div>\n");
  fprintf(f, "        </div>\n");
}

static void write_variable_html(FILE *f, doc_variable_t *var) {
  fprintf(f, "        <div class=\"variable\" id=\"%s\">\n", var->name);
  fprintf(f, "            <div class=\"variable-header\">\n");
  fprintf(f, "                <h3>%s</h3>\n", var->name);
  fprintf(f, "            </div>\n");
  fprintf(f, "            <div class=\"variable-body\">\n");

  if (var->type.has_hint) {
    fprintf(f, "                <p><strong>Type:</strong> ");
    write_type_hint_html(f, &var->type);
    fprintf(f, "</p>\n");
  }

  if (var->doc) {
    write_comment_html(f, var->doc);
  }

  fprintf(f, "            </div>\n");
  fprintf(f, "        </div>\n");
}

static void write_class_html(FILE *f, doc_class_t *cls) {
  fprintf(f, "        <div class=\"class\" id=\"%s\">\n", cls->name);
  fprintf(f, "            <div class=\"class-header\">\n");
  fprintf(f, "                <h2>%s</h2>\n", cls->name);
  fprintf(f, "            </div>\n");
  fprintf(f, "            <div class=\"class-body\">\n");

  if (cls->doc) {
    write_comment_html(f, cls->doc);
  }

  // Class methods
  if (cls->methods) {
    fprintf(f, "                <h3>Methods</h3>\n");
    doc_function_t *method = cls->methods;
    while (method) {
      write_function_html(f, method, cls->name);
      method = method->next;
    }
  }

  // Class fields
  if (cls->fields) {
    fprintf(f, "                <h3>Fields</h3>\n");
    doc_variable_t *field = cls->fields;
    while (field) {
      write_variable_html(f, field);
      field = field->next;
    }
  }

  fprintf(f, "            </div>\n");
  fprintf(f, "        </div>\n");
}

bool generate_docs_html(doc_module_t *module, const doc_options_t *options) {
  if (!module || !options)
    return false;

  char output_path[1024];
  snprintf(output_path, sizeof(output_path), "%s/%s.html", options->output_dir,
           module->name);

  FILE *f = fopen(output_path, "w");
  if (!f)
    return false;

  char title[256];
  snprintf(title, sizeof(title), "%s - %s", module->name,
           options->title ? options->title : "Documentation");

  write_html_header(f, title, options->theme);

  // Module header
  fprintf(f, "        <div class=\"header\">\n");
  fprintf(f, "            <h1>%s</h1>\n", module->name);
  if (module->doc) {
    write_comment_html(f, module->doc);
  }
  fprintf(f, "        </div>\n");

  // Navigation
  bool has_content = module->functions || module->variables || module->classes;
  if (has_content) {
    fprintf(f, "        <div class=\"nav\">\n");
    fprintf(f, "            <ul>\n");

    if (module->functions) {
      fprintf(
          f, "                <li><a href=\"#functions\">Functions</a></li>\n");
    }

    if (module->variables) {
      fprintf(
          f, "                <li><a href=\"#variables\">Variables</a></li>\n");
    }

    if (module->classes) {
      fprintf(f, "                <li><a href=\"#classes\">Classes</a></li>\n");
    }

    fprintf(f, "            </ul>\n");
    fprintf(f, "        </div>\n");
  }

  // Functions section
  if (module->functions) {
    fprintf(f, "        <div class=\"section\" id=\"functions\">\n");
    fprintf(f, "            <h2>Functions</h2>\n");
    doc_function_t *func = module->functions;
    while (func) {
      if (!func->is_method) { // Only include non-method functions
        write_function_html(f, func, NULL);
      }
      func = func->next;
    }
    fprintf(f, "        </div>\n");
  }

  // Variables section
  if (module->variables) {
    fprintf(f, "        <div class=\"section\" id=\"variables\">\n");
    fprintf(f, "            <h2>Variables</h2>\n");
    doc_variable_t *var = module->variables;
    while (var) {
      write_variable_html(f, var);
      var = var->next;
    }
    fprintf(f, "        </div>\n");
  }

  // Classes section
  if (module->classes) {
    fprintf(f, "        <div class=\"section\" id=\"classes\">\n");
    fprintf(f, "            <h2>Classes</h2>\n");
    doc_class_t *cls = module->classes;
    while (cls) {
      write_class_html(f, cls);
      cls = cls->next;
    }
    fprintf(f, "        </div>\n");
  }

  write_html_footer(f);

  fclose(f);
  return true;
}
