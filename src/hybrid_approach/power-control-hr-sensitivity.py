import subprocess
import time
import os
import sys
import math
StartTime = time.time()
SET_SPEED = "/home/hankhoffmann/tools/powerQoS/pySetCPUSpeed.py"
POWER_MON = "/home/hankhoffmann/tools/powerQoS/pyWattsup-hank.py"
RAPL_POWER_MON = "/local/huazhe/decision_tree/RaplPowerMonitor"

class PowerControl:

    def __init__(self, PwrCap, AppName, HeartbeatFileName, CommandLine):
        self.AppName = AppName
        self.AppNameShort =AppName[0:8]
        self.PwrCap = float(PwrCap)
      #  self.isFinished = 0
        self.phase = 0
        self.PerfDictionary = {(0,0,0):0}
        self.PwrDictionary = {(0,0,0):0}
        self.CurConfig = (0,0,0)
       # self.NextConfig = (0,0,0)
        self.CoreNumber = 0
        self.frequency = 16
        self.CommandLine = CommandLine
        self.MemoCtrl = 2
        self.PerfFileLength = 0
        self.CurCore= 0
        self.CurFreq= 16
        self.HeartbeatFileName = HeartbeatFileName

        print self.AppNameShort
    
    def PwrBSearch(self,lowbound, highbound, PwrCap):
        head = lowbound
        tail = highbound
        self.PerfDictionary[(head,1,2)],self.PwrDictionary[(head,1,2)] = self.GetFeedback((head,1,2))
        self.PerfDictionary[(tail,1,2)],self.PwrDictionary[(tail,1,2)] = self.GetFeedback((tail,1,2))
        while(head +1 < tail):
            MidPointer = (head + tail )/2
            self.PerfDictionary[(MidPointer,1,2)],self.PwrDictionary[(MidPointer,1,2)] = self.GetFeedback((MidPointer,1,2))
            if self.PwrDictionary[(MidPointer,1,2)] < PwrCap:
                head = MidPointer
            else:
                tail = MidPointer
        if self.PwrDictionary[(tail,1,2)] > PwrCap:
            return head
        else:
            return tail
    
    def PerfBSearch(self,lowbound, highbound):
        self.PerfDictionary[(highbound,16,2)],self.PwrDictionary[(highbound,16,2)] = self.GetFeedback((highbound,16,2))
        self.PerfDictionary[(lowbound,16,2)],self.PwrDictionary[(lowbound,16,2)] = self.GetFeedback((lowbound,16,2))
        head = lowbound
        tail = highbound
        if self.PerfDictionary[(highbound,16,2)] > self.PerfDictionary[(lowbound,16,2)]:
            while(head+1<tail):
                MidPointer = (head +tail)/2
                self.PerfDictionary[(MidPointer,16,2)],self.PwrDictionary[(MidPointer,16,2)] = self.GetFeedback((MidPointer,16,2))
                if self.PerfDictionary[(MidPointer,16,2)] < self.PerfDictionary[(tail,16,2)]:
                    head = MidPointer
                else:
                    TmpPointer = MidPointer +1
                    self.PerfDictionary[(TmpPointer,16,2)],self.PwrDictionary[(TmpPointer,16,2)] = self.GetFeedback((TmpPointer,16,2))
                    if self.PerfDictionary[(TmpPointer,16,2)] > self.PerfDictionary[(MidPointer,16,2)]:
                        head = TmpPointer
                    else:
                        tail = MidPointer

        else:
            while(head+1<tail):
                MidPointer = (head +tail)/2
                self.PerfDictionary[(MidPointer,16,2)],self.PwrDictionary[(MidPointer,16,2)] = self.GetFeedback((MidPointer,16,2))
                if self.PerfDictionary[(MidPointer,16,2)] < self.PerfDictionary[(head,16,2)]:
                    tail = MidPointer
                else:
                    TmpPointer = MidPointer -1
                    self.PerfDictionary[(TmpPointer,16,2)],self.PwrDictionary[(TmpPointer,16,2)] = self.GetFeedback((TmpPointer,16,2))
                    if self.PerfDictionary[(TmpPointer,16,2)] > self.PerfDictionary[(MidPointer,16,2)]:
                        tail = TmpPointer
                    else:
                        head = MidPointer
        if self.PerfDictionary[(head,16,2)] > self.PerfDictionary[(tail,16,2)]:
            return head
        else:
            return tail

        


    def FreqBsearch(self,CoreNumber):
        head = 1
        tail =16
        self.PerfDictionary[(CoreNumber,head,2)],self.PwrDictionary[(CoreNumber,head,2)] = self.GetFeedback((CoreNumber,head,2))
        self.PerfDictionary[(CoreNumber,tail,2)],self.PwrDictionary[(CoreNumber,tail,2)] = self.GetFeedback((CoreNumber,tail,2))
        while (head +1 <tail):
            MidPointer = (head + tail)/2
            self.PerfDictionary[(CoreNumber,MidPointer,2)],self.PwrDictionary[(CoreNumber,MidPointer,2)] = self.GetFeedback((CoreNumber,MidPointer,2))
            if (self.PwrDictionary[(CoreNumber,MidPointer,2)] < self.PwrCap):
                head = MidPointer
            else:
                tail = MidPointer
        if self.PwrDictionary[(CoreNumber,tail,2)] > self.PwrCap:
            return head
        else:
            return tail
            
    def GetPowerDistAndSet(self,CoreNumber):
        power1 =0.0
        pwoer2 =0.0
        if CoreNumber <9 or (CoreNumber >33):
            power1 = self.PwrCap-40.0
            power2 = 0
        else:
            if CoreNumber > 8 and CoreNumber <17:
                power1 = math.ceil(((self.PwrCap-40.0)/CoreNumber ) * 8)
                power2 = self.PwrCap-40.0 -power1
            else:
                if CoreNumber > 16 and CoreNumber < 25:
                    power2 = math.floor(((self.PwrCap-40.0)/CoreNumber ) * 8)
                    power1 = self.PwrCap-40.0 -power2
                else:
                    power1 = math.ceil(((self.PwrCap-40.0)/CoreNumber ) * 16)
                    power2 = self.PwrCap-40.0 -power1
        os.system("sudo /var/tmp/RAPL/RaplSetPowerSeprate "+str(power1+20)+" "+str(power2+20))


    def Decision(self):
        self.RunApp(8,16,2)
        if self.phase == 0:
            self.CurConfig = (8,16,2)
            self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
            self.CurConfig = (1,16,2)
            self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
            self.CurConfig = (40,16,2)
            self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
            
            if self.PerfDictionary[self.CurConfig] < max(self.PerfDictionary[(1,16,2)],self.PerfDictionary[(8,16,2)]):
                self.CurConfig = (1,16,2)
                self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
                self.CurConfig = (16,16,2)
                self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
                if self.PerfDictionary[self.CurConfig] < self.PerfDictionary[(8,16,2)]or self.PerfDictionary[self.CurConfig] < self.PerfDictionary[(1,16,2)]:
                    #bs 1,8
                    self.CoreNumber = self.PerfBSearch(1, 8)
                else:
                    #bs 33,40
                    self.CoreNumber = self.PerfBSearch(8,16)
            else:
                self.CurConfig = (32,16,2)
                self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
                if self.PerfDictionary[self.CurConfig] < self.PerfDictionary[(40,16,2)]:
                    #bs 8,16
                    self.CoreNumber = self.PerfBSearch(32,40)
                else:
                    #bs 16,32
                    self.CoreNumber = self.PerfBSearch(16,32)

                                
                        
            return 1
                
