package routes

import (
	"database/sql"
	"encoding/json"
	"log"
	"net/http"
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
// @description  This request obtains a list containing all NSW harvest areas and their current status
// @Tags         HarvestAreas
// @Produce      json
// @Success      200  {object}  HarvestAreas
// @Failure      404
// @Failure      500
// @Router       /oyster_regions/harvest_areas/list [get]
func ListHarvestAreasRoute(w http.ResponseWriter, r *http.Request, db *sql.DB) {

	log.Printf("[GET]: Unique harvest areas list.")

	var harvestAreas HarvestAreas

	query := "SELECT DISTINCT ON (name) last_updated, program_name, location, name, classification, status, " +
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
			&harvestArea.HarvestName, &status.Classification, &status.State, &status.TimeProcessed,
			&status.StatusReason, &status.StatusPrevReason)

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

	err = json.NewEncoder(w).Encode(harvestAreas)

	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		log.Printf("JSON encode error: %s\n", err)
	}

}
