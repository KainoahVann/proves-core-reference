module Components {
    @ component for radfets on cygnet payload computer
    passive component radfetComponent {

        
        #----------#
        # Commands #
        #----------#

        @start reading
        sync command START_READINGS()

        @stop reasing
        sync command STOP_READINGS()

        @single reading test command
        sync command TAKE_READING()

        

        #----------#
        #  events  #
        #----------#

        @logged when radfets start 
        event ReadingsStarted() severity activity high format "RADFETs started"

        @logged when readings stop
        event ReadingsStopped() severity activity high format "RADFETs stopped"

        @logged when single test reading taken
        event ReadingTaken(
            sensor: U8 @< which sensor (1 or 2)
            adcValue: U16 @< raw value read
        ) \
            severity activity high format "RADFET {}, ADC={}"


        @sensor enabled event
        event SensorEnabled(sensor: U8) severity activity high format "RADFET {} enabled"

        @sensor disabled event
        event SensorDisabled(sensor: U8) severity activity high format "RADFET {} disabled"

        @data send event
        event DataSent(
            sensor: U8
            dataSize: U32
        ) \ 
            severity activity high format "RADFET {} data sent {} bytes to fc"


        #-------------#
        #  telemetry  #
        #-------------#

        #radfet 1
        @ adc val
        telemetry RADFET1adc: U16
        @state
        telemetry RADFET1state: U8

        #radfet 2 
        @adc val
        telemetry RADFET2adc: U16
        @state 
        telemetry RADFET2state: U8


        @total readings 
        telemetry TotalReadings: U32


        #-------------#
        #    Ports    #
        #-------------#

        @receiving calls from rate group
        sync input port schedIn: Svc.Sched

        @setting GPIO for RADFET control
        output port gpioSet: Drv.GpioWrite

        @port for adc vals
        output port adcRead: Fw.BufferSend

        @ port for sending rad data back via UART
        output port dataOut: Fw.BufferSend

        @ port for storing data to flash
        output port storeData: Fw.BufferSend



        



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