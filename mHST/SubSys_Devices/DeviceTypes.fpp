module Devices {

  @ Set of Device modes
  enum DevideMode {
      OFF <@ device is off
      STBY <@ device is in standby mode; on, not operating
      NOMINAL <@ device is operating nominally
      FAULT <@ device is on and faulted; cannot be operated
  }

}