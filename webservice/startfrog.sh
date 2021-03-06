#! /bin/bash -x
if [ `hostname` == "applejack" ]; then
    FROGPATH="/scratch2/www/lamachine"
    LOGFILE="/scratch2/www/webservices-lst/live/writable/tscan/frog.log"
    THREADS=8
elif [ `hostname` == "mlp01" ]; then
    FROGPATH="/var/www/lamachine"
    LOGFILE="/var/www/webservices-lst/live/writable/logs/tscan.frog.err"
    THREADS=8
else
    FROGPATH="/usr/local/"
    LOGFILE="/tmp/frog-tscan.log"
    THREADS=0 #all we can get
fi
PORT=7001
ID=tscan
CONFIG=${FROGPATH}/share/frog/nld/tscan-frog.cfg

mv $LOGFILE $LOGFILE.sav # poor mans solution

frog -X --id=${ID} -c ${CONFIG} --skip=mp -S${PORT} 2> $LOGFILE
