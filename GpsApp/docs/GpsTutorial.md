# F' GPS Tutorial

## Introduction

This tutorial is intended to help you extend your F' development capabilities from the [MathComponent](https://github.com/nasa/fprime/tree/devel/docs/Tutorials/MathComponent) and [CrossCompilation](https://github.com/nasa/fprime/tree/devel/docs/Tutorials/CrossCompilation) and serve as some background for the [RPI demo](https://github.com/nasa/fprime/tree/devel/RPI). 

Specifically this tutorial will employ some of the provided F' components to use the data bus on an embedded target to connect another device to the target in order to start building a complete embedded system. In this case we will use a GPS device, connected over UART, to a Raspberry Pi running F' on a Linux-based OS. This will be following the Application-Manager-Driver pattern described in the [F' Users Guide](https://nasa.github.io/fprime/UsersGuide/best/app-man-drv.html) with our Gps component being the GPS device "Manager" in that pattern.

This tutorial will also cover some aspects of the topology development. This process with include things like working with the `Main.cpp` file to build a TopologyState to include the GPS device, configuring components in the `instances.fpp` file, and using `include <file>.fppi` approach to break up some of the files into smaller more focused parts. In order this tutorial will walk through:
 - Setting up the directory structure and topology files
 - Developing a Gps component

Before diving in we should point out what this tutorial *is not* so that you understand the other tasks that you will need to work as you develop your own embedded system using the F' architecture and framework. This tutorial is not:
 - A GPS or GNSS tutorial; we will not cover how to use a GNSS/GPS device as part of your system. In particular we will not develop navigation or timing functions from the GPS device.
 - A systems architecture tutorial; we will not cover how to develop a set of requirements on what your system will need to do, allocate those functions to components, and then laying out those components into a topology with ports and types. 
 - A coding style or software system development guide. This tutorial has been developed by the community and as such does not follow the JPL styles used in the JPL-developed tutorials. This tutorial will also not address development techniques like configuration management, file naming/location, or unit test practices.

We call out the above because these are tasks you really should be doing with your team as you develop your system. At best we'll include notes along the lines of the above when we get to things like "name this per your style guide."

**Prerequisites:** This tutorial assumes the you have executed and understood the: 
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

## Setup the Application Directory Structure

**Create the GpsApp, Top, and Gps directories:**
Go to the `fprime` directory at the top of the repository branch and run `mkdir GpsApp` to create the directory for the GpsApp deployment (this is following the pattern with the `/Ref` application, with `/GpsApp` in place of `/Ref`). Change into that directory (`cd GpsApp`) and then create the directory for the topology (`mkdir Top`) and the Gps component (`mkdir Gps`). The `Top` directory will contain the topology information for the application "GpsApp" and the `GpsApp/Gps` directory will contain the files for your GPS device.

Within the topology we can and will refer to other components in directories outside the `GpsApp` directory, as with the previous tutorials. This would be a good discussion with your team, as to how you want to structure the directories for multi-purpose components vs application-specific directories.

## Create the top-level CMakeLists.txt
In the `/GpsApp` directory (not in `/GpsApp/Top` or `/GpsApp/Gps`) create a file `CMakeLists.txt` with the following content:
```
####
# 'GpsApp' Deployment:
#
# This sets up the build for the 'GpsApp' Application, including the custom reference
# components. In addition, it imports FPrime.cmake, which includes the core F Prime
# components.
#
# This file has several sections.
#
# 1. Header Section: define basic properties of the build
# 2. F prime core: includes all F prime core components, and build-system properties
# 3. Local subdirectories: contains all deployment specific directory additions
####

##
# Section 1: Basic Project Setup
#
# This contains the basic project information. Specifically, a cmake version and
# project definition.
##
cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0048 NEW)
project(GpsApp VERSION 1.0.0 LANGUAGES C CXX)

##
# Section 2: F prime Core
#
# This includes all of the F prime core components, and imports the make-system. F prime core
# components will be placed in the F-Prime binary subdirectory to keep them from
# colliding with deployment specific items.
##
include("${CMAKE_CURRENT_LIST_DIR}/../cmake/FPrime.cmake")
# NOTE: register custom targets between these two lines
include("${CMAKE_CURRENT_LIST_DIR}/../cmake/FPrime-Code.cmake")
##
# Section 3: Components and Topology
#
# This section includes deployment specific directories. This allows use of non-
# core components in the topology, which is also added here.
##
# Add component subdirectories
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../Ref/PingReceiver/")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../Ref/RecvBuffApp/")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../Ref/SendBuffApp/")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Gps")

# Add Topology subdirectory
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Top/")

set(SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/Top/Main.cpp")
set(MOD_DEPS ${PROJECT_NAME}/Top)

register_fprime_deployment()
# The following compile options will only apply to the deployment executable.
# The extra warnings trigger in core F Prime so we don't apply them there.
target_compile_options("${PROJECT_NAME}" PUBLIC -Wall)
target_compile_options("${PROJECT_NAME}" PUBLIC -Wextra)
target_compile_options("${PROJECT_NAME}" PUBLIC -Werror)
#target_compile_options("${PROJECT_NAME}" PUBLIC -Wshadow)
target_compile_options("${PROJECT_NAME}" PUBLIC -Wconversion)
target_compile_options("${PROJECT_NAME}" PUBLIC -Wsign-conversion)
target_compile_options("${PROJECT_NAME}" PUBLIC -Wformat-security)
target_compile_options("${PROJECT_NAME}" PUBLIC -Wnon-virtual-dtor)
target_compile_options("${PROJECT_NAME}" PUBLIC -Wold-style-cast)
target_compile_options("${PROJECT_NAME}" PUBLIC -Woverloaded-virtual)
target_compile_options("${PROJECT_NAME}" PUBLIC -Wno-unused-parameter)
target_compile_options("${PROJECT_NAME}" PUBLIC -Wundef)
set_property(TARGET "${PROJECT_NAME}" PROPERTY CXX_STANDARD 11)
```
Note that the top-level `CMakeLists.txt` covers the name of the application (`project(GpsApp VERSION 1.0.0 LANGUAGES C CXX)`), the import of the F' core, and then inlcudes the path to any componet subdirectorys. In this case we'll include `add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../Ref/PingReceiver/")` among others from within the `/Ref` direcotry, as well as the `/GpsApp/Gps` component we will develop. The top-level `CMakeLists.txt` also sets the path to the topology and the `Main.cpp` file.

## The GpsApp Topology
This tutorial will walk through some more steps involved in working with the application topology within the `/GpsApp/Top` directory. The steps are:
 - Define and create the FPP models for the topology (instances.fpp and topology.fpp)
 - Create the Main.cpp file
 - Create the TopologyDefs.hpp and TopologyDefs.cpp files
 - Create the `CMakeList.txt` file for the topology

### Construct the Topology FPP Model
**Create the FPP model files:**
In the `GpsApp/Top` directory create a set of files:
 - `instances.fpp` : this will define the list of [component instances](https://fprime-community.github.io/fpp/fpp-users-guide.html#Defining-Component-Instances) and their names in the application
 - `topology.fpp` : this will define the [relationships between the component](https://fprime-community.github.io/fpp/fpp-users-guide.html#Defining-Topologies) 
 - `GpsAppTopologyDefs.hpp` and `GpsAppTopologyDefs.cpp` : these contain definitions used through the topology
 - `Main.cpp` : this will setup and run the application on the target
 - `CMakeLists.txt` : this is the CMake instructions for the topology

**Complete the instances.fpp file:**
Open the `instances.fpp` file and fill in the following content:
```
module GpsApp {

  # ----------------------------------------------------------------------
  # Defaults
  # ----------------------------------------------------------------------

  module Default {

    constant queueSize = 10

    constant stackSize = 64 * 1024 

  }

  # ----------------------------------------------------------------------
  # Active component instances
  # ----------------------------------------------------------------------

  instance rateGroup1Comp: Svc.ActiveRateGroup base id 0x0200 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 99 \
  {

    phase Fpp.ToCpp.Phases.configObjects """
    NATIVE_UINT_TYPE context[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    """

    phase Fpp.ToCpp.Phases.instances """
    Svc::ActiveRateGroup rateGroup1Comp(
        FW_OPTIONAL_NAME("rateGroup1Comp"),
        ConfigObjects::rateGroup1Comp::context,
        FW_NUM_ARRAY_ELEMENTS(ConfigObjects::rateGroup1Comp::context)
    );
    """

  }

  instance rateGroup2Comp: Svc.ActiveRateGroup base id 0x0300 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 99 \
  {

    phase Fpp.ToCpp.Phases.configObjects """
    NATIVE_UINT_TYPE context[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    """

    phase Fpp.ToCpp.Phases.instances """
    Svc::ActiveRateGroup rateGroup2Comp(
        FW_OPTIONAL_NAME("rateGroup2Comp"),
        ConfigObjects::rateGroup2Comp::context,
        FW_NUM_ARRAY_ELEMENTS(ConfigObjects::rateGroup2Comp::context)
    );
    """

  }

  instance rateGroup3Comp: Svc.ActiveRateGroup base id 0x0400 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 99 \
  {

    phase Fpp.ToCpp.Phases.configObjects """
    NATIVE_UINT_TYPE context[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    """

    phase Fpp.ToCpp.Phases.instances """
    Svc::ActiveRateGroup rateGroup3Comp(
        FW_OPTIONAL_NAME("rateGroup3Comp"),
        ConfigObjects::rateGroup3Comp::context,
        FW_NUM_ARRAY_ELEMENTS(ConfigObjects::rateGroup3Comp::context)
    );
    """

  }

  instance cmdDisp: Svc.CommandDispatcher base id 0x0500 \
    queue size 20 \
    stack size Default.stackSize \
    priority 30

  instance cmdSeq: Svc.CmdSequencer base id 0x0600 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 30 \
  {

    phase Fpp.ToCpp.Phases.configConstants """
    enum {
      BUFFER_SIZE = 5*1024,
      TIMEOUT = 30
    };
    """

    phase Fpp.ToCpp.Phases.configComponents """
    cmdSeq.allocateBuffer(
        0,
        Allocation::mallocator,
        ConfigConstants::cmdSeq::BUFFER_SIZE
    );
    """

    phase Fpp.ToCpp.Phases.tearDownComponents """
    cmdSeq.deallocateBuffer(Allocation::mallocator);
    """

  }

  instance fileDownlink: Svc.FileDownlink base id 0x0700 \
    queue size 30 \
    stack size Default.stackSize \
    priority 90 \
  {

    phase Fpp.ToCpp.Phases.configConstants """
    enum {
      TIMEOUT = 1000,
      COOLDOWN = 1000,
      CYCLE_TIME = 1000,
      FILE_QUEUE_DEPTH = 10
    };
    """

    phase Fpp.ToCpp.Phases.configComponents """
    fileDownlink.configure(
        ConfigConstants::fileDownlink::TIMEOUT,
        ConfigConstants::fileDownlink::COOLDOWN,
        ConfigConstants::fileDownlink::CYCLE_TIME,
        ConfigConstants::fileDownlink::FILE_QUEUE_DEPTH
    );
    """

  }

  instance fileManager: Svc.FileManager base id 0x0800 \
    queue size 30 \
    stack size Default.stackSize \
    priority 90

  instance fileUplink: Svc.FileUplink base id 0x0900 \
    queue size 30 \
    stack size Default.stackSize \
    priority 90

  instance pingRcvr: Ref.PingReceiver base id 0x0A00 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 90

  instance eventLogger: Svc.ActiveLogger base id 0x0B00 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 98

  instance chanTlm: Svc.TlmChan base id 0x0C00 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 97

  instance prmDb: Svc.PrmDb base id 0x0D00 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 96 \
  {

    phase Fpp.ToCpp.Phases.instances """
    Svc::PrmDb prmDb(FW_OPTIONAL_NAME("prmDb"), "PrmDb.dat");
    """

    phase Fpp.ToCpp.Phases.readParameters """
    prmDb.readParamFile();
    """

  }

  instance GPS: GpsApp.Gps base id 0x0F00 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 99

  # ----------------------------------------------------------------------
  # Queued component instances
  # ----------------------------------------------------------------------

  instance $health: Svc.Health base id 0x2000 \
    queue size 25 \
  {

    phase Fpp.ToCpp.Phases.configConstants """
    enum {
      WATCHDOG_CODE = 0x123
    };
    """

    phase Fpp.ToCpp.Phases.configComponents """
    health.setPingEntries(
        ConfigObjects::health::pingEntries,
        FW_NUM_ARRAY_ELEMENTS(ConfigObjects::health::pingEntries),
        ConfigConstants::health::WATCHDOG_CODE
    );
    """

  }

  instance sendBuffComp: Ref.SendBuff base id 0x2600 \
    queue size Default.queueSize

  instance mathReceiver: Ref.MathReceiver base id 0x2700 \
    queue size Default.queueSize

  # ----------------------------------------------------------------------
  # Passive component instances
  # ----------------------------------------------------------------------

  @ Communications driver. May be swapped with other comm drivers like UART
  @ Note: Here we have TCP reliable uplink and UDP (low latency) downlink
  instance comm: Drv.ByteStreamDriverModel base id 0x4000 \
    at "../../Drv/TcpClient/TcpClient.hpp" \
  {

    phase Fpp.ToCpp.Phases.instances """
    Drv::TcpClient comm(FW_OPTIONAL_NAME("comm"));
    """

    phase Fpp.ToCpp.Phases.configConstants """
    enum {
      PRIORITY = 99,
      STACK_SIZE = Default::stackSize
    };
    """

    phase Fpp.ToCpp.Phases.startTasks """
    // Initialize socket server if and only if there is a valid specification
    if (state.hostName != nullptr && state.portNumber != 0) {
        Os::TaskString name("ReceiveTask");
        // Uplink is configured for receive so a socket task is started
        comm.configure(state.hostName, state.portNumber);
        comm.startSocketTask(
            name,
            true,
            ConfigConstants::comm::PRIORITY,
            ConfigConstants::comm::STACK_SIZE
        );
    }
    """

    phase Fpp.ToCpp.Phases.freeThreads """
    comm.stopSocketTask();
    (void) comm.joinSocketTask(nullptr);
    """

  }

  instance downlink: Svc.Framer base id 0x4100 {

    phase Fpp.ToCpp.Phases.configObjects """
    Svc::FprimeFraming framing;
    """

    phase Fpp.ToCpp.Phases.configComponents """
    downlink.setup(ConfigObjects::downlink::framing);
    """

  }

  instance fatalAdapter: Svc.AssertFatalAdapter base id 0x4200

  instance fatalHandler: Svc.FatalHandler base id 0x4300

  instance fileUplinkBufferManager: Svc.BufferManager base id 0x4400 {

    phase Fpp.ToCpp.Phases.configConstants """
    enum {
      STORE_SIZE = 3000,
      QUEUE_SIZE = 30,
      MGR_ID = 200
    };
    """

    phase Fpp.ToCpp.Phases.configComponents """
    Svc::BufferManager::BufferBins upBuffMgrBins;
    memset(&upBuffMgrBins, 0, sizeof(upBuffMgrBins));
    {
      using namespace ConfigConstants::fileUplinkBufferManager;
      upBuffMgrBins.bins[0].bufferSize = STORE_SIZE;
      upBuffMgrBins.bins[0].numBuffers = QUEUE_SIZE;
      fileUplinkBufferManager.setup(
          MGR_ID,
          0,
          Allocation::mallocator,
          upBuffMgrBins
      );
    }
    """

    phase Fpp.ToCpp.Phases.tearDownComponents """
    fileUplinkBufferManager.cleanup();
    """

  }

  instance linuxTime: Svc.Time base id 0x4500 \
    at "../../Svc/LinuxTime/LinuxTime.hpp" \
  {

    phase Fpp.ToCpp.Phases.instances """
    Svc::LinuxTime linuxTime(FW_OPTIONAL_NAME("linuxTime"));
    """

  }

  instance linuxTimer: Svc.LinuxTimer base id 0x1600 \
  {

    phase Fpp.ToCpp.Phases.instances """
    //Svc::LinuxTimer linuxTimer(FW_OPTIONAL_NAME("linuxTimer"));
    """

    phase Fpp.ToCpp.Phases.stopTasks """
    linuxTimer.quit();
    """

  }

  instance rateGroupDriverComp: Svc.RateGroupDriver base id 0x4600 {

    phase Fpp.ToCpp.Phases.configObjects """
    NATIVE_INT_TYPE rgDivs[Svc::RateGroupDriver::DIVIDER_SIZE] = { 1, 2, 4 };
    """

    phase Fpp.ToCpp.Phases.instances """
    Svc::RateGroupDriver rateGroupDriverComp(
        FW_OPTIONAL_NAME("rateGroupDriverComp"),
        ConfigObjects::rateGroupDriverComp::rgDivs,
        FW_NUM_ARRAY_ELEMENTS(ConfigObjects::rateGroupDriverComp::rgDivs)
    );
    """

  }

  instance recvBuffComp: Ref.RecvBuff base id 0x4700

  instance staticMemory: Svc.StaticMemory base id 0x4800

  instance textLogger: Svc.PassiveTextLogger base id 0x4900

  instance uplink: Svc.Deframer base id 0x4A00 {

    phase Fpp.ToCpp.Phases.configObjects """
    Svc::FprimeDeframing deframing;
    """

    phase Fpp.ToCpp.Phases.configComponents """
    uplink.setup(ConfigObjects::uplink::deframing);
    """

  }

  instance systemResources: Svc.SystemResources base id 0x4B00

  instance GPS_SERIAL: Drv.LinuxSerialDriver base id 0x4C00 \
  {
    phase Fpp.ToCpp.Phases.configComponents  """
    {
    const bool status = GPS_SERIAL.open(
      state.device,
      Drv::LinuxSerialDriverComponentImpl::BAUD_9600,
      Drv::LinuxSerialDriverComponentImpl::NO_FLOW,
      Drv::LinuxSerialDriverComponentImpl::PARITY_NONE,
      true
    );
    if (!status) {
      Fw::Logger::logMsg("[ERROR] Could not open GPS UART: %s\\n", reinterpret_cast<POINTER_CAST>(state.device));
      Init::status = false;
      }
    else {
      Fw::Logger::logMsg("[INFO] Opened GPS UART driver: %s\\n", reinterpret_cast<POINTER_CAST>(state.device));
      Init::status = true;
    }
    }
    """
    
    phase Fpp.ToCpp.Phases.startTasks """
    if (Init::status) {
      GPS_SERIAL.startReadThread();
    }
    else {
      Fw::Logger::logMsg("[ERROR] Initialization failed; not starting GPS UART driver\\n");
    }
    """

    phase Fpp.ToCpp.Phases.stopTasks """
    GPS_SERIAL.quitReadThread();
    """
  }

}
```

TODO - discussion

**Complete the topology.fpp file:**
Open the `topology.fpp` file and fill in the following content:
```
module GpsApp {

  # ----------------------------------------------------------------------
  # Symbolic constants for port numbers
  # ----------------------------------------------------------------------

    enum Ports_RateGroups {
      rateGroup1
      rateGroup2
      rateGroup3
    }

    enum Ports_StaticMemory {
      downlink
      uplink
    }

  topology GpsApp {

    # ----------------------------------------------------------------------
    # Instances used in the topology
    # ----------------------------------------------------------------------

    instance $health
    instance chanTlm
    instance cmdDisp
    instance cmdSeq
    instance comm
    instance downlink
    instance eventLogger
    instance fatalAdapter
    instance fatalHandler
    instance fileDownlink
    instance fileManager
    instance fileUplink
    instance fileUplinkBufferManager
    instance linuxTime
    instance linuxTimer
    instance GPS_SERIAL
    instance GPS
    instance pingRcvr
    instance prmDb
    instance rateGroup1Comp
    instance rateGroup2Comp
    instance rateGroup3Comp
    instance rateGroupDriverComp
    instance recvBuffComp
    instance sendBuffComp
    instance staticMemory
    instance systemResources
    instance textLogger
    instance uplink
    
    # ----------------------------------------------------------------------
    # Pattern graph specifiers
    # ----------------------------------------------------------------------

    command connections instance cmdDisp

    event connections instance eventLogger

    param connections instance prmDb

    telemetry connections instance chanTlm

    text event connections instance textLogger

    time connections instance linuxTime

    health connections instance $health

    # ----------------------------------------------------------------------
    # Direct graph specifiers
    # ----------------------------------------------------------------------

    connections Downlink {

      chanTlm.PktSend -> downlink.comIn
      eventLogger.PktSend -> downlink.comIn
      fileDownlink.bufferSendOut -> downlink.bufferIn

      downlink.framedAllocate -> staticMemory.bufferAllocate[Ports_StaticMemory.downlink]
      downlink.framedOut -> comm.send
      downlink.bufferDeallocate -> fileDownlink.bufferReturn

      comm.deallocate -> staticMemory.bufferDeallocate[Ports_StaticMemory.downlink]

    }

    connections FaultProtection {
      eventLogger.FatalAnnounce -> fatalHandler.FatalReceive
    }

    connections RateGroups {

      # Timer
      linuxTimer.CycleOut -> rateGroupDriverComp.CycleIn

      # Rate group 1
      rateGroupDriverComp.CycleOut[Ports_RateGroups.rateGroup1] -> rateGroup1Comp.CycleIn
      rateGroup1Comp.RateGroupMemberOut[0] -> chanTlm.Run
      rateGroup1Comp.RateGroupMemberOut[1] -> fileDownlink.Run
      rateGroup1Comp.RateGroupMemberOut[2] -> systemResources.run
      rateGroup1Comp.RateGroupMemberOut[3] -> uplink.schedIn

      # Rate group 2
      rateGroupDriverComp.CycleOut[Ports_RateGroups.rateGroup2] -> rateGroup2Comp.CycleIn
      rateGroup2Comp.RateGroupMemberOut[0] -> cmdSeq.schedIn
      rateGroup2Comp.RateGroupMemberOut[1] -> sendBuffComp.SchedIn

      # Rate group 3
      rateGroupDriverComp.CycleOut[Ports_RateGroups.rateGroup3] -> rateGroup3Comp.CycleIn
      rateGroup3Comp.RateGroupMemberOut[0] -> $health.Run
      rateGroup3Comp.RateGroupMemberOut[1] -> fileUplinkBufferManager.schedIn

    }

    connections Sequencer {
      cmdSeq.comCmdOut -> cmdDisp.seqCmdBuff
      cmdDisp.seqCmdStatus -> cmdSeq.cmdResponseIn
    }

    connections Uplink {

      comm.allocate -> staticMemory.bufferAllocate[Ports_StaticMemory.uplink]
      comm.$recv -> uplink.framedIn
      uplink.framedDeallocate -> staticMemory.bufferDeallocate[Ports_StaticMemory.uplink]

      uplink.comOut -> cmdDisp.seqCmdBuff
      cmdDisp.seqCmdStatus -> uplink.cmdResponseIn

      uplink.bufferAllocate -> fileUplinkBufferManager.bufferGetCallee
      uplink.bufferOut -> fileUplink.bufferSendIn
      uplink.bufferDeallocate -> fileUplinkBufferManager.bufferSendIn
      fileUplink.bufferSendOut -> fileUplinkBufferManager.bufferSendIn

    }

    connections Gps {
      GPS_SERIAL.serialRecv -> GPS.serialRecv
      GPS.serialBufferOut -> GPS_SERIAL.readBufferSend
      GPS.serialWrite -> GPS_SERIAL.serialSend
    }

  }

}
```

TODO - discussion

**Complete the GpsAppTopologyDefs.hpp file:**
Open the `GpsAppTopologyDefs.hpp` file and fill in the following content:
```
#ifndef GpsAppTopologyDefs_HPP
#define GpsAppTopologyDefs_HPP

#include "Fw/Types/MallocAllocator.hpp"
#include "Fw/Logger/Logger.hpp"
#include "GpsApp/Top/FppConstantsAc.hpp"
#include "Svc/FramingProtocol/FprimeProtocol.hpp"
#include "Svc/LinuxTimer/LinuxTimer.hpp"

namespace GpsApp {

  // Declare the Linux timer here so it is visible in main
  extern Svc::LinuxTimer linuxTimer;

  namespace Allocation {

    // Malloc allocator for topology construction
    extern Fw::MallocAllocator mallocator;

  }

  namespace Init {

    // Initialization status
    extern bool status;

  }

  // State for topology construction
  struct TopologyState {
    TopologyState() :
      hostName(""),
      portNumber(0),
      device("")
    {

    }
    TopologyState(
        const char *hostName,
        U32 portNumber,
        const char *device
    ) :
      hostName(hostName),
      portNumber(portNumber),
      device(device)
    {

    }
    const char* hostName;
    U32 portNumber;
    const char *device;
  };

  // Health ping entries
  namespace PingEntries {
    namespace chanTlm { enum { WARN = 3, FATAL = 5 }; }
    namespace cmdDisp { enum { WARN = 3, FATAL = 5 }; }
    namespace cmdSeq { enum { WARN = 3, FATAL = 5 }; }
    namespace eventLogger { enum { WARN = 3, FATAL = 5 }; }
    namespace fileDownlink { enum { WARN = 3, FATAL = 5 }; }
    namespace fileManager { enum { WARN = 3, FATAL = 5 }; }
    namespace fileUplink { enum { WARN = 3, FATAL = 5 }; }
    namespace pingRcvr { enum { WARN = 3, FATAL = 5 }; }
    namespace prmDb { enum { WARN = 3, FATAL = 5 }; }
    namespace rateGroup1Comp { enum { WARN = 3, FATAL = 5 }; }
    namespace rateGroup2Comp { enum { WARN = 3, FATAL = 5 }; }
    namespace rateGroup3Comp { enum { WARN = 3, FATAL = 5 }; }
    namespace GPS { enum { WARN = 3, FATAL = 5 }; }
  }

}

#endif
```

TODO - discussion

**Complete the GpsAppTopologyDefs.cpp file:**
Open the `GpsAppTopologyDefs.cpp` file and fill in the following content:
```
#include "GpsApp/Top/GpsAppTopologyDefs.hpp"

namespace GpsApp {

  namespace Allocation {

    Fw::MallocAllocator mallocator;

  }

    namespace Init {

    bool status = true;

  }

  Svc::LinuxTimer linuxTimer(FW_OPTIONAL_NAME("linuxTimer"));

}
```

TODO - discussion

**Complete the Main.cpp file:**
Open the `Main.cpp` file and fill in the following content:
```
#include <getopt.h>
#include <cstdlib>
#include <ctype.h>

#include <Os/Log.hpp>
#include <GpsApp/Top/GpsAppTopologyAc.hpp>

void print_usage(const char* app) {
    (void) printf("Usage: ./%s [options]\n-p\tport_number\n-a\thostname/IP address\n",app);
}

#include <signal.h>
#include <cstdio>

GpsApp::TopologyState state;
// Enable the console logging provided by Os::Log
Os::Log logger;

volatile sig_atomic_t terminate = 0;

// Handle a signal, e.g. control-C
static void sighandler(int signum) {
    // Call the teardown function
    // This causes the Linux timer to quit
    GpsApp::teardown(state);
    terminate = 1;
}

void run1cycle() {
    // call interrupt to emulate a clock
    GpsApp::blockDrv.callIsr();
    Os::Task::delay(1000); //10Hz
}

void runcycles(NATIVE_INT_TYPE cycles) {
    if (cycles == -1) {
        while (true) {
            run1cycle();
        }
    }

    for (NATIVE_INT_TYPE cycle = 0; cycle < cycles; cycle++) {
        run1cycle();
    }
}

int main(int argc, char* argv[]) {
    U32 port_number = 0; // Invalid port number forced
    I32 option;
    char *hostname;
    char *device;
    option = 0;
    hostname = nullptr;
    device = nullptr;

    while ((option = getopt(argc, argv, "hp:a:d:")) != -1){
        switch(option) {
            case 'h':
                print_usage(argv[0]);
                return 0;
                break;
            case 'p':
                port_number = static_cast<U32>(atoi(optarg));
                break;
            case 'a':
                hostname = optarg;
                break;
            case 'd':
                device = optarg;
                break;
            case '?':
                return 1;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // console prompt for quit
    (void) printf("Hit Ctrl-C to quit\n");

    // run setup
    state = GpsApp::TopologyState(hostname, port_number,device);
    GpsApp::setup(state);

    // register signal handlers to exit program
    signal(SIGINT,sighandler);
    signal(SIGTERM,sighandler);

    // Start the Linux timer.
    // The timer runs on the main thread until it quits
    // in the teardown function, called from the signal
    // handler.
    GpsApp::linuxTimer.startTimer(1000); //!< 10Hz

    // Signal handler was called, and linuxTimer quit.
    // Time to exit the program.
    // Give time for threads to exit.
    (void) printf("Waiting for threads...\n");
    Os::Task::delay(1000);

    (void) printf("Exiting...\n");

    return 0;
}
```

TODO - discussion

**Complete the CMakeLists.txt file:**
Open the `CMakeLists.txt` file and fill in the following content:
```
####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
####

set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/instances.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/topology.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/GpsAppTopologyDefs.cpp"
)
set(MOD_DEPS
  Fw/Logger
  Svc/LinuxTime
  # Communication Implementations
  Drv/Udp
  Drv/TcpClient
)

register_fprime_module()
```

TODO - discussion

## The Gps Component
Unlike the [MathComponent Tutorial](https://github.com/nasa/fprime/tree/devel/docs/Tutorials/MathComponent) we can jump directly to creating the `Gps` component, since this component will only make use of existing ports and types. The steps are:

-  Construct the FPP model
-  Add the model to the project
-  Build the stub implementation
 - Complete the implementation

### Construct the Gps Component FPP Model

**Create the FPP model files:**
In the `GpsApp/Gps` directory create a set of files:
 - `Gps.fpp` : this will define the ports for the Gps component
 - `CMakeLists.txt` : this will define the files for the CMake process
 - `cmds.fppi` : we will include this within `Gps.fpp` per the FPP User’s Guide section on [Include Specifiers](https://fprime-community.github.io/fpp/fpp-users-guide.html#Defining-Components_Include-Specifiers) and define the commands executed by the Gps component here
 - `events.fppi` : also included in `Gps.fpp`, in this case to define event messaged generated by the Gps component
 - `tlm.fppi` : also included in `Gps.fpp`, in this case to define telemetry generated by the Gps component
 - `param.fppi` : also included in `Gps.fpp`, this file will be blank for this component

Per the FPP Users Guide, using include files will help us break up and structure our work. The approach here breaks up the commands, events, telemetry, and parameters as separate files for the Gps component, and (if we were all on one team) we could make this our policy to always make each component have a definition (in our case `Gps.fpp`) with separate files for the inputs and outputs for that component in the same directory. *Please discuss how your team will break up work* we'll use this approach here, but adopt an approach that works for your team.

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
```
This is an active component per the [F' Users Guide](https://nasa.github.io/fprime/UsersGuide/user/port-comp-top.html#components-f-modules).  

The `include "<file>.fppi"` works like any other include statement. We will fill these files in next.

This component only makes use of the special ports typed by existing provided F' components, so these are all listed under `special ports`. Note that the names for the ports used here (e.g. `serialBufferOut`) will show up in the autogenerated stub files for us to fill in; talk with your team on how you want to name ports. We would capture user-defined ports for application-specific functions under `general ports`.

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
This is the complete list of commands our Gps component will execute. The `REPORT_STATUS` command will cause the Gps component to emit the lock status event message defined in the `events.fppi` file below. This will help us determine if our component is able to receive and execute commands. 

The `COLD_START` command will be relayed from the Gps component across the UART interface to the GPS device to force the GPS to "forget" the stored ephemeris data. We are using this as an example command here because it is easy to see execute; the GPS device will drop lock on the GPS satellites as it deletes all stored satellite data and goes through the process to reacquire them. This particular command will be specific to the GPS device; if you are using a different device, read the datasheet for the device to find the version of this command (or similar) for the device you are using. 

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
This is the complete list of event messages that will be output by the Gps component and routed out the communication link. These events are intended to allow the operator insight into the status of the GPS device. The `REPORT_STATUS` command in the `cmds.fppi` file above will force a generation of one of these event messages, pending the status of the GPS device lock.

Note we will make use of log messages that will print to the user console; those will not be captured in the `events.fppi` file.

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
This is the complete list of telemetry parameters that will be output by the Gps component and routed out the communication link. We will need to write code in the Gps component to parse these telemetry values from the raw data sent from the GPS device over the UART. This set of telemetry will be the as-reported by the GPS device values, as processed by the Gps component. 

Note that we are not including the GPS time or date relayed by the device; how to work with that data would be something you should work with your architecture.

We will leave the `params.fppi` file blank for this tutorial.

Open the `CMakeLists.txt` file and add the following contents:
```
# Register the standard build
set(SOURCE_FILES
	"${CMAKE_CURRENT_LIST_DIR}/Gps.fpp"
)
register_fprime_module()
```
## Creating the GpsApp Deployment
