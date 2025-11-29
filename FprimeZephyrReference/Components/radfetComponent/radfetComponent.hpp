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

    // handler for ports
    void schedIn_handler(FwIndexType portNum, U32 context) override;
      
    // handler for commands
    void START_READINGS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void STOP_READINGS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void TAKE_READING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void READ_RADFET_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U8 moduleId, U8 radfetId) override;
    
    // helper methods
    void initializeGpio();
    void enableModule(U8 moduleId);
    void disableModule(U8 moduleId);
    bool readRadfet(U8 moduleId, U8 radfetId);
    void sendDataToFc(U8 moduleId, U8 radfetId, U16 adcValue);
    U16 readAdcValue(U8 adcPin);
    void setRadfetControl(U8 moduleId, U8 radfetId);

    // variables
    bool m_autoReadingsActive;           //!< Automatic readings flag
    bool m_module1Enabled;               // Module 1 enabled state
    bool m_module2Enabled;               // Module 2 enabled state
    U8 m_currentRadfetReading;           // Which RADFET to read next (0 = none, 1 = R1, 2 = R2)

    // Last ADC readings
    U16 m_module1Radfet1Adc;
    U16 m_module1Radfet2Adc;
    U16 m_module2Radfet1Adc;
    U16 m_module2Radfet2Adc;

    // GPIO pin constants
    static const FwIndexType MODULE1_ENABLE = 3;
    static const FwIndexType MODULE1_R1_CTRL = 2; 
    static const FwIndexType MODULE1_R2_CTRL = 4;
    static const FwIndexType MODULE2_ENABLE = 6;
    static const FwIndexType MODULE2_R1_CTRL = 5;
    static const FwIndexType MODULE2_R2_CTRL = 7;
    
    // ADC pin constants
    static const FwIndexType MODULE1_ADC_PIN = 27;  // GPIO 27 = ADC1
    static const FwIndexType MODULE2_ADC_PIN = 29;  // GPIO 29 = ADC3
};

}  // namespace Components

#endif