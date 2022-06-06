#-----
# events
#-----

@ notification on GPS lock acquired
event GPS_LOCK_ACQUIRED \
severity activity high \
id 0 \
format "GPS lock acquired"

@ warning on GPS lock lost
event GPS_LOCK_LOST \
severity warning high \
id 1 \
format "GPS lock lost"