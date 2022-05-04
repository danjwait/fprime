// ======================================================================
// \title  Gps.cpp
// \author djwait
// \brief  cpp file for Gps component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================


#include <GpsApp/Gps/Gps.hpp>
#include "Fw/Types/BasicTypes.hpp"
#include "Fw/Logger/Logger.hpp" // DJW this was in the Gps tutorial, not the stub

#include <cstring>

namespace GpsApp {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  Gps ::
    Gps(
        const char *const compName
    ) :
      GpsComponentBase(compName),
    // initialize the lock to "false"
    m_locked(false)
  {

  }
  // DJW only added locked status to above from stub; tutorial has this as an if

  void Gps ::
    init(
        const NATIVE_INT_TYPE queueDepth,
        const NATIVE_INT_TYPE instance
    )
  {
    GpsComponentBase::init(queueDepth, instance);
  }

  // Step 0: 
  // The linux serial driver keeps its storage extenal.
  // This means we need to supply it with some buffers to work with.
  // This code will loop through our member variables holding buffers 
  // and send them to the linux serial driver. 'preamble' is 
  // automatically called after the system is constructed, before the 
  // system runs at steady-state. This allows for initialization code 
  // which invokes working ports

  void Gps :: preamble()
  {
    for (NATIVE_INT_TYPE buffer =0; buffer < NUM_UART_BUFFERS; buffer ++) {
      // Assign the raw data to the buffer. Make sure to include the side
      // of the region assigned.
      this->m_recvBuffers[buffer].setData(this->m_uartBuffers[buffer]);
      this->m_recvBuffers[buffer].setSize(UART_READ_BUFF_SIZE);
      // Invoke the port to send the buffer out
      this->serialBufferOut_out(0,this->m_recvBuffers[buffer]);
    }
    Fw::Logger::logMsg("[INFO] Preamble size: %d\n", NUM_UART_BUFFERS); // DEBUG
  }

  Gps ::
    ~Gps()
  {

  }

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  // Step 1: serialIn
  //
  // Impliment a handler to respond to the serial device sending data buffer
  // containing the GPS data. This will handle the serial message. 

