module Components {
    @ component for radfets on cygnet payload computer
    active component radfetComponent {

        
        #----------#
        # Commands #
        #----------#

        @start reading
        sync command START_READINGS()

        @stop reasing
        sync command STOP_READINGS()

        @ Read specific RADFET
        sync command READ_RADFET(
            moduleNum: U8 @< Module number (1 or 2)
            radfet: U8 @< RADFET number (1 or 2)
        )

        
        #----------#
        #  events  #
        #----------#

        @logged when radfets start 
        event ReadingsStarted() severity activity high format "RADFETs started"

        @logged when readings stop
        event ReadingsStopped() severity activity high format "RADFETs stopped"

        @logged when single test reading taken
        event ReadingTaken(
            moduleNum: U8 @< Module (1 or 2)
            radfet: U8 @< RADFET (1 or 2)
            adcValue: U16 @< raw value read
        ) \
            severity activity high format "Module {}, RADFET {}, ADC={}"


        @module enabled event
        event ModuleEnabled(moduleNum: U8) severity activity low format "Module {} enabled"

        @module disabled event
        event ModuleDisabled(moduleNum: U8) severity activity low format "Module {} disabled"

        @data send event
        event DataSent(
            moduleNum: U8
            radfet: U8
            dataSize: U32
        ) \ 
            severity activity low format "Module {} RADFET {} data sent {} bytes to fc"

        @Sensor error
        event SensorError(
            moduleNum: U8
            radfet: U8
        ) \
            severity warning high format "Module {} RADFET {} error"

        @Invalid RADFET selection error
        event InvalidSelection(
            moduleNum: U8
            radfet: U8
        ) \
            severity warning high format "Invalid: Module {} RADFET {} (R1/R2 conflict)"

        #-------------#
        #  telemetry  #
        #-------------#

        # module 1
        @module 1 RADFET 1 ADC value
        telemetry MODULE1_RADFET1_adc: U16

        @module 1 RADFET 2 ADC value
        telemetry MODULE1_RADFET2_adc: U16

        @module 1 state
        telemetry MODULE1_state: U8

        # module 2
        @module 2 RADFET 1 ADC value
        telemetry MODULE2_RADFET1_adc: U16

        @module 2 RADFET 2 ADC value
        telemetry MODULE2_RADFET2_adc: U16

        @module 2 state
        telemetry MODULE2_state: U8

        @total readings 
        telemetry TotalReadings: U32


        #-------------#
        #    Ports    #
        #-------------#

        @receiving calls from rate group
        sync input port schedIn: Svc.Sched

        @Recieve commands from payloadCom(proves FC)
        async input port commandIn: Drv.ByteStreamData

        @setting GPIO for RADFET control
        output port gpioSet: [6] Drv.GpioWrite

        #@port for adc vals
        #output port adcRead: Fw.BufferSend

        @ port for sending rad data back via UART
        output port dataOut: Drv.ByteStreamSend

        #@ port for storing data to flash
        #output port storeData: Fw.BufferSend

        #@ port for sending buffers to be safely destroyed
        output port bufferReturn: Fw.BufferSend



        



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
