// ======================================================================
// \title  RADFETHandler.hpp
// \author kai
// \brief  hpp file for RADFETHandler component implementation class
// ======================================================================

#ifndef FprimeZephyrReference_RADFETHandler_HPP
#define FprimeZephyrReference_RADFETHandler_HPP

#include "FprimeZephyrReference/Components/RADFETHandler/RADFETHandlerComponentAc.hpp"

namespace FprimeZephyrReference {

class RADFETHandler final : public RADFETHandlerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct RADFETHandler object
    RADFETHandler(const char* const compName  //!< The component name
    );

    //! Destroy RADFETHandler object
    ~RADFETHandler();
};

private:

  //----------//
 // handlers //
//----------//

void dataIn_handler(FwIndexType portNum, Fw::Buffer& buffer, const Drv::ByteStreamStatus& status) override;

void schedIn_handler(FwIndexType portNum, U32 context) override;

  //------------------//
 // command handlers //
//------------------//

void START_READINGS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 interval) override;
void STOP_READINGS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
void TAKE_READING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
void SEND_COMMAND_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, cosnt Fw::CmdStringArg& cmd) override;

  //------------------//
 // helper methods   //
//------------------//


// Take single rad reading
void takeRadReading();

// Parse rad data from sensor
bool parseRadData(const U8* data, U32, size, U32& radiationValue);

// Store reading to flash w/ timestamp
bool storeReadingToFlash(U32 radiationValue, U32 timestamp);

// Send command to sensor
void sendSensorCommand(const char* command);

// Process data
void processSensorData();

  //--------------------//
 // Conversion methods //
//--------------------//

// Convert raw counts to dose w/ equation
F32 convertRadDose(U32 rawCounts);

// convert raw counts to dose rate 
F32 convertDoseRate(U32 rawCounts);

// Validate raw data before converting
bool validateRawData(U32 rawCounts);



  //------------------//
 //    variables     //
//------------------//

// reading state

bool m_periodicReadings = Falase;
U32 m_readingInterval = 0;
U32 m_readingsCount = 0;

// conversion state

F32 m_lastDose = 0.0;
F32 m_lastDoseRate = 0.0;
U32 m_last_rawCounts = 0;
Fw::Time m_lastReadingTime;

// Data buffer
static constexpr U32 DATA_BUFFER_SIZE = 64;
U8 m_dataBuffer[DATA_BUFFER_SIZE];
U32 m_dataBufferSize = 0;

// current reading
U32 m_currentRadiation = 0;

// File Management
Os::File m_dataFile;
std::string m_currentDataFile;
bool m_fileOpen = false;
U32 m_storedReadings = 0;

// Sensor protocol
static constexpr U32 RESPONSE_START_MARKER = 0xAA;
static constexpr U32 RESPONZE_SIZE = 8;


}  // namespace FprimeZephyrReference

#endif
