from pithy import *
from simple_example_class import Communication as Node
from time import sleep
import time
import numpy


hostsite = 'http://localhost:port you set forwarder to'
n = Node(hostsite)
number_of_reads_averaged = 600 # number is set in firmware - for noise reduction
resistance_value = 100 # value of your resistor
res = resistance_value
capacitor_value = 25.1 #your capacitor value
file_name = "foldername/filename"

print n.sread()

start_rest_time = 0
cycles = 1
min_potential = -0.5
max_potential = 1
read_delay = 0
DAC2_value = 2.5
relative_to_ocv = False

def cyclic_voltammetry():
    start_rest_time = 0
    cycles = 5
    min_potential = -0.5
    max_potential = 0.5
    rate = 0.5
    read_delay = 0
    DAC2_value = 2.5
    relative_to_ocv = False
    time_start = time.time()
    cycle = 0
    n.ocv()
    sleep(2)
    n.set_DAC2(DAC2_value)
    time.sleep(start_rest_time)
#--------------------------------#    
    read = n.parsedread(res) 
    output = read['adc-ref_adc']
    print "adc-ref_adc",output
    print "dac2: %r" %read['dac2']
    if relative_to_ocv:
        min_potential = min_potential + output #V
        max_potential = max_potential + output #V
    print "max_potential ", max_potential
    print "min_potential ", min_potential

    while cycle < cycles:
   	    while output < max_potential:
   	        print "attempt potential: %r" %output
   	        step_time = time.time()
   	        voltage = n.potentiostat(output)
   	        output = output + .005
   	        while ((time.time()-step_time) < (5/rate)):
   	            pass	
   	    output = max_potential
 	    while output > min_potential:
   	        print "attempt potential: %r" %output
   	        step_time = time.time()
   	        voltage = n.potentiostat(output)
   	        output = output - .005
   	        while (time.time()-step_time) < (5/(rate)):
   	            pass
    cycle += 1
   	
print n.changefile(file_name)
sleep(.05)
n.swrite('-0000')
sleep(.05)
sleep(1)
print n.startCSV(file_name)
sleep(.05)
cyclic_voltammetry()
print n.stopCSV()

