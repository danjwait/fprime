// ======================================================================
// \title  Gps.hpp
// \author djwait
// \brief  hpp file for Gps component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================

#ifndef Gps_HPP
#define Gps_HPP

//#include "GpsApp/Gps/Gps.hpp"
#include "GpsApp/Gps/GpsComponentAc.hpp"

// Define memory footprint of buffers
// Define a count of buffers & size of each.
// Allow Gps component to manage its own buffers

#define NUM_UART_BUFFERS 20
#define UART_READ_BUFF_SIZE 1024

namespace GpsApp {

  class Gps :
   public GpsComponentBase
  {

    /**
     * @brief GpsPacket:
     * A structured containing the information in the GPS location packet
     * received via a NEMA GPS receiver
     */
    struct GpsPacket {
      float utcTime;
      float dmNS;
      char northSouth;
      float dmEW;
      char eastWest;
      unsigned int lock;
      unsigned int count;
      float filler;
      float altitude;
    };

    public:

      // ----------------------------------------------------------------------
      // Construction, initialization, and destruction
      // ----------------------------------------------------------------------

      //! Construct object Gps
      //!
      Gps(
#if FW_OBJECT_NAMES == 1
          const char *const compName /*!< The component name*/
#else
          void
#endif
      );

      //! Initialize object Gps
      //!
      void init(
          const NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
          const NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
      );

      //! Preamble
      //!
      void preamble();

      //! Destroy object Gps
      //!
      ~Gps();

    PRIVATE:

      // ----------------------------------------------------------------------
      // Handler implementations for user-defined typed input ports
      // ----------------------------------------------------------------------

      //! Handler implementation for serialRecv
      //!
      void serialRecv_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          Fw::Buffer &serBuffer, /*!< 
      Buffer containing data
      */
          Drv::SerialReadStatus &status /*!< 
      Status of read
      */
      );

    PRIVATE:

      // ----------------------------------------------------------------------
      // Command handler implementations
      // ----------------------------------------------------------------------

      //! Implementation for REPORT_STATUS command handler
      //! command to force an EVR reporting lock status
      void REPORT_STATUS_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq /*!< The command sequence number*/
      );
      //!< Has deviced acquired GPS lock?
      bool m_locked;
      //!< Create member variables to store buffers and data array
      // that those buffers use for storage
      Fw::Buffer m_recvBuffers[NUM_UART_BUFFERS];
      BYTE m_uartBuffers[NUM_UART_BUFFERS][UART_READ_BUFF_SIZE];
  };

} // end namespace GpsApp

#endif
