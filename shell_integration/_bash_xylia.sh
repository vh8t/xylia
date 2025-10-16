_xylia_completions() {
  local cur prev subcmds opts files
  COMPREPLY=()
  cur="${COMP_WORDS[COMP_CWORD]}"
  prev="${COMP_WORDS[COMP_CWORD - 1]}"

  opts="--help -h --version -V --verbose -v --bash --zsh --bash"
  subcmds="run test repl docs help version"

  if [[ ${COMP_CWORD} -eq 1 ]]; then
    COMPREPLY=($(compgen -W "${opts} ${subcmds}" -- "$cur"))
    return 0
  fi

  case "${COMP_WORDS[1]}" in
  run)
    COMPREPLY=($(compgen -f -X '!*.xyl' -- "$cur"))
    ;;
  docs)
    COMPREPLY=($(compgen -d -- "$cur"))
    ;;
  repl | help | version)
    COMPREPLY=()
    ;;
  *)
    COMPREPLY=($(compgen -f -- "$cur"))
    ;;
  esac
}

complete -F _xylia_completions xylia
