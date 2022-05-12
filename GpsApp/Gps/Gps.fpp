module GpsApp {

    @ Component for reading GPS strings from GPS hardware
    active component Gps {

        include "cmdTypes.fppi"

        #-----
        # general ports
        #-----
        #@ The ping input port
        #async input port PingIn: Svc.Ping

        #@ The ping input port
        #output port PingOut: Svc.Ping

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

        @ time get port
        time get port timeGetOut

        @ telemetry port
        telemetry port tlmOut

        @ output port for writing commands over UART to device
        output port serialWrite: Drv.SerialWrite

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

        @ command to change GPS baud rate
        async command SET_BAUD_RATE (
            baudRate: BaudRate @< the baud rate
        ) \
        opcode 1

        #-----
        # telemetry
        #-----

        @ current latitude
        telemetry LATITUDE: F32 id 0

        @ current longitude
        telemetry LONGITUDE: F32 id 1

        @ current altitude
        telemetry ALTITUDE: F32 id 2

        @ current number of satellites
        telemetry SV_COUNT: U32 id 3

        @ current lock status
        telemetry LOCK_STATUS: U32 id 4

        @ current GPS-relative velocity
        telemetry VELO_KM_SEC: F32 id 5

        @ current true ground track
        telemetry TRACK_TRUE_DEG: F32 id 6

        @ current magnetic heading
        telemetry TRACK_MAG_DEG: F32 id 7

        @ current magnetic variation
        telemetry MAG_VAR_DEG: F32 id 8

        @ current dilution of precision
        telemetry PDOP: F32 id 9

    }
}