// ======================================================================
// \title  radfetComponent.cpp
// \author kai
// \brief  cpp file for radfetComponent component implementation class
// ======================================================================

#include "FprimeZephyrReference/Components/radfetComponent/radfetComponent.hpp"
#include "Fw/Logger/Logger.hpp"
#include <zephyr/drivers/adc.h>
#include "Fw/Time/TimeInterval.hpp"

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

radfetComponent ::radfetComponent(const char* const compName) : radfetComponentComponentBase(compName), 
    m_autoReadingsActive(false),
    m_module1Enabled(false),
    m_module2Enabled(false),
    m_currentRadfetReading(0),
    m_module1Radfet1Adc(0),
    m_module1Radfet2Adc(0),
    m_module2Radfet1Adc(0),
    m_module2Radfet2Adc(0)
 {
    initializeGpio();

    this->tlmWrite_Module1Radfet1Adc(0);
    this->tlmWrite_Module1Radfet2Adc(0);
    this->tlmWrite_Module2Radfet1Adc(0);
    this->tlmWrite_Module2Radfet2Adc(0);
    this->tlmWrite_Module1State(0);
    this->tlmWrite_Module2State(0);

 }

radfetComponent ::~radfetComponent() {}

// handler implementation
void radfetComponent ::schedIn_handler(FwIndexType portNum, U32 context) {
    // Take automatic readings if enabled
    if (m_autoReadingsActive) {
        // Alternate between R1 and R2 readings for each enabled module
        if (m_currentRadfetReading == 0 || m_currentRadfetReading == 2) {
            // Read R1 from enabled modules
            if (m_module1Enabled) {
                readRadfet(1, 1);
            }
            if (m_module2Enabled) {
                readRadfet(2, 1);
            }
            m_currentRadfetReading = 1;
        } else {
            // Read R2 from enabled modules
            if (m_module1Enabled) {
                readRadfet(1, 2);
            }
            if (m_module2Enabled) {
                readRadfet(2, 2);
            }
            m_currentRadfetReading = 2;
        }
    }
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void radfetComponent ::START_READINGS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    m_autoReadingsActive = true;
    m_currentRadfetReading = 0; // Start with R1
    
    // Enable both modules
    enableModule(1);
    enableModule(2);
    
    this->log_ACTIVITY_HI_ReadingsStarted();
    this->tlmWrite_Module1State(1);
    this->tlmWrite_Module2State(1);
    
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void radfetComponent ::STOP_READINGS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    m_autoReadingsActive = false;
    
    // Disable both modules
    disableModule(1);
    disableModule(2);
    
    this->log_ACTIVITY_HI_ReadingsStopped();
    this->tlmWrite_Module1State(0);
    this->tlmWrite_Module2State(0);
    
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void radfetComponent ::TAKE_READING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    bool success = false;
    
    // Take one reading from each enabled module (R1 only for single reading)
    if (m_module1Enabled) {
        success = readRadfet(1, 1) || success;
    }
    if (m_module2Enabled) {
        success = readRadfet(2, 1) || success;
    }
    
    if (success) {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
    } else {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
    }
}

void radfetComponent ::READ_RADFET_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U8 moduleId, U8 radfetId) {
    // Validate inputs
    if ((moduleId != 1 && moduleId != 2) || (radfetId != 1 && radfetId != 2)) {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::VALIDATION_ERROR);
        return;
    }
    
    // Enable module if not already enabled
    if (moduleId == 1 && !m_module1Enabled) {
        enableModule(1);
    } else if (moduleId == 2 && !m_module2Enabled) {
        enableModule(2);
    }
    
    // Take the specific reading
    bool success = readRadfet(moduleId, radfetId);
    
    if (success) {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
    } else {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
    }
}

// ----------------------------------------------------------------------
// Private helper methods
// ----------------------------------------------------------------------

void radfetComponent ::initializeGpio() {
    // Initialize all RADFET control pins to off (safe state)
    this->gpioSet_out(MODULE1_ENABLE, Fw::Logic::LOW);
    this->gpioSet_out(MODULE1_R1_CTRL, Fw::Logic::LOW);
    this->gpioSet_out(MODULE1_R2_CTRL, Fw::Logic::LOW);
    this->gpioSet_out(MODULE2_ENABLE, Fw::Logic::LOW);
    this->gpioSet_out(MODULE2_R1_CTRL, Fw::Logic::LOW);
    this->gpioSet_out(MODULE2_R2_CTRL, Fw::Logic::LOW);
}

void radfetComponent ::enableModule(U8 moduleId) {
    if (moduleId == 1) {
        this->gpioSet_out(MODULE1_ENABLE, Fw::Logic::HIGH);
        m_module1Enabled = true;
        this->log_ACTIVITY_HI_ModuleEnabled(1);
    } else if (moduleId == 2) {
        this->gpioSet_out(MODULE2_ENABLE, Fw::Logic::HIGH);
        m_module2Enabled = true;
        this->log_ACTIVITY_HI_ModuleEnabled(2);
    }
}

void radfetComponent ::disableModule(U8 moduleId) {
    if (moduleId == 1) {
        // Turn off all control pins for module 1
        this->gpioSet_out(MODULE1_ENABLE, Fw::Logic::LOW);
        this->gpioSet_out(MODULE1_R1_CTRL, Fw::Logic::LOW);
        this->gpioSet_out(MODULE1_R2_CTRL, Fw::Logic::LOW);
        m_module1Enabled = false;
        this->log_ACTIVITY_HI_ModuleDisabled(1);
    } else if (moduleId == 2) {
        // Turn off all control pins for module 2
        this->gpioSet_out(MODULE2_ENABLE, Fw::Logic::LOW);
        this->gpioSet_out(MODULE2_R1_CTRL, Fw::Logic::LOW);
        this->gpioSet_out(MODULE2_R2_CTRL, Fw::Logic::LOW);
        m_module2Enabled = false;
        this->log_ACTIVITY_HI_ModuleDisabled(2);
    }
}

