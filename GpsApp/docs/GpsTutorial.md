# F' GPS Tutorial

## Table of Contents

This tutororial is intended to help you extend your F' development capabilties from the [MathComponent](https://github.com/nasa/fprime/tree/devel/docs/Tutorials/MathComponent) and [CrossCompilation](https://github.com/nasa/fprime/tree/devel/docs/Tutorials/CrossCompilation) and serve as some background for the [RPI demo](https://github.com/nasa/fprime/tree/devel/RPI). 

Specifically this tutorial will employ some of the provided F' components to use the data bus on an embedded target in order to connect another device to the target in order to start building a complete embedded system. In this case we will use a GPS device, connected over UART, to a Raspberry Pi running F' on a Linux-based OS. This will be follwing the Application-Manager-Driver pattern described in the [F' Users Guide](https://nasa.github.io/fprime/UsersGuide/best/app-man-drv.html) with our Gps component being the GPS device "Manager" in that pattern.

This tutorial will also cover some aspects of the Topology development, including working with the `Main.cpp` file to include the GPS device and using `include <file>.fppi` approach to break up some of the files into smaller more focused parts. 

Before diving in we should point out what this tutorial *is not* so that you understand the other tasks that you will need to work as you develop your own embedded system using the F' architecture and framework. This tutorial is not:
 - A GPS or GNSS tutorial; we will not cover how to use a GNSS/GPS device as part of your system. In particular we will not develop navigation or timing functions from the GPS device
 - A systems architecture tutorial; we will not cover how to develop a set of requirements on what your system will need to do, allocate those functions to components, and then laying out those components into a topology with ports and types. 
 - A coding style or software system development guide. This tutorial has been developed by the community and as such does not follow the JPL styles used in the JPL-developed tutorials. This tutorial will also not address development techniques like configuration management, file naming/location, or unit test practices.

We call out the above because these are tasks you really should be doing with your team as you develop your system. At best we'll include notes along the lines of the above when we get to things like "name this per your sytle guide."

**Prerequisites:** This tutorial assumes the you have exctuted and understood the: 
 - [Getting Started Tutorial](../GettingStarted/Tutorial.md)
 - [MathComponent Tutorial](../MathComponent/Tutorial.md)
 - [CrossCompilation Tutorial](https://github.com/nasa/fprime/tree/devel/docs/Tutorials/CrossCompilation).

As such, this tutorial builds on the prerequisites in those tutorials. This tutorial will make extensive use of the [FPP Users Guide](https://fprime-community.github.io/fpp/fpp-users-guide.html) as well, so please read through that and refer back to it as we go.

We have written this guide making use of a [Raspberry Pi 4](https://www.raspberrypi.com/products/raspberry-pi-4-model-b/) as the embedded target and the [Adafruit Ultimate GPS FeatherWing](https://www.adafruit.com/product/3133) as the connected device. Due to this being written during COVID times, we have not been able to procure let alone test alternate hardware sets. We do want to point out that both Raspberry Pi and Adafruit teams provide extensive documentation and support, so please consider supporting them as you work to learn embedded systems.

**F´ Version:** This tutorial is designed to work with release `v3.0.0`.

Working on this tutorial will modify some files under version control in the F' git repository. Therefore it is a good idea to do this work on a new branch. For example:

```bash
git checkout -b GpsApp v3.0.0
```
If you wish, you can save your work by committing to this branch.

<a name="Gps-Component"></name>
<a name="The-Gps-Component"></a>
## The Gps Component

Unlike the [MathComponent Tutorial](https://github.com/nasa/fprime/tree/devel/docs/Tutorials/MathComponent) We can jump directly to creating the `Gps` component, since this component will only make use of existing ports and types. The steps are:

-  Construct the FPP model
-  Add the model to the project
-  Build the stub implementation
 - Complete the implementation

### Construct the FPP Model

**Create the GpsApp and Gps directories:**
Go to the `fprime` directory at the top of the repository branch and run `mkdir GpsApp` to create the directory for the GpsApp deployment. Change into that directory (`cd GpsApp`) and then create the directory for the Gps component (`mkdir Gps`) and then change into that direcory. This directory will contain the files for your GPS device.

**Create the FPP model files:**
In the `GpsApp/Gps` directory create a set of files:
 - `Gps.fpp` : this will define the ports for the Gps component
 - `cmds.fppi` : we will include this in `Gps.fpp` per the FPP User’s Guide section on [Include Specifiers](https://fprime-community.github.io/fpp/fpp-users-guide.html#Defining-Components_Include-Specifiers) and define the commands executed by the Gps component here
 - `events.fppi` : also included in `Gps.fpp`, in this case to define event messaged generated by the Gps component
 - `tlm.fppi` : also included in `Gps.fpp`, in this case to define telemetry generated by the Gps component
 - `param.fppi` : also included in `Gps.fpp`, this file will be blank for this component

Per the FPP Users Guide, using include files will help us break up and structure our work. The appproach here breaks up the commands, events, telemetry, and parameters as seprate files for the Gps component, and (if we were all on one team) we could make this our policy to always make each component have a definition (in our case `Gps.fpp`) with separate files for the inputs and outputs for that component in the same directory. *Please discuss how your team will break up work* we'll use this approach here, but adopt an approach that works for your team.

Open the `Gps.fpp` file and add the following contents:
```
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

        @ receive serial data port to read data from UART
        async input port serialRecv: Drv.SerialRead

        @ serial buffer port for device to write into over UART
        output port serialBufferOut: Fw.BufferSend

        #-----
        # special ports
        #-----

    }
}
```
This is an active component per the [F' Users Guide](https://nasa.github.io/fprime/UsersGuide/user/port-comp-top.html#components-f-modules).  

The `include "<file>.fppi"` works like any other include statement. We will fill these files in next.

This component only makes use of ports typed by existing porvided F' components, so these are all listed under `general ports`. Note that the names for the ports used here (e.g. `serialBufferOut`) will show up in the autogenerated stub files for us to fill in; talk with your team on how you want to name ports. 

In our hypothetical team, we would capture custom ports under `special ports`.

Open the `cmds.fppi` file and add the following contents:
```
#-----
# commands
#-----

@ force an EVR reporting lock status
async command REPORT_STATUS \
opcode 0

@ force cold start on reboot
async command COLD_START \
opcode 1
```
This is the complete list of commands our Gps component will execute. Of note, the `COLD_START` command will be relayed from the Gps component across the UART interface to the GPS device to force the GPS to "forget" the stored ephemeris data. We are using this as an example command here because it is easy to see execute; the GPS device will drop lock on the GPS satellites as it deletes all stored satellite data and goes through the process to reacquire them. This particular command will be sepcific to the GPS device; if you are using a different device, read the datasheet for the device to find the version of this command (or similar) for the device you are using. 

Open the `events.fppi` file and add the following contents:
```
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
```
This is the complete list of event messages that will be output by the Gps component and routed out the communication link. Note we will make use of log messages that will print to the user console; those will not be captured in the `events.fppi` file.

Open the `tlm.fppi` file and add the following contents:
```
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
telemetry VEL_KM_SEC: F32 id 5

@ current true ground track
telemetry TRACK_TRUE_DEG: F32 id 6

@ current magnetic heading
telemetry TRACK_MAG_DEG: F32 id 7

@ current magnetic variation
telemetry MAG_VAR_DEG: F32 id 8

@ current dilution of precision
telemetry PDOP: F32 id 9
```
This is the complete list of telemetry parameters that will be output by the Gps component and routed out the communication link. We will need to write code in the Gps component to parse these telemetry values from the raw data sent from the GPS device over the UART.

We will leave the `params.fppi` file blank for this tutorial.

## Creating the GpsApp Deployment
