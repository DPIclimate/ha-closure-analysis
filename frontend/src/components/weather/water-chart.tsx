import React from 'react';
import {Chart as ChartJS, registerables} from 'chart.js';
import {Line} from "react-chartjs-2";
import {Ubidots} from "../datasources";
import "chartjs-adapter-date-fns";
import {enAU} from "date-fns/locale";

ChartJS.register(...registerables);

export class WaterChart extends React.Component<any, any> {
    constructor(props?: any) {
        super(props);
        this.state = {
            timestamps: [],
            salinity_data: [],
            temperature_data: []
        }
    }
    componentDidMount() {
        const variables_base_url = "https://industrial.ubidots.com.au/api/v2.0/devices/";
        const variables_end_url = "/variables";
        const variables_url = variables_base_url.concat(this.props.deviceId, variables_end_url);
        fetch(variables_url,  {headers: {
            "X-Auth-Token": "BBAU-89jVfECLkxlQasPB7qBU71Z5y6L5uZ", "Accept": "application/json"
        },
        method: "GET"})
            .then(res => res.json())
            .then(res => {
                const ids = res.results
                    .filter((variable: Ubidots.Variable) => {
                        if (variable.name.indexOf("temperature") !== -1
                            || variable.name.indexOf("salinity") !== -1) {
                            return variable;
                        }
                    }).map((variable: Ubidots.Variable) => {
                        return variable.id;
                    });

                const current_time = new Date().getTime();
                const previous_time = current_time - 604800000; // One week
                const req_body = {
                    variables: ids,
                    columns: ["variable.name", "value.value", "timestamp"],
                    join_dataframes: false,
                    start: previous_time,
                    end: current_time
                }

                fetch("https://industrial.api.ubidots.com.au/api/v1.6/data/raw/series", {
                    headers: {
                        "X-Auth-Token": "BBAU-89jVfECLkxlQasPB7qBU71Z5y6L5uZ",
                        "Content-Type": "application/json",
                        "Accept": "application/json"
                    },
                    body: JSON.stringify(req_body),
                    method: "POST"
                })
                    .then(res => res.json())
                    .then(res => {
                        let salinity_values: {x: string, y: number}[] = [];
                        let temperature_values: {x: string, y: number}[] = [];
                        res.results.map((values: [string, number, number][], index: number) => {
                            values.map((value: [string, number, number]) => {
                                const date = new Date(value[2]).toLocaleString("en-AU", {
                                    day: "numeric",
                                    month: "numeric",
                                    hour: "numeric",
                                    minute: "numeric"
                                });
                                // First variable is the variable name
                                if (value[0] === "temperature") {
                                    temperature_values.push({x: date, y: value[1]})
                                } else if(value[0] === "salinity") {
                                    salinity_values.push({x: date, y: value[1]})
                                }
                            });
                        });

                        this.setState({
                            salinity_data: salinity_values.reverse(),
                            temperature_data: temperature_values.reverse()
                        });

                    });
            });
    }

    render() {

        if(this.state.temperature_data.length !== 0 ){
            const data = {
                datasets: [
                    {
                        label: 'Water Temperature',
                        backgroundColor: 'rgb(250, 140, 0)',
                        borderColor: 'rgb(250, 140, 0)',
                        showLine: true,
                        pointRadius: 0,
                        tension: 0.4,
                        data: this.state.temperature_data
                    },
                    {
                        label: 'Salinity',
                        backgroundColor: 'rgb(29, 129, 162)',
                        borderColor: 'rgb(29, 129, 162)',
                        showLine: true,
                        pointRadius: 0,
                        tension: 0.4,
                        data: this.state.salinity_data
                    },
                ],
            };

            const options = {
                type: 'scatter',
                maintainAspectRatio: true,
                responsive: true,
                showLine: true,
                interaction: {
                    intersect: false,
                    axis: "x" as const,
                    mode: 'index' as const
                },
                tooltip: {
                    enable: true,
                },
                scales: {
                    y: {
                        min: 0,
                        max: 40
                    },
                    x: {
                        adapters: {
                            date: {locale: enAU},
                            type: "time",
                            time: {
                                parser: 'd/M h:m aaa',
                                unit: "day",
                            },
                        },
                        title: {
                            display: true,
                            text: "Date"
                        },
                        ticks: {
                            source: "data",
                            maxTicksLimit: 10
                        },
                    }
                }
            }

            return (
                <div className="accordian-container">
                    <div>
                        <Line data={data} options={options}/>
                    </div>
                </div>
            )

        }
        return (
            <div className="accordian-container">
                <div>
                    <h4>Fetching data...</h4>
                </div>
            </div>
        )


    }
}