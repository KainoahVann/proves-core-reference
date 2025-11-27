module Components {
    passive component RADFETHandler {

        @ Start Radiation Readings
        sync command START_READINGS(
            interval: U32 @< Reading interval in seconds
        )

        @ Stop Radiation Readings  
        sync command STOP_READINGS()

        @ Take immediate radiation reading
        sync command TAKE_READING()

        @ Send command to radFET sensor
        sync command SEND_COMMAND(cmd: string)

        event ReadingStarted(interval: U32) severity activity high format "Started periodic readings every {} seconds"
        event ReadingStopped() severity activity high format "Stopped Radiation Readings"
        event ReadingComplete(value: U32, timestamp: U32) severity activity high format "Radiation reading: {} counts at timestamp {}"
        event StorageError(error: U32) severity warning high format "Flash storage error: {}"
        event SensorError(error: U32) severity warning high format "Sensor communication error: {}"
        event CommandSuccess(cmd: string) severity activity high format "Command {} sent successfully"
        event RawDataReceived(counts: U32) severity activity low format "Raw sensor Counts: {}"
        event DoseCalculated(rawCounts: U32, dose: F32) severity activity high format "Converted {} counts to {} rads"
        event ConversionError(rawCounts: U32, errorCode: U8) severity warning high format "Failed to convert counts {} (error: {})"
        event DataFileOpened(path: string) severity activity high format "Data file opened: {}"

        param CalibrationA: F32 default 1.0
        param CalibrationB: F32 default 0.0

        telemetry RawCounts: U32
        telemetry RadiationDose: F32
        telemetry DoseRate: F32  
        telemetry SensorStatus: U8
        telemetry StorageUsage: U32
        telemetry ReadingsCount: U32

        @ Receives data from PayloadCom
        sync input port dataIn: Drv.ByteStreamData

        @ Sends commands to RADFET via PayloadCom  
        output port commandOut: Drv.ByteStreamData

        @ Port for scheduling readings periodically
        sync input port schedIn: Svc.Sched

        time get port timeCaller
        command reg port cmdRegOut
        command recv port cmdIn
        command resp port cmdResponseOut
        text event port logTextOut
        event port logOut
        telemetry port tlmOut
        param get port prmGetOut
        param set port prmSetOut
    }
}
