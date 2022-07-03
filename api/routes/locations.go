package routes

import (
	"database/sql"
	"encoding/json"
	"log"
	"net/http"
)

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

// ListLocationsRoute ... Get a list of locations
// @Summary      Get a list of unique NSW oyster harvesting locations
// @description  There are currently 28 oyster harvesting locations in NSW this methood obtains a list containing
// @description  their latitude and longitude, name, closest BOM weather station and other important values.
// @Tags         Locations
// @Produce      json
// @Success      200  {object}  Locations
// @Failure      404
// @Failure      500
// @Router       /oyster_regions/list [get]
func ListLocationsRoute(w http.ResponseWriter, r *http.Request, db *sql.DB) {

	log.Printf("[GET]: Unique locations list.")

	var locations Locations

	query := "SELECT last_updated, fa_program_name, ww_latitude, ww_longitude, bom_location, bom_location_id, " +
		"bom_latitude, bom_longitude, bom_distance FROM harvest_lookup;"

	rows, err := db.Query(query)
	if err != nil {
		http.Error(w, err.Error(), http.StatusNotFound)
		log.Printf("Locations request failed: %s\n", err)
		return
	}

	defer func(rows *sql.Rows) {
		err := rows.Close()
		if err != nil {
			log.Printf("Error closing rows from query: %s\n", err)
		}
	}(rows)

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
			http.Error(w, err.Error(), http.StatusInternalServerError)
			log.Printf("Locational precipitation query failed: %s\n", err)
			return
		}

		program.BOM = bomStation
		location.ProgramInfo = program
		results = append(results, location)
		count++
	}

	locations.Results = results
	locations.Count = count

	err = json.NewEncoder(w).Encode(locations)

	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		log.Printf("JSON encode error: %s\n", err)
	}

}
