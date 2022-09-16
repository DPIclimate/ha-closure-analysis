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

type HarvestAreas struct {
	Count   int32         `json:"count"`
	Results []HarvestArea `json:"results"`
}

type HarvestArea struct {
	LastUpdated     string `json:"last_updated"`
	ProgramName     string `json:"program_name"`
	HarvestLocation string `json:"location_name"`
	HarvestName     string `json:"harvest_name"`
	HarvestId       string `json:"harvest_id"`
	HAStatus        Status `json:"status"`
}

type Status struct {
	Classification   string `json:"classification"`
	State            string `json:"state"`
	TimeProcessed    string `json:"time_processed"`
	StatusReason     string `json:"reason"`
	StatusPrevReason string `json:"previous_reason"`
}

// ListHarvestAreasRoute ... Get a list of all NSW oyster harvesting areas
// @Summary      Get a list of unique NSW oyster harvesting areas.
// @description  This request obtains a list containing all NSW oyster harvest areas and their current statuses.
// @Tags         Oyster Harvest Areas
// @Produce      json
// @Success      200  {object}  HarvestAreas
// @Failure      404
// @Failure      500
// @Router       /oyster_regions/harvest_areas/list [get]
func ListHarvestAreasRoute(w http.ResponseWriter, r *http.Request, db *sql.DB) {

	w.Header().Set("Access-Control-Allow-Headers", "*")
	w.Header().Set("Access-Control-Allow-Origin", "*")
	w.Header().Set("Access-Control-Allow-Methods", "GET")
	w.Header().Set("Content-Type", "application/json")

	log.Printf("[GET]: Unique harvest areas list.")

	var harvestAreas HarvestAreas

	query := "SELECT DISTINCT ON (name) last_updated, program_name, location, name, id, classification, status, " +
		"time_processed, status_reason, status_prev_reason FROM harvest_area ORDER BY name, time_processed DESC;"
	rows, err := db.Query(query)
	if err != nil {
		http.Error(w, err.Error(), http.StatusNotFound)
		log.Printf("Harvest area list request failed: %s\n", err)
		return
	}

	defer func(rows *sql.Rows) {
		err := rows.Close()
		if err != nil {
			log.Printf("Error closing rows from query: %s\n", err)
		}
	}(rows)

	results := make([]HarvestArea, 0)
	var count int32 = 0
	for rows.Next() {
		var harvestArea HarvestArea
		var status Status
		err = rows.Scan(&harvestArea.LastUpdated, &harvestArea.ProgramName, &harvestArea.HarvestLocation,
			&harvestArea.HarvestName, &harvestArea.HarvestId, &status.Classification, &status.State,
			&status.TimeProcessed, &status.StatusReason, &status.StatusPrevReason)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			log.Printf("Locational precipitation query failed: %s\n", err)
			return
		}

		harvestArea.HAStatus = status
		results = append(results, harvestArea)
		count++
	}

	harvestAreas.Results = results
	harvestAreas.Count = count

	encoder := json.NewEncoder(w)
	err = encoder.Encode(harvestAreas)

	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		log.Printf("JSON encode error: %s\n", err)
	}

}

// GetHarvestArea ... Get the status of a NSW oyster harvesting area
// @Summary      Get the current status of a NSW oyster harvesting area with data from NSW Food Authority.
// @description  Contains open/close information for a NSW oyster harvest area based on its unique ID.
// @Tags         Oyster Harvest Areas
// @Produce      json
// @Success      200  {object}  HarvestArea
// @Failure      404
// @Failure      500
// @Param harvest_id path integer true "Unique harvest area ID"
// @Router       /oyster_regions/harvest_areas/{harvest_id} [get]
func GetHarvestArea(w http.ResponseWriter, r *http.Request, db *sql.DB) {

	w.Header().Set("Access-Control-Allow-Headers", "*")
	w.Header().Set("Access-Control-Allow-Origin", "*")
	w.Header().Set("Access-Control-Allow-Methods", "GET")
	w.Header().Set("Content-Type", "application/json")

	var harvestArea HarvestArea
	var locationStatus Status

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
	program_id := args["harvest_id"]

	stmt, err := db.PrepareContext(ctx, "SELECT last_updated, program_name, location, name, id, "+
		"classification, status, time_processed, status_reason, status_prev_reason "+
		"FROM harvest_area WHERE id = $1;")

	if err != nil {
		log.Printf("Harvest area select error: %s\n", err)
	}

	err = stmt.QueryRowContext(ctx, program_id).Scan(
		&harvestArea.LastUpdated,
		&harvestArea.ProgramName,
		&harvestArea.HarvestLocation,
		&harvestArea.HarvestName,
		&harvestArea.HarvestId,
		&locationStatus.Classification,
		&locationStatus.State,
		&locationStatus.TimeProcessed,
		&locationStatus.StatusReason,
		&locationStatus.StatusPrevReason)

	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		log.Printf("Query failed: %s\n", err)
		return
	}

	harvestArea.HAStatus = locationStatus

	defer stmt.Close()

	encoder := json.NewEncoder(w)
	encoder.SetIndent("", "  ")
	err = encoder.Encode(harvestArea)

	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		log.Printf("JSON encode error: %s\n", err)
		return
	}

}

