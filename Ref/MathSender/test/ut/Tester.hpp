// ======================================================================
// \title  MathSender/test/ut/Tester.hpp
// \author djwait
// \brief  hpp file for MathSender test harness implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================

#ifndef TESTER_HPP
#define TESTER_HPP

#include "GTestBase.hpp"
#include "Ref/MathSender/MathSender.hpp"

namespace Ref {

  class Tester :
    public MathSenderGTestBase
  {

      // ----------------------------------------------------------------------
      // Construction and destruction
      // ----------------------------------------------------------------------

    public:

      //! Construct object Tester
      //!
      Tester();

      //! Destroy object Tester
      //!
      ~Tester();

    public:

      // ----------------------------------------------------------------------
      // Tests
      // ----------------------------------------------------------------------

      //! Test an ADD command
      void testAddCommand();

      //! Test an SUB command
      void testSubCommand();

      //! Test an MUL command
      void testMulCommand();

      //! Test an DIV command
      void testDivCommand();

    private:

      // ----------------------------------------------------------------------
      // Handlers for typed from ports
      // ----------------------------------------------------------------------

      //! Handler for from_mathOpOut
      //!
      void from_mathOpOut_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          F32 val1, /*!< 
      the first operand
      */
          const Ref::MathOp &op, /*!< 
      the operation
      */
          F32 val2 /*!< 
      the second operand
      */
      );

    private:

      // ----------------------------------------------------------------------
      // Helper methods
      // ----------------------------------------------------------------------

      //! Connect ports
      //!
      void connectPorts();

      //! Initialize components
      //!
      void initComponents();

      //! Test a DO_MATH command
      void testDoMath(MathOp op);

    private:

      // ----------------------------------------------------------------------
      // Variables
      // ----------------------------------------------------------------------

      //! The component under test
      //!
      MathSender component;

  };

} // end namespace Ref

#endif
