import React from 'react';
import mapboxgl from 'mapbox-gl';
import 'mapbox-gl/dist/mapbox-gl.css';
import './map.css';
import {Region, Regions} from "./navbar";

(mapboxgl as any).accessToken = 'pk.eyJ1IjoiaGFydmV5YmF0ZXMiLCJhIjoiY2w3c2k4N3kzMDdvejN4bnh4YnA4bWtuYSJ9.hSGj3OSZE89Ub1e7LkH48Q';

export class MapRegion extends React.Component<any, any> {
    private readonly mapContainer: React.RefObject<HTMLDivElement>;
    constructor(props?: any) {
        super(props);
        this.state = {
            zoom: 12,
        }
        this.mapContainer = React.createRef();
    }

    componentDidMount(){
        fetch("http://localhost:8080/oyster_regions/list", {
            headers: {"Accept": "*/*", "User-Agent": "NSW DPI"},
            method: "GET"
        })
            .then(res => res.json())
            .then(res =>
                this.setState({
                })
            );
    }

    render() {
        try {
            //// Build map
            const map =  new mapboxgl.Map({
                container: this.mapContainer.current!,
                style: 'mapbox://styles/mapbox/outdoors-v11',
                center: [150.149884, -35.691361],
                zoom: this.state.zoom
            });

            // Add region coordinates
            map.on('load', () => {
                map.addLayer({
                    id: "moonlight-ha",
                    type: "fill",
                    source: {
                        type: "vector",
                        url: "mapbox://harveybates.agkgixvw"
                    },
                    "source-layer": "Moonlight-0lgqru",
                    paint: {
                        "fill-color": "#198754",
                        "fill-opacity": 0.8,
                        "fill-outline-color": "#198754"
                    }
                });
                map.addLayer({
                    id: "rocky-point-ha",
                    type: "fill",
                    source: {
                        type: "vector",
                        url: "mapbox://harveybates.4z3oh4gx"
                    },
                    "source-layer": "Rocky_Point-8lfu2v",
                    paint: {
                        "fill-color": "#dc3545",
                        "fill-opacity": 0.8,
                        "fill-outline-color": "#dc3545"
                    }
                });
                map.addLayer({
                    id: "waterfall-ha",
                    type: "fill",
                    source: {
                        type: "vector",
                        url: "mapbox://harveybates.2nxfbjv7"
                    },
                    "source-layer": "Waterfall-7bq3le",
                    paint: {
                        "fill-color": "#dc3545",
                        "fill-opacity": 0.8,
                        "fill-outline-color": "#dc3545"
                    }
                });
            });
            return (
                <div ref={this.mapContainer} className="map-region-container"></div>
            )
        }
        catch(e) {
            return (
                <div ref={this.mapContainer} className="map-container">Map not available</div>
            )
        }
    }

}