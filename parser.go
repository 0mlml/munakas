package main

import (
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"golang.org/x/text/cases"
	"golang.org/x/text/language"
)

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
		ID:               mapName,
		Name:             formatMapName(mapName),
		PosX:             0,
		PosY:             0,
		Scale:            1.0,
		Rotate:           0,
		VerticalSections: make(map[string]VerticalSection),
	}

	lines := strings.Split(content, "\n")
	for i := 0; i < len(lines); i++ {
		line := strings.TrimSpace(lines[i])

		if line == "" || strings.HasPrefix(line, "//") {
			continue
		}

		if strings.Contains(line, "\"pos_x\"") {
			value := extractQuotedValue(line)
			if val, err := strconv.ParseFloat(value, 64); err == nil {
				mapInfo.PosX = val
			}
		} else if strings.Contains(line, "\"pos_y\"") {
			value := extractQuotedValue(line)
			if val, err := strconv.ParseFloat(value, 64); err == nil {
				mapInfo.PosY = val
			}
		} else if strings.Contains(line, "\"scale\"") {
			value := extractQuotedValue(line)
			if val, err := strconv.ParseFloat(value, 64); err == nil {
				mapInfo.Scale = val
			}
		} else if strings.Contains(line, "\"rotate\"") {
			value := extractQuotedValue(line)
			if val, err := strconv.ParseFloat(value, 64); err == nil {
				mapInfo.Rotate = val
			}
		} else if strings.Contains(line, "\"verticalsections\"") {
			i = parseVerticalSections(lines, i, &mapInfo)
		}
	}

	return mapInfo, nil
}

func extractQuotedValue(line string) string {
	parts := strings.Split(line, "\"")
	if len(parts) >= 4 {
		return parts[3]
	}
	return ""
}

func parseVerticalSections(lines []string, startLine int, mapInfo *MapInfo) int {
	lineIndex := startLine

	for lineIndex < len(lines) {
		line := strings.TrimSpace(lines[lineIndex])
		if strings.Contains(line, "{") {
			break
		}
		lineIndex++
	}

	if lineIndex >= len(lines) {
		return lineIndex
	}

	lineIndex++

	braceCount := 1
	var currentSection string
	var currentAltMin, currentAltMax float64
	var inSection bool

	for lineIndex < len(lines) && braceCount > 0 {
		line := strings.TrimSpace(lines[lineIndex])

		if line == "" || strings.HasPrefix(line, "//") {
			lineIndex++
			continue
		}

		if strings.Contains(line, "{") {
			braceCount++
		} else if strings.Contains(line, "}") {
			braceCount--
			if inSection && braceCount == 1 {
				mapInfo.VerticalSections[currentSection] = VerticalSection{
					AltitudeMin: currentAltMin,
					AltitudeMax: currentAltMax,
				}
				inSection = false
			}
			if braceCount == 0 {
				return lineIndex
			}
		}

		if !inSection && braceCount == 1 && strings.HasPrefix(line, "\"") {
			quoteParts := strings.Split(line, "\"")
			if len(quoteParts) >= 2 {
				currentSection = quoteParts[1]
				inSection = true
				currentAltMin = 0
				currentAltMax = 0
			}
		} else if inSection {
			if strings.Contains(line, "\"AltitudeMin\"") {
				value := extractQuotedValue(line)
				if val, err := strconv.ParseFloat(value, 64); err == nil {
					currentAltMin = val
				}
			} else if strings.Contains(line, "\"AltitudeMax\"") {
				value := extractQuotedValue(line)
				if val, err := strconv.ParseFloat(value, 64); err == nil {
					currentAltMax = val
				}
			}
		}

		lineIndex++
	}

	return lineIndex
}

func formatMapName(mapID string) string {
	displayName := mapID
	prefixes := []string{"de_", "cs_", "ar_"}

	for _, prefix := range prefixes {
		if strings.HasPrefix(mapID, prefix) {
			displayName = strings.TrimPrefix(mapID, prefix)
			displayName = cases.Title(language.English).String(displayName)
			return displayName
		}
	}

	return cases.Title(language.English).String(mapID)
}

func getAvailableMaps() ([]MapInfo, error) {
	var maps []MapInfo
	processedMapIDs := make(map[string]bool)

	configFiles, err := filepath.Glob("static/cs2-radar-images/*.txt")
	if err != nil {
		return nil, err
	}

	for _, configFile := range configFiles {
		baseName := filepath.Base(configFile)
		mapID := strings.TrimSuffix(baseName, ".txt")

		if processedMapIDs[mapID] {
			continue
		}

		pngPath := fmt.Sprintf("static/cs2-radar-images/%s.png", mapID)
		if _, err := os.Stat(pngPath); os.IsNotExist(err) {
			log.Printf("Warning: Config file %s exists but no corresponding .png found", baseName)
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
		processedMapIDs[mapID] = true
	}

	return maps, nil
}
