module GpsApp {

    @ Component for reading GPS strings from GPS hardware
    active component Gps {

        #-----
        # general ports
        #-----

        #@ the rate group scheduler input
        #sync input port schedIn: Svc.Sched
        
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

        @ Time get port
        time get port timeGetOut

        @ telemetry port
        telemetry port tlmOut

        @ receive serial data port
        async input port serialRecv: Drv.SerialRead

        @ serial buffer port
        output port serialBufferOut: Fw.BufferSend

        #-----
        # parameters
        #-----

        #-----
        # events
        #-----

        @ notification on GPS lock acquired
        event GPS_LOCK_ACQUIRED \
        severity activity high \
        id 0 \
        format "GPS lock acquired"

        @ warning on GPS lock lost
        event GPS_LOCK_LOST \
        severity warning high \
        id 1 \
        format "GPS lock lost"

        #-----
        # commands
        #-----

        @ command to force an EVR reporting lock status
        async command REPORT_STATUS \
        opcode 0

        #-----
        # telemetry
        #-----

        @ current latitude
        telemetry GPS_LATITUDE: F32 id 0

        @ current longitude
        telemetry GPS_LONGITUDE: F32 id 1

        @ current altitude
        telemetry GPS_ALTITUDE: F32 id 2

        @ current number of satellites
        telemetry GPS_SV_COUNT: F32 id 3

        @ current lock status
        telemetry GPS_LOCK_STATUS: U32 id 4
    }
}