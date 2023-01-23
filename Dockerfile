FROM ubuntu:20.04

# Disable Prompt During Packages Installation
ARG DEBIAN_FRONTEND=noninteractive

# Update Ubuntu Software repository
RUN apt-get update
RUN apt update
RUN apt upgrade -y

# Install required packages. libssl-dev, libpq, postgresql-server-dev-all required for building cmake 3.23.0
RUN apt install -y cron curl git cmake golang-go wget libssl-dev libpq-dev postgresql-server-dev-all
RUN rm -rf /var/lib/apt/lists/*
RUN apt clean

RUN apt-get update -y

RUN apt install -y libcurl4-openssl-dev

WORKDIR "/opt/"
RUN wget https://github.com/Kitware/CMake/releases/download/v3.23.0/cmake-3.23.0.tar.gz
RUN tar -xvzf cmake-3.23.0.tar.gz
RUN rm cmake-3.23.0.tar.gz
WORKDIR "/opt/cmake-3.23.0/"
RUN ./configure
RUN make -s
RUN make install -s
#RUN ln -s /opt/cmake-3.23.0/bin/* /usr/bin

WORKDIR "/root/"
RUN git clone https://github.com/DaveGamble/cJSON

WORKDIR "/root/cJSON/"
RUN mkdir build

WORKDIR "/root/cJSON/build/"
RUN cmake ..
RUN make install

# In a directory of your choice, clone the repository.
WORKDIR "/root/"
RUN git clone https://github.com/DPIclimate/ha-closure-analysis

WORKDIR "/root/ha-closure-analysis/"
RUN touch .env
RUN echo "UBI_TOKEN=UBI_KEY" >> .env
RUN echo "WW_TOKEN=WW_KEY" >> .env
RUN echo "IBM_TOKEN=None" >> .env
RUN mkdir build

WORKDIR "/root/ha-closure-analysis/build/"
RUN cmake ..
RUN make
CMD ./bin/program

WORKDIR "/root/ha-closure-analysis/api/"
RUN go get .
RUN go run .

WORKDIR "/root/ha-closure-analysis/frontend/"
RUN curl -sL https://deb.nodesource.com/setup_14.x | bash
RUN apt-get install nodejs -y
RUN npm install
RUN npm start


