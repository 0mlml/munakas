package main

import "flag"

func main() {
	readerPath := flag.String("reader", "", "Path to the reader executable")
	flag.Parse()

	if *readerPath == "" {
		panic("reader path is required")
	}

	outputChannel := make(chan string)
	go spawnReader(*readerPath, outputChannel)
	for output := range outputChannel {
		println(output)
	}
}