#get the heartbeat info
    def GetFeedback(self,config):
        print config
        #PowerFileName = self.CurFolder+'socket_power.txt'
        #PowerFileName = 'socket_power.txt'
        #PerfFileName = self.CurFolder+'heartbeat.log'
        PerfFileName = self.HeartbeatFileName
        if config in self.PerfDictionary:
            return self.PerfDictionary[config], self.PwrDictionary[config]
        else:
           # subprocess.call([SET_SPEED, "-S", str(16-self.CurConfig[1])])
           # subprocess.call(["sudo","-E","numactl","--interleave=0-"+str(self.CurConfig[2]-1),"--physcpubind=0-"+str(self.CurConfig[0]-1),ExeFileName])
         #   self.RunApp(config[0],config[1],config[2])
            self.AdjustConfig(config[0],config[1],config[2])
            self.GetPowerDistAndSet(config[0])

        #    PowerFile = open(PowerFileName,'r')
            PerfFile = open(PerfFileName,'r')
        #    PowerFileLines= PowerFile.readlines()
            PerfFileLines= PerfFile.readlines()[1:]
            self.PerfFileLength = len(PerfFileLines)
            TmpTime = time.time()
            time.sleep(2)
            if int(config[0]) == 40 or int(config[0]==32) or int(config[0]==16)or int(config[0]==8):
                time.sleep(3)
            #    print "sleep 5"
         #   counter = 0
            print "len(PerfFileLines)", len(PerfFileLines)
            print "self.PerfFileLength",self.PerfFileLength
            print "wait...."
            while ((len(PerfFileLines) - self.PerfFileLength) <10)and((len(PerfFileLines) - self.PerfFileLength) < 2 or (time.time()- TmpTime < 10)):
              #  PowerFile.close()
                PerfFile.close()
                #get Socket Power
                time.sleep(0.1)
              #  print "sleep 0.1"
               # print "get Socket Power"
              #  os.system("sudo "+RAPL_POWER_MON)
              #  PowerFile = open(PowerFileName,'r')
                PerfFile = open(PerfFileName,'r')
             #   PowerFileLines= PowerFile.readlines()
                PerfFileLines= PerfFile.readlines()[2:]
             #   counter += 1
             #   print PowerFileLines
                # if counter % 10 == 0:
                #   os.system(POWER_MON+" stop > power.txt")
            print "waiting time: "+str(time.time() - TmpTime)
           # os.system("sudo "+RAPL_POWER_MON)
            #PowerFile = open(PowerFileName,'r')
            #PowerFileLines= PowerFile.readlines()
            
            CurLength = len(PerfFileLines) - self.PerfFileLength

            print "CurLength=",CurLength
                # print "sleep 0.1"
            #os.system("ps -ef | grep "+self.CurFolder+self.AppName+" | awk '{print $2}' | sudo xargs kill -9")
           # os.system("ps -ef | grep "+self.CurFolder+self.AppName+" | awk '{print $2}' | sudo xargs kill -9")
            SumPerf = 0
            j =0
            AvergeInterval = 0
            heartbeat = 0.0
            if self.PerfFileLength == 0:
                CurLength = CurLength -1
            print "long(PerfFileLines[-1].split()[2])=",long(PerfFileLines[-1].split()[2])
            print "long(PerfFileLines[-CurLength-1].split()[2])=",long(PerfFileLines[-CurLength-1].split()[2])
            TotalInterval =(long(PerfFileLines[-1].split()[2]) - long(PerfFileLines[-CurLength-1].split()[2]))

            AvergeInterval = TotalInterval/ float(CurLength)
            
            for i in range(-CurLength,0):
                LinePerf = PerfFileLines[i].split()

                TimeInterval = long(LinePerf[2]) - long(PerfFileLines[i-1].split()[2])
                
                heartbeat = heartbeat + 1
                if TimeInterval < AvergeInterval / 100 :
                    heartbeat = heartbeat - 1
                SumPerf +=  float(LinePerf[4])
            AvgPerf = heartbeat/TotalInterval
            
           # j = 0
           # SumPwr = 0
           # for i in range(len(PowerFileLines)):
            #    LinePwr = PowerFileLines[i].split()
            #    SumPwr += float(LinePwr[0])
             #   j +=1
            #AvgPwr = float(SumPwr)/j
            print AvgPerf, 0
            return float(AvgPerf), float(0)



    def RunApp(self,CoreNumber, freq, MemoCtrl):
        if CoreNumber <33:
            os.system(SET_SPEED+' -S '+str(16-freq))
            #os.system(POWER_MON+" start")
            print "sudo -E numactl --interleave=0-"+str(MemoCtrl-1)+" --physcpubind=0-"+str(CoreNumber-1)+" "+self.CommandLine+" &"
            os.system("sudo -E numactl --interleave=0-"+str(MemoCtrl-1)+" --physcpubind=0-"+str(CoreNumber-1)+" "+self.CommandLine+" &")
        else:
            os.system(SET_SPEED+' -S '+str(16-freq))
            #os.system(POWER_MON+" start")
           # os.system(POWER_MON+" stop > power.txt")
        self.CurCore = CoreNumber
        self.GetPowerDistAndSet(CoreNumber)

    def AdjustConfig(self, CoreNumber,freq,MemoCtrl):
        StartTime = time.time()
        proc = subprocess.Popen(["pgrep "+self.AppNameShort], stdout=subprocess.PIPE, shell=True)
        (out, err) = proc.communicate()
        
        list_out = out.split('\n')
        while len(list_out) !=2:
            proc = subprocess.Popen(["pgrep "+self.AppNameShort], stdout=subprocess.PIPE, shell=True)
            (out, err) = proc.communicate()
            list_out = out.split('\n')

        
        if self.CurFreq != freq:
            os.system(SET_SPEED+' -S '+str(16-freq))
        
        if self.CurCore != CoreNumber:
            if CoreNumber <33:
                print self.AppNameShort
            #    os.system("for i in $(pgrep "+self.AppName+" | xargs ps -mo pid,tid,fname,user,psr -p | awk 'NR > 2  {print $2}');do sudo taskset -pc 0-"+str(CoreNumber-1)+" $i > /dev/null & done")
                result1 = subprocess.check_output("for i in $(pgrep "+self.AppNameShort+" | xargs pstree -p|grep -o \"([[:digit:]]*)\" |grep -o \"[[:digit:]]*\");do sudo taskset -pc 0-"+str(CoreNumber-1)+" $i & done",shell=True)
            else:
                #os.system("for i in $(pgrep "+self.AppName+" | xargs ps -mo pid,tid,fname,user,psr -p | awk 'NR > 2  {print $2}');do sudo taskset -pc 0-7,16-"+str(CoreNumber-17)+" $i > /dev/null & done")
                #result1 = subprocess.check_output("for i in $(pgrep \"+self.AppNameShort\");do for j in $(pstree -p $i|grep -o \"([[:digit:]]*)\" |grep -o \"[[:digit:]]*\");do sudo taskset -pc 0-7,16-"+str(CoreNumber-17)+" $j & done;done",shell=True)
                result1 = subprocess.check_output("for i in $(pgrep "+self.AppNameShort+" | xargs pstree -p|grep -o \"([[:digit:]]*)\" |grep -o \"[[:digit:]]*\");do sudo taskset -pc 0-7,16-"+str(CoreNumber-17)+" $i & done",shell=True)
        self.GetPowerDistAndSet(CoreNumber)
        self.CurCore = CoreNumber
        self.CurFreq = freq
        EndTime = time.time()
        print (EndTime - StartTime)

