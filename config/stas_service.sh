#!/bin/sh

GEARMAN="/usr/local/sbin/gearmand"
STAS="/home/boolpae/Dev/stas/bin/Debug/Linux_x86_64/itf_stas_odbc"
#G_PID="/home/boolpae/Dev/stas/config/gearmand.pid"
G_PID="/home/boolpae/gearmand.pid"
S_PID="/home/boolpae/Dev/stas/config/itf_stas.pid"
G_OPT="--pid-file=${G_PID} -d"
S_OPT="-i /home/boolpae/Dev/stas/config/stas.conf --verbose DEBUG --pid-file=${S_PID} -d"


echo "Gearman Exec Opt"
echo ${GEARMAN} ${G_OPT}

echo "ITF_STAS Exec Opt"
echo ${STAS} ${S_OPT}

if [ -e ${G_PID} ]
then
	GPID=`cat ${G_PID}`
	if ps -p ${GPID} > /dev/null
	then
		echo "GEARMAND(${GPID}) is running"
	else
		echo "GEARMAND(${GPID}) is not running"
	fi
else
	echo 'File not found('${G_PID}')'
fi

if [ -e ${S_PID} ]
then
	SPID=`cat ${S_PID}`
	if ps -p ${SPID} > /dev/null
	then
		echo "STAS(${SPID}) is running"
	else
		echo "STAS(${SPID}) is not running"
	fi
else
	echo 'File not found('${S_PID}')'
fi
