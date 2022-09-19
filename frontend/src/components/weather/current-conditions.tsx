import React from 'react';
import Tabs from "react-bootstrap/Tabs";
import Tab from "react-bootstrap/Tab";
import {WaterTable} from "./water-table";

export class CurrentConditions extends React.Component<any, any> {
    constructor(props?: any) {
        super(props);
        this.state = {
        }
    }

    componentDidMount(){
    }

    render() {
        return (
            <div className="accordian-container">
                <h2>Current Conditions</h2>
                <Tabs
                    defaultActiveKey="water"
                    id="uncontrolled-tab-example"
                    className="mb-3"
                >
                    <Tab eventKey="water" title="Water">
                        <WaterTable/>
                    </Tab>
                    <Tab eventKey="weather" title="Weather">
                        <p>Hello</p>
                    </Tab>
                    <Tab eventKey="map" title="MapRegions">
                        A map
                    </Tab>
                </Tabs>
            </div>
        )
    }
}