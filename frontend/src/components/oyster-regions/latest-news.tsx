import React from 'react';
import Accordion from "react-bootstrap/Accordion";
import Badge from "react-bootstrap/Badge";

export class LatestNews extends React.Component<any, any> {
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
                <h2>Latest News</h2>
                <Accordion>
                    <Accordion.Item eventKey="0">
                        <Accordion.Header>Server Error &nbsp;
                            <Badge pill bg="danger">
                                Error
                            </Badge>{' '}
                            &nbsp;
                            <Badge pill bg="light" text="dark">
                                30th August 2022
                            </Badge>{' '}
                        </Accordion.Header>
                        <Accordion.Body>
                            We are experiencing an error with our data provider. We are working to fix this issue ASAP.
                        </Accordion.Body>
                    </Accordion.Item>
                    <Accordion.Item eventKey="1">
                        <Accordion.Header>Maintenance &nbsp;
                            <Badge pill bg="success">
                                Information
                            </Badge>{' '}
                            &nbsp;
                            <Badge pill bg="light" text="dark">
                                3rd August 2022
                            </Badge>{' '}
                        </Accordion.Header>
                        <Accordion.Body>
                            The FarmDecisionTECH team conducted maintenance on the buoys and weather station.
                        </Accordion.Body>
                    </Accordion.Item>
                </Accordion>
            </div>
        )
    }

}