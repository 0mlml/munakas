package main

import (
	"bufio"
	"fmt"
	"os/exec"
)

func spawnReader(path string, outputChannel chan string) {
	process := exec.Command(path)

	stdout, err := process.StdoutPipe()
	if err != nil {
		panic(err)
	}

	stderr, err := process.StderrPipe()
	if err != nil {
		panic(err)
	}

	err = process.Start()
	if err != nil {
		panic(err)
	}

	// stdout reader
	go func() {
		scanner := bufio.NewScanner(stdout)
		for scanner.Scan() {
			output := scanner.Text()
			outputChannel <- output
		}
	}()

	// stderr reader
	go func() {
		scanner := bufio.NewScanner(stderr)
		for scanner.Scan() {
			output := scanner.Text()
			fmt.Println(output)
		}
	}()
}
