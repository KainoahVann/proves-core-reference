// ======================================================================
// \title  radfetComponent.cpp
// \author kai
// \brief  cpp file for radfetComponent component implementation class
// ======================================================================

#include "FprimeZephyrReference/Components/radfetComponent/radfetComponent.hpp"
#include <Fw/Types/Assert.hpp>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>

// ADC configuration
#define ADC_NODE DT_NODELABEL(adc)
#define ADC_RESOLUTION 12
#define ADC_GAIN ADC_GAIN_1
#define ADC_REFERENCE ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME_DEFAULT

// ADC channels
#define MODULE1_ADC_CHANNEL 1 // GPIO27_ADC1
#define MODULE2_ADC_CHANNEL 3 // GPIO29_ADC3

// GPIO port indices (must match .fpp definition)
#define GPIO_MODULE1_ENABLE  0
#define GPIO_MODULE1_R1_CTRL 1
#define GPIO_MODULE1_R2_CTRL 2
#define GPIO_MODULE2_ENABLE  3
#define GPIO_MODULE2_R1_CTRL 4
#define GPIO_MODULE2_R2_CTRL 5

// Timing
#define SETTLING_TIME_MS 10  // Wait time after enabling before ADC read


namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

radfetComponent ::radfetComponent(const char* const compName)  
    : radfetComponentComponentBase(compName),
      m_reading(false),
      m_totalReadings(0),
      m_module1Enabled(false),
      m_module2Enabled(false),
      m_currentModule(0),
      m_currentRadfet(0)
{}

