import React from 'react';
import {LatestNews} from "../components/latest-news";
import {CurrentConditions} from "../components/current-conditions";
import {ReportError} from "../components/report-error";
import {HarvestStatus} from "../components/harvest-status";

export default class ClydeRiver extends React.Component<any, any>{

    render(){
        return (
            <div>
                <h1>Clyde River</h1>
                <HarvestStatus/>
                <CurrentConditions/>
                <LatestNews/>
                <ReportError/>
            </div>
        )
    }
}
