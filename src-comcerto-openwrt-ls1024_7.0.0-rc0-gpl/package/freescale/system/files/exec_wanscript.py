#
#This python script executes the wanscrpt if the current wanstats are old by 600sec
import os
import os.path
import time

last_modified = os.path.getmtime("/tmp/stats/wanstats.tmp1")
#print( "Last modified at %s" % time.ctime(last_modified))

current_time = time.time() 
#print("current time is %s Difference is %d" % (time.ctime(current_time), (current_time - last_modified) ))

if(current_time - last_modified > 600):
        os.system('wanscript 2>/dev/null')
else:
        print("No need to execute")
