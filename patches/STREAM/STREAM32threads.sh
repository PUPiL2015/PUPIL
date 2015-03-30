rm -Rf results
mkdir -p results

export HEARTBEAT_ENABLED_DIR=heartenabled/
export OMP_NUM_THREADS=32

source pre-run.sh


ITERS=10

rm -Rf ${HEARTBEAT_ENABLED_DIR}

mkdir -p ${HEARTBEAT_ENABLED_DIR}


SET_SPEED=/home/hankhoffmann/tools/powerQoS/pySetCPUSpeed.py
POWER_MON=/home/hankhoffmann/tools/powerQoS/pyWattsup-hank.py

#sudo ls > /dev/null

mkdir -p ${HEARTBEAT_ENABLED_DIR}

for i in {0..31}
  do

  for j in {0..15} 
    do

    freq=`expr 15 - $j`

    $SET_SPEED -S $freq


    for k in {0..1}
      do
      
      #$LOGGER $NUMBER >log.txt &
      
      #sudo -E taskset -c 0-$i ${BINARY} ${ARGS} 
      #sudo -E taskset -c 0,1 ${BINARY} ${ARGS}  
     # sudo -E numactl --interleave=0-$k --physcpubind=0-$i ${BINARY} ${ARGS} >& /dev/null
      HR=''
      power=''
      joules=''
      c=1
    while [[ $HR = '' ]]||[[ $power = '' ]]||[[ $joules = '' ]]||[[ $c -le 0 ]]
      do


      $POWER_MON start
      sudo -E numactl --interleave=0-$k --physcpubind=0-$i ${BINARY} ${ARGS}
	#sudo -E taskset -c 0-$i ${BINARY} ${ARGS} >& /dev/null
      $POWER_MON stop > power.txt

      HR=`tail -n 1 STREAM_heartbeat.log | awk '// {print $4}'`
      power=`cat power.txt | awk '/Pavg/ {print $2}'`
      joules=`cat power.txt | awk '/Joules/ {print $2}'`
      power2=`tail -n 1 STREAM_heartbeat.log | awk '// {print $11}'`
     joules2=`echo "scale=4; $NUMBER / $HR * $power2" | bc`
	c=$(echo "$power2 > 0" | bc)
  #    echo $i $j $k $HR $power2 $joules2 $power $joules >> shmoo2.results

      done
      echo $i $j $k $HR $power2 $power $joules >> STREAM32threads.results
      

      sleep 5
    done

  done
     # MEMS=`ipcs | grep huazhe | awk '{print $2}'`
#for k in $MEMS; do echo Freeing $k; ipcrm -m $k; done
 MEMS=`ipcs | grep root | awk '{print $2}'`
      for k in $MEMS;
      do 
          echo sudo Freeing $k;
          sudo ipcrm -m $k;
      done


done



