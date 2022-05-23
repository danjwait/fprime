#include "GpsApp/Top/GpsAppTopologyDefs.hpp"

namespace GpsApp {

  namespace Allocation {

    Fw::MallocAllocator mallocator;

  }

    namespace Init {

    bool status = true;

  }

  Drv::BlockDriver blockDrv(FW_OPTIONAL_NAME("blockDrv"));

  // this is here in RPI demo; 
  // seems like it should also work in instances.fpp but it doesn't
  //Svc::LinuxTimer linuxTimer(FW_OPTIONAL_NAME("linuxTimer"));

}
