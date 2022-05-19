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
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Gps")

# Add Topology subdirectory
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Top/")

set(SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/Top/Main.cpp")
set(MOD_DEPS ${PROJECT_NAME}/Top)

register_fprime_deployment()
```
Note that the top-level `CMakeLists.txt` covers the name of the application (`project(GpsApp VERSION 1.0.0 LANGUAGES C CXX)`), the import of the F' core, and then inlcudes the path to any componet subdirectorys. In this case we'll include `add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../Ref/PingReceiver/")` among others from within the `/Ref` direcotry, as well as the `/GpsApp/Gps` component we will develop. The top-level `CMakeLists.txt` also sets the path to the topology and the `Main.cpp` file (within the `/GpsApp/Top` directory, in this case).

## Create the GpsApp Topology
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
    Svc::LinuxTimer linuxTimer(FW_OPTIONAL_NAME("linuxTimer"));
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
    instance prmDb
    instance rateGroup1Comp
    instance rateGroup2Comp
    instance rateGroup3Comp
    instance rateGroupDriverComp
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
```c++
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
```c++
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
```c++
#include <getopt.h>
#include <cstdlib>
#include <ctype.h>
#include <signal.h>
#include <cstdio>

#include <Os/Log.hpp>
#include <GpsApp/Top/GpsAppTopologyAc.hpp>

void print_usage(const char* app) {
    (void) printf("Usage: ./%s [options]\n-p\tport_number\n-a\thostname/IP address\n",app);
}

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

`Main.cpp` as written takes command-line arguments at start that are used to create the `TopologyState` which is passed to `GpsApp::setup`. Note in this example the serial port used to communicate with the GPS device is passed in, as is the host IP address and port. It may be better for your application to set those values some other way, for example within `GpsAppTopologyDefs.hpp` or `instances.fpp`

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

We will leave the `param.fppi` file blank for this tutorial.

Open the `CMakeLists.txt` file and add the following contents:
```
# Register the standard build
set(SOURCE_FILES
	"${CMAKE_CURRENT_LIST_DIR}/Gps.fpp"
)
register_fprime_module()
```

### Impliment the model files
In `/GpsApp` directory run `fprime-util generate` to generate the build cache files for the native system, and then `fprime-util generate raspberrypi` for the Raspberry Pi. 

Change into the `/GpsApp/Gps` component directory and run the implimentation command `fprime-util impl` to create the stub implimentation of the Gps component. There should be two new files in the `/GpsApp/Gps` directory:
 - `GpsComponentImpl.hpp-template`
 - `GpsComponentImpl.cpp-template`

Rename the files for the components (we'll use these names; please work with your team for your prefered component naming):
```
mv GpsComponentImpl.hpp-template Gps.hpp
mv GpsComponentImpl.cpp-template Gps.cpp
```
The new stub implimentation files should build, so test that before filling out the stubs by running the build command(s):
```
fprime-util build --jobs 8
fprime-util build raspberrypi --jobs 8
```
The first command builds on the host machine OS, the second for the Raspberry Pi. Note the `--jobs` option is how many cores to run on the host, per the note in the [Math Component Tutorial](fprime-util build raspberrypi --jobs 8). 

The stubs should look like this; first the Gps.hpp:
```c++
// ======================================================================
// \title  Gps.hpp
// \author djwait
// \brief  hpp file for Gps component implementation class
// ======================================================================

#ifndef Gps_HPP
#define Gps_HPP

#include "GpsApp/Gps/GpsComponentAc.hpp"

// Define memory footprint of buffers
// Define a count of buffers & size of each.
// Allow Gps component to manage its own buffers
#define NUM_UART_BUFFERS 5 
#define UART_READ_BUFF_SIZE 40 

namespace GpsApp {

  class Gps :
    public GpsComponentBase
  {

    public:

      // ----------------------------------------------------------------------
      // Construction, initialization, and destruction
      // ----------------------------------------------------------------------

      //! Construct object Gps
      //!
      Gps(
          const char *const compName /*!< The component name*/
      );

      //! Initialize object Gps
      //!
      void init(
          const NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
          const NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
      );

      //! Destroy object Gps
      //!
      ~Gps();

    PRIVATE:

      // ----------------------------------------------------------------------
      // Handler implementations for user-defined typed input ports
      // ----------------------------------------------------------------------

      //! Handler implementation for serialRecv
      //!
      void serialRecv_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          Fw::Buffer &serBuffer, /*!< 
      Buffer containing data
      */
          Drv::SerialReadStatus &status /*!< 
      Status of read
      */
      );

    PRIVATE:

      // ----------------------------------------------------------------------
      // Command handler implementations
      // ----------------------------------------------------------------------

      //! Implementation for REPORT_STATUS command handler
      //! force an EVR reporting lock status
      void REPORT_STATUS_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq /*!< The command sequence number*/
      );

      //! Implementation for COLD_START command handler
      //! force cold start on reboot
      void COLD_START_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq /*!< The command sequence number*/
      );


    };

} // end namespace GpsApp

