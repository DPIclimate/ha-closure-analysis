package main

import (
	"database/sql"
	"encoding/json"
	"fmt"
	"github.com/gorilla/mux"
	"log"
	"net/http"
)

type Location struct {
	LastUpdated   string         `json:"last_updated"`
	ProgramName   string         `json:"program_name"`
	LocationBOMID string         `json:"bom_id"`
	Results       []LocationData `json:"results"`
}

type LocationData struct {
	Timestamp     string  `json:"timestamp"`
	Precipitation float64 `json:"precipitation"`
	DataType      string  `json:"data_type"`
}

func LocationalPrecipitationRoute(w http.ResponseWriter, r *http.Request, db *sql.DB) {

	var loc Location

	args := mux.Vars(r)
	locationID := args["location_id"]

	fmt.Printf("Request for precipitation data for location: %s\n", locationID)

	query := fmt.Sprintf("SELECT last_updated, program_name, bom_location_id FROM weather "+
		"WHERE bom_location_id = '%s' LIMIT 1;", locationID)
	err := db.QueryRow(query).Scan(&loc.LastUpdated, &loc.ProgramName, &loc.LocationBOMID)
	if err != nil {
		log.Fatalf("Locational precipitation query failed: %s\n", err)
	}

	query = fmt.Sprintf("SELECT ts, precipitation, data_type FROM weather "+
		"WHERE bom_location_id = '%s' LIMIT 15;", locationID)
	rows, err := db.Query(query)
	if err != nil {
		log.Fatalf("Locational precipitation query failed: %s\n", err)
	}

	dataset := make([]LocationData, 0)
	var data LocationData
	for rows.Next() {
		err = rows.Scan(&data.Timestamp, &data.Precipitation, &data.DataType)
		dataset = append(dataset, data)
		if err != nil {
			log.Fatalf("Locational precipitation query failed: %s\n", err)
		}
	}

	loc.Results = dataset

	err = json.NewEncoder(w).Encode(loc)

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
