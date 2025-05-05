package main

import (
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"
	"unsafe"
)

/*
#cgo LDFLAGS: -L${SRCDIR} -lmunakas
#include <stdlib.h>
#include "c/bridge.h"
*/
import "C"

func InitializeReader() error {
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

func pollPlayerData() {
	refreshRate := 30

	for {
		cJson := C.get_player_list_json()
		goJson := C.GoString(cJson)

		C.free(unsafe.Pointer(cJson))

		if goJson != "[]" {
			broadcast <- goJson
		}

		time.Sleep(time.Duration(refreshRate) * time.Millisecond)
	}
}
