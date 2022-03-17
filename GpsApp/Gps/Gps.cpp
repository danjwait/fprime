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
    ) : GpsComponentBase(compName)
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
  // system runs at steady-stte. This allows for initialization code 
  // whihc invokes working ports

  void Gps :: preamble()
  {
    for (NATIVE_INT_TYPE buffer =0; buffer < NUM_UART_BUFFERS; buffer ++) {
      // Assign the raw data to the buffer. Make sure to include the side
      // of the region assigned.
      this->m_recvBuffers[buffer].setData((U64)this->m_uartBuffers[buffer]);
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

  // Step 1: serialIn
  //
  // Impliment a handler to respond to the serial device sending data buffer
  // containing the GPS data. This will handle the serial message. 

  void Gps ::
    serialRecv_handler(
        const NATIVE_INT_TYPE portNum,
        Fw::Buffer &serBuffer, 
        Drv::SerialReadStatus &status // DJW this reference is differnt in tutorial
    )
  {
    // local variable definitions
    int status = 0;
    float lat = 0.0f, lon = 0.0f;
    GpsPacket packet;
    // Grab the sice (used amount of buffer) and a pointer to data in buffer
    U32 buffsize = static_cast<U32>(serBuffer.getSize());
    char* pointer = reinterpret_cast<char*>(serBuffer.getData());

    // check for invalid read status, log an error, return buffer & abort if there is problem
    if (serial_status != Drv::SER_OK) {
      Fw::Logger::logMsg("[WARNING] Received buffer with bad packet: %d\n", serial_status);
      // Must return the buffer or serial driver won't be able to reuse it.
      // Same buffer send call from preamble is used; since buffer size was overwritten to
      // hold the actual data size, need to reset it to full size before returning it.
      serBuffer.setSize(UART_READ_BUFF_SIZE);
      this->serialBufferOut_out(0, serBuffer);
      return;
    }
    // If not enough data is available for a full message, return the buffer and abort.
    else if (buffsize < 24 ) {
      // Must return the buffer or serial driver won't be able to reuse it.
      // Same buffer send call from preamble is used; since buffer size was overwritten to
      // hold the actual data size, need to reset it to full size before returning it.
      serBuffer.setSize(UART_READ_BUFF_SIZE);
      this->serialBufferOut_out(0,serBuffer);
      return;
    }

    // DJW stopped here
    // Step 2: parsing
    
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
    // TODO
    this->cmdResponse_out(opCode,cmdSeq,Fw::CmdResponse::OK);
  }

} // end namespace GpsApp
