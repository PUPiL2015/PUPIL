import numpy
from sklearn import linear_model

training_data_X = []
training_data_Y = []
weight_list=[]
# features: #sockets, #hyperthreads, #cores, level_of_DVFS,#MC

benchmarkName= ['blackscholes','PLSA','kmeans','swish','bfs','jacobi','swaptions','x264','bodytrack','btree','cfd','particlefilter','svm_rfe','HOP','ScalParC','fluidanimate','dijkstra','STREAM','kmeansnf','vips']
optimalPath = '/Users/huazhezhang/Project/data/project/true_optimal'

for benchmark_name in benchmarkName:
    file = open(optimalPath+'/'+benchmark_name+'/'+benchmark_name+'32threads_raw.results','r')
    lines = file.readlines()
    for line in lines:
        words = line.split()
        socket_number = 0
        hyperthread_number = 0
        core_number = 0
        #DVFS level
        dvfs_level = int(words[1])
        #MC
        MC_number = int(words[2])
        
        # core_number, hyperthread, socket
        if int(words[0])< 8:
            core_number = int(words[0])+1
       #     socket_number = 1
       #     hyperthread_number = 0
        elif int(words[0]) <32:
            core_number = int(words[0])+1
       #     socket_number =2
       #     hyperthread_number= int(words[0]) -15
        else:
            core_number = int(words[0]) -24 +1
         #   socket_number = 1
         #   hyperthread_number = int(words[0]) -31



        training_data_X.append([socket_number, core_number, hyperthread_number, dvfs_level, MC_number])
        #print words[5]
        #print [socket_number, core_number, hyperthread_number, dvfs_level, MC_number]
        power = 0.0
        if benchmark_name not in ['bodytrack','swaptions','x264','swish']:
            power = float(words[5])
        else:
            power = float(words[6])
        training_data_Y.append(power)
        if power > 400:
            print benchmark_name
      #  if int(words[0]) ==31 and int(words[1]) == 15:
       #     weight_list.append(100)
       # else:
       #     weight_list.append(1)

    file.close()
regr = linear_model.LinearRegression()
regr.fit(training_data_X, training_data_Y, weight_list)
print regr.predict([[1,15,16,15,1],[1,1,16,15,1]])
#system power using kmean's as baseline
#system_power=[129.7, 180.5, 227.3, 274.7, 336.7]
system_power=[0,0,0,0,0]
for i in range(0,5):
    for benchmark_name in benchmarkName:
        file = open(optimalPath+'/'+benchmark_name+'/'+benchmark_name+'32threads.results','r')
        lines = file.readlines()
        system_power[i] += float(lines[i].split()[2])
    system_power[i] = system_power[i] /20.0
print system_power
#finding the config
config_power_list =[]
for socket_number in range(1,3):
    for core_number in range(0,16):
        for hyperthread_number in range(1,core_number+2):
            for dvfs_level in range(0, 16):
                for MC_number in range(0,2):
                    pwr = regr.predict([[socket_number, core_number, hyperthread_number,dvfs_level, MC_number]])
                    config_power_list.append([socket_number, core_number, hyperthread_number,dvfs_level, MC_number, pwr])
config = []
max_power =0.0
for power in system_power:
    for config_power in config_power_list:
        if config_power[5] < power and config_power[5] > max_power:
            max_power = config_power[5]
            configuration = config_power[0:5]
    config.append(configuration)

print config




