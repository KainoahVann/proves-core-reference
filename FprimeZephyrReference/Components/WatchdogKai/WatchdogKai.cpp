// ======================================================================
// \title  WatchdogKai.cpp
// \author kai
// \brief  cpp file for WatchdogKai component implementation class
// ======================================================================

#include "FprimeZephyrReference/Components/WatchdogKai/WatchdogKai.hpp"
#include <Fw/Types/BasicTypes.hpp>

namespace Components {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

WatchdogKai::WatchdogKai(const char* const compName) 
    : WatchdogKaiComponentBase(compName),
      m_secondsSinceLastPet(0),
      m_totalPets(0),
      m_timeoutCount(0),
      m_isMonitoring(false),
      m_currentState(WatchdogState::STOPPED),
      m_resetRequested(false)
{
    // Initialize telemetry to known values
    this->tlmWrite_TimeSinceLastPet(m_secondsSinceLastPet);
    this->tlmWrite_TotalPets(m_totalPets);
    this->tlmWrite_TimeoutCount(m_timeoutCount);
    this->tlmWrite_WatchdogState(m_currentState);
}

WatchdogKai::~WatchdogKai() {}

// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------

void WatchdogKai::run_handler(FwIndexType portNum, U32 context) {
    // Only check timer if monitoring is active
    if (!m_isMonitoring) {
        return;
    }
    
    // Increment time since last pet
    m_secondsSinceLastPet++;
    
    // Update telemetry
    this->tlmWrite_TimeSinceLastPet(m_secondsSinceLastPet);
    
    // Check for state transitions
    if (m_secondsSinceLastPet >= 5) {
        // TIMEOUT STATE - External watchdog will reset system
        if (m_currentState != WatchdogState::TIMEOUT) {
            m_currentState = WatchdogState::TIMEOUT;
            m_timeoutCount++;
            
            // Emit timeout event
            this->log_FATAL_WatchdogTimeout();
            
            // Update telemetry
            this->tlmWrite_TimeoutCount(m_timeoutCount);
            this->tlmWrite_WatchdogState(m_currentState);
        }
    }
    else if (m_secondsSinceLastPet >= 3) {
        // WARNING STATE
        if (m_currentState != WatchdogState::WARNING) {
            m_currentState = WatchdogState::WARNING;
            
            // Emit warning event
            this->log_WARNING_HI_WatchdogWarning();
            this->tlmWrite_WatchdogState(m_currentState);
        }
        
        // Toggle GPIO 19 to pet the external watchdog
        this->gpioWrite_out(0, Fw::Logic::HIGH);
        this->gpioWrite_out(0, Fw::Logic::LOW);
        
        // Reset timer and update state
        m_secondsSinceLastPet = 0;
        m_totalPets++;
        m_currentState = WatchdogState::HEALTHY;
        
        this->log_ACTIVITY_HI_WatchdogPet();
        this->tlmWrite_TotalPets(m_totalPets);
        this->tlmWrite_TimeSinceLastPet(0);
        this->tlmWrite_WatchdogState(m_currentState);
    }
    else {
        // HEALTHY STATE - Toggle GPIO normally
        if (m_secondsSinceLastPet == 1) {  // Toggle every second when healthy
            this->gpioWrite_out(0, Fw::Logic::HIGH);
            this->gpioWrite_out(0, Fw::Logic::LOW);
            
            m_secondsSinceLastPet = 0;
            m_totalPets++;
            
            this->log_ACTIVITY_HI_WatchdogPet();
            this->tlmWrite_TotalPets(m_totalPets);
            this->tlmWrite_TimeSinceLastPet(0);
        }
        
        // Update state if needed
        if (m_currentState != WatchdogState::HEALTHY) {
            m_currentState = WatchdogState::HEALTHY;
            this->tlmWrite_WatchdogState(m_currentState);
        }
    }
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void WatchdogKai::START_MONITORING_cmdHandler(
    FwOpcodeType opCode, U32 cmdSeq) {
    
    // Start monitoring
    m_isMonitoring = true;
    m_secondsSinceLastPet = 0;
    m_currentState = WatchdogState::HEALTHY;
    m_resetRequested = false;
    
    // Update all telemetry channels
    this->tlmWrite_TimeSinceLastPet(0);
    this->tlmWrite_WatchdogState(m_currentState);
    
    // Send command response
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void WatchdogKai::STOP_MONITORING_cmdHandler(
    FwOpcodeType opCode, U32 cmdSeq) {
    
    // Stop monitoring
    m_isMonitoring = false;
    m_currentState = WatchdogState::STOPPED;
    m_resetRequested = false;
    
    // Update telemetry
    this->tlmWrite_WatchdogState(m_currentState);
    
    // Send command response
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void WatchdogKai::PET_WATCHDOG_cmdHandler(
    FwOpcodeType opCode, U32 cmdSeq) {
    
    if (m_isMonitoring) {
        // Reset the timer
        m_secondsSinceLastPet = 0;
        m_totalPets++;
        m_resetRequested = false;
        
        // Toggle GPIO 19
        this->gpioWrite_out(0, Fw::Logic::HIGH);
        this->gpioWrite_out(0, Fw::Logic::LOW);
        
        // Emit pet event and update telemetry
        this->log_ACTIVITY_HI_WatchdogPet();
        this->tlmWrite_TotalPets(m_totalPets);
        this->tlmWrite_TimeSinceLastPet(0);
        
        // Return to healthy state if we were in warning or timeout
        if (m_currentState != WatchdogState::HEALTHY) {
            m_currentState = WatchdogState::HEALTHY;
            this->tlmWrite_WatchdogState(m_currentState);
        }
    }
    
    // Always send command response, even if not monitoring
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

} // namespace Components