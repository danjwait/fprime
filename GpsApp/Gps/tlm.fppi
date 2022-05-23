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