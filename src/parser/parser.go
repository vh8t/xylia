package parser

import (
	"crypto/rand"
	"fmt"
	"math/big"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"xyl/src/lexer"
)

const (
	upper  = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	lower  = "abcdefghijklmnopqrstuvwxyz"
	digits = "0123456789"

	printNumText = `dump:
  pushq %rbp
  movq %rsp, %rbp
  subq $64, %rsp
  movq %rdi, -56(%rbp)
  movq $1, -8(%rbp)
  movl $32, %eax
  subq -8(%rbp), %rax
  movb $10, -48(%rbp,%rax)
.L2:
  movq -56(%rbp), %rcx
  movabsq $-3689348814741910323, %rdx
  movq %rcx, %rax
  mulq %rdx
  shrq $3, %rdx
  movq %rdx, %rax
  salq $2, %rax
  addq %rdx, %rax
  addq %rax, %rax
  subq %rax, %rcx
  movq %rcx, %rdx
  movl %edx, %eax
  leal 48(%rax), %edx
  movl $31, %eax
  subq -8(%rbp), %rax
  movb %dl, -48(%rbp,%rax)
  addq $1, -8(%rbp)
  movq -56(%rbp), %rax
  movabsq $-3689348814741910323, %rdx
  mulq %rdx
  movq %rdx, %rax
  shrq $3, %rax
  movq %rax, -56(%rbp)
  cmpq $0, -56(%rbp)
  jne .L2
  movl $32, %eax
  subq -8(%rbp), %rax
  leaq -48(%rbp), %rdx
  leaq (%rdx,%rax), %rcx
  movq -8(%rbp), %rax
  movq %rax, %rdx
  movq %rcx, %rsi
  movl $1, %edi
  movl $0, %eax
  movq $1,%rax
  syscall
  nop
  leave
  ret`
)

type Label struct {
	Name    string
	HasElse bool
	IsWhile bool
	IsFunc  bool
}

type Function struct {
	Name string
	Args map[string]int
}

func randLabel(length int, chars string) (string, error) {
	result := make([]byte, length)
	for i := range result {
		n, err := rand.Int(rand.Reader, big.NewInt(int64(len(chars))))
		if err != nil {
			return "", err
		}
		result[i] = chars[n.Int64()]
	}
	return string(result), nil
}

func contains(list []Function, target Function) bool {
	for _, i := range list {
		if i.Name == target.Name {
			return true
		}
	}
	return false
}

func containsArg(list []Function, target string) bool {
	for _, i := range list {
		if _, ok := i.Args[target]; ok {
			return true
		}
	}
	return false
}

func containsStr(list []Function, target string) bool {
	for _, i := range list {
		if i.Name == target {
			return true
		}
	}
	return false
}

func strContains(list []string, target string) bool {
	for _, i := range list {
		if i == target {
			return true
		}
	}
	return false
}

func findFunc(list []Function, target string) Function {
	for _, i := range list {
		if i.Name == target {
			return i
		}
	}
	return Function{}
}

