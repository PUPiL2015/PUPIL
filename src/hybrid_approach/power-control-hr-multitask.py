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

    def __init__(self, PwrCap, AppName, HeartbeatFileName, CommandLine,standardPerf):
        self.AppName = AppName
        self.AppNameShort =[]
        for appname in AppName:
            appnameShort = appname[0:8]
            self.AppNameShort.append(appnameShort)
        self.standardPerf = standardPerf
        #self.standardPerf = [11.245919,143.834678,327.098961,4.777705]
        #self.standardPerf = [0.0, 0.0, 0.0, 0.0]
        self.PwrCap = float(PwrCap)
      #  self.isFinished = 0
        self.phase = 0
        self.PerfDictionary = {(0,0,0):0}
        self.PwrDictionary = {(0,0,0):0}
        self.CurConfig = (0,0,0)
       # self.NextConfig = (0,0,0)
        self.CoreNumber = 0
        self.frequency = 16
        for i in range(len(CommandLine)):
            if CommandLine[i][-1] == '\n':
                CommandLine[i] = CommandLine[i][:-1]
        
        self.CommandLine = CommandLine
        self.HeartbeatFileName = HeartbeatFileName
        self.MemoCtrl = 2
        self.PerfFileLength = [0,0,0,0]
        self.CurCore= 0
        self.CurFreq= 16
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
            print 11111111111,self.PwrDictionary[(CoreNumber,MidPointer,2)], self.PwrCap
            print 11111111111,(self.PwrDictionary[(CoreNumber,MidPointer,2)] < self.PwrCap)
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
            self.CurConfig = (16,16,2)
            self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
            
            if self.PerfDictionary[self.CurConfig] < self.PerfDictionary[(8,16,2)]:
                self.CurConfig = (1,16,2)
                self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
                self.CurConfig = (40,16,2)
                self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
                if self.PerfDictionary[self.CurConfig] < self.PerfDictionary[(8,16,2)]or self.PerfDictionary[self.CurConfig] < self.PerfDictionary[(1,16,2)]:
                    #bs 1,8
                    self.CoreNumber = self.PerfBSearch(1, 8)
                else:
                    #bs 33,40
                    self.CoreNumber = self.PerfBSearch(33,40)
            else:
                self.CurConfig = (32,16,2)
                self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
                if self.PerfDictionary[self.CurConfig] < self.PerfDictionary[(16,16,2)]:
                    #bs 8,16
                    self.CoreNumber = self.PerfBSearch(8,16)
                else:
                    #bs 16,32
                    self.CoreNumber = self.PerfBSearch(16,32)
                        
            return 1
                
