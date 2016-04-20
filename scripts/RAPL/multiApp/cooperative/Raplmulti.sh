export HEARTBEAT_ENABLED_DIR=heartenabled/
export OMP_NUM_THREADS=8
export IM_CONCURRENCY=8
rm -Rf ${HEARTBEAT_ENABLED_DIR}

SET_SPEED=/home/hankhoffmann/tools/powerQoS/pySetCPUSpeed.py
POWER_MON=/home/hankhoffmann/tools/powerQoS/pyWattsup-hank.py

benchmark1=vips_heartbeat.log
benchmark2=kmeans_heartbeat.log
benchmark3=HOP_heartbeat.log
benchmark4=STREAM_heartbeat.log

mkdir -p ${HEARTBEAT_ENABLED_DIR}

$SET_SPEED -S 0

for i in {4..4}

      do
      HR=''
      power=''
      joules=''
      c=1
      m=`expr 60 + $i \* 40`

     while [[ $HR1 = '' ]]||[[ $power = '' ]]||[[ $joules = '' ]]||[[ $c -le 0 ]]
      do
      #sudo /var/tmp/RAPL/RaplSetPower $m
      $POWER_MON start
      sudo -E numactl --interleave=0-1 --physcpubind=0-30 /local/benchmarks/parsec-2.0/pkgs/apps/vips/inst/x86_64-linux.gcc-hooks/bin/vips im_benchmarkn /local/scratch/scratch/hankhoffmann/orion_18000x18000.v output.v 5 &
      sudo -E numactl --interleave=0-1 --physcpubind=0-30 bash -c 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/huazhe/intel64;export HEARTBEAT_ENABLED_DIR=heartenabled/; /var/tmp/NU-MineBench-3.0/src/kmeans_test/example -i /var/tmp/NU-MineBench-3.0/datasets/kmeans/edge -b -o -f -p 8 -n 2 -m 3' &
      sudo -E numactl --interleave=0-1 --physcpubind=0-30 /local/huazhe/development/STREAM/stream_c.exe &
      sudo -E numactl --interleave=0-1 --physcpubind=0-30 /local/huazhe/iterationProgram/iterate2 HOP 100 "bash -c 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/huazhe/intel64;export HEARTBEAT_ENABLED_DIR=heartenabled/; /var/tmp/NU-MineBench-3.0/src/HOP/para_hop 491520 /var/tmp/NU-MineBench-3.0/datasets/HOP/particles_0_128 64 16 -1 8'" &

	sleep 10
	while [[ $(pgrep vips) != '' ]]&&[[ $(pgrep example) != '' ]]&&[[ $(pgrep stream) != '' ]]&&[[ $(pgrep iterate2) != '' ]]
	do
      	sleep 1
	done
      sudo pkill vips
      sudo pkill example
      sudo pkill stream
      sudo pkill iterate2
      $POWER_MON stop > power.txt
        echo 33333333333333
      
      HR1=$(echo "(`cat $benchmark1| wc -l` - 2) *1000000000/(`tail -n 1 $benchmark1 | awk '// {print $3}'` - `cat $benchmark1 | awk 'NR==2 {print $3}'`)" |bc -l)
        echo 1111111111111111

      HR2=$(echo "(`cat $benchmark2| wc -l` - 2) *1000000000/(`tail -n 1 $benchmark2 | awk '// {print $3}'` - `cat $benchmark2 | awk 'NR==2 {print $3}'`)" |bc -l)
        echo 2222222222222222

      HR3=$(echo "(`cat $benchmark3| wc -l` - 2) *1000000000/(`tail -n 1 $benchmark3 | awk '// {print $3}'` - `cat $benchmark3 | awk 'NR==2 {print $3}'`)" |bc -l)

      HR4=$(echo "(`cat $benchmark4| wc -l` - 2) *1000000000/(`tail -n 1 $benchmark4 | awk '// {print $3}'` - `cat $benchmark4 | awk 'NR==2 {print $3}'`)" |bc -l)


      power=`cat power.txt | awk '/Pavg/ {print $2}'`
      joules=`cat power.txt | awk '/Joules/ {print $2}'`
  #    power2=`tail -n 1 heartbeat.log | awk '// {print $10}'`
  #   joules2=`echo "scale=4; $NUMBER / $HR * $power2" | bc`
      c=$(echo "$power > 0" | bc)
  #    echo $i $j $k $HR $power2 $joules2 $power $joules >> shmoo2.results

     done
      echo $m $HR1 $HR2 $HR3 $HR4 $power2 $power $joules >> Raplmulti.results
      
      sleep 5

      MEMS=`ipcs | grep root | awk '{print $2}'`
      for k in $MEMS;
      do 
          echo sudo Freeing $k;
          sudo ipcrm -m $k;
      done
  done
 