radfetComponent ::~radfetComponent() {
    disableModule(1);
    disableModule(2);
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

//void radfetComponent ::TODO_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // TODO
//    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
//}

// Command: Start periodic readings
void radfetComponent::START_READINGS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    m_reading = true;
    this->log_ACTIVITY_HI_ReadingsStarted();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// Command: Stop readings
void radfetComponent::STOP_READINGS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    m_reading = false;
    
    // Disable both modules
    disableModule(1);
    disableModule(2);
    
    this->log_ACTIVITY_HI_ReadingsStopped();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// Command: Take reading
void radfetComponent::READ_RADFET_cmdHandler(
    FwOpcodeType opCode, 
    U32 cmdSeq,
    U8 moduleNum,
    U8 radfet
) {
    // Validate inputs
    if (moduleNum < 1 || moduleNum > 2 || radfet < 1 || radfet > 2) {
        this->log_WARNING_HI_InvalidSelection(moduleNum, radfet);
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::VALIDATION_ERROR);
        return;
    }
    
    // Read the specified RADFET
    readRadfet(moduleNum, radfet);
    
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// Port handler: Scheduled input for periodic readings
void radfetComponent::schedIn_handler(FwIndexType portNum, U32 context) {
    if (m_reading) {
        // Read all 4 RADFETs in sequence
        readRadfet(1, 1);
        readRadfet(1, 2);
        readRadfet(2, 1);
        readRadfet(2, 2);
        
        m_totalReadings++;
        this->tlmWrite_TotalReadings(m_totalReadings);
    }
}

// Helper: Enable Module via GPIO
void radfetComponent::enableModule(U8 moduleNum) {
    U8 enablePort = (moduleNum == 1) ? GPIO_MODULE1_ENABLE : GPIO_MODULE2_ENABLE;
    
    Fw::Logic state = Fw::Logic::HIGH;
    this->gpioSet_out(enablePort, state);
    
    if (moduleNum == 1) {
        m_module1Enabled = true;
        this->tlmWrite_MODULE1_state(1);
    } else {
        m_module2Enabled = true;
        this->tlmWrite_MODULE2_state(1);
    }
    
    this->log_ACTIVITY_LO_ModuleEnabled(moduleNum);
}

// Helper: Disable Module
void radfetComponent::disableModule(U8 moduleNum) {
    U8 enablePort = (moduleNum == 1) ? GPIO_MODULE1_ENABLE : GPIO_MODULE2_ENABLE;
    U8 r1Port = (moduleNum == 1) ? GPIO_MODULE1_R1_CTRL : GPIO_MODULE2_R1_CTRL;
    U8 r2Port = (moduleNum == 1) ? GPIO_MODULE1_R2_CTRL : GPIO_MODULE2_R2_CTRL;
    
    // Disable all control signals
    Fw::Logic low = Fw::Logic::LOW;
    this->gpioSet_out(r1Port, low);
    this->gpioSet_out(r2Port, low);
    this->gpioSet_out(enablePort, low);
    
    if (moduleNum == 1) {
        m_module1Enabled = false;
        this->tlmWrite_MODULE1_state(0);
    } else {
        m_module2Enabled = false;
        this->tlmWrite_MODULE2_state(0);
    }
    
    this->log_ACTIVITY_LO_ModuleDisabled(moduleNum);
}

// Helper: Read specified RADFET
void radfetComponent::selectRadfet(U8 moduleNum, U8 radfet) {
    U8 r1Port = (moduleNum == 1) ? GPIO_MODULE1_R1_CTRL : GPIO_MODULE2_R1_CTRL;
    U8 r2Port = (moduleNum == 1) ? GPIO_MODULE1_R2_CTRL : GPIO_MODULE2_R2_CTRL;
    
    // CRITICAL: R1 and R2 cannot both be HIGH
    if (radfet == 1) {
        // Select RADFET 1: R1=HIGH, R2=LOW
        this->gpioSet_out(r2Port, Fw::Logic::LOW);  // Set R2 LOW first
        k_msleep(1);  // Small delay
        this->gpioSet_out(r1Port, Fw::Logic::HIGH); // Then R1 HIGH
    } else {
        // Select RADFET 2: R1=LOW, R2=HIGH
        this->gpioSet_out(r1Port, Fw::Logic::LOW);  // Set R1 LOW first
        k_msleep(1);  // Small delay
        this->gpioSet_out(r2Port, Fw::Logic::HIGH); // Then R2 HIGH
    }
    
    m_currentModule = moduleNum;
    m_currentRadfet = radfet;
}

// Helper: Deselect RADFETs
void radfetComponent::deselectRadfet(U8 moduleNum) {
    U8 r1Port = (moduleNum == 1) ? GPIO_MODULE1_R1_CTRL : GPIO_MODULE2_R1_CTRL;
    U8 r2Port = (moduleNum == 1) ? GPIO_MODULE1_R2_CTRL : GPIO_MODULE2_R2_CTRL;
    
    // Set both control signals LOW
    this->gpioSet_out(r1Port, Fw::Logic::LOW);
    this->gpioSet_out(r2Port, Fw::Logic::LOW);
}

// Helper: Read ADC and send to FC
void radfetComponent::readRadfet(U8 moduleNum, U8 radfet) {
    // 1. Enable the module
    enableModule(moduleNum);
    
    // 2. Select the RADFET
    selectRadfet(moduleNum, radfet);
    
    // 3. Wait for settling
    k_msleep(SETTLING_TIME_MS);
    
    // 4. Read ADC
    U16 adcValue = readADC(moduleNum);
    
    // 5. Deselect RADFET
    deselectRadfet(moduleNum);
    
    // 6. Disable module
    disableModule(moduleNum);
    
    // 7. Update telemetry
    updateTelemetry(moduleNum, radfet, adcValue);
    
    // 8. Log event
    this->log_ACTIVITY_HI_ReadingTaken(moduleNum, radfet, adcValue);
    
    // 9. Send to flight computer
    sendToFC(moduleNum, radfet, adcValue);
}

U16 radfetComponent::readADC(U8 moduleNum) {
    // Get ADC device
    const struct device *adc_dev = DEVICE_DT_GET(ADC_NODE);
    if (!device_is_ready(adc_dev)) {
        this->log_WARNING_HI_SensorError(moduleNum, m_currentRadfet);
        return 0;
    }
    
    // Select ADC channel based on module
    U8 channel = (moduleNum == 1) ? MODULE1_ADC_CHANNEL : MODULE2_ADC_CHANNEL;
    
    // Configure ADC channel
    struct adc_channel_cfg channel_cfg = {
        .gain = ADC_GAIN,
        .reference = ADC_REFERENCE,
        .acquisition_time = ADC_ACQUISITION_TIME,
        .channel_id = channel,
        .differential = 0
    };
    
    adc_channel_setup(adc_dev, &channel_cfg);
    
    // Read ADC
    U16 adc_buffer = 0;
    struct adc_sequence sequence = {
        .channels = BIT(channel),
        .buffer = &adc_buffer,
        .buffer_size = sizeof(adc_buffer),
        .resolution = ADC_RESOLUTION,
    };
    
    int ret = adc_read(adc_dev, &sequence);
    
    return (ret == 0) ? adc_buffer : 0;
}

void radfetComponent::updateTelemetry(U8 moduleNum, U8 radfet, U16 value) {
    if (moduleNum == 1) {
        if (radfet == 1) {
            this->tlmWrite_MODULE1_RADFET1_adc(value);
        } else {
            this->tlmWrite_MODULE1_RADFET2_adc(value);
        }
    } else {
        if (radfet == 1) {
            this->tlmWrite_MODULE2_RADFET1_adc(value);
        } else {
            this->tlmWrite_MODULE2_RADFET2_adc(value);
        }
    }
}

void radfetComponent::sendToFC(U8 moduleNum, U8 radfet, U16 adcValue) {
    // Format packet for flight computer
    // [START_MARKER, module, radfet, adc_high, adc_low, checksum]
    U8 packet[6];
    packet[0] = 0xAA;  // START_MARKER
    packet[1] = moduleNum;
    packet[2] = radfet;
    packet[3] = (adcValue >> 8) & 0xFF;  // High byte
    packet[4] = adcValue & 0xFF;         // Low byte
    packet[5] = packet[0] ^ packet[1] ^ packet[2] ^ packet[3] ^ packet[4];  // XOR checksum
    
    // Send via UART to FC
    Fw::Buffer dataBuffer(packet, 6);
    this->dataOut_out(0, dataBuffer);
    
    this->log_ACTIVITY_LO_DataSent(moduleNum, radfet, 6);
}


}  // namespace Components
