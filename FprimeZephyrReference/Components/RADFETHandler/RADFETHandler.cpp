#include "FprimeZephyrReference/Components/RADFETHandler/RADFETHandler.hpp"
#include <Fw/Types/Assert.hpp>
#include <Os/File.hpp>
#include <cstring>
#include <string>

namespace Components {

RADFETHandler ::RADFETHandler(const char* const compName) : RADFETHandlerComponentBase(compName),
    m_periodicReadings(false),
    m_readingInterval(0),
    m_readingsCount(0),
    m_storedReadings(0),
    m_lastRadiationDose(0.0),
    m_lastDoseRate(0.0),
    m_lastRawCounts(0),
    m_dataBufferSize(0),
    m_currentRadiation(0),
    m_fileOpen(false),
    m_lastReadingTimestamp(0)
{
    memset(m_dataBuffer, 0, DATA_BUFFER_SIZE);
}

RADFETHandler ::~RADFETHandler() {
    if(m_fileOpen){
        m_dataFile.close();
        m_fileOpen = false;
    }
}

void RADFETHandler ::dataIn_handler(FwIndexType portNum, Fw::Buffer& buffer, const Drv::ByteStreamStatus& status){
    if(status != Drv::ByteStreamStatus::OP_OK){
        this->log_WARNING_HI_SensorError(1);
        return;
    }

    if (!buffer.isValid()){
        return;
    }

    const U8* data = buffer.getData();
    U32 dataSize = static_cast<U32>(buffer.getSize());

    if(dataSize >= 4){
        this->log_ACTIVITY_LO_RawDataReceived(static_cast<U32>(data[3]) << 8 | static_cast<U32>(data[4]));
        //this->log_ACTIVITY_LO_RawDataReceived(data[4]);
    }

    if (!accumulateSensorData(data, dataSize)){
        processSensorData();
        clearDataBuffer();
        accumulateSensorData(data, dataSize);
    }

    processSensorData();
}

void RADFETHandler::schedIn_handler(FwIndexType portNum, U32 context){
    if (m_periodicReadings){
        takeRadiationReading();
    }
}

void RADFETHandler::START_READINGS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 interval){
    m_periodicReadings = true;
    m_readingInterval = interval;

    if(!m_fileOpen){
        openDataFile();
    }

    sendSensorCommand("START\n");

    this->log_ACTIVITY_HI_ReadingStarted(interval);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void RADFETHandler::STOP_READINGS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    m_periodicReadings = false;
    sendSensorCommand("STOP\n");
    this->log_ACTIVITY_HI_ReadingStopped();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void RADFETHandler::TAKE_READING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq){
    takeRadiationReading();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void RADFETHandler::SEND_COMMAND_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, const Fw::CmdStringArg& cmd){
    Fw::CmdStringArg tempCmd = cmd;
    tempCmd += "\n";
    Fw::Buffer commandBuffer(reinterpret_cast<U8*>(const_cast<char*>(tempCmd.toChar())), tempCmd.length());

    this->commandOut_out(0, commandBuffer, Drv::ByteStreamStatus::OP_OK);

    Fw::LogStringArg logCmd(cmd);
    this->log_ACTIVITY_HI_CommandSuccess(logCmd);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

bool RADFETHandler::accumulateSensorData(const U8* data, U32 size){
    if(m_dataBufferSize + size > DATA_BUFFER_SIZE){
        return false;
    }

    memcpy(&m_dataBuffer[m_dataBufferSize], data, size);
    m_dataBufferSize += size;
    return true;
}

void RADFETHandler::clearDataBuffer(){
    m_dataBufferSize = 0;
    memset(m_dataBuffer, 0, DATA_BUFFER_SIZE);
}

void RADFETHandler::processSensorData(){
    while(m_dataBufferSize >= RESPONSE_SIZE){
        U32 rawCounts = 0;

        if(parseRadiationData(m_dataBuffer, m_dataBufferSize, rawCounts)){
            this->tlmWrite_RawCounts(rawCounts);
            this->log_ACTIVITY_LO_RawDataReceived(rawCounts);

            if(validateRawData(rawCounts)){
                F32 radiationDose = convertToRadiationDose(rawCounts);
                F32 doseRate = convertToDoseRate(rawCounts);

                this->tlmWrite_RadiationDose(radiationDose);
                this->tlmWrite_DoseRate(doseRate);
                this->tlmWrite_ReadingsCount(++m_readingsCount);
                this->tlmWrite_SensorStatus(0);

                U32 timestamp = m_readingsCount;
                this->log_ACTIVITY_HI_ReadingComplete(rawCounts, timestamp);

                if(storeReadingToFlash(rawCounts, radiationDose, doseRate, timestamp)){
                    m_storedReadings++;
                    this->tlmWrite_StorageUsage(static_cast<U32>((m_storedReadings * 100) / MAX_STORED_READINGS));
                }else{
                    this->log_WARNING_HI_StorageError(1);
                }
            }else{
                this->tlmWrite_SensorStatus(1);
            }
        }
        removeProcessedData(RESPONSE_SIZE);
    }
}

bool RADFETHandler::parseRadiationData(const U8* data, U32 size, U32& rawCounts){
    if (size < RESPONSE_SIZE){
        return false;
    }

    if(data[0] != RESPONSE_START_MARKER){
        return false;
    }
        
    U8 moduleNum = data[1];  // Module 1 or 2
    U8 radfetNum = data[2];  // RADFET 1 or 2

    rawCounts = (static_cast<U32>(data[3]) << 8) | static_cast<U32>(data[4]);

    U8 calculatedChecksum = 0;
    for(U32 i = 0; i < RESPONSE_SIZE - 1; i++){
        calculatedChecksum ^= data[i];
    }

    if(calculatedChecksum != data[RESPONSE_SIZE -1]){
        this->log_WARNING_HI_SensorError(2);
        return false;
    }

    return true;
}

void RADFETHandler::removeProcessedData(U32 size){
    if(size >= m_dataBufferSize){
        clearDataBuffer();
    }else{
        memmove(m_dataBuffer, &m_dataBuffer[size], m_dataBufferSize - size);
        m_dataBufferSize -= size;
    }
}

bool RADFETHandler::validateRawData(U32 rawCounts){
    if(rawCounts == 0xFFFFFFFF){
        this->log_WARNING_HI_ConversionError(rawCounts, 1);
        return false;
    }

    if(rawCounts > 0xFFFF){
        this->log_WARNING_HI_ConversionError(rawCounts,2);
        return false;
    }

    return true;
}

F32 RADFETHandler::convertToRadiationDose(U32 rawCounts){
    Fw::ParamValid valid;
    F32 calibrationA = this->paramGet_CalibrationA(valid);
    F32 calibrationB = this->paramGet_CalibrationB(valid);

    F32 dose = (calibrationA * static_cast<F32>(rawCounts)) + calibrationB;

    this->log_ACTIVITY_HI_DoseCalculated(rawCounts, dose);
    return dose;
}

F32 RADFETHandler::convertToDoseRate(U32 rawCounts){
    F32 currentDose = convertToRadiationDose(rawCounts);

    if(m_lastReadingTimestamp > 0){
        F32 timeDelta = 1.0f;
        if(timeDelta > 0.1f){
            m_lastDoseRate = (currentDose - m_lastRadiationDose) / timeDelta;
        }
    }

    m_lastRadiationDose = currentDose;
    m_lastRawCounts = rawCounts;
    m_lastReadingTimestamp = m_readingsCount;

    return m_lastDoseRate;
}

void RADFETHandler::takeRadiationReading(){
    sendSensorCommand("MEASURE\n");
}

void RADFETHandler::sendSensorCommand(const char* command){
    Fw::Buffer commandBuffer(reinterpret_cast<U8*>(const_cast<char*>(command)), strlen(command));
    this->commandOut_out(0, commandBuffer, Drv::ByteStreamStatus::OP_OK);
}

bool RADFETHandler::storeReadingToFlash(U32 rawCounts, F32 radiationDose, F32 doseRate, U32 timestamp){
    struct RadiationReading{
        U32 timestamp;
        U32 rawCounts;
        F32 radiationDose;
        F32 doseRate;
        U32 checksum;
    } reading;

    reading.timestamp = timestamp;
    reading.rawCounts = rawCounts;
    reading.radiationDose = radiationDose;
    reading.doseRate = doseRate;
    reading.checksum = calculateChecksum(&reading, sizeof(reading) - sizeof(U32));

    if(m_fileOpen){
        FwSizeType writeSize = sizeof(reading);
        Os::File::Status status = m_dataFile.write(
            reinterpret_cast<U8*>(&reading),
            writeSize,
            Os::File::WaitType::WAIT
);
        return (status == Os::File::OP_OK);
    }
    return false;
}

U32 RADFETHandler::calculateChecksum(void* data, U32 size){
    U32 checksum = 0;
    U8* byteData = static_cast<U8*>(data);

    for(U32 i=0; i< size; i++){
        checksum += byteData[i];
    }
    return checksum;
}

void RADFETHandler::openDataFile(){
    char filename[64];
    snprintf(filename, sizeof(filename), "/radfet_data.bin");   
    m_currentDataFile = filename;

    Os::File::Status status = m_dataFile.open(m_currentDataFile.c_str(), Os::File::OPEN_WRITE);

    if(status == Os::File::OP_OK){
        m_fileOpen = true;
        this->log_ACTIVITY_HI_DataFileOpened(Fw::LogStringArg(m_currentDataFile.c_str()));
    }else{
        this->log_WARNING_HI_StorageError(2);
    }
}

}
