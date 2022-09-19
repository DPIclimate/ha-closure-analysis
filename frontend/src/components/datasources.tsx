export namespace Ubidots {
    /* ****** Devices ******
     *  Can query by organisation just to get a list of devices in a
     *  specific region (i.e. Clyde River)
     */
    export interface Device {
        name: string,
        id: string,
        isActive: boolean
        variables: string
        properties: DeviceLocation
    }

    export interface DeviceLocation {
        _location_fixed: {
            lat: number,
            lng: number
        }
    }

    /* ****** Variables ******
     * Required to get a list of variables from each device.
     */
    export interface Variables {
        count: number,
        results: Variable
    }

    export interface Variable {
        name: string,
        id: string,
        lastValue: Value,
        device: Device,
    }

    /* ****** Values ****** */
    // Latest values from device:
    // URL: https://industrial.api.ubidots.com.au/api/v2.0/devices/<device_key>/_/values/last
    export interface LastValues {
        salinity: Value,
        temperature: Value
    }

    export interface Value {
        value: number
        timestamp: number
    }

    export interface RawValues {
        results: [string, number, number][][]
        columns: string[][]
    }

}
