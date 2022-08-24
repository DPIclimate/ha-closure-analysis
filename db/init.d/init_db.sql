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
    id                                  int NOT NULL,                       -- Unique ID from NSW Food Authority
    classification                      text NOT NULL,                      -- Restrictions and classifications
    status                              text NOT NULL,                      -- Open or Closed
    time_processed                      timestamptz NOT NULL,               -- Time NSW FA updated status
    status_reason                       text NOT NULL,                      -- Reason for latest status
    status_prev_reason                  text NOT NULL,                      -- Reason for the previous status (N/A if not found)
    UNIQUE(time_processed, id, name, status)
);

-- This table contains information regarding the expected outlook for each harvest area.
-- For example, if the harvest area is expected to close, why will it close and when
-- is it predicted to close.
CREATE TABLE IF NOT EXISTS harvest_outlook (
    last_updated                        timestamptz DEFAULT NOW() NOT NULL, -- Time this information was last updated
    program_name                        text NOT NULL,                      -- Area name (E.g. Clyde River)
    location                            text NOT NULL,                      -- Harvest location (can be a zone)
    name                                text NOT NULL,                      -- Harvest area name (e.g. Moonlight)
    id                                  int NOT NULL,                       -- Unique ID from NSW Food Authority
    closed                              boolean NOT NULL,                   -- Is harvest area currently closed?
    to_close                            boolean NOT NULL,                   -- Is the harvest area predicted to close?
    closure_type                        text NOT NULL,                      -- Why is it going to close? Rainfall...
    closure_reason                      text NOT NULL,                      -- Extended reason why HA is closing
    closure_date                        timestamptz NOT NULL,               -- What date will the HA close? Predicted...
    closure_severity                    float NOT NULL,                     -- What is the severity of this closure? (the index)
    est_closure_time                    text NOT NULL                       -- How long will it close for?
);

-- Each harvest area is grouped by location. This location is obtained by searching for
-- each harvest areas location on Willy Weather. Willy Weather provides a latitude and longitude
-- for a searched query (e.g. searching for "Batemans Bay" on Willy Weather will provide the
-- latitude and longitude coodinates for Batemans Bay). These coordinates are then compared
-- to a list of latitude and longitude coordinates for BOM weather stations. This comparison
-- results in the closest BOM weather station being found and its distance in km being calcualted.
-- This table provides all the above information in an easy to query form.
CREATE TABLE IF NOT EXISTS harvest_lookup (
    last_updated                        timestamptz DEFAULT NOW() NOT NULL, -- Time this information was last updated
    fa_program_name                     text NOT NULL,                      -- NSW Food Authority program name
    fa_program_id                       SERIAL PRIMARY KEY NOT NULL,        -- Unique identifier for a program location
    ww_location                         text NOT NULL,                      -- Location name from Willy Weather
    ww_location_id                      int NOT NULL,                       -- Location ID from Willy Weather
    ww_latitude                         float NOT NULL,                     -- Latitude from Willy Weather (represents HA latitude)
    ww_longitude                        float NOT NULL,                     -- Longitude from Willy Weather (represents HA longitude)
    bom_location                        text NOT NULL,                      -- BOM weather station name
    bom_location_id                     text NOT NULL,                      -- BOM weather station ID
    bom_latitude                        float NOT NULL,                     -- BOM weather station latitude
    bom_longitude                       float NOT NULL,                     -- BOM weather station longitude
    bom_distance                        float NOT NULL,                     -- BOM weather station distance from HA
    UNIQUE(fa_program_name)
);

-- Daily calculated 7 day weather forecast from Willy Weather. Note, need to confirm exact probabilities
-- on precipitation data.
CREATE TABLE IF NOT EXISTS weather_ww (
    last_updated                        timestamptz DEFAULT NOW() NOT NULL, -- Time this information was last updated
    location                            text NOT NULL,                      -- Location name
    location_id                         int NOT NULL,                       -- Location ID
    ts                                  timestamptz NOT NULL,               -- Datetime information is valid for
    rainfall_start_range                int,                                -- 25 % probability of min rainfall in mm
    rainfall_end_range                  int,                                -- 25 % probability of max rainfall in mm
    rainfall_range_divider              char,                               -- Range code divider (e.g. '<', '-' or '>')
    rainfall_range_code                 text NOT NULL,                      -- Range code (e.g. "5-15") in mm
    rainfall_probability_of_any         int,                                -- Probability of 0.2 mm of rainfall in grid
    UNIQUE(location_id, ts)
);

