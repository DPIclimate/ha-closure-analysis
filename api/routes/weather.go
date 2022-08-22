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
	LastUpdated string                 `json:"last_updated"`
	ProgramName string                 `json:"program_name"`
	ProgramID   string                 `json:"bom_id"`
	Count       int32                  `json:"count"`
	Results     []PrecipitationResults `json:"results"`
}

type PrecipitationResults struct {
	Timestamp     string  `json:"timestamp"`
	Precipitation float64 `json:"precipitation"`
	DataType      string  `json:"data_type"`
}

// LocationalPrecipitationRoute ... Get precipitation (observed and forecast) information for a harvest area.
// @Summary      Get observed and forecast for a particular harvest area.
// @description  Obtains the forecast and obeserved (if available) precipitation data for a particular oyster farming
// region.
// @Tags         Precipitation
// @Produce      json
// @Success      200  {object}  Precipitation
// @Failure      404
// @Failure      500
// @Param program_id path integer true "Unique program area ID"
// @Router       /oyster_regions/weather/precipitation/{program_id} [get]
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
	locationID := args["program_id"]

	log.Printf("Request for precipitation data for location: %s\n", locationID)

	// First query to get the locations basic information
	stmt, err := db.PrepareContext(ctx, "SELECT last_updated, program_name, program_id "+
		"FROM weather WHERE program_id = $1 LIMIT 1;")
	err = stmt.QueryRowContext(ctx, locationID).Scan(&precip.LastUpdated, &precip.ProgramName,
		&precip.ProgramID)
	if err != nil {
		http.Error(w, err.Error(), http.StatusNotFound)
		log.Printf("Locational precipitation query failed: %s\n", err)
	}

	// Second query to get a select dataset from a particular locaiton
	stmt, err = db.PrepareContext(ctx, "SELECT ts, precipitation, data_type FROM weather "+
		"WHERE program_id = $1 ORDER BY ts DESC LIMIT 20;")
	if err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		log.Printf("Locational precipitation query failed: %s\n", err)
	}

	defer stmt.Close()

	rows, err := stmt.QueryContext(ctx, locationID)

	dataset := make([]PrecipitationResults, 0)
	var data PrecipitationResults
	var count int32 = 0
	for rows.Next() {
		err = rows.Scan(&data.Timestamp, &data.Precipitation, &data.DataType)
		dataset = append(dataset, data)
		if err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			log.Printf("Locational precipitation query failed: %s\n", err)
		}
		count++
	}

	precip.Results = dataset
	precip.Count = count

	err = json.NewEncoder(w).Encode(precip)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		log.Printf("JSON encode error: %s\n", err)
	}

	defer func(rows *sql.Rows) {
		err := rows.Close()
		if err != nil {
			log.Printf("Error closing rows from query: %s\n", err)
		}
	}(rows)

}
