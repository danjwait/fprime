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