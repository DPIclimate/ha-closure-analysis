import React from 'react';
import {LatestNews} from "../components/oyster-regions/latest-news";
import {CurrentConditions} from "../components/weather/current-conditions";
import {ReportError} from "../components/utils/report-error";
import {HarvestStatus} from "../components/oyster-regions/harvest-status";
import MapWindy from "../components/maps/map-windy";

export default class ClydeRiver extends React.Component<any, any>{

    render(){
        return (
            <div>
                <h1 className="accordian-container">Clyde River</h1>
                <HarvestStatus/>
                <CurrentConditions/>
                <LatestNews/>
            </div>
        )
    }
}
