package main

import "database/sql"

func main() {
	psqlConn := PSQLConnection()

	HTTPRouter(psqlConn)

	defer func(psqlConn *sql.DB) {
		err := psqlConn.Close()
		if err != nil {

		}
	}(psqlConn)

}
