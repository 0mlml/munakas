package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"

	"github.com/gorilla/websocket"
	"golang.org/x/text/cases"
	"golang.org/x/text/language"
)

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true // TODO: Redo this
	},
}

var clients = make(map[*websocket.Conn]bool)
var broadcast = make(chan string)

type MapInfo struct {
	ID               string                     `json:"id"`
	Name             string                     `json:"name"`
	PosX             float64                    `json:"pos_x"`
	PosY             float64                    `json:"pos_y"`
	Scale            float64                    `json:"scale"`
	Rotate           float64                    `json:"rotate"`
	VerticalSections map[string]VerticalSection `json:"verticalSections,omitempty"`
}

type VerticalSection struct {
	AltitudeMax float64 `json:"altitudeMax"`
	AltitudeMin float64 `json:"altitudeMin"`
}

func parseMapConfig(mapName string) (MapInfo, error) {
	filePath := fmt.Sprintf("static/cs2-radar-images/%s.txt", mapName)

	data, err := os.ReadFile(filePath)
	if err != nil {
		return MapInfo{}, err
	}

	content := string(data)
	mapInfo := MapInfo{
		ID:     mapName,
		Name:   formatMapName(mapName),
		PosX:   0,
		PosY:   0,
		Scale:  1.0,
		Rotate: 0,
	}

	if match := regexp.MustCompile(`"pos_x"\s+"([^"]+)"`).FindStringSubmatch(content); len(match) > 1 {
		if val, err := strconv.ParseFloat(match[1], 64); err == nil {
			mapInfo.PosX = val
		}
	}

	if match := regexp.MustCompile(`"pos_y"\s+"([^"]+)"`).FindStringSubmatch(content); len(match) > 1 {
		if val, err := strconv.ParseFloat(match[1], 64); err == nil {
			mapInfo.PosY = val
		}
	}

	if match := regexp.MustCompile(`"scale"\s+"([^"]+)"`).FindStringSubmatch(content); len(match) > 1 {
		if val, err := strconv.ParseFloat(match[1], 64); err == nil {
			mapInfo.Scale = val
		}
	}

	if match := regexp.MustCompile(`"rotate"\s+"([^"]+)"`).FindStringSubmatch(content); len(match) > 1 {
		if val, err := strconv.ParseFloat(match[1], 64); err == nil {
			mapInfo.Rotate = val
		}
	}

	if strings.Contains(content, "verticalsections") {
		verticalSections := make(map[string]VerticalSection)

		if defaultSection := extractVerticalSection(content, "default"); defaultSection != nil {
			verticalSections["default"] = *defaultSection
		}

		if strings.Contains(content, "lower") {
			lowerStart := strings.Index(content, "lower")
			if lowerStart > 0 {
				lowerContent := content[lowerStart:]
				if lowerSection := extractVerticalSection(lowerContent, "lower"); lowerSection != nil {
					verticalSections["lower"] = *lowerSection
				}
			}
		}

		if len(verticalSections) > 0 {
			mapInfo.VerticalSections = verticalSections
		}
	}

	return mapInfo, nil
}

func extractVerticalSection(content, sectionName string) *VerticalSection {
	section := VerticalSection{}
	hasData := false

	if match := regexp.MustCompile(`"AltitudeMax"\s+"([^"]+)"`).FindStringSubmatch(content); len(match) > 1 {
		if val, err := strconv.ParseFloat(match[1], 64); err == nil {
			section.AltitudeMax = val
			hasData = true
		}
	}

	if match := regexp.MustCompile(`"AltitudeMin"\s+"([^"]+)"`).FindStringSubmatch(content); len(match) > 1 {
		if val, err := strconv.ParseFloat(match[1], 64); err == nil {
			section.AltitudeMin = val
			hasData = true
		}
	}

	if hasData {
		return &section
	}
	return nil
}

func formatMapName(mapID string) string {
	displayName := mapID
	if strings.HasPrefix(mapID, "de_") {
		displayName = strings.TrimPrefix(mapID, "de_")
		displayName = cases.Title(language.English).String(displayName)
	} else if strings.HasPrefix(mapID, "cs_") {
		displayName = strings.TrimPrefix(mapID, "cs_")
		displayName = cases.Title(language.English).String(displayName)
	} else if strings.HasPrefix(mapID, "ar_") {
		displayName = strings.TrimPrefix(mapID, "ar_")
		displayName = cases.Title(language.English).String(displayName)
	}
	return displayName
}

func getAvailableMaps() ([]MapInfo, error) {
	var maps []MapInfo

	files, err := filepath.Glob("static/cs2-radar-images/*_radar_psd.png")
	if err != nil {
		return nil, err
	}

	for _, file := range files {
		baseName := filepath.Base(file)
		mapID := strings.TrimSuffix(baseName, "_radar_psd.png")

		if strings.HasSuffix(mapID, "_lower") {
			continue
		}

		mapInfo, err := parseMapConfig(mapID)
		if err != nil {
			log.Printf("Warning: Could not parse config for %s: %v", mapID, err)
			mapInfo = MapInfo{
				ID:    mapID,
				Name:  formatMapName(mapID),
				PosX:  0,
				PosY:  0,
				Scale: 1.0,
			}
		}

		maps = append(maps, mapInfo)
	}

	return maps, nil
}

func handleConnections(w http.ResponseWriter, r *http.Request) {
	ws, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Fatal(err)
	}
	defer ws.Close()

	clients[ws] = true

	maps, err := getAvailableMaps()
	if err != nil {
		log.Printf("Error getting maps: %v", err)
	} else {
		mapsJSON, _ := json.Marshal(map[string]interface{}{
			"type": "maps_list",
			"maps": maps,
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

func handleMessages() {
	for {
		msg := <-broadcast

		msg = `{"type":"player_data","data":` + msg + `}`

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