#endif

```
and the Gps.cpp:
```c++
// ======================================================================
// \title  Gps.cpp
// \author djwait
// \brief  cpp file for Gps component implementation class
// ======================================================================


#include <GpsApp/Gps/Gps.hpp>
#include "Fw/Types/BasicTypes.hpp"

namespace GpsApp {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  Gps ::
    Gps(
        const char *const compName
    ) : GpsComponentBase(compName)
  {

  }

  void Gps ::
    init(
        const NATIVE_INT_TYPE queueDepth,
        const NATIVE_INT_TYPE instance
    )
  {
    GpsComponentBase::init(queueDepth, instance);
  }

  Gps ::
    ~Gps()
  {

  }

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  void Gps ::
    serialRecv_handler(
        const NATIVE_INT_TYPE portNum,
        Fw::Buffer &serBuffer,
        Drv::SerialReadStatus &status
    )
  {
    // TODO
  }

  // ----------------------------------------------------------------------
  // Command handler implementations
  // ----------------------------------------------------------------------

  void Gps ::
    REPORT_STATUS_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    // TODO
    this->cmdResponse_out(opCode,cmdSeq,Fw::CmdResponse::OK);
  }

  void Gps ::
    COLD_START_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    // TODO
    this->cmdResponse_out(opCode,cmdSeq,Fw::CmdResponse::OK);
  }

} // end namespace GpsApp
```

### Complete the Gps Implimentation
Now we will have to fill in the code that is stubbed out in the stub files. Look for the `// TODO` comments in the autogenerated files.

