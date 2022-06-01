module Sys {

  @ Set of system modes
  enum SysMode {
    OFF @< System completely powered off
    CONFIG @< System setup for configutation
    SLEEP @< System waiting for wake timer
    SUN_TRACK @< System moving to point arrays at sun
    IMG_OPS @< System performing imaging operations
    COMM @< System performing comm operations
  }

}