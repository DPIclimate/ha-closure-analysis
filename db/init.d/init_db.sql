SET timezone TO "Australia/Sydney";

-- Harvest area information from NSW Food Authority
-- New rows are only updated if the timeprocessed and status has changed
-- This data is obtained through a JSON request to a NSW Food Authority server
-- which contains xml data descibing each harvest areas status.
CREATE TABLE IF NOT EXISTS harvest_area (
    last_updated                        timestamptz DEFAULT NOW() NOT NULL, -- Time this information was last updated
    program_name                        text NOT NULL,                      -- Area name (E.g. Clyde River)
    location                            text NOT NULL,                      -- Harvest location (can be a zone)
    name                                text NOT NULL,                      -- Harvest area name (e.g. Moonlight)
    classification                      text NOT NULL,                      -- Restrictions and classifications
    status                              text NOT NULL,                      -- Open or Closed
    time_processed                      timestamptz NOT NULL,               -- Time NSW FA updated status
    status_reason                       text NOT NULL,                      -- Reason for latest status
    status_prev_reason                  text NOT NULL,                      -- Reason for the previous status (N/A if not found)
    UNIQUE(time_processed, name, status)
);

CREATE TABLE IF NOT EXISTS harvest_outlook (
    last_updated                        timestamptz DEFAULT NOW() NOT NULL, -- Time this information was last updated
    closed                              boolean NOT NULL,                   -- Is harvest area currently closed?
    to_close                            boolean NOT NULL,                   -- Is the harvest area predicted to close?
    closure_type                        text NOT NULL,                      -- Why is it going to close? Rainfall...
    closure_reason                      text NOT NULL,                      -- Extended reason why HA is closing
    closure_date                        timestamptz NOT NULL,               -- What date will the HA close? Predicted...
    closure_severity                    text NOT NULL,                      -- What is the severity of this closure? (the index)
    est_closure_time                    text NOT NULL                       -- How long will it close for?
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
    location_id                 text NOT NULL, -- TODO need to import
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
    forecast_precipitation      float,
    observed_precipitation      float,
    UNIQUE(ts, program_name)
)
