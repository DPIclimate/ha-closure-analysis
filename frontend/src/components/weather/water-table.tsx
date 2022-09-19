import React from 'react';
import DataTable, {ExpanderComponentProps, TableColumn} from 'react-data-table-component';
import {Ubidots} from "../datasources";
import {WaterChart} from "./water-chart";

interface DataRow {
    buoy: string,
    salinity: number,
    temperature: number,
    date: string,
    device_id: string
}

export class WaterTable extends React.Component<any, any> {
    constructor(props?: any) {
        super(props);
        this.state = {
            n_devices: 0,
            devices: [],
            last_values: [],
            table_data: []
        }
    }

    async componentDidMount() {

        fetch("https://industrial.api.ubidots.com.au/api/v2.0/devices/", {
            headers: {
                "X-Auth-Token": "BBAU-89jVfECLkxlQasPB7qBU71Z5y6L5uZ",
                "Accept": "application/json"
            },
            method: "GET"
        })
            .then(res => res.json())
            .then(res => {
                const devices: Ubidots.Device[] = res.results
                    .filter((x: Ubidots.Device) => {
                        // Find devices with buoy in their name
                        return x.name.indexOf("Buoy") !== -1;
                    })
                    .map((device: Ubidots.Device, i: number) => {
                    return device;
                });

                this.setState({
                    n_devices: devices.length
                });

                let table_data: DataRow[] = [];
                if(devices){
                    const last_values_urls = devices.map((device: Ubidots.Device) => {
                        const base_url = "https://industrial.api.ubidots.com.au/api/v2.0/devices/"
                        const end_url = "/_/values/last"
                        return {
                            url: base_url.concat(device.id, end_url),
                            name: device.name,
                            device_id: device.id
                        }
                    });

                    let count = 0;
                    Promise.all(last_values_urls.map((value: {device_id: string, name: string, url: string})  => fetch(value.url,
                            {headers: {
                            "X-Auth-Token": "BBAU-89jVfECLkxlQasPB7qBU71Z5y6L5uZ",
                            "Accept": "application/json"
                        }
                    })
                        .then(res => res.json())
                        .then(res => {
                            const last_values: Ubidots.LastValues = res;
                            this.setState({
                                last_values: last_values
                            })

                            const ts = new Date(last_values.salinity.timestamp).toLocaleString();
                            table_data.push({
                                buoy: value.name,
                                salinity: last_values.salinity.value,
                                temperature: last_values.temperature.value,
                                date: ts,
                                device_id: value.device_id
                            });
                        })
                    ));
                }
                this.setState({
                    devices: devices,
                    table_data: table_data
                })
            })
            .catch(_ => this.setState({
                devices: [],
                last_values: []
            }));
    }


    render() {
        const ExpandedComponent: React.FC<ExpanderComponentProps<DataRow>> = ({ data }) => {
            return <WaterChart deviceId={data.device_id}/>
        };

        const columns: TableColumn<DataRow>[] = [
            {
                name: "Buoy",
                sortField: "bouy",
                selector: (row: { buoy: string; }) => row.buoy,
                sortable: true
            },
            {
                name: "Salinity",
                selector: (row: { salinity: number; }) => row.salinity,
                sortable: true,
                conditionalCellStyles: [
                    {
                        when: row => row.salinity > 22,
                        style: {
                            color: "#198754",
                        }
                    },
                    {
                        when: row => row.salinity <= 22 ,
                        style: {
                            color: "#ffc107",
                        }
                    },
                    {
                        when: row => row.salinity <= 19,
                        style: {
                            color: "#dc3545",
                        }
                    }
                ]
            },
            {
                name: "Temperature",
                selector: (row: { temperature: number; }) => row.temperature,
                sortable: true
            },
            {
                name: "Date",
                selector: (row: { date: string; }) => row.date,
                sortable: true
            }
        ];

        if(this.state.table_data.length === this.state.n_devices){
            return (
                <DataTable columns={columns} data={this.state.table_data}
                           defaultSortFieldId={1}
                           expandableRows
                           expandableRowsComponent={ExpandedComponent}
                           dense/>
            )
        }
    }

}