package main

import (
	"encoding/json"
	"log"
	"net/http"
	"unicode/utf8"

	"github.com/gorilla/websocket"
)

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true // TODO: Redo this
	},
}

var clients = make(map[*websocket.Conn]bool)
var broadcast = make(chan string)

func handleConnections(w http.ResponseWriter, r *http.Request) {
	ws, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Printf("Error upgrading connection: %v", err)
		return
	}
	defer ws.Close()

	clients[ws] = true

	maps, err := getAvailableMaps()
	if err != nil {
		log.Printf("Error getting maps: %v", err)
	} else {
		mapsJSON, _ := json.Marshal(map[string]interface{}{
			"type":     "maps_list",
			"maps":     maps,
			"map_name": getMapName(),
		})
		ws.WriteMessage(websocket.TextMessage, mapsJSON)
	}

	for {
		_, _, err := ws.ReadMessage()
		if err != nil {
			delete(clients, ws)
			break
		}
	}
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
		return "{}"
	}

	return input
}

func handleMessages() {
	for {
		msg := <-broadcast

		msg = sanitizeJSON(msg)

		for client := range clients {
			err := client.WriteMessage(websocket.TextMessage, []byte(msg))
			if err != nil {
				log.Printf("Error: %v", err)
				client.Close()
				delete(clients, client)
			}
		}
	}
}

func main() {
	fs := http.FileServer(http.Dir("./static"))
	http.Handle("/", fs)

	http.HandleFunc("/ws", handleConnections)

	go handleMessages()

	err := InitializeReader()
	if err != nil {
		log.Fatal("Error initializing reader: ", err)
	}

	log.Println("Server starting at http://localhost:8080")
	err = http.ListenAndServe(":8080", nil)
	if err != nil {
		log.Fatal("ListenAndServe: ", err)
	}
}
