<h2 align="center">
    Oyster Farm Environmental Monitoring & Prediction Analysis
</h2>
<p align="center">
  <a href="#about">About</a> •
  <a href="#install">Install</a> •
  <a href="#license">License</a>
</p>

<p align="center">
  <a href="https://github.com/DPIclimate/ha-closure-analysis/actions">
    <img src="https://github.com/DPIclimate/ha-closure-analysis/actions/workflows/main.yml/badge.svg"
         alt="Gitter">
  </a>
</p>

## About

Oyster harvest areas close due to rainfall, salinity, algal blooms and various other non-environmental factors.
This works aims to predict closures relating to rainfall (i.e. freshwater input). This process is essentially 
attempting to quantify the ammount of fresh water that has enteted a river system (through precipitation) and
then calculating a risk factor based on previous events. The same is done for temperature accumulation.

## Install
### Install dependencies
#### cJSON
A lightweight C JSON parsing and building library [cJSON](https://github.com/DaveGamble/cJSON).

This process requires [cmake](https://cmake.org/).
```bash
# In a directory of your choosing
git clone https://github.com/DaveGamble/cJSON
cd cJSON
mkdir build
cd build

cmake ..
make
```
### ENV Variables
This project uses several environmental variables. 
You need to provide these or the program will not run.
```bash
export UBI_TOKEN="<your_ubidots_token>"
export WW_TOKEN="<your_willy_weather_token>"
export IBM_TOKEN="<your_ibm_token>"
```

### Clone
Clone this repository from your command line:

```bash
git clone https://github.com/DPIclimate/ha-closure-analysis
cd ha-closure-analysis
```

### Build & Run
```bash
make
./bin/program
```

## License
This project is MIT licensed, as found in the LICENCE file.

---
> Contact [Harvey Bates](mailto:harvey.bates@dpi.nsw.gov.au)

