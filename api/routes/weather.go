package routes

import (
	"database/sql"
	"encoding/json"
	"fmt"
	"github.com/gorilla/mux"
	"log"
	"net/http"
)

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
