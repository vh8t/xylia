package lexer

import (
	"fmt"
	"os"
)

type TokenType uint
type Tokens []Token

type Lexer struct {
	Filename string
	Contents []byte
	Tokens   Tokens
	Position int
	Row      int
	Col      int
	Errors   []error
	IsLib    bool
	Clean    bool
}

type Token struct {
	Kind  TokenType
	Value string
	Row   int
	Col   int
}

const (
	OPERATOR TokenType = iota
	IDENTIFIER
	CHAR_ARG
	VOID_ARG
	BOOL_ARG
	INT_ARG
	PTR_ARG
	KEYWORD
	SYSCALL
	STRING
	IMPORT
	CALL
	PROC
	BOOL
	INT
)

func NewLexer(filename string, isLib, clean bool) (*Lexer, error) {
	contents, err := os.ReadFile(filename)
	if err != nil {
		return nil, err
	}

	lexer := &Lexer{
		Filename: filename,
		Contents: contents,
		Tokens:   Tokens{},
		Position: 0,
		Row:      1,
		Col:      1,
		Errors:   []error{},
		IsLib:    isLib,
		Clean:    clean,
	}
	return lexer, nil
}

func (t *Tokens) AppendToken(kind TokenType, value string, row, col int) {
	*t = append(*t, Token{
		Kind:  kind,
		Value: value,
		Row:   row,
		Col:   col,
	})
}

func (l *Lexer) AtEnd() bool {
	return len(l.Contents) == 0 || l.Position >= len(l.Contents)
}

func (l *Lexer) Peek() byte {
	if !l.AtEnd() {
		return l.Contents[l.Position]
	}
	return 0
}

func (l *Lexer) Move() {
	if l.Peek() == '\n' {
		l.Col = 0
		l.Row++
	}
	l.Col++
	l.Position++
}

func (l *Lexer) IsSpace() bool {
	ch := l.Peek()
	return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'
}

func (l *Lexer) LexArg() (TokenType, string) {
	var kind TokenType
	var buf string
	ch := l.Peek()
	row, col := l.Row, l.Col
	for l.IsAlpha() {
		l.Move()
		buf += string(ch)
		ch = l.Peek()
	}
	switch buf {
	case "ptr":
		kind = PTR_ARG
	case "int":
		kind = INT_ARG
	case "bool":
		kind = BOOL_ARG
	case "char":
		kind = CHAR_ARG
	case "in", "void":
		kind = VOID_ARG
		return kind, ""
	default:
		l.NewError("%d:%d %s Error: Invalid type : `%s`", row, col, l.Filename, buf)
		return VOID_ARG, ""
	}

	for l.IsSpace() {
		l.Move()
	}

	ch = l.Peek()
	row, col = l.Row, l.Col
	if !l.IsAlpha() {
		l.NewError("%d:%d %s Error: Invalid char : `%c`", row, col, l.Filename, ch)
		return VOID_ARG, ""
	}

	buf = ""
	for l.IsAlpha() || l.IsInt() {
		l.Move()
		buf += string(ch)
		ch = l.Peek()
	}
	return kind, buf
}

func (l *Lexer) LexInt() string {
	var buf string
	ch := l.Peek()
	for l.IsInt() {
		l.Move()
		buf += string(ch)
		ch = l.Peek()
	}
	return buf
}

func (l *Lexer) NewError(format string, a ...any) {
	l.Errors = append(l.Errors, fmt.Errorf(format, a...))
}

func (l *Lexer) IsOp() bool {
	ch := l.Peek()
	ops := []byte{'+', '-', '*', '/', '=', '<', '>', '!'}
	for _, op := range ops {
		if ch == op {
			return true
		}
	}
	return false
}

func (l *Lexer) IsInt() bool {
	ch := l.Peek()
	return '0' <= ch && ch <= '9'
}

func (l *Lexer) IsAlpha() bool {
	ch := l.Peek()
	return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch < 'Z') || ch == '_'
}

func (l *Lexer) LexToken() {
	for l.IsSpace() {
		l.Move()
	}

	if l.AtEnd() {
		return
	}

	ch := l.Peek()
	row, col := l.Row, l.Col

	if l.IsInt() {
		value := l.LexInt()
		l.Tokens.AppendToken(INT, value, row, col)
	} else if l.IsOp() {
		l.Tokens.AppendToken(OPERATOR, string(ch), row, col)
		l.Move()
	} else if ch == '#' {
		for l.Peek() != '\n' {
			l.Move()
		}
	} else if ch == '"' {
		var str string
		l.Move()
		for l.Peek() != '"' {
			char := l.Peek()
			if char == '\n' {
				l.NewError("%d:%d %s Error: Unclosed string", row, col, l.Filename)
				break
			}
			str += string(char)
			l.Move()
		}
		l.Tokens.AppendToken(STRING, str, row, col)
		l.Move()
	} else if l.IsAlpha() {
		var str string
		for l.IsAlpha() || l.IsInt() {
			str += string(l.Peek())
			l.Move()
		}

		switch str {
		case "true", "false":
			l.Tokens.AppendToken(BOOL, str, row, col)
		case "syscall":
			for l.IsSpace() {
				l.Move()
			}
			if !l.IsInt() {
				l.NewError("%d:%d %s Error: Expected integer got : `%c`", l.Row, l.Col, l.Filename, l.Peek())
			}
			value := l.LexInt()
			l.Tokens.AppendToken(SYSCALL, value, row, col)
		case "proc":
			for l.IsSpace() {
				l.Move()
			}
			if !l.IsAlpha() {
				l.NewError("%d:%d %s Error: Expected ident got : `%c`", l.Row, l.Col, l.Filename, l.Peek())
			}
			var value string
			for l.IsAlpha() || l.IsInt() {
				value += string(l.Peek())
				l.Move()
			}
			l.Tokens.AppendToken(PROC, value, row, col)
			for {
				for l.IsSpace() {
					l.Move()
				}

				row, col = l.Row, l.Col
				kind, name := l.LexArg()
				l.Tokens.AppendToken(kind, name, row, col)
				if kind == VOID_ARG {
					break
				}
			}
		case "import":
			for l.IsSpace() {
				l.Move()
			}
			value := ""
			for l.IsAlpha() || l.Peek() == '.' {
				value += string(l.Peek())
				l.Move()
			}
			l.Tokens.AppendToken(IMPORT, value, row, col)
		case "dup", "drop", "swap", "inc", "dec", "dump", "return", "if", "end", "else", "while", "do", "derefc", "derefi", "buffer":
			l.Tokens.AppendToken(KEYWORD, str, row, col)
		default:
			l.Tokens.AppendToken(CALL, str, row, col)
		}
	} else {
		l.NewError("%d:%d %s Error: Unknown character : `%c`", row, col, l.Filename, ch)
		l.Move()
	}
}

func (l *Lexer) Lex() {
	for !l.AtEnd() {
		l.LexToken()
	}
}
