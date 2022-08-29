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
// @Summary      Get a list of unique NSW oyster harvesting areas
// @description  This request obtains a list containing all NSW oyster harvest areas and their current statuses.
// @Tags         Oyster Harvest Areas
// @Produce      json
// @Success      200  {object}  HarvestAreas
// @Failure      404
// @Failure      500
// @Router       /oyster_regions/harvest_areas/list [get]
func ListHarvestAreasRoute(w http.ResponseWriter, r *http.Request, db *sql.DB) {

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
	encoder.SetIndent("", "  ")
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
