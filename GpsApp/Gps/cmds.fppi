#-----
# commands
#-----

@ force an EVR reporting lock status
async command REPORT_STATUS \
opcode 0

@ force cold start on reboot
async command COLD_START \
opcode 1