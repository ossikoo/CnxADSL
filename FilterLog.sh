#!/bin/bash
# This script is used to filter all logs to only Linux entries

DIRS=". KernelModule DownLoadApp"

for dir in ${DIRS};
do
	gawk -f FilterLog.awk ${dir}/Log.txt >${dir}/LogTemp.txt
	rm -f ${dir}/Log.txt
	mv ${dir}/LogTemp.txt ${dir}/Log.txt
done
