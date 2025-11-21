module Components {

    # ----------------------------------------------------------------------
    # ENUM DEFINITIONS
    # ----------------------------------------------------------------------
    @ Watchdog state enumeration
    enum WatchdogState {
        STOPPED @< Monitoring stopped
        HEALTHY @< Normal operation
        WARNING @< 3 seconds without pet
        TIMEOUT @< 5 seconds without pet - system hung
    }

    # ----------------------------------------------------------------------
    # COMPONENT DEFINITION
    # ----------------------------------------------------------------------
    @ Component for petting external watchdog circuit on GPIO 19
    passive component WatchdogKai {
        
        # ----------------------------------------------------------------------
        # COMMANDS
        # ----------------------------------------------------------------------
        @ Command to start watchdog petting
        sync command START_MONITORING

        @ Command to stop watchdog petting
        sync command STOP_MONITORING

        @ Command to manually pet watchdog
        sync command PET_WATCHDOG

        # ----------------------------------------------------------------------
        # TELEMETRY
        # ----------------------------------------------------------------------
        @ Seconds since last pet
        telemetry TimeSinceLastPet: U32

        @ Total number of successful pets
        telemetry TotalPets: U32

        @ Current watchdog state
        telemetry WatchdogState: WatchdogState

        @ Count of timeouts detected
        telemetry TimeoutCount: U32

        # ----------------------------------------------------------------------
        # EVENTS
        # ----------------------------------------------------------------------
        @ Watchdog successfully pet
        event WatchdogPet() \
            severity activity high \
            format "Watchdog petted on GPIO 19"

        @ Warning: 3 seconds without pet
        event WatchdogWarning() \
            severity warning high \
            format "Watchdog warning: 3 seconds without pet - GPIO 19 not toggled"

        @ Timeout: 5 seconds without pet
        event WatchdogTimeout() \
            severity fatal \
            format "WATCHDOG TIMEOUT: 5 seconds without pet - system may reset"

        # ----------------------------------------------------------------------
        # PORTS
        # ----------------------------------------------------------------------
        @ Called periodically by scheduler to check timer and toggle GPIO
        sync input port run: Svc.Sched
        
        @ Port to toggle GPIO 19 (watchdog pet signal)
        output port gpioWrite: Drv.GpioWrite

        # ----------------------------------------------------------------------
        # STANDARD PORTS
        # ----------------------------------------------------------------------
        time get port timeCaller
        command reg port cmdRegOut
        command recv port cmdIn
        command resp port cmdResponseOut
        text event port logTextOut
        event port logOut
        telemetry port tlmOut
    }
}