func Parse(lex lexer.Lexer, libs []string) (string, string, []Function, []string) {
	if len(lex.Errors) != 0 {
		for _, err := range lex.Errors {
			fmt.Println(err)
		}
		os.Exit(1)
	}

	xylHome := os.Getenv("XYL_HOME")
	if xylHome == "" {
		fmt.Println("Error: Could not find `XYL_HOME` env variable")
		os.Exit(1)
	}

	registers := []string{"rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"}
	var text, data string
	var functions []Function
	var ifQueue []Label
	var funcQueue []Function

	for i := 0; i < len(lex.Tokens); i++ {
		token := lex.Tokens[i]
		if token.Kind == lexer.INT {
			text += "\t## PUSH ##\n"
			text += fmt.Sprintf("\tmovq $%s, %%rax\n", token.Value)
			text += "\tpush %rax\n"
		} else if token.Kind == lexer.OPERATOR {
			switch token.Value {
			case "+":
				text += "\t## ADD ##\n"
				text += "\tpop %rbx\n\tpop %rax\n"
				text += "\taddq %rbx, %rax\n"
				text += "\tpush %rax\n"
			case "-":
				text += "\t## SUB ##\n"
				text += "\tpop %rbx\n\tpop %rax\n"
				text += "\tsubq %rbx, %rax\n"
				text += "\tpush %rax\n"
			case "*":
				text += "\t## MUL ##\n"
				text += "\tpop %rbx\n\tpop %rax\n"
				text += "\timulq %rbx\n"
				text += "\tpush %rax\n"
			case "/":
				fmt.Println("Division not implemented")
				os.Exit(1)
			case "=":
				text += "\t## EQUAL ##\n"
				text += "\tpop %rax\n"
				text += "\tpop %rbx\n"
				text += "\txor %rcx, %rcx\n"
				text += "\tmovq $1, %rdx\n"
				text += "\tcmpq %rax, %rbx\n"
				text += "\tcmove %rdx, %rcx\n"
				text += "\tpush %rcx\n"
			case "!":
				text += "\t## NOT EQUAL ##\n"
				text += "\tpop %rax\n"
				text += "\tpop %rbx\n"
				text += "\txor %rcx, %rcx\n"
				text += "\tmovq $1, %rdx\n"
				text += "\tcmpq %rax, %rbx\n"
				text += "\tcmovne %rdx, %rcx\n"
				text += "\tpush %rcx\n"
			case "<":
				text += "\t## LESS THAN ##\n"
				text += "\tpop %rax\n"
				text += "\tpop %rbx\n"
				text += "\txor %rcx, %rcx\n"
				text += "\tmovq $1, %rdx\n"
				text += "\tcmpq %rax, %rbx\n"
				text += "\tcmovl %rdx, %rcx\n"
				text += "\tpush %rcx\n"
			case ">":
				text += "\t## GREATER THAN ##\n"
				text += "\tpop %rax\n"
				text += "\tpop %rbx\n"
				text += "\txor %rcx, %rcx\n"
				text += "\tmovq $1, %rdx\n"
				text += "\tcmpq %rax, %rbx\n"
				text += "\tcmovg %rdx, %rcx\n"
				text += "\tpush %rcx\n"
			}
		} else if token.Kind == lexer.BOOL {
			if token.Value == "true" {
				text += "\t## TRUE ##\n"
				text += "\tmovq $1, %rax\n"
				text += "\tpush %rax\n"
			} else if token.Value == "false" {
				text += "\t## FALSE ##\n"
				text += "\tmovq $0, %rax\n"
				text += "\tpush %rax\n"
			}
		} else if token.Kind == lexer.SYSCALL {
			num, err := strconv.Atoi(token.Value)
			if err != nil {
				fmt.Printf("%d:%d %s Error: Invalid number : `%s`\n", token.Row, token.Col, lex.Filename, token.Value)
				os.Exit(1)
			}
			if num > 7 {
				fmt.Printf("%d:%d %s Error: Syscall can only range from 1-7 got : `%d`\n", token.Row, token.Col, lex.Filename, num)
				os.Exit(1)
			}
			text += "\t## SYSCALL ##\n"
			for i := num - 1; i >= 0; i-- {
				text += fmt.Sprintf("\tpop %%%s\n", registers[i])
			}
			text += "\tsyscall\n"
			text += "\tpush %rax\n"
		} else if token.Kind == lexer.STRING {
			text += "\t## STRING ##\n"
			label, _ := randLabel(9, upper+lower+digits)
			text += fmt.Sprintf("\tmovq $_%s, %%rax\n", label)
			text += "\tpush %rax\n"
			data += fmt.Sprintf("\t_%s: .asciz \"%s\"\n", label, token.Value)
		} else if token.Kind == lexer.IMPORT {
			text += fmt.Sprintf("\t## IMPORT %s ##\n", token.Value)
			parts := strings.Split(token.Value, ".")
			if len(parts) == 0 {
				fmt.Printf("%d:%d %s Error: Import statement missing library\n", token.Row, token.Col, lex.Filename)
				os.Exit(1)
			}
			libPath := filepath.Join(xylHome, "lib", filepath.Join(parts...)) + ".xyl"
			if _, err := os.Stat(libPath); os.IsNotExist(err) {
				pwd, err := os.Getwd()
				if err != nil {
					fmt.Printf("%d:%d %s Error: Imported library could not be found : `%s`\n", token.Row, token.Col, lex.Filename, token.Value)
					os.Exit(1)
				}
				modulePath := filepath.Join(pwd, filepath.Join(parts...)) + ".xyl"
				if _, err := os.Stat(modulePath); os.IsNotExist(err) {
					fmt.Printf("%d:%d %s Error: Imported library could not be found : `%s`\n", token.Row, token.Col, lex.Filename, token.Value)
					os.Exit(1)
				}
				libPath = modulePath
			}
			if !strContains(libs, libPath) {
				l, err := lexer.NewLexer(libPath, true, lex.Clean)
				if err != nil {
					fmt.Printf("Could not compile lib `%s`\n%s\n", libPath, err.Error())
					os.Exit(1)
				}

				l.Lex()
				libText, libData, libFunctions, newLibs := Parse(*l, libs)
				text += libText
				data += libData
				for _, fn := range libFunctions {
					if contains(functions, fn) {
						fmt.Printf("Duplicate function `%s` imported\n", fn.Name)
						os.Exit(1)
					}
				}
				functions = append(functions, libFunctions...)
				libs = newLibs
				libs = append(libs, libPath)
			}
			text += fmt.Sprintf("\t## FILE %s ##\n", lex.Filename)
		} else if token.Kind == lexer.PROC {
			// TODO: Make sure the user cant use reserved keywords
			text += "## PROC ##\n"
			text += fmt.Sprintf("%s:\n", token.Value)
			text += "\tpush %rbp\n"
			text += "\tmovq %rsp, %rbp\n"
			ifQueue = append(ifQueue, Label{token.Value, false, false, true})
			args := make(map[string]int)
			var index int
			for {
				i++
				newTok := lex.Tokens[i]
				if newTok.Kind == lexer.VOID_ARG {
					break
				} else {
					switch newTok.Kind {
					case lexer.BOOL_ARG, lexer.CHAR_ARG, lexer.INT_ARG, lexer.PTR_ARG:
						if _, ok := args[newTok.Value]; ok {
							fmt.Printf("%d:%d %s Error: Duplicate argument name : `%s`\n", newTok.Row, newTok.Col, lex.Filename, newTok.Value)
							os.Exit(1)
						}
						args[newTok.Value] = index
					default:
						fmt.Printf("%d:%d %s Error: Unknown argument : `%s`\n", newTok.Row, newTok.Col, lex.Filename, newTok.Value)
						os.Exit(1)
					}
				}
				index++
			}
			functions = append(functions, Function{token.Value, args})
			funcQueue = append(funcQueue, Function{token.Value, args})
		} else if token.Kind == lexer.KEYWORD {
			switch token.Value {
			case "dup":
				text += "\t## DUP ##\n"
				text += "\tpop %rax\n"
				text += "\tpush %rax\n"
				text += "\tpush %rax\n"
			case "drop":
				text += "\t## DROP ##\n"
				text += "\tpop %rax\n"
			case "swap":
				text += "\t## SWAP ##\n"
				text += "\tpop %rax\n"
				text += "\tpop %rbx\n"
				text += "\tpush %rax\n"
				text += "\tpush %rbx\n"
			case "inc":
				text += "\t## INC ##\n"
				text += "\tpop %rax\n"
				text += "\tinc %rax\n"
				text += "\tpush %rax\n"
			case "dec":
				text += "\t## DEC ##\n"
				text += "\tpop %rax\n"
				text += "\tsub %rax\n"
				text += "\tpush %rax\n"
			case "dump":
				text += "\t## DUMP ##\n"
				text += "\tpop %rdi\n"
				text += "\tcall dump\n"
			case "if":
				label, _ := randLabel(10, upper+lower+digits)
				text += "\t## IF ##\n"
				text += "\tpop %rax\n"
				text += "\ttest %rax, %rax\n"
				text += fmt.Sprintf("\tje else_%s\n", label)
				ifQueue = append(ifQueue, Label{label, false, false, false})
			case "else":
				if len(ifQueue) < 1 || ifQueue[len(ifQueue)-1].HasElse {
					fmt.Printf("%d:%d %s Error: Could not find reference for `else` instruction\n", token.Row, token.Col, lex.Filename)
					os.Exit(1)
				}
				ifQueue[len(ifQueue)-1].HasElse = true
				label := ifQueue[len(ifQueue)-1].Name
				text += "\t## ELSE ##\n"
				text += fmt.Sprintf("\tjmp end_%s\n", label)
				text += fmt.Sprintf("else_%s:\n", label)
			case "end":
				if len(ifQueue) < 1 {
					fmt.Printf("%d:%d %s Error: Could not find reference for `end` instruction\n", token.Row, token.Col, lex.Filename)
					os.Exit(1)
				}
				text += "\t## END ##\n"
				label := ifQueue[len(ifQueue)-1].Name
				if !ifQueue[len(ifQueue)-1].IsFunc {
					if ifQueue[len(ifQueue)-1].IsWhile {
						text += fmt.Sprintf("\tjmp while_%s\n", label)
					} else {
						if !ifQueue[len(ifQueue)-1].HasElse {
							text += fmt.Sprintf("else_%s:\n", label)
						}
					}
					text += fmt.Sprintf("end_%s:\n", label)
				} else {
					text += "\tpop %rax\n"
					text += "\tmov %rbp, %rsp\n"
					text += "\tpop %rbp\n"
					text += "\tret\n"
					if len(funcQueue) > 1 {
						funcQueue = funcQueue[:len(funcQueue)-1]
					}
				}
				ifQueue = ifQueue[:len(ifQueue)-1]
			case "while":
				label, _ := randLabel(7, upper+lower+digits)
				text += "\t## WHILE ##\n"
				text += fmt.Sprintf("while_%s:\n", label)
				ifQueue = append(ifQueue, Label{label, false, true, false})
			case "do":
				if len(ifQueue) < 1 {
					fmt.Printf("%d:%d %s Error: Could not find reference for `do` instruction\n", token.Row, token.Col, lex.Filename)
					os.Exit(1)
				}
				label := ifQueue[len(ifQueue)-1].Name
				text += "\t## DO ##\n"
				text += "\tpop %rax\n"
				text += "\ttest %rax, %rax\n"
				text += fmt.Sprintf("\tje end_%s\n", label)
			case "return":
				text += "\t## RETURN ##\n"
				text += "\tpop %rax\n"
				text += "\tmov %rbp, %rsp\n"
				text += "\tpop %rbp\n"
				text += "\tret\n"
			case "derefc":
				text += "\t## DEREFC ##\n"
				text += "\tpop %rax\n"
				text += "\txor %rbx, %rbx\n"
				text += "\tmov (%rax), %bl\n"
				text += "\tpush %rbx\n"
			case "derefi":
				text += "\t## DEREFI ##\n"
				text += "\tpop %rax\n"
				text += "\tmov (%rax), %rbx\n"
				text += "\tpush %rbx\n"
			}
		} else if token.Kind == lexer.CALL {
			if containsStr(functions, token.Value) {
				text += fmt.Sprintf("\t## CALL %s ##\n", token.Value)
				text += fmt.Sprintf("\tcall %s\n", token.Value)
				for range len(findFunc(functions, token.Value).Args) {
					text += "\tpop %rbx\n"
				}
				text += "\tpush %rax\n"
			} else {
				if len(funcQueue) > 0 {
					if _, ok := funcQueue[len(funcQueue)-1].Args[token.Value]; ok {
						text += fmt.Sprintf("\t## GET ARG %s ##\n", token.Value)
						offset := (len(funcQueue[len(funcQueue)-1].Args) - 1) - funcQueue[len(funcQueue)-1].Args[token.Value]
						text += "\tmovq %rbp, %rax\n"
						text += fmt.Sprintf("\tadd $%d, %%rax\n", offset*8+16)
						text += "\tmovq (%rax), %rbx\n"
						text += "\tpush %rbx\n"
					} else {
						fmt.Printf("%d:%d %s Error: Unknown argument `%s`\n", token.Row, token.Col, lex.Filename, token.Value)
						os.Exit(1)
					}
				} else {
					fmt.Printf("%d:%d %s Error: Unknown function `%s`\n", token.Row, token.Col, lex.Filename, token.Value)
					os.Exit(1)
				}
			}
		}
	}

	var code string
	if !lex.IsLib {
		code = fmt.Sprintf(".section .data\n%s\n.section .text\n\t.global _start\n%s\n%s\n_start:\n\tcall main\n\tpush %%rax\n\tmovq $60, %%rax\n\tpop %%rdi\n\tsyscall\n", data, printNumText, text)
	} else {
		return text, data, functions, libs
	}

	baseName := filepath.Base(lex.Filename)
	ext := filepath.Ext(lex.Filename)

	fileName := baseName[:len(baseName)-len(ext)]
	fileOut := fileName + ".asm"
	err := os.WriteFile(fileOut, []byte(code), 0644)
	if err != nil {
		fmt.Printf("Error: %s\n", err)
		os.Exit(1)
	}

	as := exec.Command("as", "-o", fileName+".o", fileOut)
	output, err := as.Output()
	if err != nil {
		fmt.Println("Error while compiling ", fileOut)
		fmt.Println(output)
		fmt.Println(err)
		os.Exit(1)
	}

	ld := exec.Command("ld", "-o", fileName, fileName+".o")
	output, err = ld.Output()
	if err != nil {
		fmt.Println("Error while linking ", fileName, ".o")
		fmt.Println(output)
		fmt.Println(err)
		os.Exit(1)
	}

	if lex.Clean {
		err = os.Remove(fileOut)
		if err != nil {
			fmt.Println("Error while removing ", fileOut)
			fmt.Println(err)
			os.Exit(1)
		}

		err = os.Remove(fileName + ".o")
		if err != nil {
			fmt.Println("Error while removing ", fileName, ".o")
			fmt.Println(err)
			os.Exit(1)
		}
	}

	return "", "", []Function{}, []string{}
}
