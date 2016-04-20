rm -Rf results mkdir -p results
declare -a benchmark=('blackscholes' 'PLSA' 'kmeans' 'swish' 'bfs' 'jacobi' 'swaptions' 'x264' 'bodytrack' 'btree' 'cfd' 'particlefilter' 'svm_rfe' 'HOP' 'ScalParC' 'fluidanimate' 'dijkstra' 'STREAM' 'kmeansnf' 'vips')
declare -a command=('/local/benchmarks/parsec-2.0/pkgs/apps/blackscholes/obj/x86_64-linux.gcc-hooks/blackscholes 32 300000' \
'bash -c "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/huazhe/intel64;export HEARTBEAT_ENABLED_DIR=heartenabled/; /var/tmp/NU-MineBench-3.0/src/PLSA/parasw.mt /home/huazhe/Project/NU-MineBench-3.0/datasets/PLSA/30k_1.txt /home/huazhe/Project/NU-MineBench-3.0/datasets/PLSA/30k_2.txt /home/huazhe/Project/NU-MineBench-3.0/datasets/PLSA/pam120.bla 600 400 3 3 1 32"' \
'')
export HEARTBEAT_ENABLED_DIR=heartenabled/
LD_LIBRARY_PATH=/home/huazhe/intel64
export LD_LIBRARY_PATH

source pre-run.sh

rm -Rf ${HEARTBEAT_ENABLED_DIR}

mkdir -p ${HEARTBEAT_ENABLED_DIR}


SET_SPEED=/home/hankhoffmann/tools/powerQoS/pySetCPUSpeed.py
POWER_MON=/home/hankhoffmann/tools/powerQoS/pyWattsup-hank.py

#sudo ls > /dev/null

mkdir -p ${HEARTBEAT_ENABLED_DIR}
for p in {1..20}
    benchmark[$p]

    for n in 1 2 3 4 5
    do
        if [ "$n" -eq 1 ]; then
            i=14
            j=4
            k=0
        elif [ "$n" -eq 2 ];then
            i=15
            j=9
            k=0
        elif [ "$n" -eq 3 ];then
            i=23
            j=11
            k=1
        elif [ "$n" -eq 4 ];then
            i=25
            j=15
            k=0
        else
            i=31
            j=15
            k=1
    
    freq=`expr 15 - $j`

    $SET_SPEED -S $freq
    for k in {1..1}
      do
	HR=''
	power=''
	joules=''
	c=1
	while [[ $HR = '' ]]||[[ $power = '' ]]||[[ $joules = '' ]]||[[ $c -le 0 ]]
	do

      $POWER_MON start
      
      #$LOGGER $NUMBER >log.txt &
      
      #sudo -E taskset -c 0-$i ${BINARY} ${ARGS} 
      #sudo -E taskset -c 0,1 ${BINARY} ${ARGS} 
      sudo -E numactl --interleave=0-$k --physcpubind=0-$i ${BINARY} ${ARGS}

      $POWER_MON stop > power.txt
      
      HR=`tail -n 1 x264_heartbeat.log | awk '// {print $4}'`
      power=`cat power.txt | awk '/Pavg/ {print $2}'`
      joules=`cat power.txt | awk '/Joules/ {print $2}'`
      power2=`tail -n 1 x264_heartbeat.log | awk '// {print $10}'`
      joules2=`echo "scale=4; $NUMBER / $HR * $power2" | bc`
      c=$(echo "$power2 > 0" | bc)

	done
      echo $i $j $k $HR $power2 $joules2 $power $joules >> x264_32threads.results
      
      
      cp power.txt power-static-oracle.txt
      cp heartbeat.log heartbeat-static-oracle.log
      

      sleep 20
    done

  done 
      MEMS=`ipcs | grep root | awk '{print $2}'`
      for k in $MEMS;
      do
          echo sudo Freeing $k;
          sudo ipcrm -m $k;
      done
done