CommandLine =""
os.system("sudo /var/tmp/RAPL/RaplPowerLimitDisable")
for i in range(4,len(sys.argv)):
    CommandLine  = CommandLine+" "+sys.argv[i]
PC= PowerControl(sys.argv[1],sys.argv[2],sys.argv[3],CommandLine)
print "CommandLine", CommandLine
tmp1 = PC.Decision()
print PC.PerfDictionary
print PC.PwrDictionary
print PC.CoreNumber,PC.frequency,PC.MemoCtrl
PC.AdjustConfig(PC.CoreNumber,PC.frequency,PC.MemoCtrl)
file = open(PC.HeartbeatFileName,'r')
StartLength= len(file.readlines()[1:])
file.close()
print (time.time() - StartTime)
result = subprocess.check_output("pgrep "+PC.AppNameShort+" > /dev/null; echo $?", shell=True)
while (result =='0\n'):
    result = subprocess.check_output("pgrep "+PC.AppNameShort+" > /dev/null; echo $?", shell=True)
    time.sleep(1.0)

file = open(PC.HeartbeatFileName,'r')
Endlength= len(file.readlines()[1:])
file.close()
os.system("echo "+str(Endlength - StartLength)+" >> Length.txt")

file = open("converged_configuration",'a')
file.write(str(PC.PwrCap)+" ("+str(PC.CoreNumber)+","+str(PC.frequency)+","+str(PC.MemoCtrl)+")")
print result,"finished"


