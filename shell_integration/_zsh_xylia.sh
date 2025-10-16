_xylia() {
  local -a opts subcmds
  local curcontext="$curcontext" state line
  typeset -A opt_args

  opts=(
    '(-h --help)'{-h,--help}'[Show this help message]'
    '(-V --version)'{-V,--version}'[Show version information]'
    '(-v --verbose)'{-v,--verbose}'[Enable verbose output]'
    '(--zsh)'--zsh'[Print script to set up zsh shell integration]'
    '(--bash)'--bash'[Print script to set up bash shell integration]'
  )

  subcmds=(
    'run:Run a Xylia script file'
    'test:Runs the tests from a Xylia script file'
    'repl:Start interactive REPL session'
    'docs:Generate documentation from source files'
    'help:Show this help message'
    'version:Show version information'
  )

  _arguments -C \
    $opts \
    '1: :->subcmds' \
    '*::arg:->args'

  case $state in
    subcmds)
      _describe 'subcommand' subcmds
      ;;
    args)
      case $words[1] in
        run)
          _arguments \
            '1:file:_files -g "*.xyl"' \
            '*:args:'
          ;;
        docs)
          _arguments \
            '1:input:_files -/' \
            '*:args:'
          ;;
        repl)
          _message "Starts an interactive REPL session"
          ;;
        help|version)
          _message "No additional arguments"
          ;;
        *)
          _files -g "*.xyl"
          ;;
      esac
    ;;
  esac
}

compdef _xylia xylia
