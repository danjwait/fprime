#include "GpsApp/Top/GpsAppTopologyDefs.hpp"

namespace GpsApp {

    namespace Allocation {
  
      Fw::MallocAllocator mallocator;
  
    }

    namespace Init {

        bool status = true;
    
    }
  
    Drv::BlockDriver blockDrv(FW_OPTIONAL_NAME("blockDrv"));

  }