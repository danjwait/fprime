// ======================================================================
// \title  Gps.cpp
// \author cfeldewerth
// \brief  cpp file for Gps component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================

#include <ctype.h>
#include <GpsApp/Gps/Gps.hpp>
#include "Fw/Types/BasicTypes.hpp"
#include "Fw/Logger/Logger.hpp"

namespace GpsApp {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  Gps ::
    Gps(
        const char *const compName
    ) : GpsComponentBase(compName)
  {
    // initialize the lock to "false" on construction
    m_locked = false;
  }

  void Gps ::
    init(
        const NATIVE_INT_TYPE queueDepth,
        const NATIVE_INT_TYPE instance
    )
  {
    GpsComponentBase::init(queueDepth, instance);
  }

  // The linux serial driver keeps its storage externally;
  // this code will loop through our member variables holding buffers 
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
  }

  Gps ::
    ~Gps()
  {

  }

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  void Gps ::
    serialRecv_handler(
        const NATIVE_INT_TYPE portNum,
        Fw::Buffer &serBuffer,
        Drv::SerialReadStatus &status
    )
  {
    // Local variable definitions
    // starting values for lat & lon
    float lat = 0.0f, lon = 0.0f;

    // pointer to $GPGGA sentence in string
    char *gpgga;
    // search status for $GPGGA, set to false
    int statusGga = 0;
    // pointer to $GPGSA sentence in string
    char *gpgsa;
    // search status for $GPGSA, set to false
    int statusGsa = 0;
    // pointer to $GPRMC sentence in string
    char *gprmc;
    // search status for $GPRMC, set to false
    int statusRmc = 0;
    // pointer to $GPVTGsentence in string
    char *gpvtg;
    // search status for $GPVTG, set to false
    int statusVtg = 0;

    // store parsed GNSS data
    GpsPacket packet;

    // Grab the size (used amount of buffer) and a pointer to data in buffer
    U32 buffsize = static_cast<U32>(serBuffer.getSize());
    char* pointer = reinterpret_cast<char*>(serBuffer.getData());

    // Check for invalid read status, log an error, return buffer & abort if there is problem
    // FPP v.3.0 change here:
    if (status != Drv::SerialReadStatus::SER_OK) {
      Fw::Logger::logMsg("[WARNING] Received buffer with bad packet: %d\n", status.e); 
      // Must return the buffer or serial driver won't be able to reuse it.
      // Same buffer send call from preamble is used; since buffer size was overwritten to
      // hold the actual data size, need to reset it to full size before returning it.
      serBuffer.setSize(UART_READ_BUFF_SIZE);
      this->serialBufferOut_out(0, serBuffer);
      return;
    }

    // Good read; pull data from buffer and parse sentence string fragment; 
    // if it is not printable, set character to '*'
    char uMsg[serBuffer.getSize()+1];
    char* bPtr = reinterpret_cast<char*>(serBuffer.getData());
    for (NATIVE_UINT_TYPE byte = 0; byte < serBuffer.getSize(); byte++) {
        uMsg[byte] = isprint(bPtr[byte])?bPtr[byte]:'*';
    }
    // chop off end characters from this sentence string fragment
    uMsg[sizeof(uMsg)-1] = 0;
    // if there is no previous sentence string fragment, start one
    if (strlen(this->m_holder) == 0) {
      strcpy(this->m_holder,uMsg);
    }
    else {
      // concatenate this sentence string fragment with previous sentence string
      strcat(this->m_holder,uMsg);
    }

    // When the sentence string is long enough, go looking for data
    // 700 characters is start of one $GPGGA to end of another $GPGGA
    // on my Adafruit Feather GPS Ultimate, so should be sure to get one GPGGA
    // with 512 characters (seems to work well). 
    if (strlen(this->m_holder) > 512) {

      // GPGGA: Global Positioning System Fix Data
      // Find the sentence data starting with $GPGGA
      gpgga = strstr(this->m_holder,"$GPGGA");
      // parse out the data from the sentence string after GPGGA
      statusGga = sscanf(gpgga + 7,"%f,%f,%c,%f,%c,%u,%u,%f,%f,M,%f,M,%f,%*c",
      &packet.utcTime, &packet.dmNS, &packet.northSouth,
      &packet.dmEW, &packet.eastWest, &packet.lock,
      &packet.count, &packet.HDOP,&packet.altitude, 
      &packet.heightWgs84, &packet.dGpsupdate);

      // GPGSA: GPS DOP and active satellites
      // Find the sentence data starting with $GPGSA
      gpgsa = strstr(this->m_holder,"$GPGSA");
      // parse out the data from the sentence string after GPGSA
      statusGsa = sscanf(gpgsa + 7,"%*s,%f,%f,%f[^*]%*c",
      &packet.PDOP,&packet.HDOP,&packet.VDOP);

      // GPRMC: Recommended minimum specific GPS/Transit data
      // Find the sentence data starting with $GPRMC
      gprmc = strstr(this->m_holder,"$GPRMC");
      // parse out the data from the sentence string after GPRMC
      statusRmc = sscanf(gprmc + 7,"%f,%c,%f,%c,%f,%c,%f,%u,%f,%c,%c",
      &packet.utcTime, &packet.posStatus,
      &packet.dmNS, &packet.northSouth,
      &packet.dmEW, &packet.eastWest,
      &packet.trackTrue, &packet.date,
      &packet.magVar, &packet.magVarDir, &packet.mode);

      // GPVTG: Track Made Good and Ground Speed
      // Find the sentence data starting with $GPVTG
      gpvtg = strstr(this->m_holder,"$GPVTG");
      // parse out the data from the sentence string after GPVTG
      statusVtg = sscanf(gpvtg + 7,"%f,T,%f,M,%f,N,%f,K,%c,%*c",
      &packet.trackTrue, &packet.trackMag,
      &packet.speedKnots, &packet.speedKmHr, &packet.mode);
      
      // once parsed, empty out the sentence string holder
      memset(this->m_holder,0,UART_READ_BUFF_SIZE);
    }

    // If failed to find GPGGA then return buffer and abort
    if (statusGga == 0) {
      // Must return the buffer or serial driver won't be able to reuse it.
      // Same buffer send call from preamble is used; since buffer size was overwritten to
      // hold the actual data size, need to reset it to full size before returning it.
      serBuffer.setSize(UART_READ_BUFF_SIZE);
      this->serialBufferOut_out(0,serBuffer);
      return;
    }
    // If found an incomplete message log error, return buffer, and abort
    else if (statusGga != 11) {
      // Must return the buffer or serial driver won't be able to reuse it.
      // Same buffer send call from preamble is used; since buffer size was overwritten to
      // hold the actual data size, need to reset it to full size before returning it.
      serBuffer.setSize(UART_READ_BUFF_SIZE);
      this->serialBufferOut_out(0,serBuffer);
      return;
    }

    // Parse the data from the sentence into telemetry
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

    // Generate telemetry
    this->tlmWrite_LATITUDE(lat);
    this->tlmWrite_LONGITUDE(lon);
    this->tlmWrite_ALTITUDE(packet.altitude);
    this->tlmWrite_VEL_KM_SEC(packet.speedKmHr/3600);
    this->tlmWrite_TRACK_TRUE_DEG(packet.trackTrue);
    this->tlmWrite_TRACK_MAG_DEG(packet.trackMag);
    this->tlmWrite_MAG_VAR_DEG(packet.magVar);
    this->tlmWrite_PDOP(packet.PDOP);
    this->tlmWrite_SV_COUNT(packet.count);
    this->tlmWrite_LOCK_STATUS(packet.lock);

    // Send EVR on change in lock status
    // Only generate lock status event on change
    if (packet.lock == 0 && m_locked) {
      m_locked = false;
      log_WARNING_HI_GPS_LOCK_LOST();
    } else if (packet.lock != 0 && !m_locked) {
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
    // Generate EVR of present lock status
    if (m_locked) {
      log_ACTIVITY_HI_GPS_LOCK_ACQUIRED();
    } else {
      log_WARNING_HI_GPS_LOCK_LOST();
    }
    // complete command
    this->cmdResponse_out(opCode,cmdSeq,Fw::CmdResponse::OK);
  }

  void Gps ::
    COLD_START_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    // create command to force cold start
    char commandString[] = "$PMTK104*37\r\n";

    // send the command out over serial
    Fw::Buffer cmdBuffer;
    cmdBuffer.setSize(strlen(commandString));
    cmdBuffer.setData(reinterpret_cast<U8*>(commandString));
    this->serialWrite_out(0, cmdBuffer);

    // complete command
    this->cmdResponse_out(opCode,cmdSeq,Fw::CmdResponse::OK);
  }

} // end namespace GpsApp
