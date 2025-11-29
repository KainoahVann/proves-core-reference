// ======================================================================
// \title  radfetComponent.hpp
// \author kai
// \brief  hpp file for radfetComponent component implementation class
// ======================================================================

#ifndef Components_radfetComponent_HPP
#define Components_radfetComponent_HPP

#include "FprimeZephyrReference/Components/radfetComponent/radfetComponentComponentAc.hpp"

namespace Components {

class radfetComponent final : public radfetComponentComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct radfetComponent object
    radfetComponent(const char* const compName  //!< The component name
    );

    //! Destroy radfetComponent object
    ~radfetComponent();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command TODO
    //!
    //! TODO
    //void TODO_cmdHandler(FwOpcodeType opCode,  //!< The opcode
    //                     U32 cmdSeq            //!< The command sequence number
    //                     ) override;
    //! Handler for START_READINGS command
    void START_READINGS_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
    ) override;

    //! Handler for STOP_READINGS command
    void STOP_READINGS_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
    ) override;

    //! Handler for READ_RADFET command
    void READ_RADFET_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq,
        U8 moduleNum,
        U8 radfet
    ) override;

    //! Handler for schedIn port
    void schedIn_handler(
        FwIndexType portNum,
        U32 context
    ) override;

    // State variables
    bool m_reading;           // Are we taking periodic readings?
    U32 m_totalReadings;      // Counter for total readings
    bool m_module1Enabled;    // Module 1 power state
    bool m_module2Enabled;    // Module 2 power state
    U8 m_currentModule;       // Currently selected module
    U8 m_currentRadfet;       // Currently selected RADFET
    
    // Helper methods
    void enableModule(U8 module);
    void disableModule(U8 module);
    void selectRadfet(U8 module, U8 radfet);
    void deselectRadfet(U8 module);
    void readRadfet(U8 module, U8 radfet);
    U16 readADC(U8 module);
    void updateTelemetry(U8 module, U8 radfet, U16 value);
    void sendToFC(U8 module, U8 radfet, U16 adcValue);
};

}  // namespace Components

#endif
