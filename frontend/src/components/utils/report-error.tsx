import React from 'react';

// Bootstrap
import FloatingLabel from 'react-bootstrap/FloatingLabel';
import Button from 'react-bootstrap/Button';
import Form from 'react-bootstrap/Form';
import Modal from 'react-bootstrap/Modal';

export class ReportError extends React.Component<any, any> {
    constructor(props?: any) {
        super(props);
        this.state = {
            show_signup: false
        }
    }

    componentDidMount(){
    }

    handleSignup = () =>  {
        this.setState((prevState: { show_signup: boolean; }) => ({
            show_signup: !prevState.show_signup
        }));
    }

    render() {
        return (
            <div className="accordian-container">
                <Button variant="primary" onClick={this.handleSignup}>
                    Sign Up for Alerts
                </Button>{' '}
                <Modal
                    show={this.state.show_signup}
                    onHide={this.handleSignup}
                    backdrop="static"
                    keyboard={false}
                >
                    <Modal.Header closeButton>
                        <Modal.Title>Sign Up for Alerts</Modal.Title>
                    </Modal.Header>
                    <Modal.Body>
                        <p>
                            Enter your phone and/or email address below to get alerts and
                            notifications regarding the Clyde River.
                        </p>
                        <FloatingLabel
                            controlId="floatingInput"
                            label="Mobile phone number"
                            className="mb-3"
                        >
                            <Form.Control type="phone" placeholder="+61 xxx xxx xxx" />
                        </FloatingLabel>
                        <FloatingLabel
                            controlId="floatingInput"
                            label="Email address"
                            className="mb-3"
                        >
                            <Form.Control type="email" placeholder="name@example.com" />
                        </FloatingLabel>
                    </Modal.Body>
                    <Modal.Footer>
                        <Button variant="primary">Submit</Button>{' '}
                    </Modal.Footer>
                </Modal>
            </div>
        )
    }
}
