#ifndef FprimeZephyrReference_RADFETHandler_HPP
#define FprimeZephyrReference_RADFETHandler_HPP

#include <FprimeZephyrReference/Components/RADFETHandler/RADFETHandlerComponentAc.hpp>
#include <Os/File.hpp>
#include <string>

namespace Components {

class RADFETHandler : public RADFETHandlerComponentBase {
  public:
    RADFETHandler(const char* const compName);
    ~RADFETHandler();

  private:
    void dataIn_handler(FwIndexType portNum, Fw::Buffer& buffer, const Drv::ByteStreamStatus& status) override;    
    void schedIn_handler(FwIndexType portNum, U32 context) override;

    void START_READINGS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 interval) override;
    void STOP_READINGS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void TAKE_READING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void SEND_COMMAND_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, const Fw::CmdStringArg& cmd) override;

    bool accumulateSensorData(const U8* data, U32 size);
    void clearDataBuffer();
    void processSensorData();
    bool parseRadiationData(const U8* data, U32 size, U32& rawCounts);
    void removeProcessedData(U32 size);
    bool validateRawData(U32 rawCounts);
    F32 convertToRadiationDose(U32 rawCounts);
    F32 convertToDoseRate(U32 rawCounts);
    void takeRadiationReading();
    void sendSensorCommand(const char* command);
    bool storeReadingToFlash(U32 rawCounts, F32 radiationDose, F32 doseRate, U32 timestamp);
    U32 calculateChecksum(void* data, U32 size);
    void openDataFile();

    bool m_periodicReadings;
    U32 m_readingInterval;
    U32 m_readingsCount;
    U32 m_storedReadings;
    F32 m_lastRadiationDose;
    F32 m_lastDoseRate;
    U32 m_lastRawCounts;

    static constexpr U32 DATA_BUFFER_SIZE = 128;
    U8 m_dataBuffer[DATA_BUFFER_SIZE];
    U32 m_dataBufferSize;

    U32 m_currentRadiation;

    Os::File m_dataFile;
    std::string m_currentDataFile;
    bool m_fileOpen;

    U32 m_lastReadingTimestamp;


    static constexpr U32 RESPONSE_START_MARKER = 0xAA;
    static constexpr U32 RESPONSE_SIZE = 6;
    static constexpr U32 MAX_STORED_READINGS = 10000;
};

}
#endif
