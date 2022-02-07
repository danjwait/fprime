module Ref {
    @ component for sending a math operation
    active component MathSender {

        #-----
        # General ports
        #-----

        @ port for sending the operation request
        output port mathOpOut: MathOp

        @ port for receiving the operation result
        async input port mathResultIn: MathResult

        #-----
        # Special ports
        #-----

        @ command receive port
        command recv port cmdIn

        @ command registration port
        command reg port cmdRegOut

        @ command reponse port
        command resp port cmdResponseOut

        @ event port
        event port eventOut

        @ telemetry port
        telemetry port tlmOut

        @ text event port
        text event port textEventOut

        @ time get port
        time get port timeGetOut

        #-----
        # commands
        #-----

        @ do math operation
        async command DO_MATH(
            val1: F32 @< the first operand
            op: MathOp @< the math operation
            val2: F32 @< the second operand
        )

        #-----
        # events
        #-----

        @ math command received
        event COMMAND_RECV(
            val1: F32 @< the first operand
            op: MathOp @< the operation
            val2: F32 @< the second operand
        ) \
        severity activity low \
        format "Math command received: {f} {} {f}"

        @ received math result
        event RESULT(
            result: F32 @< the math result
        ) \
        severity activity high \
        format "Math result is {f}"

        #-----
        # telemetry
        #-----

        @ the first value
        telemetry VAL1: F32

        @ the operation
        telemetry OP: MathOp

        @ the second value
        telemetry VAL2: F32

        @ the result
        telemetry RESULT: F32
    }
}