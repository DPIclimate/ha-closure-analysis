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

type Outlook struct {
	Location    Location `json:"location"`
	LastUpdated string   `json:"last_updated"`
	Timestamp   string   `json:"timestamp"`
	FloodIndex  float32  `json:"flood_index"`
}

// Outlook  ...  Get the flood risk outlook for a NSW oyster production region.
// @Summary      Get the flood risk outlook for a NSW oyster production region.
// @description  Contains flood risk for the next 10 days based on model for a particular oyster production region.
// @Tags         Weather
// @Produce      json
// @Success      200  {object}  HarvestArea
// @Failure      404
// @Failure      500
// @Param program_id path integer true "Unique program ID"
// @Router       /oyster_regions/{program_id}/outlook [get]
func OutlookRoute(w http.ResponseWriter, r *http.Request, db *sql.DB) {

	w.Header().Set("Access-Control-Allow-Origin", "*")

	var outlook Outlook

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

	args := mux.Vars(r)
	program_id := args["program_id"]

	log.Printf("Request outlook for program_id: %s\n", program_id)

	// Get program information
	var location Location
	var program Program
	var bomStation BOMStation

	stmt, err := db.PrepareContext(ctx, "SELECT last_updated, fa_program_name, fa_program_id, ww_latitude, "+
		"ww_longitude, bom_location, bom_location_id, bom_latitude, bom_longitude, bom_distance "+
		"FROM harvest_lookup WHERE fa_program_id = $1;")

	if err != nil {
		log.Printf("Program select error: %s\n", err)
	}

	err = stmt.QueryRowContext(ctx, program_id).Scan(
		&location.LastUpdated,
		&program.Name,
		&program.ID,
		&program.Latitude,
		&program.Longitude,
		&bomStation.Name,
		&bomStation.ID,
		&bomStation.Latitude,
		&bomStation.Longitude,
		&bomStation.Distance)

	log.Printf("Program found: %s\n", program.Name)

	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		log.Printf("Program query failed: %s\n", err)
	}

	defer stmt.Close()

	program.BOM = bomStation
	location.ProgramInfo = program

	outlook.Location = location

	// Get current flood index
	stmt, err = db.PrepareContext(ctx, "SELECT last_updated, ts, normalised_precip FROM weather "+
		"WHERE program_id = $1 AND normalised_precip IS NOT NULL ORDER BY ts DESC LIMIT 1;")
	if err != nil {
		http.Error(w, err.Error(), http.StatusNotFound)
		log.Printf("Query failed: %s\n", err)
		return
	}

	err = stmt.QueryRowContext(ctx, program_id).Scan(&outlook.LastUpdated, &outlook.Timestamp, &outlook.FloodIndex)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		log.Printf("Query failed: %s\n", err)
		return
	}

	defer stmt.Close()

	encoder := json.NewEncoder(w)
	encoder.SetIndent("", "  ")
	err = encoder.Encode(outlook)

	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		log.Printf("JSON encode error: %s\n", err)
		return
	}

}
