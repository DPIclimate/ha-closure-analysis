package main

import (
	"database/sql"
	"github.com/gorilla/mux"
	_ "github.com/lib/pq"
	httpSwagger "github.com/swaggo/http-swagger"
	"log"
	"net/http"
	_ "oyster_api/docs"
	"oyster_api/routes"
)

func HTTPRouter(psqlConn *sql.DB) {
	router := mux.NewRouter().StrictSlash(true)

	router.HandleFunc("/oyster_regions/list", func(w http.ResponseWriter, r *http.Request) {
		routes.ListLocationsRoute(w, r, psqlConn)
	})

	router.HandleFunc("/oyster_regions/harvest_areas/list", func(w http.ResponseWriter, r *http.Request) {
		routes.ListHarvestAreasRoute(w, r, psqlConn)
	})

	router.HandleFunc("/oyster_regions/harvest_areas/{harvest_id}", func(w http.ResponseWriter, r *http.Request) {
		routes.GetHarvestArea(w, r, psqlConn)
	})

	router.HandleFunc("/oyster_regions/{program_id}/weather/precipitation", func(w http.ResponseWriter,
		r *http.Request) {
		routes.LocationalPrecipitationRoute(w, r, psqlConn)
	})

	router.HandleFunc("/oyster_regions/{program_id}/outlook", func(w http.ResponseWriter, r *http.Request) {
		routes.OutlookRoute(w, r, psqlConn)
	})

	// Docs founds at /swagger/index.html
	router.PathPrefix("/swagger").Handler(httpSwagger.WrapHandler)

	err := http.ListenAndServe(":8080", router)
	if err != nil {
		log.Fatalf("Error setting up HTTP server: %s\n", err)
	}
}
