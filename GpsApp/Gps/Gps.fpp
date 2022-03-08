module GpsApp {

    @ Component for reading GPS strings from GPS hardware
    active component Gps {

        #-----
        # general ports
        #-----

        #-----
        # special ports
        #-----

        @ command receive port
        command recv port cmdIn

        @ command registration port
        command reg port cmdRegOut

        @ command reponse port
        command resp port cmdResponseOut

        @ event port
        event port eventOut

        @ text event port
        text event port textEventOut

        @ telemetry port
        telemetry port tlmOut

        @ receive serial data port
        async input port serialRecv: Drv.SerialRead

        @ serial buffer port
        output port serialBufferOut: Fw.BufferSend

        #-----
        # commands
        #-----

        @ report lock status
        async command REPORT_STATUS(
            
        )
    }



}