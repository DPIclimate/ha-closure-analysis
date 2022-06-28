SET timezone = "Australia/Sydney";

CREATE TABLE IF NOT EXISTS harvest_area (
    last_updated                timestamptz DEFAULT NOW() NOT NULL,
    program_name                text NOT NULL,
    location                    text NOT NULL,
    name                        text NOT NULL,
    classification              text NOT NULL,
    status                      text NOT NULL,
    time_processed              timestamptz NOT NULL,
    status_reason               text NOT NULL,
    status_prev_reason          text NOT NULL,
    UNIQUE(time_processed, name, status)
);

CREATE TABLE IF NOT EXISTS harvest_outlook (
    last_updated                timestamptz DEFAULT NOW() NOT NULL,
    closed                      boolean NOT NULL,
    to_close                    boolean NOT NULL,
    closure_type                text NOT NULL,
    closure_reason              text NOT NULL,
    closure_date                timestamptz NOT NULL,
    closure_severity            text NOT NULL,
    est_closure_time            text NOT NULL
);

CREATE TABLE IF NOT EXISTS harvest_lookup (
    last_updated                timestamptz DEFAULT NOW() NOT NULL,
    fa_program_name             text NOT NULL,
    ww_location                 text NOT NULL,
    ww_location_id              int NOT NULL,
    ww_latitude                 float NOT NULL,
    ww_longitude                float NOT NULL,
    bom_location                text NOT NULL,
    bom_location_id             text NOT NULL,
    bom_latitude                float NOT NULL,
    bom_longitude               float NOT NULL,
    bom_distance                float NOT NULL,
    UNIQUE(fa_program_name)
);

CREATE TABLE IF NOT EXISTS weather_ww (
    last_updated                    timestamptz DEFAULT NOW() NOT NULL,
    location                        text NOT NULL,
    location_id                     int NOT NULL,
    ts                              timestamptz NOT NULL,
    rainfall_start_range            int,
    rainfall_end_range              int,
    rainfall_range_divider          char,
    rainfall_range_code             text NOT NULL,
    rainfall_probability_of_any     int,
    UNIQUE(location_id, ts)
);

CREATE TABLE IF NOT EXISTS weather_bom (
    last_updated                timestamptz DEFAULT NOW() NOT NULL,
    location                    text NOT NULL,
    ts                          timestamptz NOT NULL,
    precipitation               float,
    max_temperature             float,
    min_temperature             float,
    UNIQUE(location, ts)
);

CREATE TABLE IF NOT EXISTS weather_ibm_eis (
    last_updated                timestamptz DEFAULT NOW() NOT NULL,
    location                    text NOT NULL,
    ww_location_id              text NOT NULL,
    bom_location_id             text NOT NULL,
    latitude                    float,
    longitude                   float,
    ts                          timestamptz NOT NULL,
    precipitation               float,
    max_temperature             float,
    min_temperature             float,
    UNIQUE(ts, ww_location_id, bom_location_id)
);

CREATE TABLE IF NOT EXISTS weather_fdt (
    location                    text NOT NULL,
    latitude                    float,
    longitude                   float,
    distance_from_ha            float,
    ts                          timestamptz,
    precipitation               float,
    max_temperature             float,
    min_temperature             float
);

-- This is the main table where weather data are stored for querying
-- IBM EIS predicted precipitation data are inserted first
-- If BOM data exists for the same time and location these data are overwritten
-- This makes this table contain both observed rainfall and predicted rainfall
-- for all locations which should be easier to query
CREATE TABLE IF NOT EXISTS weather (
    last_updated                timestamptz DEFAULT NOW() NOT NULL,
    latitude                    float NOT NULL,
    longitude                   float NOT NULL,
    ts                          timestamptz NOT NULL,
    program_name                text NOT NULL,
    bom_location_id             text,
    data_type                   text NOT NULL, -- "forecast", "observed"
    precipitation               float NOT NULL,
    UNIQUE(ts, program_name)
)