Open the `Gps.hpp` file and add the conents for `struct GpsPacket` and `Additional member functions & variables`:
```c++
// ======================================================================
// \title  Gps.hpp
// \author djwait
// \brief  hpp file for Gps component implementation class
// ======================================================================

#ifndef Gps_HPP
#define Gps_HPP

#include "GpsApp/Gps/GpsComponentAc.hpp"

// Define memory footprint of buffers
// Define a count of buffers & size of each.
// Allow Gps component to manage its own buffers
#define NUM_UART_BUFFERS 5 
#define UART_READ_BUFF_SIZE 40 

namespace GpsApp {

  class Gps :
    public GpsComponentBase
  {

    public:

      // ----------------------------------------------------------------------
      // Types
      // ----------------------------------------------------------------------

      /*** @brief GpsPacket:
       * * A structured containing the information in the GPS location packet
       * * received via a NEMA GPS receiver
       * * units are as-read from receiver, before parsing*/
      struct GpsPacket {
        float utcTime; // UTC of position hhmmss.ss
        unsigned int date; // Date: ddmmyy

        float dmNS; // Latitude (DDmm.mm)
        char northSouth; // Latitude direction: (N = North, S = South)
        float dmEW; // Longitude (DDDmm.mm)
        char eastWest; // Longitude direction: (E = East, W = West)
        float speedKnots; //Speed over ground, knots
        float speedKmHr; // Speed, kilometres/hour
        float heightWgs84; // Undulation between the geoid and the WGS84 ellipsoid
        float altitude; // Antenna altitude above/below mean sea level
        float dGpsupdate; // Age of correction data (in seconds)
        char dgpsStation[20];
        float trackTrue; // Track made good, degrees True
        float trackMag; // Track made good, degrees Magnetic
        float magVar; // Magnetic variation, degrees
        char magVarDir; // Mag variation direction E/W; (E) subtracts from True course
        char posStatus; // Position status (A = data valid, V = data invalid)
        unsigned int lock; // GPS quality; 0 = Fix not available or invalid
        unsigned int count; // Number of satellites in use
        float HDOP; // Horizontal dilution of precision, 
        float PDOP; // Position dilution of precision
        float VDOP;// Vertical dilution of precision
        char mode; //Positioning system mode indicator
        float filler;
      };

      // ----------------------------------------------------------------------
      // Construction, initialization, and destruction
      // ----------------------------------------------------------------------

      //! Construct object Gps
      //!
      Gps(
          const char *const compName /*!< The component name*/
      );

      //! Initialize object Gps
      //!
      void init(
          const NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
          const NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
      );

      //! Destroy object Gps
      //!
      ~Gps();

    PRIVATE:

      // ----------------------------------------------------------------------
      // Handler implementations for user-defined typed input ports
      // ----------------------------------------------------------------------

      //! Handler implementation for serialRecv
      //!
      void serialRecv_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          Fw::Buffer &serBuffer, /*!< 
      Buffer containing data
      */
          Drv::SerialReadStatus &status /*!< 
      Status of read
      */
      );

    PRIVATE:

      // ----------------------------------------------------------------------
      // Command handler implementations
      // ----------------------------------------------------------------------

      //! Implementation for REPORT_STATUS command handler
      //! force an EVR reporting lock status
      void REPORT_STATUS_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq /*!< The command sequence number*/
      );

      //! Implementation for COLD_START command handler
      //! force cold start on reboot
      void COLD_START_cmdHandler(
          const FwOpcodeType opCode, /*!< The opcode*/
          const U32 cmdSeq /*!< The command sequence number*/
      );

      // ----------------------------------------------------------------------
      // Additional member functions & variables
      // ----------------------------------------------------------------------

      //! This will be called once when task starts up
      void preamble() override;

      //!< Has deviced acquired GPS lock?
      bool m_locked;

      //!< Create member variables to store buffers and data array
      // that those buffers use for storage
      Fw::Buffer m_recvBuffers[NUM_UART_BUFFERS];
      BYTE m_uartBuffers[NUM_UART_BUFFERS][UART_READ_BUFF_SIZE];
      char m_holder[UART_READ_BUFF_SIZE];

    };

} // end namespace GpsApp

#endif
```
The autocoding sets up the Gps.hpp file; we only needed to add those sections that we will use in the Gps.cpp file.

