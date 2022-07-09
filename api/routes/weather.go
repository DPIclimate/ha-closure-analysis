package routes

import (
	"context"
	"database/sql"
	"encoding/json"
	"github.com/gorilla/mux"
	"log"
	"net/http"
	"time"
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

	var (
		ctx    context.Context
		cancel context.CancelFunc
	)

	// If the request contains a timeout parameter this request will cancel
	// when the timeout elapses
	timeout, err := time.ParseDuration(r.FormValue("timeout"))
	if err == nil {
		ctx, cancel = context.WithTimeout(context.Background(), timeout)
	} else {
		ctx, cancel = context.WithCancel(context.Background())
	}

	defer cancel()

	var precip Precipitation

	args := mux.Vars(r)
	locationID := args["location_id"]

	log.Printf("Request for precipitation data for location: %s\n", locationID)

	// First query to get the locations basic information
	stmt, err := db.PrepareContext(ctx, "SELECT last_updated, program_name, bom_location_id "+
		"FROM weather WHERE bom_location_id = $1 LIMIT 1;")
	err = stmt.QueryRowContext(ctx, locationID).Scan(&precip.LastUpdated, &precip.ProgramName,
		&precip.LocationBOMID)
	if err != nil {
		http.Error(w, err.Error(), http.StatusNotFound)
		log.Fatalf("Locational precipitation query failed: %s\n", err)
	}

	// Second query to get a select dataset from a particular locaiton
	stmt, err = db.PrepareContext(ctx, "SELECT ts, precipitation, data_type FROM weather "+
		"WHERE bom_location_id = $1 LIMIT 15;")
	if err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		log.Fatalf("Locational precipitation query failed: %s\n", err)
	}

	defer stmt.Close()

	rows, err := stmt.QueryContext(ctx, locationID)

	dataset := make([]PrecipitationResults, 0)
	var data PrecipitationResults
	for rows.Next() {
		err = rows.Scan(&data.Timestamp, &data.Precipitation, &data.DataType)
		dataset = append(dataset, data)
		if err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			log.Fatalf("Locational precipitation query failed: %s\n", err)
		}
	}

	precip.Results = dataset

	err = json.NewEncoder(w).Encode(precip)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		log.Fatalf("JSON encode error: %s\n", err)
	}

	defer func(rows *sql.Rows) {
		err := rows.Close()
		if err != nil {
			log.Fatalf("Error closing rows from query: %s\n", err)
		}
	}(rows)

}