bool radfetComponent ::readRadfet(U8 moduleId, U8 radfetId) {
    U16 adcValue = 0;
    U8 adcPin = 0;
    
    // Get the correct ADC pin for the module
    if (moduleId == 1) {
        adcPin = MODULE1_ADC_PIN;
    } else if (moduleId == 2) {
        adcPin = MODULE2_ADC_PIN;
    } else {
        return false;
    }
    
    // Set the correct control pins (R1 and R2 should not be enabled at the same time)
    setRadfetControl(moduleId, radfetId);

    // small delay
    Os::Task::delay(Fw::TimeInterval(0, 10000));    
    // Read ADC value
    adcValue = readAdcValue(adcPin);
    
    // Update telemetry and store value
    if (moduleId == 1) {
        if (radfetId == 1) {
            m_module1Radfet1Adc = adcValue;
            this->tlmWrite_Module1Radfet1Adc(adcValue);
        } else {
            m_module1Radfet2Adc = adcValue;
            this->tlmWrite_Module1Radfet2Adc(adcValue);
        }
    } else {
        if (radfetId == 1) {
            m_module2Radfet1Adc = adcValue;
            this->tlmWrite_Module2Radfet1Adc(adcValue);
        } else {
            m_module2Radfet2Adc = adcValue;
            this->tlmWrite_Module2Radfet2Adc(adcValue);
        }
    }
    
    // Log the reading
    this->log_ACTIVITY_HI_ReadingTaken(moduleId, radfetId, adcValue);
    
    // Send to flight computer via UART
    sendDataToFc(moduleId, radfetId, adcValue);
    
    return true;
}

void radfetComponent ::setRadfetControl(U8 moduleId, U8 radfetId) {
    if (moduleId == 1) {
        if (radfetId == 1) {
            // Enable R1, disable R2
            this->gpioSet_out(MODULE1_R1_CTRL, Fw::Logic::HIGH);
            this->gpioSet_out(MODULE1_R2_CTRL, Fw::Logic::LOW);
        } else {
            // Enable R2, disable R1
            this->gpioSet_out(MODULE1_R1_CTRL, Fw::Logic::LOW);
            this->gpioSet_out(MODULE1_R2_CTRL, Fw::Logic::HIGH);
        }
    } else if (moduleId == 2) {
        if (radfetId == 1) {
            // Enable R1, disable R2
            this->gpioSet_out(MODULE2_R1_CTRL, Fw::Logic::HIGH);
            this->gpioSet_out(MODULE2_R2_CTRL, Fw::Logic::LOW);
        } else {
            // Enable R2, disable R1
            this->gpioSet_out(MODULE2_R1_CTRL, Fw::Logic::LOW);
            this->gpioSet_out(MODULE2_R2_CTRL, Fw::Logic::HIGH);
        }
    }
}

void radfetComponent ::sendDataToFc(U8 moduleId, U8 radfetId, U16 adcValue) {
    // Data packet: [moduleId][radfetId][adcValueHigh][adcValueLow]
    U8 data[4];
    data[0] = moduleId;
    data[1] = radfetId;
    data[2] = (adcValue >> 8) & 0xFF;  // High byte
    data[3] = adcValue & 0xFF;         // Low byte
    
    // Send raw bytes via UART
    Fw::Buffer sendBuffer;
    sendBuffer.setData(data);
    sendBuffer.setSize(sizeof(data));
    
    // Send via UART to flight computer
    this->dataOut_out(0, sendBuffer);
    
    // Log that data was sent successfully
    this->log_ACTIVITY_HI_DataSent(moduleId, radfetId, sizeof(data));
}

U16 radfetComponent ::readAdcValue(U8 adcPin) {
    // Try to find the ADC device - use the correct device tree label
    const struct device *adc_dev = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(adc0));  // Try adc0
    
    if (adc_dev == NULL) {
        adc_dev = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(adc1));  // Try adc1
    }
    
    if (adc_dev == NULL) {
        Fw::Logger::log("[ERROR] No ADC device found!\n");
        return 0;
    }
    
    if (!device_is_ready(adc_dev)) {
        Fw::Logger::log("[ERROR] ADC device not ready\n");
        return 0;
    }
    
    Fw::Logger::log("[INFO] Using ADC device: %s\n", adc_dev->name);
    
    // Rest of ADC reading code...
    // ADC channel configuration
    struct adc_channel_cfg channel_cfg = {
        .gain = ADC_GAIN_1,
        .reference = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id = adcPin, 
    };
    
    int ret = adc_channel_setup(adc_dev, &channel_cfg);
    if (ret != 0) {
        Fw::Logger::log("[ERROR] ADC channel setup failed: %d\n", ret);
        return 0;
    }
    
    int16_t buf;
    struct adc_sequence sequence = {
        .channels = BIT(channel_cfg.channel_id),
        .buffer = &buf,
        .buffer_size = sizeof(buf),
        .resolution = 12,
    };
    
    ret = adc_read(adc_dev, &sequence);
    if (ret == 0) {
        U16 value = (U16)(buf > 0 ? buf : 0);  // Ensure positive value
        Fw::Logger::log("[INFO] Real ADC reading from pin %d: 0x%04X\n", adcPin, value);
        return value;
    }
    
    Fw::Logger::log("[ERROR] ADC read failed on pin %d: %d\n", adcPin, ret);
    return 0;
}

}  // namespace Components