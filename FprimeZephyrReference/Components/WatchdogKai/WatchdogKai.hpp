#ifndef FprimeZephyrReference_Components_WatchdogKai_HPP
#define FprimeZephyrReference_Components_WatchdogKai_HPP

#include "FprimeZephyrReference/Components/WatchdogKai/WatchdogKaiComponentAc.hpp"

namespace Components {

class WatchdogKai : public WatchdogKaiComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct WatchdogKai object
    WatchdogKai(const char* compName);

    //! Destroy WatchdogKai object  
    ~WatchdogKai();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for user-defined typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for run
    void run_handler(
        FwIndexType portNum, //!< The port number
        U32 context          //!< The call order
    ) override;

    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command START_MONITORING
    void START_MONITORING_cmdHandler(
        FwOpcodeType opCode, //!< The opcode
        U32 cmdSeq           //!< The command sequence number
    ) override;

    //! Handler implementation for command STOP_MONITORING
    void STOP_MONITORING_cmdHandler(
        FwOpcodeType opCode, //!< The opcode  
        U32 cmdSeq           //!< The command sequence number
    ) override;

    //! Handler implementation for command PET_WATCHDOG
    void PET_WATCHDOG_cmdHandler(
        FwOpcodeType opCode, //!< The opcode
        U32 cmdSeq           //!< The command sequence number
    ) override;

  private:
    // Member variables
    U32 m_secondsSinceLastPet; //!< Seconds since last pet
    U32 m_totalPets;           //!< Total successful pets
    U32 m_timeoutCount;        //!< Number of timeouts that occurred
    bool m_isMonitoring;       //!< Whether monitoring is active
    WatchdogState m_currentState; //!< Current state machine state
    bool m_resetRequested;     //!< Track if we've already requested reset
};

} // namespace Components

#endif