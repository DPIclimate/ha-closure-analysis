package main

import (
	"database/sql"
	"github.com/gorilla/mux"
	_ "github.com/lib/pq"
	"log"
	"net/http"
)

func HTTPRouter(psqlConn *sql.DB) {
	router := mux.NewRouter().StrictSlash(true)

	router.HandleFunc("/weather/precipitation/{location_id}", func(w http.ResponseWriter, r *http.Request) {
		LocationalPrecipitationRoute(w, r, psqlConn)
	})

	err := http.ListenAndServe(":8080", router)
	if err != nil {
		log.Fatalf("Error setting up HTTP server: %s\n", err)
	}
}
