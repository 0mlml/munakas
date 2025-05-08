package main

import (
	"encoding/json"
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"
	"unicode/utf8"
	"unsafe"
)

/*
#cgo LDFLAGS: -L${SRCDIR} -lmunakas
#include <stdlib.h>
#include "c/bridge.h"
*/
import "C"

func InitializeReader() error {
	if !C.init() {
		panic("Failed to initialize")
	}
	setupCleanupHandler()

	go pollPlayerData()
	return nil
}

func setupCleanupHandler() {
	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt, syscall.SIGTERM)
	go func() {
		<-c
		log.Println("Cleaning up resources...")
		C.cleanup_game_connection()
		os.Exit(0)
	}()
}

func sanitizeJSON(input string) string {
	if !utf8.ValidString(input) {
		validBytes := make([]byte, 0, len(input))
		for i, c := range input {
			if c == utf8.RuneError {
				continue
			}
			validBytes = append(validBytes, input[i])
		}
		input = string(validBytes)
	}

	var js json.RawMessage
	if err := json.Unmarshal([]byte(input), &js); err != nil {
		log.Printf("Invalid JSON from C function: %v", err)
		return "[]"
	}

	return input
}

func pollPlayerData() {
	refreshRate := 30

	for {
		cJson := C.get_player_list_json()
		goJson := C.GoString(cJson)

		C.free(unsafe.Pointer(cJson))

		if goJson != "[]" {
			sanitizedJson := sanitizeJSON(goJson)
			if sanitizedJson != "[]" {
				broadcast <- sanitizedJson
			}
		}

		time.Sleep(time.Duration(refreshRate) * time.Millisecond)
	}
}