#get the heartbeat info
    def GetFeedback(self,config):
        print config
        #PowerFileName = 'socket_power.txt'
        PerfFileName = []
        for name in self.HeartbeatFileName:
            PerfFileName.append(name+'_heartbeat.log')

        #PerfFileName = self.CurFolder+'heartbeat.log'
        #PerfFileName = 'heartbeat.log'
        if config in self.PerfDictionary:
            return self.PerfDictionary[config], self.PwrDictionary[config]
        else:
           # subprocess.call([SET_SPEED, "-S", str(16-self.CurConfig[1])])
           # subprocess.call(["sudo","-E","numactl","--interleave=0-"+str(self.CurConfig[2]-1),"--physcpubind=0-"+str(self.CurConfig[0]-1),ExeFileName])
         #   self.RunApp(config[0],config[1],config[2])
         
            self.AdjustConfig(config[0],config[1],config[2])

            self.GetPowerDistAndSet(config[0])
            NormolizedPerf = 0.0
            time.sleep(2)
            TmpTime = time.time()

            for i in range(len(PerfFileName)):
            
            
        #    PowerFile = open(PowerFileName,'r')
                PerfFile = open(PerfFileName[i],'r')
        #    PowerFileLines= PowerFile.readlines()
                PerfFileLines= PerfFile.readlines()[1:]
                self.PerfFileLength[i] = len(PerfFileLines)
                
            for i in range(len(PerfFileName)):
                PerfFile = open(PerfFileName[i],'r')
        #    PowerFileLines= PowerFile.readlines()
                PerfFileLines= PerfFile.readlines()[1:]
         #   if int(config[0]) == 40 and int(config[1]==1):
          #      time.sleep(4)
        #        print "sleep 5"
         #   counter = 0
                print "len(PerfFileLines)", len(PerfFileLines)
                print "self.PerfFileLength",self.PerfFileLength
                print "wait...."
                while ((len(PerfFileLines) - self.PerfFileLength[i]) <5 or (time.time()- TmpTime < 5))and((len(PerfFileLines) - self.PerfFileLength[i]) < 3 or (time.time()- TmpTime < 10)):
                #while (((len(PerfFileLines) - self.PerfFileLength[i]) <8))or(time.time()- TmpTime < 5):

              #  PowerFile.close()
                    PerfFile.close()
                #get Socket Power
                    time.sleep(0.1)
              #  print "sleep 0.1"
               # print "get Socket Power"
              #  os.system("sudo "+RAPL_POWER_MON)
              #  PowerFile = open(PowerFileName,'r')
                    PerfFile = open(PerfFileName[i],'r')
             #   PowerFileLines= PowerFile.readlines()
                    PerfFileLines= PerfFile.readlines()[1:]
                #   counter += 1
             #   print PowerFileLines
                # if counter % 10 == 0:
                    #   os.system(POWER_MON+" stop > power.txt")
                print "waiting time: "+str(time.time() - TmpTime)
                
                #os.system("sudo "+RAPL_POWER_MON)
                
            for i in range(len(PerfFileName)):
                PerfFile = open(PerfFileName[i],'r')
                PerfFileLines= PerfFile.readlines()[1:]
         #       PowerFile = open(PowerFileName,'r')
          #      PowerFileLines= PowerFile.readlines()
            
                CurLength = len(PerfFileLines) - self.PerfFileLength[i]
                # print "sleep 0.1"
            #os.system("ps -ef | grep "+self.CurFolder+self.AppName+" | awk '{print $2}' | sudo xargs kill -9")
           # os.system("ps -ef | grep "+self.CurFolder+self.AppName+" | awk '{print $2}' | sudo xargs kill -9")
                SumPerf = 0.0
                j =0
                AvergeInterval = 0.0
                heartbeat = 0.0
                if self.PerfFileLength[i] == 0:
                    CurLength = CurLength -1
                print "CurLength:"+str(CurLength)+"len(PerfFileLines):"+str(len(PerfFileLines))
                TotalInterval =(long(PerfFileLines[-1].split()[2]) - long(PerfFileLines[0].split()[2]))
                CurInterval = (long(PerfFileLines[-1].split()[2]) - long(PerfFileLines[-CurLength-1].split()[2]))
                AvergeInterval = TotalInterval/ float(len(PerfFileLines)-1)
            
                for j in range(-CurLength,0):
                    LinePerf = PerfFileLines[j].split()

                    TimeInterval = long(LinePerf[2]) - long(PerfFileLines[j-1].split()[2])
                
                    heartbeat = heartbeat + 1
                    if TimeInterval < AvergeInterval / 100.0:
                        heartbeat = heartbeat - 1
                print 'heartbeat',heartbeat
                print 'TotalInterval',TotalInterval
                AvgPerf = float(heartbeat)* 1000000000/CurInterval
             #   if config[0] == 8:
              #      self.standardPerf[i] = AvgPerf
                NormolizedPerf += AvgPerf/self.standardPerf[i]
                print 'self.standardPerf',self.standardPerf
                print PerfFileName[i],'AvgPerf='+str(AvgPerf)
            
            
         #   j = 0
          #  SumPwr = 0
           # for i in range(len(PowerFileLines)):
            #    LinePwr = PowerFileLines[i].split()
             #   SumPwr += float(LinePwr[0])
              #  j +=1
           # AvgPwr = float(SumPwr)/j
            
            print NormolizedPerf, self.PwrCap
            return float(NormolizedPerf), float(self.PwrCap)


    def RunApp(self,CoreNumber, freq, MemoCtrl):
        if CoreNumber <33:
            os.system(SET_SPEED+' -S '+str(16-freq))
            #os.system(POWER_MON+" start")
            for commandline in self.CommandLine:

                print "sudo -E numactl --interleave=0-"+str(MemoCtrl-1)+" --physcpubind=0-"+str(CoreNumber-1)+" "+commandline+" > /dev/null 2>&1 &"
                os.system("sudo -E numactl --interleave=0-"+str(MemoCtrl-1)+" --physcpubind=0-"+str(CoreNumber-1)+" "+commandline+" > /dev/null 2>&1 &")
        else:
            os.system(SET_SPEED+' -S '+str(16-freq))
            #os.system(POWER_MON+" start")
            for appnameShort in self.CommandLine:
                os.system("sudo -E numactl --interleave=0-"+str(MemoCtrl-1)+" --physcpubind=0-7,16-"+str(CoreNumber-17)+" "+commandline+" > /dev/null 2>&1 &")
           # os.system(POWER_MON+" stop > power.txt")
        self.CurCore = CoreNumber
        self.GetPowerDistAndSet(CoreNumber)

    def AdjustConfig(self, CoreNumber,freq,MemoCtrl):
        StartTime = time.time()

        if self.CurFreq != freq:
            os.system(SET_SPEED+' -S '+str(16-freq))
        
        if self.CurCore != CoreNumber:
            if CoreNumber <33:
                for appnameShort in self.AppNameShort:
                    print "for i in $(pgrep "+appnameShort+" | xargs ps -mo pid,tid,fname,user,psr -p | awk 'NR > 1  {print $2}');do sudo taskset -pc 0-"+str(CoreNumber-1)+" $i > /dev/null & done"
                    print appnameShort
            #    os.system("for i in $(pgrep "+self.AppName+" | xargs ps -mo pid,tid,fname,user,psr -p | awk 'NR > 2  {print $2}');do sudo taskset -pc 0-"+str(CoreNumber-1)+" $i > /dev/null & done")
                    result1 = subprocess.check_output("for i in $(pgrep "+appnameShort+" | xargs pstree -p|grep -o \"([[:digit:]]*)\" |grep -o \"[[:digit:]]*\");do sudo taskset -pc 0-"+str(CoreNumber-1)+" $i & done",shell=True)
            else:
                for appnameShort in self.AppNameShort:
                #os.system("for i in $(pgrep "+self.AppName+" | xargs ps -mo pid,tid,fname,user,psr -p | awk 'NR > 2  {print $2}');do sudo taskset -pc 0-7,16-"+str(CoreNumber-17)+" $i > /dev/null & done")
                    result1 = subprocess.check_output("for i in $(pgrep "+appnameShort+" | xargs pstree -p|grep -o \"([[:digit:]]*)\" |grep -o \"[[:digit:]]*\");do sudo taskset -pc 0-7,16-"+str(CoreNumber-17)+" $i & done",shell=True)
        self.GetPowerDistAndSet(CoreNumber)
        self.CurCore = CoreNumber
        self.CurFreq = freq
        EndTime = time.time()
        print (EndTime - StartTime)

