package main

import (
	"database/sql"
	"fmt"
	_ "github.com/lib/pq"
	"log"
	"time"
)

func PSQLConnection() *sql.DB {
	const (
		hostname = "localhost"
		port     = 5432
		username = "postgres"
		password = "admin"
		dbname   = "oyster_db"
	)

	log.Print("Connecting to PSQL database...")

	connStr := fmt.Sprintf("host=%s port=%d user=%s password=%s dbname=%s sslmode=disable",
		hostname, port, username, password, dbname)

	psqlConn, err := sql.Open("postgres", connStr)

	if err != nil {
		log.Fatalf("PSQL Database connection error: %s\n", err)
	}

	psqlConn.SetConnMaxLifetime(time.Minute * 3)
	psqlConn.SetMaxOpenConns(2)
	psqlConn.SetMaxIdleConns(2)

	err = psqlConn.Ping()
	if err != nil {
		log.Fatalf("error:\n%s\n", err)
	}

	fmt.Println("success")

	return psqlConn
}
