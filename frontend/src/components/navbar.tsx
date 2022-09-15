import React from 'react';
import {BrowserRouter, Link, Route, Routes} from 'react-router-dom';
import Home from '../views/home';

// Bootstrap
import Container from 'react-bootstrap/Container';
import Nav from 'react-bootstrap/Nav';
import Navbar from 'react-bootstrap/Navbar';
import NavDropdown from 'react-bootstrap/NavDropdown';
import ClydeRiver from "../views/clyderiver";

export interface Regions {
	count: number,
	results: Region[]
}

export interface Region {
	last_updated: string,
	program_info: ProgramInfo
}

export interface ProgramInfo {
	name: string,
	latitude: number,
	longitude: number
}

export class NavbarNative extends React.Component<any, any> {
	constructor(props?: any) {
		super(props);
		this.state = {
			count: 0,
			regions: {},
			regionsVisible: false
		}
	}

	componentDidMount(){
		fetch("http://localhost:8080/oyster_regions/list", {
			headers: {"Accept": "*/*", "User-Agent": "NSW DPI"},
			method: "GET"
		})
			.then(res => res.json())
			.then(res =>
				this.setState({
					count: res.count,
					regions: res.results
				})
			)
			.catch(e => this.setState({
				count: 0,
				regions: {}
			}));
	}

	loadRegions(regions: Regions) {
		return regions.results.map((region: Region) => {
			return (
				<NavDropdown.Item href={region.program_info.name}>
					<span>{region.program_info.name}</span>
				</NavDropdown.Item>
			)
		});
	}

	loadRegionRoutes(regions: Regions){
		return regions.results.map((region: Region) => {
			return (
				<Route key={region.program_info.name}
					   path="/clyderiver" />
			)
		});
	}

	render() {
		if(this.state.count == 0){
			return (
				<div>Loading...</div>
			)
		} else {
			const regions: Regions = {count: this.state.count,
				results: this.state.regions};
			return (

				<BrowserRouter>
					<Navbar bg="light" expand="lg">
						<Container>
							<Navbar.Brand href="/home">NSW Oyster Farming</Navbar.Brand>
							<Navbar.Toggle aria-controls="basic-navbar-nav" />
							<Navbar.Collapse id="basic-navbar-nav">
								<Nav className="me-auto">
									<Nav.Link href="/home">Home</Nav.Link>
									<NavDropdown title="Regions" id="basic-nav-dropdown">
										{this.loadRegions(regions)}
									</NavDropdown>
								</Nav>
							</Navbar.Collapse>
						</Container>
					</Navbar>
					<Routes>
						<Route path="/home" element={<Home/>} />
						<Route path="/clyderiver" element={<ClydeRiver/>}/>
						{this.loadRegionRoutes(regions)}
					</Routes>
				</BrowserRouter>
			);
		}
	}
}