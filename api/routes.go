package main

import (
	"database/sql"
	"encoding/json"
	"fmt"
	"github.com/gorilla/mux"
	"log"
	"net/http"
)

type HarvestAreas struct {
	Count   int16       `json:"count"`
	Results HarvestArea `json:"results"`
}

type HarvestArea struct {
	LastUpdated      string `json:"last_updated"`
	ProgramName      string `json:"program_name"`
	HarvestLocation  string `json:"location_name"`
	HarvestName      string `json:"harvest_name"`
	Classification   string `json:"classification"`
	Status           string `json:"status"`
	TimeProcessed    string `json:"time_processed"`
	StatusReason     string `json:"status_reason"`
	StatusPrevReason string `json:"status_prev_reason"`
}

func HarvestAreaRoute(w *http.ResponseWriter, r *http.Request, db *sql.DB) {
	// TODO add harvest area lookup
}

type Locations struct {
	Count   int32      `json:"count"`
	Results []Location `json:"results"`
}

type Location struct {
	LastUpdated string  `json:"last_updated"`
	ProgramInfo Program `json:"program_info"`
}

type Program struct {
	Name      string     `json:"name"`
	Latitude  string     `json:"latitude"`
	Longitude string     `json:"longitude"`
	BOM       BOMStation `json:"bom_info"`
}

type BOMStation struct {
	Name      string  `json:"name"`
	ID        string  `json:"id"`
	Latitude  float64 `json:"latitude"`
	Longitude float64 `json:"longitude"`
	Distance  float64 `json:"distance_from_program"`
}

func ListLocationsRoute(w http.ResponseWriter, r *http.Request, db *sql.DB) {

	var locations Locations

	query := "SELECT last_updated, fa_program_name, ww_latitude, ww_longitude, bom_location, bom_location_id, " +
		"bom_latitude, bom_longitude, bom_distance FROM harvest_lookup;"

	fmt.Printf(query)

	rows, err := db.Query(query)
	if err != nil {
		log.Printf("Locations request failed: %s\n", err)
	}

	results := make([]Location, 0)
	var count int32 = 0
	for rows.Next() {
		var location Location
		var program Program
		var bomStation BOMStation
		err = rows.Scan(&location.LastUpdated, &program.Name, &program.Latitude,
			&program.Longitude, &bomStation.Name, &bomStation.ID, &bomStation.Latitude, &bomStation.Longitude,
			&bomStation.Distance)
		if err != nil {
			log.Fatalf("Locational precipitation query failed: %s\n", err)
		} else {
			program.BOM = bomStation
			location.ProgramInfo = program
			results = append(results, location)
			count++
		}
	}

	locations.Results = results
	locations.Count = count

	err = json.NewEncoder(w).Encode(locations)

	if err != nil {
		log.Fatalf("JSON encode error: %s\n", err)
	}

	defer func(rows *sql.Rows) {
		err := rows.Close()
		if err != nil {
			log.Fatalf("Error closing rows from query: %s\n", err)
		}
	}(rows)

}

type Precipitation struct {
	LastUpdated   string                 `json:"last_updated"`
	ProgramName   string                 `json:"program_name"`
	LocationBOMID string                 `json:"bom_id"`
	Results       []PrecipitationResults `json:"results"`
}

type PrecipitationResults struct {
	Timestamp     string  `json:"timestamp"`
	Precipitation float64 `json:"precipitation"`
	DataType      string  `json:"data_type"`
}

func LocationalPrecipitationRoute(w http.ResponseWriter, r *http.Request, db *sql.DB) {

	var precip Precipitation

	args := mux.Vars(r)
	locationID := args["location_id"]

	log.Printf("Request for precipitation data for location: %s\n", locationID)

	query := fmt.Sprintf("SELECT last_updated, program_name, bom_location_id FROM weather "+
		"WHERE bom_location_id = '%s' LIMIT 1;", locationID)
	err := db.QueryRow(query).Scan(&precip.LastUpdated, &precip.ProgramName, &precip.LocationBOMID)
	if err != nil {
		log.Fatalf("Locational precipitation query failed: %s\n", err)
	}

	query = fmt.Sprintf("SELECT ts, precipitation, data_type FROM weather "+
		"WHERE bom_location_id = '%s' LIMIT 15;", locationID)
	rows, err := db.Query(query)
	if err != nil {
		log.Fatalf("Locational precipitation query failed: %s\n", err)
	}

	dataset := make([]PrecipitationResults, 0)
	var data PrecipitationResults
	for rows.Next() {
		err = rows.Scan(&data.Timestamp, &data.Precipitation, &data.DataType)
		dataset = append(dataset, data)
		if err != nil {
			log.Fatalf("Locational precipitation query failed: %s\n", err)
		}
	}

	precip.Results = dataset

	err = json.NewEncoder(w).Encode(precip)

	if err != nil {
		log.Fatalf("JSON encode error: %s\n", err)
	}

	defer func(rows *sql.Rows) {
		err := rows.Close()
		if err != nil {
			log.Fatalf("Error closing rows from query: %s\n", err)
		}
	}(rows)

}
