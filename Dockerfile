FROM ubuntu:20.04

# Disable Prompt During Packages Installation
ARG DEBIAN_FRONTEND=noninteractive

# Update Ubuntu Software repository
RUN apt-get update && apt-get upgrade - y

# Install required packages. libssl-dev, libpq, postgresql-server-dev-all required for building cmake 3.23.0
RUN apt-get install -y \
  'cron' \
  'curl' \
  'git' \
  'golang-go' \
  'wget' \
  'libssl-dev' \
  'libpq-dev' \
  'postgresql-server-dev-all' \
  'libcurl4-openssl-dev'

# Cleanup apt
RUN rm -rf /var/lib/apt/lists/* && apt clean

# Install CMake from source
WORKDIR "/opt/"
RUN \
  wget https://github.com/Kitware/CMake/releases/download/v3.23.0/cmake-3.23.0.tar.gz && \ 
  tar -xvzf cmake-3.23.0.tar.gz && \ 
  rm cmake-3.23.0.tar.gz && \
  cd cmake-3.23.0/ && \
  ./configure && \
  make -s && \ 
  make install -s && \
  ln -s /opt/cmake-3.23.0/bin/* /usr/bin

WORKDIR "/root/"
RUN \
  git clone https://github.com/DaveGamble/cJSON && \
  cd cJSON/ && \
  mkdir build && \
  cd build/ && \
  cmake .. && \
  make install

# In a directory of your choice, clone the repository.
WORKDIR "/root/"
RUN \
  git clone https://github.com/DPIclimate/ha-closure-analysis && \
  cd ha-closure-analysis/

# Add environment variables
WORKDIR "/root/ha-closure-analysis"
RUN \
  touch .env && \
  echo "UBI_TOKEN=UBI_KEY" >> .env && \
  echo "WW_TOKEN=WW_KEY" >> .env && \
  echo "IBM_TOKEN=None" >> .env

# Build harvest-area-closure analysis
WORKDIR "/root/ha-closure-analysis/"
RUN \
  mkdir build && \
  cd build/ && \
  cmake .. && \
  make
# ./bin/program

# Build and run API
WORKDIR "/root/ha-closure-analysis/api/"
RUN go get . && go run .

# Build and run webserver
WORKDIR "/root/ha-closure-analysis/frontend/"
RUN \
  curl -sL https://deb.nodesource.com/setup_14.x | bash && \
  apt-get install nodejs -y && \
  npm install && \
  npm start
  
