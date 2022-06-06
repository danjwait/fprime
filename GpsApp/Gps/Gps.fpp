module GpsApp {

    @ Component for working with a GPS device
    active component Gps {

        include "cmds.fppi"
        include "events.fppi"
        include "tlm.fppi"
        include "param.fppi"

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

        @ command response port
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

        @ receive serial data port to read data from UART
        async input port serialRecv: Drv.SerialRead

        @ serial buffer port for device to write into over UART
        output port serialBufferOut: Fw.BufferSend
    }
}