Open the `Gps.cpp` file and add the following contents where you see the `// TODO` entries:
```c++
// ======================================================================
// \title  Gps.cpp
// \author djwait
// \brief  cpp file for Gps component implementation class
// ======================================================================

#include <cstring>
#include <ctype.h>
#include <GpsApp/Gps/Gps.hpp>
#include "Fw/Types/BasicTypes.hpp"
#include "Fw/Logger/Logger.hpp" 


namespace GpsApp {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  Gps ::
    Gps(
        const char *const compName
    ) : GpsComponentBase(compName),
    // initialize the lock to "false" on construction
    m_locked(false)
  {

  }

  void Gps ::
    init(
        const NATIVE_INT_TYPE queueDepth,
        const NATIVE_INT_TYPE instance
    )
  {
    GpsComponentBase::init(queueDepth, instance);
  }

  // The linux serial driver keeps its storage externally;
  // this code will loop through our member variables holding buffers 
  // and send them to the linux serial driver. 'preamble' is 
  // automatically called after the system is constructed, before the 
  // system runs at steady-state. This allows for initialization code 
  // which invokes working ports
  void Gps :: preamble()
  {
    for (NATIVE_INT_TYPE buffer =0; buffer < NUM_UART_BUFFERS; buffer ++) {
      // Assign the raw data to the buffer. Make sure to include the side
      // of the region assigned.
      this->m_recvBuffers[buffer].setData(this->m_uartBuffers[buffer]);
      this->m_recvBuffers[buffer].setSize(UART_READ_BUFF_SIZE);
      // Invoke the port to send the buffer out
      this->serialBufferOut_out(0,this->m_recvBuffers[buffer]);
    }
  }

  Gps ::
    ~Gps()
  {

  }

  // ----------------------------------------------------------------------
  // Handler implementations for user-defined typed input ports
  // ----------------------------------------------------------------------

  void Gps ::
    serialRecv_handler(
        const NATIVE_INT_TYPE portNum,
        Fw::Buffer &serBuffer,
        Drv::SerialReadStatus &serial_status
    )
  {
    // Local variable definitions
    // starting values for lat & lon
    float lat = 0.0f, lon = 0.0f;

    // pointer to $GPGGA sentence in string
    char *gpgga;
    // search status for $GPGGA, set to false
    int statusGga = 0;
    // pointer to $GPGSA sentence in string
    char *gpgsa;
    // search status for $GPGSA, set to false
    int statusGsa = 0;
    // pointer to $GPRMC sentence in string
    char *gprmc;
    // search status for $GPRMC, set to false
    int statusRmc = 0;
    // pointer to $GPVTGsentence in string
    char *gpvtg;
    // search status for $GPVTG, set to false
    int statusVtg = 0;

    // store parsed GNSS data
    GpsPacket packet;

    // Grab the size (used amount of buffer) and a pointer to data in buffer
    U32 buffsize = static_cast<U32>(serBuffer.getSize());
    char* pointer = reinterpret_cast<char*>(serBuffer.getData());

    // Check for invalid read status, log an error, return buffer & abort if there is problem
    // FPP v.3.0 change here:
    if (serial_status != Drv::SerialReadStatus::SER_OK) {
      Fw::Logger::logMsg("[WARNING] Received buffer with bad packet: %d\n", serial_status.e); 
      // Must return the buffer or serial driver won't be able to reuse it.
      // Same buffer send call from preamble is used; since buffer size was overwritten to
      // hold the actual data size, need to reset it to full size before returning it.
      serBuffer.setSize(UART_READ_BUFF_SIZE);
      this->serialBufferOut_out(0, serBuffer);
      return;
    }

    // Good read; pull data from buffer and parse sentence string fragment; 
    // if it is not printable, set character to '*'
    char uMsg[serBuffer.getSize()+1];
    char* bPtr = reinterpret_cast<char*>(serBuffer.getData());
    for (NATIVE_UINT_TYPE byte = 0; byte < serBuffer.getSize(); byte++) {
        uMsg[byte] = isprint(bPtr[byte])?bPtr[byte]:'*';
    }
    // chop off end characters from this sentence string fragment
    uMsg[sizeof(uMsg)-1] = 0;
    // if there is no previous sentence string fragment, start one
    if (strlen(this->m_holder) == 0) {
      strcpy(this->m_holder,uMsg);
    }
    else {
      // concatenate this sentence string fragment with previous sentence string
      strcat(this->m_holder,uMsg);
    }

    // When the sentence string is long enough, go looking for data
    // 700 characters is start of one $GPGGA to end of another $GPGGA
    // on my Adafruit Feather GPS Ultimate, so should be sure to get one GPGGA
    // with 512 characters (seems to work well). 
    if (strlen(this->m_holder) > 512) {

      // GPGGA: Global Positioning System Fix Data
      // Find the sentence data starting with $GPGGA
      gpgga = strstr(this->m_holder,"$GPGGA");
      // parse out the data from the sentence string after GPGGA
      statusGga = sscanf(gpgga + 7,"%f,%f,%c,%f,%c,%u,%u,%f,%f,M,%f,M,%f,%*c",
      &packet.utcTime, &packet.dmNS, &packet.northSouth,
      &packet.dmEW, &packet.eastWest, &packet.lock,
      &packet.count, &packet.HDOP,&packet.altitude, 
      &packet.heightWgs84, &packet.dGpsupdate);

      // GPGSA: GPS DOP and active satellites
      // Find the sentence data starting with $GPGSA
      gpgsa = strstr(this->m_holder,"$GPGSA");
      // parse out the data from the sentence string after GPGSA
      statusGsa = sscanf(gpgsa + 7,"%*s,%f,%f,%f[^*]%*c",
      &packet.PDOP,&packet.HDOP,&packet.VDOP);

      // GPRMC: Recommended minimum specific GPS/Transit data
      // Find the sentence data starting with $GPRMC
      gprmc = strstr(this->m_holder,"$GPRMC");
      // parse out the data from the sentence string after GPRMC
      statusRmc = sscanf(gprmc + 7,"%f,%c,%f,%c,%f,%c,%f,%u,%f,%c,%c",
      &packet.utcTime, &packet.posStatus,
      &packet.dmNS, &packet.northSouth,
      &packet.dmEW, &packet.eastWest,
      &packet.trackTrue, &packet.date,
      &packet.magVar, &packet.magVarDir, &packet.mode);

      // GPVTG: Track Made Good and Ground Speed
      // Find the sentence data starting with $GPVTG
      gpvtg = strstr(this->m_holder,"$GPVTG");
      // parse out the data from the sentence string after GPVTG
      statusVtg = sscanf(gpvtg + 7,"%f,T,%f,M,%f,N,%f,K,%c,%*c",
      &packet.trackTrue, &packet.trackMag,
      &packet.speedKnots, &packet.speedKmHr, &packet.mode);
      
      // once parsed, empty out the sentence string holder
      memset(this->m_holder,0,UART_READ_BUFF_SIZE);
    }
    
    // If failed to find GPGGA then return buffer and abort
    if (statusGga == 0) {
      // Must return the buffer or serial driver won't be able to reuse it.
      // Same buffer send call from preamble is used; since buffer size was overwritten to
      // hold the actual data size, need to reset it to full size before returning it.
      serBuffer.setSize(UART_READ_BUFF_SIZE);
      this->serialBufferOut_out(0,serBuffer);
      return;
    }
    // If found an incomplete message log error, return buffer, and abort
    else if (statusGga != 11) {
      // Must return the buffer or serial driver won't be able to reuse it.
      // Same buffer send call from preamble is used; since buffer size was overwritten to
      // hold the actual data size, need to reset it to full size before returning it.
      serBuffer.setSize(UART_READ_BUFF_SIZE);
      this->serialBufferOut_out(0,serBuffer);
      return;
    }

    // Parse the data from the sentence into telemetry
    // GPS packet locations are of format ddmm.mmmm
    // Convert to lat/lon in decimal degrees
    // Latitude degrees, and minutes converted to degrees & multiply by direction
    lat = (U32)(packet.dmNS/100.0f);
    lat = lat + (packet.dmNS - (lat * 100.0f))/60.0f;
    lat = lat * ((packet.northSouth == 'N') ? 1 : -1);

    // Longitude degrees, and minutes converted to degrees & multiply by direction
    lon = (U32)(packet.dmEW/100.0f);
    lon = lon + (packet.dmEW - (lon * 100.0f))/60.0f;
    lon = lon * ((packet.eastWest == 'E') ? 1 : -1);

    // Generate telemetry
    this->tlmWrite_LATITUDE(lat);
    this->tlmWrite_LONGITUDE(lon);
    this->tlmWrite_ALTITUDE(packet.altitude);
    this->tlmWrite_VEL_KM_SEC(packet.speedKmHr/3600);
    this->tlmWrite_TRACK_TRUE_DEG(packet.trackTrue);
    this->tlmWrite_TRACK_MAG_DEG(packet.trackMag);
    this->tlmWrite_MAG_VAR_DEG(packet.magVar);
    this->tlmWrite_PDOP(packet.PDOP);
    this->tlmWrite_SV_COUNT(packet.count);
    this->tlmWrite_LOCK_STATUS(packet.lock);

    // Send EVR on change in lock status
    // Only generate lock status event on change
    if (packet.lock == 0 && m_locked) {
      m_locked = false;
      log_WARNING_HI_GPS_LOCK_LOST();
    } else if (packet.lock != 0 && !m_locked) {
      m_locked = true;
      log_ACTIVITY_HI_GPS_LOCK_ACQUIRED();
    }

    // Must return the buffer or serial driver won't be able to reuse it.
    // Same buffer send call from preamble is used; since buffer size was overwritten to
    // hold the actual data size, need to reset it to full size before returning it.
    serBuffer.setSize(UART_READ_BUFF_SIZE);
    this->serialBufferOut_out(0,serBuffer);
  }

  // ----------------------------------------------------------------------
  // Command handler implementations
  // ----------------------------------------------------------------------

  void Gps ::
    REPORT_STATUS_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    // Generate EVR of present lock status
    if (m_locked) {
      log_ACTIVITY_HI_GPS_LOCK_ACQUIRED();
    } else {
      log_WARNING_HI_GPS_LOCK_LOST();
    }
    // complete command
    this->cmdResponse_out(opCode,cmdSeq,Fw::CmdResponse::OK);
  }

  void Gps ::
    COLD_START_cmdHandler(
        const FwOpcodeType opCode,
        const U32 cmdSeq
    )
  {
    // create command to force cold start
    char commandString[] = "$PMTK104*37\r\n";

    // send the command out over serial
    Fw::Buffer cmdBuffer;
    cmdBuffer.setSize(strlen(commandString));
    cmdBuffer.setData(reinterpret_cast<U8*>(commandString));
    this->serialWrite_out(0, cmdBuffer);

    // complete command
    this->cmdResponse_out(opCode,cmdSeq,Fw::CmdResponse::OK);
  }

} // end namespace GpsApp
```
In the above, we added the `void Gps :: preamble()` to setup the buffers to send data back and forth between the Gps component and the serial driver.

