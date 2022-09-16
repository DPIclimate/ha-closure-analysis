import React from 'react';
import Tabs from "react-bootstrap/Tabs";
import Tab from "react-bootstrap/Tab";
import DataTable, {TableColumn} from 'react-data-table-component';
import WindyMap from "./windymap";

interface DataRow {
    buoy: number,
    salinity: number,
    temperature: number,
    date: string
}

export class CurrentConditions extends React.Component<any, any> {
    constructor(props?: any) {
        super(props);
        this.state = {
            data: []
        }
    }

    componentDidMount(){
        const tabledata = [
            {
                id: 1,
                buoy: 1,
                salinity: 2.2,
                temperature: 12.5,
                date: "12-12-2022"
            }
        ];
        this.setState({
            data: tabledata
        });
    }

    render() {
        const columns: TableColumn<DataRow>[] = [
            {
                name: "Buoy",
                selector: (row: { buoy: number; }) => row.buoy,
            },
            {
                name: "Salinity",
                selector: (row: { salinity: number; }) => row.salinity,
            },
            {
                name: "Temperature",
                selector: (row: { temperature: number; }) => row.temperature,
            },
            {
                name: "Date",
                selector: (row: { date: string; }) => row.date,
            }
        ];

        if(this.state.data.length != 0){
            return (
                <div className="accordian-container">
                    <h2>Current Conditions</h2>
                    <Tabs
                        defaultActiveKey="water"
                        id="uncontrolled-tab-example"
                        className="mb-3"
                    >
                        <Tab eventKey="water" title="Water">
                            <DataTable columns={columns} data={this.state.data}/>
                        </Tab>
                        <Tab eventKey="weather" title="Weather">
                            <p>Hello</p>
                        </Tab>
                        <Tab eventKey="map" title="Map">
                            A map
                        </Tab>
                    </Tabs>
                </div>
            )
        }
    }

}