CommandLine =""
os.system("sudo /var/tmp/RAPL/RaplPowerLimitDisable")
inputfile = open(sys.argv[2],'r')
powercap = sys.argv[1]
AppNameList =[]
CommandLineList=[]
HeartbeatFileName=[]
standardPerf=[]
for line in inputfile.readlines():
    words = line.split(None, 3)
    HeartbeatFileName.append(words[0])
    AppNameList.append(words[1])
    standardPerf.append(float(words[2]))
    CommandLineList.append(words[3])


#for i in range(3,len(sys.argv)):
#    CommandLine  = CommandLine+" "+sys.argv[i]





PC= PowerControl(powercap,AppNameList,HeartbeatFileName,CommandLineList,standardPerf)
print "CommandLine", CommandLine
tmp1 = PC.Decision()
print PC.PerfDictionary
print PC.PwrDictionary
print PC.CoreNumber,PC.frequency,PC.MemoCtrl
PC.AdjustConfig(PC.CoreNumber,PC.frequency,PC.MemoCtrl)

#get heartbeat file length
for j in range(len(PC.AppNameShort)):
    file = open(PC.HeartbeatFileName[j]+'_heartbeat.log','r')
    PC.PerfFileLength[j] = len(file.readlines()[1:])

#file = open("heartbeat.log",'r')
#StartLength= len(file.readlines()[1:])
#file.close()
print (time.time() - StartTime)
result0 = subprocess.check_output("pgrep "+PC.AppNameShort[0]+" > /dev/null; echo $?", shell=True)
result1 = subprocess.check_output("pgrep "+PC.AppNameShort[1]+" > /dev/null; echo $?", shell=True)
result2 = subprocess.check_output("pgrep "+PC.AppNameShort[2]+" > /dev/null; echo $?", shell=True)
result3 = subprocess.check_output("pgrep "+PC.AppNameShort[3]+" > /dev/null; echo $?", shell=True)
while (result0 =='0\n' and result1 =='0\n'and result2 =='0\n' and result3 =='0\n'):
    time.sleep(1.0)
    result0 = subprocess.check_output("pgrep "+PC.AppNameShort[0]+" > /dev/null; echo $?", shell=True)
    result1 = subprocess.check_output("pgrep "+PC.AppNameShort[1]+" > /dev/null; echo $?", shell=True)
    result2 = subprocess.check_output("pgrep "+PC.AppNameShort[2]+" > /dev/null; echo $?", shell=True)
    result3 = subprocess.check_output("pgrep "+PC.AppNameShort[3]+" > /dev/null; echo $?", shell=True)