  void Gps ::
    serialRecv_handler(
        const NATIVE_INT_TYPE portNum,  /*!< The port number*/
        Fw::Buffer &serBuffer, /*!< Buffer containing data*/
        Drv::SerialReadStatus &serial_status // DJW this reference is different in tutorial
    )
  {
    // local variable definitions
    int status = 0;
    float lat = 0.0f, lon = 0.0f;
    GpsPacket packet;
    char dgpsStation[20];

    // DEBUG
    //tries++;
    //lat = 42.42;
    //lon = 24.24;
    //packet.altitude = 420.420;
    //packet.count = 42;
    //packet.lock = 0;
    //this->tlmWrite_GPS_LATITUDE(lat);
    //this->tlmWrite_GPS_LONGITUDE(lon);
    //this->tlmWrite_GPS_ALTITUDE(packet.altitude);
    //this->tlmWrite_GPS_SV_COUNT(packet.count);
    //this->tlmWrite_GPS_LOCK_STATUS(packet.lock);
    //this->tlmWrite_GPS_ALTITUDE(tries);

    // Grab the size (used amount of buffer) and a pointer to data in buffer
    U32 buffsize = static_cast<U32>(serBuffer.getSize());
    char* pointer = reinterpret_cast<char*>(serBuffer.getData());
    this->tlmWrite_GPS_SV_COUNT(buffsize); // DEBUG

    // check for invalid read status, log an error, return buffer & abort if there is problem
    // FPP v.3.0 change here:
    if (serial_status != Drv::SerialReadStatus::SER_OK) {
      Fw::Logger::logMsg("[WARNING] Received buffer with bad packet: %d\n", serial_status.e); 
      // Must return the buffer or serial driver won't be able to reuse it.
      // Same buffer send call from preamble is used; since buffer size was overwritten to
      // hold the actual data size, need to reset it to full size before returning it.
      serBuffer.setSize(UART_READ_BUFF_SIZE);
      this->serialBufferOut_out(0, serBuffer);
      return;
    }
    
    // If not enough data is available for a full message, return the buffer and abort.
    else if (buffsize < 4 ) {
      // Must return the buffer or serial driver won't be able to reuse it.
      // Same buffer send call from preamble is used; since buffer size was overwritten to
      // hold the actual data size, need to reset it to full size before returning it.
      //Fw::Logger::logMsg("[WARNING] Unfull message buffer: %d\n", buffsize); // DJW debug
      serBuffer.setSize(UART_READ_BUFF_SIZE);
      this->serialBufferOut_out(0,serBuffer);
      this->tlmWrite_GPS_SV_COUNT(buffsize); // DEBUG
      return;
    }
    
    // Step 2: parsing
    // Parse GPS message from UART. Use standard C functions to read messages into
    // GPS package struct. If all 9 items parse, break. Else, continue to scan the 
    // block looking for messages further in
    for (U32 i = 0; i < (buffsize); i++) {
      status = sscanf(pointer, "$GPGGA,%f,%f,%c,%f,%c,%u,%u,%f,%f,M,%f,M,%f,%s",
      &packet.utcTime, &packet.dmNS, &packet.northSouth,
      &packet.dmEW, &packet.eastWest, &packet.lock,
      &packet.count, &packet.HDOP,&packet.altitude, 
      &packet.heightWgs84, &packet.dGpsupdate, dgpsStation);
      // this->tlmWrite_GPS_ALTITUDE(status); // DEBUG
      // break when all GPS items are found
      if (status == 9) {
        break;
      }
      pointer = pointer +1;
      this->tlmWrite_GPS_LATITUDE(status); // DEBUG
    }

    // If failed to find GPGGA then return buffer and abort
    if (status ==0) {
      // Must return the buffer or serial driver won't be able to reuse it.
      // Same buffer send call from preamble is used; since buffer size was overwritten to
      // hold the actual data size, need to reset it to full size before returning it.
      //Fw::Logger::logMsg("[ERROR] did not find GNGGA status: %d\n", status); // DJW debug
      serBuffer.setSize(UART_READ_BUFF_SIZE);
      this->serialBufferOut_out(0,serBuffer);
      return;
    }
    // If found an incomplete message log error, return buffer, and abort
    else if (status != 9) {
      //Fw::Logger::logMsg("[ERROR] GPS parsing incomplete status: %d\n", status); // DJW debug
      // Must return the buffer or serial driver won't be able to reuse it.
      // Same buffer send call from preamble is used; since buffer size was overwritten to
      // hold the actual data size, need to reset it to full size before returning it.
      serBuffer.setSize(UART_READ_BUFF_SIZE);
      this->serialBufferOut_out(0,serBuffer);
      this->tlmWrite_GPS_LONGITUDE(status); // DEBUG
      return;
    }

    // GPS packet locations are of format ddmm.mmmm
    // Convert to lat/lon in decimal degrees
    // Latitude degrees, and minutes converted to degrees & multiply by direction
    lat = (U32)(packet.dmNS/100.0f);
    lat = lat + (packet.dmNS - (lat * 100.0f))/60.0f;
    lat = lat * ((packet.northSouth == 'N') ? 1 : -1);

    // Longitude degrees, and minutes converted to degrees & multiply by direction
    lon = (U32)(packet.dmEW/100.0f);
    lon = lon + (packet.dmEW - (lon * 100.0f))/60.0f;
    lon = lon * ((packet.eastWest == 'E') ? 1 : -1);

    // Step 4: generate telemetry
    this->tlmWrite_GPS_LATITUDE(lat);
    this->tlmWrite_GPS_LONGITUDE(lon);
    this->tlmWrite_GPS_ALTITUDE(packet.altitude);
    this->tlmWrite_GPS_SV_COUNT(packet.count);
    this->tlmWrite_GPS_LOCK_STATUS(packet.lock);

    // Step 5: EVR on lock status
    // Only generate lock status event on change
    if (packet.lock == 0 && m_locked) {
      m_locked = false;
      log_WARNING_HI_GPS_LOCK_LOST();
    } else if (packet.lock == 1 && !m_locked) {
      m_locked = true;
      log_ACTIVITY_HI_GPS_LOCK_ACQUIRED();
    }

    // Must return the buffer or serial driver won't be able to reuse it.
    // Same buffer send call from preamble is used; since buffer size was overwritten to
    // hold the actual data size, need to reset it to full size before returning it.
    serBuffer.setSize(UART_READ_BUFF_SIZE);
    this->serialBufferOut_out(0,serBuffer);
  }

  // ----------------------------------------------------------------------
  // Command handler implementations
  // ----------------------------------------------------------------------

  void Gps ::
    REPORT_STATUS_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    // EVR on lock status
    if (m_locked) {
      log_ACTIVITY_HI_GPS_LOCK_ACQUIRED();
    } else {
      log_WARNING_HI_GPS_LOCK_LOST();
    }
    // Step 9: complete command
    this->cmdResponse_out(opCode,cmdSeq,Fw::CmdResponse::OK);
  }

} // end namespace GpsApp