-- Daily observed weather from each BOM weather station in NSW that is close to a NSW oyster harvest area.
-- Normally these data are updated a few days after they have been recorded, some sites are updated faster
-- than this. These data are provided over FTP as .csv files. There is a csv parser in this program to extract
-- the precipitation, max and min temperature.
CREATE TABLE IF NOT EXISTS weather_bom (
    last_updated                        timestamptz DEFAULT NOW() NOT NULL, -- Time this information was last updated
    location                            text NOT NULL,                      -- Location name (of BOM weather station)
    location_id                         text NOT NULL,                      -- Location ID (of BOM weather station)
    ts                                  timestamptz NOT NULL,               -- Datetime this data is relevent for
    precipitation                       float,                              -- Daily precipitation in mm
    max_temperature                     float,                              -- Daily maximum temperature
    min_temperature                     float,                              -- Daily minimum temperature
    UNIQUE(location, ts)
);

-- Daily forecasted and historical weather from IBM's environmental intelligence suite (EIS).
-- These data come from the ECMWF climate model (30 km gridded weather). The latitude and longitude in these
-- requests come from Willy Weather. Note, the model updates historical data as well as forecasted weather so values
-- are subjected to change over time.
CREATE TABLE IF NOT EXISTS weather_ibm_eis (
    last_updated                        timestamptz DEFAULT NOW() NOT NULL, -- Time this information was last updated
    location                            text NOT NULL,                      -- Location name from NSW Food Authority
    ww_location_id                      text NOT NULL,                      -- Willy Weather location ID
    bom_location_id                     text NOT NULL,                      -- Closes BOM station ID
    latitude                            float,                              -- Latitude, taken from Willy Weather
    longitude                           float,                              -- Longitude, taken from Willy Weather
    ts                                  timestamptz NOT NULL,               -- Datetime these values are relevant for
    precipitation                       float,                              -- Daily forecasted / estimated precipitation
    max_temperature                     float,                              -- Daily forecasted / estimated max temperature
    min_temperature                     float,                              -- Daily forecasted / estimated min temperature
    UNIQUE(ts, ww_location_id, bom_location_id)
);

-- This table isn't implemented yet. However, it should contain observed weather station data from
-- FarmDecisionTECH (fdt) automatic weather stations (AWS's).
CREATE TABLE IF NOT EXISTS weather_fdt (
    location                            text NOT NULL,                      -- Time this information was last updated
    latitude                            float,                              -- Latitude of weather station
    longitude                           float,                              -- Longitude of weather station
    distance_from_ha                    float,                              -- Distance from harvest area in km
    ts                                  timestamptz,                        -- Datetime these values are relevant for
    precipitation                       float,                              -- Daily observed precipitation
    max_temperature                     float,                              -- Daily observed max temperature
    min_temperature                     float                               -- Daily observed min temperature
);

-- This is the main table where weather data are stored for querying
-- IBM EIS predicted precipitation data are inserted first
-- If BOM data exists for the same time and location these data are overwritten
-- This makes this table contain both observed rainfall and predicted rainfall
-- for all locations which should be easier to query
CREATE TABLE IF NOT EXISTS weather (
    last_updated                        timestamptz DEFAULT NOW() NOT NULL, -- Time this information was last updated
    latitude                            float NOT NULL,                     -- Latitude of harvest zone
    longitude                           float NOT NULL,                     -- Longitude of harvest zone
    ts                                  timestamptz NOT NULL,               -- Datetime this information is relevent for
    program_name                        text NOT NULL,                      -- NSW FA program name
    program_id                          int NOT NULL,                       -- Unique ID (based on assigned SERIAL PRIMARY KEY in harvest_lookup)
    bom_location_id                     text,                               -- BOM weather station ID
    data_type                           text NOT NULL,                      -- "forecast" or "observed"
    precipitation                       float NOT NULL,                     -- Daily precipitation
    forecast_precipitation              float,                              -- Daily precipitation (forecasted)
    observed_precipitation              float,                              -- Daily precipitation (observed)
    forecast_zscore_precip              float,                              -- Z-Score (and log) transformed daily precipitation
    log_precip                          float,                              -- Intermediate to z-score calculation
    UNIQUE(ts, program_name)
)
