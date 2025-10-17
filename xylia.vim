" Vim syntax file
" Language: xylia
" Maintainer: vh8t
" Filenames: *.xyl

if exists("b:current_syntax")
  finish
endif

syn match xylIdentifier /\<[A-Za-z_][A-Za-z0-9_]*\>/
syn keyword xylKeyword class else enum for func if return super self let while operator unary assert break continue

syn keyword xylBoolean true false
syn keyword xylNil nil
syn keyword xylTypes bool number float function class instance builtin string vector list array file nan module range result enum Any

syn match xylModuleAccessor /\<[A-Za-z_][A-Za-z0-9_]*\>\ze::/

syn match xylEscape /\\\(\\\|n\|t\|r\|b\|x\x\{2}\|0[0-7]\{1,3}\)/ contained
syn region xylString start=/"/ skip=/\\"/ end=/"/ keepend extend contains=xylEscape

syn match xylNumber /\v\d+(\.\d+)?/

syn match xylOperator /[+\-*/^!%]/
syn match xylOperator /==\|!=\|<=\|>=\|<\|>\|&\||/
syn keyword xylOperator and or

syn match xylFuncDecl /\v(func\s+)\w+/ contains=ylKeyword,ylFuncName
syn match xylFuncName /\v\w+/ contained

syn match xylFuncCall /\<[A-Za-z_][A-Za-z0-9_]*\>\s*(/me=e-1

syn match xylComment "--.*$"

hi def link xylEscape         SpecialChar
" hi def link xylIdentifier     Identifier
hi def link xylKeyword        Keyword
hi def link xylModuleAccessor Structure
hi def link xylBoolean        Boolean
hi def link xylNil            Constant
hi def link xylTypes          Constant
hi def link xylComment        Comment
hi def link xylString         String
hi def link xylNumber         Number
hi def link xylOperator       Operator
hi def link xylFuncDecl       Keyword
hi def link xylFuncName       Function
hi def link xylFuncCall       Function

let b:current_syntax = "xylia"
