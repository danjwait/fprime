module Ref {
    @ component for receiving and performing a math operation
    queued component MathReceiver {

        #-----
        # General ports
        #-----

        @ port for receiving the math operation
        async input port mathOpIn: MathOp

        @ port for retuening the math result
        output port mathResultOut: MathResult

        @ the rate group scheduler input
        sync input port schedIn: Svc.Sched

        #-----
        # Special ports
        #-----

        @ command receive
        command recv port cmdIn

        @ command registration
        command reg port cmdRegOut

        @ command response
        command resp port cmdResponseOut

        @ event
        event port eventOut

        @ parameter get
        param get port prmGetOut

        @ parameter set
        param set port prmSetOut

        @ telemetry
        telemetry port tlmOut

        @ text event
        text event port textEventOut

        @ time get
        time get port timeGetOut

        #-----
        # parameters
        #-----

        @ the multiplier in the math operation
        param FACTOR: F32 default 1.0 id 0 \
        set opcode 10 \
        save opcode 11

        #-----
        # events
        #-----

        @ factor updated
        event FACTOR_UPDATED (
            val: F32 @< the factor value
        ) \
        severity activity high \
        id 0 \
        format "Factor updated to {f}" \
        throttle 3

        @ math operation performed
        event OPERATION_PERFORMED(
            val: MathOp @< the operation
        ) \
        severity activity high \
        id 1 \
        format "{} operation performed"

        @ event throttle cleared
        event THROTTLE_CLEARED \
        severity activity high \
        id 2 \
        format "Event throttle cleared"

        #-----
        # commands 
        #-----

        @ clear event throttle
        async command CLEAR_EVENT_THROTTLE \
        opcode 0

        #-----
        # telemetry
        #-----

        @ the operation
        telemetry OPERATION: MathOp id 0

        @ multiplication factor
        telemetry FACTOR: F32 id 1
    }
}