#include "GpsApp/Top/GpsAppTopologyDefs.hpp"

namespace GpsApp {

  namespace Allocation {

    Fw::MallocAllocator mallocator;

  }

  Drv::BlockDriver blockDrv(FW_OPTIONAL_NAME("blockDrv"));

}
