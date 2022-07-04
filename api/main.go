package main

import (
	"database/sql"
	"log"
)

// @title          NSW Oyster Status API
// @version        0.1
// @host           localhost:8080
// @BasePath       /
//
// @contact.name   Harvey Bates \@ NSW DPI
// @contact.email  harvey.bates@dpi.nsw.gov.au
// @licencse.name  MIT
// @license.url    https://github.com/DPIclimate/ha-closure-analysis/blob/master/LICENCE
func main() {
	psqlConn := PSQLConnection()

	HTTPRouter(psqlConn)

	defer func(psqlConn *sql.DB) {
		err := psqlConn.Close()
		if err != nil {
			log.Fatalf("Error closing PostgreSQL database: %s\n", err)
		}
	}(psqlConn)

}
