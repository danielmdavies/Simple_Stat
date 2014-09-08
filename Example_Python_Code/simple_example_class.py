from pithy import *
import urllib2
import time
from time import sleep
import pickle

class Communication():
    def __init__(self,base):
        self.base = base
        self.bwrite = self.base+"write/"
        self.bread = self.base+"read/"
        self.bchangefile = self.base+"setName/"
        self.bstartCSV = self.base+"startCSV/"
        self.bstopCSV = self.base+"stopCSV/"
        self.bunkill = self.base+"unkill/"
        self.debug = False
        
    def changefile(self,file_name):
        foo = urllib2.urlopen(self.bchangefile+file_name)
        sleep(.05)
        return foo.read() # if nothing this shouldn't break
    def startCSV(self,file_name):
        urllib2.urlopen(self.bunkill)
        urllib2.urlopen(self.bchangefile+file_name)
        sleep(.05)
        foo = urllib2.urlopen(self.bstartCSV+file_name)
        sleep(.05)
        return foo.read()
    
    def stopCSV(self):
        foo = urllib2.urlopen(self.bstopCSV)
         
    def sread(self):
        foo = urllib2.urlopen(self.bread)
        buff = foo.read()
        out = buff[buff.rfind("GO"):buff.rfind("ST")+2]
        return out
     
    def swrite(self,stringer):
        foo = urllib2.urlopen(self.bwrite+stringer)
        sleep(.05)
        print foo.read()
        return foo.read()
        
    def parseline(self,reading,res):
        outdict = {}
        outdict['valid'] = True
        parts = reading.split(",")
        #print parts
        if parts[0] != "GO":
	        print "reading messed up"
	        return "reading error"
        outdict['outvolt'] = int(parts[2])
        outdict['setout']= int(parts[6])
        outdict['dac1'] = self.refbasis(parts[4])
        outdict['adc'] = self.refbasis(parts[3])
        outdict['dac2'] = self.refbasis(parts[-4])
        outdict['ref_adc'] = self.refbasis(parts[-3])
        outdict['millis'] = int(parts[-2])
        #print outdict['millis']

        outdict['current'] = (float(outdict['dac1'])-float(outdict['adc']))/(float(res))
        outdict['adc-ref_adc'] = (float(outdict['adc'])-float(outdict['ref_adc']))
	outdict['dac2'] = (float(outdict['dac2'])-float(outdict['ref_adc']))
        if self.debug:
            print outdict
        return outdict
        
    def ocv(self):
        self.swrite("-0000")
         
    def parsedread(self,known_resistance):
        read = self.parseline(self.sread(),known_resistance)
        while read == "reading error":
		    print read
		    read = self.parseline(self.sread(),known_resistance)
        return read
        
    def sleep(self,time):
        self.ocv()
        for i in range(0,time):
            time.sleep(1)
         
    def set_DAC2(self,potential):
        potential = str(int(1023*(potential/5.0))).rjust(4,"0")
        self.swrite("d"+potential)
        
    def potentiostat(self,potential):
        voltage = str(int(1023*(abs(potential)/5.0))).rjust(4,"0")
        if potential < 0:
            voltage = int(voltage) + 2000
        if voltage == 2000:
            voltage = '0000'
        if self.debug:
            print 'sending arduino: ',("p"+voltage)
        voltage = str(voltage)
        self.swrite("p"+voltage)
    
    def galvanostat(self,current,res):
        delta_V = abs(current*res)
        potential = str(int(1023 * (delta_V / 5.0))).rjust(4, "0")
        if current < 0:
            potential = str(int(potential)+2000)
        self.swrite("g"+str(potential))
        
#--------------------------------------------------------------#

