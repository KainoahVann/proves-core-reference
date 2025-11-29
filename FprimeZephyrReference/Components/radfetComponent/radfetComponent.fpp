module Components {
    @ component for radfets on cygnet payload computer
    passive component radfetComponent {

        
        #----------#
        # Commands #
        #----------#

        @ Start automatic readings from both modules
        sync command START_READINGS()

        @ Stop automatic readings  
        sync command STOP_READINGS()

        @ Take single reading from active modules
        sync command TAKE_READING()

        @ Take reading from specific RADFET (module: 1-2, radfet: 1-2)
        sync command READ_RADFET(
            moduleId: U8 @< Module ID (1 or 2)
            radfetId: U8 @< RADFET ID (1 or 2)
        )


        

        #----------#
        #  events  #
        #----------#

        @ RADFET readings started
        event ReadingsStarted() severity activity high format "RADFET readings started"

        @ RADFET readings stopped
        event ReadingsStopped() severity activity high format "RADFET readings stopped"

        @ Single reading taken
        event ReadingTaken(
            moduleId: U8 @< Which module (1 or 2)
            radfetId: U8 @< Which RADFET (1 or 2) 
            adcValue: U16 @< Raw ADC value
        ) severity activity high format "Module {} RADFET {}: ADC={}"

        @ Module enabled
        event ModuleEnabled(moduleId: U8) severity activity high format "Module {} enabled"

        @ Module disabled
        event ModuleDisabled(moduleId: U8) severity activity high format "Module {} disabled"
       
        @ Data sent to flight computer
        event DataSent(
            moduleId: U8 @< Which module data was from
            radfetId: U8 @< Which RADFET data was from
            dataSize: U32 @< Size of data sent in bytes
        ) severity activity high format "Module {} RADFET {} data sent: {} bytes"

       
        #-------------#
        #  telemetry  #
        #-------------#

         @ Module 1 RADFET 1 ADC value
        telemetry Module1Radfet1Adc: U16

        @ Module 1 RADFET 2 ADC value  
        telemetry Module1Radfet2Adc: U16

        @ Module 2 RADFET 1 ADC value
        telemetry Module2Radfet1Adc: U16

        @ Module 2 RADFET 2 ADC value
        telemetry Module2Radfet2Adc: U16

        @ Module 1 state (0=off, 1=on)
        telemetry Module1State: U8

        @ Module 2 state (0=off, 1=on)
        telemetry Module2State: U8



        #-------------#
        #    Ports    #
        #-------------#

        @ Rate group driver for automatic readings
        sync input port schedIn: Svc.Sched

        @ GPIO control for RADFETs
        output port gpioSet: Drv.GpioWrite

        @ Send data to FC via UART
        output port dataOut:Drv.ByteStreamSend



        



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