rm -Rf results
mkdir -p results

export HEARTBEAT_ENABLED_DIR=heartenabled/

LD_LIBRARY_PATH=/home/huazhe/intel64
export LD_LIBRARY_PATH

export OMP_NUM_THREADS=32

rm -Rf ${HEARTBEAT_ENABLED_DIR}

SET_SPEED=/home/hankhoffmann/tools/powerQoS/pySetCPUSpeed.py
POWER_MON=/home/hankhoffmann/tools/powerQoS/pyWattsup-hank.py

#sudo ls > /dev/null

mkdir -p ${HEARTBEAT_ENABLED_DIR}

$SET_SPEED -S 0

for i in {0..40}

      do
      #$LOGGER $NUMBER >log.txt &
      
      #sudo -E taskset -c 0-$i ${BINARY} ${ARGS} 
      #sudo -E taskset -c 0,1 ${BINARY} ${ARGS}  
     #sudo -E numactl --interleave=0-$k --physcpubind=0-$i ${BINARY} ${ARGS} >& /dev/null
    
      HR=''
      power=''
      joules=''
      c=1
      m=`expr 60 + $i \* 5`

     while [[ $HR = '' ]]||[[ $power = '' ]]||[[ $joules = '' ]]||[[ $c -le 0 ]]
      do
      sudo /var/tmp/RAPL/RaplSetPower $m
      $POWER_MON start
     sudo -E bash -c 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/huazhe/intel64;export HEARTBEAT_ENABLED_DIR=heartenabled/; /var/tmp/NU-MineBench-3.0/src/kmeans_test/example -i /var/tmp/NU-MineBench-3.0/datasets/kmeans/edge -b -o -p 32 -n 3 -m 6' 
 

      $POWER_MON stop > power.txt
      
      HR=`tail -n 1 heartbeat.log | awk '// {print $4}'`
      power=`cat power.txt | awk '/Pavg/ {print $2}'`
      joules=`cat power.txt | awk '/Joules/ {print $2}'`
      power2=`tail -n 1 heartbeat.log | awk '// {print $10}'`
  #   joules2=`echo "scale=4; $NUMBER / $HR * $power2" | bc`
      c=$(echo "$power2 > 0" | bc)
  #    echo $i $j $k $HR $power2 $joules2 $power $joules >> shmoo2.results

     done
      echo $m $HR $power2 $power $joules >> Raplkmeansnf32threads.results
      
      sleep 5

      MEMS=`ipcs | grep root | awk '{print $2}'`
      for k in $MEMS;
      do 
          echo sudo Freeing $k;
          sudo ipcrm -m $k;
      done
  done
 


