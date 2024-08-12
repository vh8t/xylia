package main

import (
	"flag"
	"fmt"
	"os"
	"xyl/src/lexer"
	"xyl/src/parser"
)

const VERSION = "v0.1.0"

func usage() {
	fmt.Println("Usage:")
	fmt.Printf("  %s [options] <filename>\n", os.Args[0])
	fmt.Println()
	fmt.Println("Options:")
	fmt.Println("  -c, --clean      Clean the .o and .asm files after compilation of the program")
	fmt.Println("  -h, --help       Show this help message")
	fmt.Println("      --version    Show current version")
}

func main() {
	cLong := flag.Bool("clean", false, "")
	cShort := flag.Bool("c", false, "")
	version := flag.Bool("version", false, "")

	flag.Usage = usage
	flag.Parse()

	if *version {
		fmt.Println(VERSION)
		return
	}

	if flag.NArg() == 0 {
		usage()
		os.Exit(1)
	}

	filename := flag.Arg(0)
	clean := *cLong || *cShort

	l, err := lexer.NewLexer(filename, false, clean)
	if err != nil {
		fmt.Println(err.Error())
		usage()
		os.Exit(1)
	}
	l.Lex()

	parser.Parse(*l)
}
