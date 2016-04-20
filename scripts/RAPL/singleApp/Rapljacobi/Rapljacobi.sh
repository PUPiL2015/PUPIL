	
mkdir -p results

export HEARTBEAT_ENABLED_DIR=heartenabled/
export OMP_NUM_THREADS=128

rm -Rf ${HEARTBEAT_ENABLED_DIR}

SET_SPEED=/home/hankhoffmann/tools/powerQoS/pySetCPUSpeed.py
POWER_MON=/home/hankhoffmann/tools/powerQoS/pyWattsup-hank.py

#sudo ls > /dev/null

mkdir -p ${HEARTBEAT_ENABLED_DIR}
$SET_SPEED -S 0
for i in {4..4}

      do
      
      $POWER_MON start 
      HR=''
      power=''
      joules=''
      FinalHR=''

      m=`expr 60 + $i \* 40`

      sudo /var/tmp/RAPL/RaplSetPower $m
      sudo -E numactl --interleave=0-1 --physcpubind=0-28 /var/tmp/Heartbeat/AvsPerf/jacobi
      $POWER_MON stop > power.txt
      
      HR=`tail -n 1 jacobi_heartbeat.log | awk '// {print $4}'`
      power=`cat power.txt | awk '/Pavg/ {print $2}'`
      FinalHR=`cat jacobi_heartbeat.log | awk '{sum+=$5} END {print sum/NR}'`

      
      echo $m $HR $FinalHR $power >> Rapljacobi.results
      sleep 5
      done
      


      MEMS=`ipcs | grep root | awk '{print $2}'`
      for k in $MEMS;
      do
          echo sudo Freeing $k;
          sudo ipcrm -m $k;
      done



      #MEMS=`ipcs | grep huazhe | awk '{print $2}'`
#for k in $MEMS; do echo Freeing $k; ipcrm -m $k; done