The `void Gps :: serialRecv_handler(...)` holds a few things:
 - a way to read in each new set of data coming across the serial driver, adding the new data to the previous data and freeing the used buffers
 - once there is enough data to contain the messages to parse, there is the set of parsers that look for the NEMA strings in the data ([NovAtel ref](https://docs.novatel.com/OEM7/Content/Logs/Core_Logs.htm?tocpath=Commands%20%2526%20Logs%7CLogs%7CGNSS%20Logs%7C_____0)) and put that data into the GpsPacket structure
 - if once the data is parsed, send some of that data out as telemetry

The `void Gps :: REPORT_STATUS_cmdHandler(...)` generates the event message on the lock status; this is useful to check that the component is running, since it won't generate telemetry without sucessful parsing of the serial data.

The `void Gps :: COLD_START_cmdHandler(...)` writes a given string out the serial interface to the GPS device to force the device to run the cold start routine, where it will start looking for GPS spacecraft with no existing knowledge.

With the completed code, re-run the build commands:
```
fprime-util build --jobs 8
fprime-util build raspberrypi --jobs 8
```
The component should build sucessfully

## Creating the GpsApp Deployment
In the `/GpsApp` directory
TODO fpp-check
run `fprime-util build` and `fprime-util build raspberrypi`

## Running the GpsApp Deployment
TODO get to the raspberrypi
TODO start the app `sudo GpsApp `

