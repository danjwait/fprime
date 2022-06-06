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

  instance blockDrv: Drv.BlockDriver base id 0x0100 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 99

  instance rateGroup1Comp: Svc.ActiveRateGroup base id 0x0200 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 98 \
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
    priority 98 \
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
    priority 98 \
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
    priority 90

  instance cmdSeq: Svc.CmdSequencer base id 0x0600 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 90 \
  {

    phase Fpp.ToCpp.Phases.configConstants """
    enum {
      BUFFER_SIZE = 5*1024
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
    priority 85 \
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
    priority 85

  instance fileUplink: Svc.FileUplink base id 0x0900 \
    queue size 30 \
    stack size Default.stackSize \
    priority 85

  instance eventLogger: Svc.ActiveLogger base id 0x0B00 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 90

  instance chanTlm: Svc.TlmChan base id 0x0C00 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 92

  instance prmDb: Svc.PrmDb base id 0x0D00 \
    queue size Default.queueSize \
    stack size Default.stackSize \
    priority 91 \
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
    priority 80

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
      PRIORITY = 97,
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
      Fw::Logger::logMsg("[ERROR] Initialization failed; not starting UART driver\\n");
    }
    """

    phase Fpp.ToCpp.Phases.stopTasks """
    GPS_SERIAL.quitReadThread();
    """
  }

}