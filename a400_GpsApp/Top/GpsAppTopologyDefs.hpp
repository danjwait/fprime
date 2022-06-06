#ifndef GpsAppTopologyDefs_HPP
#define GpsAppTopologyDefs_HPP

#include "Drv/BlockDriver/BlockDriver.hpp"
#include "Fw/Types/MallocAllocator.hpp"
#include "Fw/Logger/Logger.hpp"
#include "GpsApp/Top/FppConstantsAc.hpp"
#include "Svc/FramingProtocol/FprimeProtocol.hpp"

namespace GpsApp {

  // Declare the block driver here so it is visible in main
  extern Drv::BlockDriver blockDrv;

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
    namespace blockDrv { enum { WARN = 3, FATAL = 5 }; }
    namespace chanTlm { enum { WARN = 3, FATAL = 5 }; }
    namespace cmdDisp { enum { WARN = 3, FATAL = 5 }; }
    namespace cmdSeq { enum { WARN = 3, FATAL = 5 }; }
    namespace eventLogger { enum { WARN = 3, FATAL = 5 }; }
    namespace fileDownlink { enum { WARN = 3, FATAL = 5 }; }
    namespace fileManager { enum { WARN = 3, FATAL = 5 }; }
    namespace fileUplink { enum { WARN = 3, FATAL = 5 }; }
    namespace GPS { enum { WARN = 3, FATAL = 5 }; }
    namespace prmDb { enum { WARN = 3, FATAL = 5 }; }
    namespace rateGroup1Comp { enum { WARN = 3, FATAL = 5 }; }
    namespace rateGroup2Comp { enum { WARN = 3, FATAL = 5 }; }
    namespace rateGroup3Comp { enum { WARN = 3, FATAL = 5 }; }
  }

}

#endif