NormolizedPerfList=[0.0, 0.0, 0.0, 0.0]
NormolizedPerf= 0.0
print 'PC.standardPerf', PC.standardPerf
for j in range(len(PC.AppNameShort)):
    print "sudo pkill "+PC.AppNameShort[j]+"1111111111111111111111111111111111111111111111111111111111111111111111111111111"
    os.system("sudo pkill "+PC.AppNameShort[j])
    file = open(PC.HeartbeatFileName[j]+'_heartbeat.log','r')
    CurLength = len(file.readlines()[1:])
    Length = 0
    if CurLength - PC.PerfFileLength[j] > 1000:
        Length =1000
    else:
        Length =CurLength - PC.PerfFileLength[j]
    file.close()

    file = open(PC.HeartbeatFileName[j]+'_heartbeat.log','r')
    lines = file.readlines()[-Length-1:]
    heartbeat = 0.0
    SumPerf = 0.0
    AvgPerf = 0.0
    print "Length",Length
    print "long(lines[-1].split()[2]",long(lines[-1].split()[2])
    print "long(lines[-Length-1].split()[2]",long(lines[-Length-1].split()[2])
    TotalInterval =(long(lines[-1].split()[2]) - long(lines[-Length-1].split()[2]))
    #AvergeInterval = TotalInterval/ float(Length)
   # for i in range(-Length,0):
    #    TimeInterval = long(lines[i].split()[2]) - long(lines[i-1].split()[2])
     #   heartbeat = heartbeat + 1
      #  if TimeInterval < AvergeInterval / 100 :
       #     heartbeat = heartbeat - 1
    AvgPerf = float(Length)*1000000000/TotalInterval
    NormolizedPerfList[j] = AvgPerf/PC.standardPerf[j]
    NormolizedPerf += NormolizedPerfList[j]
    file.close()
#file = open("heartbeat.log",'r')
#Endlength= len(file.readlines()[1:])
#file.close()
os.system("echo "+str(NormolizedPerfList[0])+" "+str(NormolizedPerfList[1])+" "+str(NormolizedPerfList[2])+" "+str(NormolizedPerfList[3])+" "+str(NormolizedPerf)+" >> NormalizedHR.txt")
file = open("converged_configuration",'a')
file.write(str(PC.PwrCap)+" ("+str(PC.CoreNumber)+","+str(PC.frequency)+","+str(PC.MemoCtrl)+")")
file.close()
file = open("Normalizer.txt",'a')
file.write(str(PC.PwrCap)+" "+str(PC.standardPerf[0])+" "+str(PC.standardPerf[1])+" "+str(PC.standardPerf[2])+" "+str(PC.standardPerf[3])+"\n")
file.close()
print "finished"


#pgrep jacobi| sudo xargs taskset -pc 0-13
#pgrep jacobi | sudo xargs kill -9
#pgrep jacobi | xargs ps -mo pid,tid,fname,user,psr -p
#for i in $(pgrep jacobi | xargs ps -mo pid,tid,fname,user,psr -p | awk '// {print $2}'): print $i
#j= 0;for i in $(pgrep para | xargs ps -mo pid,tid,fname,user,psr -p | awk 'NR > 2  {print $2}');do echo $i; sudo taskset -pc $j $i; j=`expr $j + 1`; done
#j =0;for i in $(pgrep nn | xargs ps -mo pid,tid,fname,user,psr -p | awk 'NR > 2  {print $2}');do sudo taskset -pc $j-$j $i; j=`expr $j + 1`; done
#j =0;for i in $(pgrep xxxx | xargs pstree -p|grep -o "([[:digit:]]*)" |grep -o "[[:digit:]]*");do sudo taskset -pc 0-31 $i; j=`expr $j + 1`; done

