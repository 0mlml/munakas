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
	if !C.init() {
		panic("Failed to initialize")
	}
	setupCleanupHandler()

	go pollData()
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

var lastMapName string

func pollData() {
	refreshRate := 30

	for {
		cJson := C.get_player_list_json()
		goJson := C.GoString(cJson)

		C.free(unsafe.Pointer(cJson))

		if goJson != "[]" {
			broadcast <- `{"type":"player_data","data":` + goJson + `}`
		}

		// cJson = C.get_bomb_state_json()
		// goJson = C.GoString(cJson)

		// C.free(unsafe.Pointer(cJson))

		// if goJson != "{}" {
		// 	broadcast <- `{"type":"bomb_state","data":` + goJson + `}`
		// }

		if lastMapName != getMapName() {
			lastMapName = getMapName()
			broadcast <- `{"type":"map_name","map_name":"` + lastMapName + `"}`
			log.Printf("Map changed to: %s", lastMapName)
		}

		time.Sleep(time.Duration(refreshRate) * time.Millisecond)
	}
}

func getMapName() string {
	cMapName := C.get_map_name_string()
	goMapName := C.GoString(cMapName)
	C.free(unsafe.Pointer(cMapName))

	return goMapName
}
