module FprimeZephyrReference {
    @ Handler component for CYGNET RADFET sensors
    passive component RADFETHandler {

        #---------------------------#
        #         Commands          #
        #---------------------------#

        @ Start Radiation Readings
        sync command START_READINGS(
            interval: U32 @< Reading interval in seconds
        )

        @ Stop Radiation Readings
        sync command STOP_READINGS()

        @ send command to radFET sensor
        sync command SEND_COMMAND(cmd: string)


        #---------------------------#
        #         Events            #
        #---------------------------#

        event ReadingStarted(interval: U32) severity activity high format "Started periodic readings every {} seconds" 

        event ReadingStopped() severity activity high format "Stopped Radiation Readings"

        event ReadingComplete(value: U32, timestamp: U32) severity activity high format "Radiation reading: {} counts at timestamp {}"

        event StorageError(error: U32) severity warning high format "Flash storage error: {}"

        event SensorError(error: U32) severity warning high format "Sensor communication error: {}"

        event CommandSuccess(cmd: string) severity activity high format "Command {} sent successfully"


        #---------------------------#
        #    Conversion Events      #
        #---------------------------#

        event RawDataReceived(counts: U32) severity activity low format "Raw sensor Counts: {}"

        event DoseCalculated(rawCounts: U32, dose: F32) severity activity high format "Converted {} counts to {} rads"

        event ConversionError(rawCounts: U32, errorCode: U8) severity warning high format "Failed to convert counts {} (error: {})"

        #---------------------------#
        #    Conversion Params      #
        #---------------------------#

        @ Calibration placeholder A 
        param CalibrationA: F32 default 1.0

        @Calibration placeholder B
        param CalibrationB: F32 default 0.0

        @ Conversion equation version placeholder 
        param EquationVersion: string default "1.0"

        #---------------------------#
        #         Telemetry         #
        #---------------------------#


        @ raw ADC counts from sensor
        telemetry RadiationRawCounts: U32

        @ Converted rad dose (placeholder eq)
        telemetry RadiationDose: F32 id 0 format "{} rads"

        @ Converted dose rate (placeholder eq)
        telemetry DoseRate: F32 id 1 format "{} rads/s"

        @ Sensor Status(0=ok, 1=error 2=calibrating)
        telemetry SensorStatus: U8

        @ Storage Available
        telemetry StorageUsage: U32


        #---------------------------#
        #         Ports             # 
        #---------------------------#

        @ sends commands to RADFET from PayloadCom
        output port commandOut: Drv.ByteStreamData

        @ Port for scheduling readings periodically
        sync input port schedIn: Svc.Sched



        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending command registrations
        command reg port cmdRegOut

        @ Port for receiving commands
        command recv port cmdIn

        @ Port for sending command responses
        command resp port cmdResponseOut

        @ Port for sending textual representation of events
        text event port logTextOut

        @ Port for sending events to downlink
        event port logOut

        @ Port for sending telemetry channels to downlink
        telemetry port tlmOut

        @ Port to return the value of a parameter
        param get port prmGetOut

        @Port to set the value of a parameter
        param set port prmSetOut

    }
}