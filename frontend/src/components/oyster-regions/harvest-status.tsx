import React from 'react';

// Bootstrap
import Accordion from "react-bootstrap/Accordion";
import Badge from "react-bootstrap/Badge";
import Alert from 'react-bootstrap/Alert';
import {MapRegion} from "../maps/map-region";
import {ReportError} from "../utils/report-error";

interface HarvestStatuses {
    count: number,
    result: HarvestArea[]
}

interface HarvestArea {
    last_updated: string,
    program_name: string,
    harvest_name: string,
    harvest_id: string,
    status: Status
}

interface Status {
    classification: string,
    state: string,
    time_processed: string,
    reason: string,
    previous_reason: string
}

export class HarvestStatus extends React.Component<any, any> {
    constructor(props?: any) {
        super(props);
        this.state = {
            count: 0,
            harvest_areas: {},
        }
    }

    componentDidMount(){
        fetch("http://localhost:8080/oyster_regions/5/status", {
            headers: {"Accept": "*/*", "User-Agent": "NSW DPI"},
            method: "GET"
        })
            .then(res => res.json())
            .then(res =>
                this.setState({
                    count: res.count,
                    harvest_areas: res.results
                })
            )
            .catch(e => this.setState({
                count: 0,
                harvest_areas: {}
            }));
    }

    loadStatuses(ha_status: HarvestStatuses){
        return ha_status.result.map((ha: HarvestArea, i: number) => {
            let rating = "success"; // Determines color (green or red)
            if(ha.status.state === "Closed"){
               rating = "danger";
            }
            return (
                <Accordion.Item key={ha.harvest_id} eventKey={String(i)}>
                    <Accordion.Header>{ha.harvest_name} &nbsp;
                        <Badge pill bg={rating}>
                            {ha.status.state}
                        </Badge>{' '}
                        &nbsp;
                        <Badge pill bg="light" text="dark">
                            {new Date(ha.status.time_processed).toLocaleString()}
                        </Badge>{' '}
                    </Accordion.Header>
                    <Accordion.Body>
                        <h3>Reason</h3>
                        <p>
                            {ha.status.reason}
                        </p>
                    </Accordion.Body>
                </Accordion.Item>
            )
        });
    }

    lastUpdated(ha_status: HarvestStatuses){
        let cutoff_time = 15; // Minutes
        let res = ha_status.result.map((ha: HarvestArea) => {
            let current_time = new Date();
            let last_updated = new Date(ha.last_updated);
            let time_diff = (current_time.getTime() - last_updated.getTime()) / 1000;
            if(time_diff > cutoff_time * 60){
                return (
                    <Alert key="0" variant="danger">
                        Error data last updated at {last_updated.toLocaleString()}
                    </Alert>
                )
            }
        })
        if(!res){
            return (
                <Alert key="1" variant="success">
                    Data last updated recently.
                </Alert>
            )
        }
        return res[0];
    }

    render() {
        if(this.state.count === 0){
            return (
                <div>Loading...</div>
            )
        }
        else {
            const harvest_areas: HarvestStatuses = {
                count: this.state.count,
                result: this.state.harvest_areas
            }
            return (
                <div className="accordian-container">
                    <div className="harvest-alerts">
                        <h2>Harvest Area Status</h2>
                        <ReportError/>
                    </div>
                    {this.lastUpdated(harvest_areas)}
                    <MapRegion/>
                    <Accordion>
                        {this.loadStatuses(harvest_areas)}
                    </Accordion>
                </div>
            )
        }
    }

}