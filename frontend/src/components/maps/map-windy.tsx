import React from "react";

export default class MapWindy extends React.Component<any, any>{
    windy: any = null;
    map: any;

    constructor(props?: any) {
        super(props);
        this.state = {}
    }

    componentDidMount() {
        const options = {
            key: "OH9DANU1rRzOQbrUeIoZrrp2vdyntqSa",
            lat: -35.694361,
            lon: 150.169884,
            zoom: 11,
        };
        // @ts-ignore
        this.windy = windyInit(options, (windyAPI: any) => {
            const {map} = windyAPI;
            this.map = map;
        });
    }

    render() {
        return (
            <div id="windy" className="map-windy" />
        );
    }
}