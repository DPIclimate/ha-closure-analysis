<h2 align="center">
    Oyster Farm Environmental Monitoring & Prediction Analysis
</h2>
<p align="center">
  <a href="#about">About</a> •
  <a href="https://dpiclimate.github.io/ha-closure-analysis/">Docs</a> •
  <a href="#install">Install</a> •
  <a href="#license">License</a>
</p>

<p align="center">
  <a href="https://github.com/DPIclimate/ha-closure-analysis/actions">
    <img src="https://github.com/DPIclimate/ha-closure-analysis/actions/workflows/main.yml/badge.svg"
         alt="documentation-badge">
  </a>
  <a href="https://github.com/DPIclimate/ha-closure-analysis/actions">
    <img src="https://github.com/DPIclimate/ha-closure-analysis/actions/workflows/cmake.yml/badge.svg"
         alt="build-badge">
  </a>
</p>

## About

Oyster harvest areas close due to rainfall, salinity, algal blooms and various other non-environmental factors.
This works aims to predict closures relating to rainfall (i.e. freshwater input). This process is essentially 
attempting to quantify the amount of fresh water that has entered a river system (through precipitation) and
then calculating a risk factor based on previous events. The same is done for temperature accumulation.

## Backend
### Dependencies
This setup assumes you are working with a clean Ubuntu image. Commands may vary based on your OS.
#### CMake (greater than v3.19.0)
Only required if this command fails:
```bash
cmake --version
```

Install only if above command fails:
```bash
apt-get install wget
cd /opt
wget https://github.com/Kitware/CMake/releases/download/v3.23.0/cmake-3.23.0.tar.gz
tar -xvzf cmake-3.23.0.tar.gz
rm cmake-3.23.0.tar.gz
cd cmake-3.23.0/
./configure
make -s
make install -s
ln -s /opt/cmake-3.23.0/bin/* /usr/bin
```

#### cJSON

A JSON parsing and building library. See: [cJSON](https://github.com/DaveGamble/cJSON)

```bash
# In a directory of your choosing
git clone https://github.com/DaveGamble/cJSON
cd cJSON
mkdir build
cd build
cmake ..
sudo make install
```

#### Curl
```bash
apt-get install curl
apt-get install libcurl4-openssl-dev
```

#### PostGreSQL
```bash
apt-get remove libpq5 # May not be nessessary
apt-get install libpq-dev
```

### Environment Variables
This project uses several environmental variables. 
You need to provide these or the program will compile but cannot be run.

```bash
# In a directory of your choosing
git clone https://github.com/DPIclimate/ha-closure-analysis
cd ha-closure-analysis
touch .env
echo "UBI_TOKEN=<your_ubidots_token>" >> .env
echo "WW_TOKEN=<your_willy_weather_token>" >> .env
echo "IBM_TOKEN=<your_ibm_token>" >> .env
```

Then to build to backend:
```bash
cd ha-closure-analysis
mkdir build
cd build
cmake ..
make
```

And run:
```bash
./bin/program # Run
```

## PostGreSQL Database
### Add PSQL Environment
The username and password are defined by you.
```bash
cd ha-closure-analysis
echo "PSQL_USERNAME=<YOUR_USERNAME>" >> .env
echo "PSQL_PASSWORD=<YOUR_PASSWORD>" >> .env
echo "PSQL_DB=oyster_db" >> .env
```

### Build and Deploy
This will build the PSQL container and populate it with tables as defined in `db/init.d/init_db.sql`
```bash
cd ha-closure-analysis
docker compose up
```

### Load from Backup
#### Expose your backup
Put your backup in the `db/data/` directory.
```bash
cd ha-closure-analysis/db/data # <-- Put <YOUR_BACKUP>.sql file here.
```

#### Enter the PSQL Container
**If** you have a database backup you want to load you can:
```bash
docker ps # Copy the container ID from this command
docker exec -it <container_id> bash
```

#### Add your Backup
You should now have a bash terminal within the databases continer. From within the container you can run:
```bash
cd /var/lib/postgres/data
psql -U <YOUR_USERNAME> -d oyster_db < <YOUR_BACKUP>.sql
````

## API
### Dependences
#### Go (Lang)
Install [Go](https://go.dev/) as per the instructurion provided on their website.
##### Build and Run
The Docker container must be running first.
```bash
cd ha-closure-analysis
cd api
go get .
go run .
```

## Front-end
### Dependencies
#### NPM & Node.js
Only required if not installed.
```bash
curl -sL https://deb.nodesource.com/setup_14.x | bash
apt-get install nodejs -y
npm --version # Validate install
```

### Build and Run
Requires Go API to be running and database online.
```bash
cd ha-closure-analysis/frontend
npm install
npm start # Starts the server 
```

## License
This project is MIT licensed, as found in the LICENCE file.