// LocationStatusRoute ... Get a list of harvest area statuses for a particular location
// @Summary      Get a list of oyster harvesting areas (and their status) for a particular region.
// @description  This request obtains list of all NSW oyster farming harvest areas for a particualar location.
// @Tags         Oyster Production Regions
// @Produce      json
// @Success      200  {object}  Locations
// @Failure      404
// @Failure      500
// @Param program_id path integer true "Unique program ID"
// @Router       /oyster_regions/{program_id}/status [get]
func LocationStatusRoute(w http.ResponseWriter, r *http.Request, db *sql.DB) {
	log.Printf("[GET]: Unique locations list.")

	w.Header().Set("Access-Control-Allow-Headers", "*")
	w.Header().Set("Access-Control-Allow-Origin", "*")
	w.Header().Set("Access-Control-Allow-Methods", "GET")
	w.Header().Set("Content-Type", "application/json")

	args := mux.Vars(r)
	program_id := args["program_id"]

	var (
		harvestAreas HarvestAreas
		ctx          context.Context
		cancel       context.CancelFunc
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

	// Program ID to get program name
	var query string = "SELECT fa_program_name FROM harvest_lookup WHERE fa_program_id = $1;"

	stmt, err := db.PrepareContext(ctx, query)

	var program_name string
	err = stmt.QueryRowContext(ctx, program_id).Scan(&program_name)

	if err != nil {
		http.Error(w, err.Error(), http.StatusNotFound)
		log.Printf("Locations request failed: %s\n", err)
		return
	}

	query = "SELECT DISTINCT ON (name) last_updated, program_name, location, name, id, " +
		"classification, status, time_processed, status_reason, status_prev_reason " +
		"FROM harvest_area WHERE program_name = $1 ORDER BY name, time_processed DESC;"

	stmt, err = db.PrepareContext(ctx, query)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		log.Printf("Locational precipitation query failed: %s\n", err)
	}

	defer stmt.Close()

	rows, err := stmt.QueryContext(ctx, program_name)

	defer func(rows *sql.Rows) {
		err := rows.Close()
		if err != nil {
			log.Printf("Error closing rows from query: %s\n", err)
		}
	}(rows)

	results := make([]HarvestArea, 0)
	var count int32 = 0
	for rows.Next() {
		var (
			harvestArea    HarvestArea
			locationStatus Status
		)
		err = rows.Scan(&harvestArea.LastUpdated,
			&harvestArea.ProgramName,
			&harvestArea.HarvestLocation,
			&harvestArea.HarvestName,
			&harvestArea.HarvestId,
			&locationStatus.Classification,
			&locationStatus.State,
			&locationStatus.TimeProcessed,
			&locationStatus.StatusReason,
			&locationStatus.StatusPrevReason)

		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			log.Printf("Locational precipitation query failed: %s\n", err)
			return
		}

		harvestArea.HAStatus = locationStatus

		results = append(results, harvestArea)
		count++
	}

	harvestAreas.Count = count
	harvestAreas.Results = results

	encoder := json.NewEncoder(w)
	encoder.SetIndent("", "  ")
	err = encoder.Encode(harvestAreas)

	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		log.Printf("JSON encode error: %s\n", err)
	}

}
