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

        @ telemetry port
        telemetry port tlmOut

        @ text event port
        text event port textEventOut

        @ time get port
        time get port timeGetOut

        @ receive serial data port
        serial read port serialRecv

        @ serial buffer port
        serial buffer port serialBufferOut

        #-----
        # commands
        #-----

        @ report lock status
        async command REPORT_STATUS(
            
        )
    }



}