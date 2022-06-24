SET timezone = "Australia/Sydney";

CREATE TABLE IF NOT EXISTS harvest_area (
    time_processed              timestamptz,
    program_name                text not null,
    location                    text not null,
    name                        text not null,
    classification              text not null,
    status                      text not null,
    last_updated                timestamptz,
    status_reason               text not null,
    status_prev_reason          text not null
);

CREATE TABLE IF NOT EXISTS harvest_outlook (
    last_updated                timestamptz,
    closed                      boolean,
    to_close                    boolean,
    closure_type                text not null,
    closure_reason              text not null,
    closure_date                timestamptz,
    closure_severity            text not null,
    est_closure_time            text not null
);


CREATE TABLE IF NOT EXISTS weather_ww (
    location                    text not null,
    latitude                    float,
    longitude                   float,
    distance_from_ha            float,
    ts                          timestamptz,
    precipitation               float,
);

CREATE TABLE IF NOT EXISTS weather_bom (
    location                    text not null,
    latitude                    float,
    longitude                   float,
    distance_from_ha            float,
    ts                          timestamptz,
    precipitation               float,
    max_temperature             float,
    min_temperature             float
);

CREATE TABLE IF NOT EXISTS weather_ibm_eis (
    location                    text not null,
    latitude                    float,
    longitude                   float,
    distance_from_ha            float,
    ts                          timestamptz,
    precipitation               float,
    max_temperature             float,
    min_temperature             float
);

CREATE TABLE IF NOT EXISTS weather_fdt (
    location                    text not null,
    latitude                    float,
    longitude                   float,
    distance_from_ha            float,
    ts                          timestamptz,
    precipitation               float,
    max_temperature             float,
    min_temperature             float
);
