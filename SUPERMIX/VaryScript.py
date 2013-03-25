#!/usr/bin/python

import os
import threading
import time

TOTALDATASIZE = 0.1 # in megabytes
BUFFSIZE = 5120 # in bytes
LOSDUPRATES = [(0.05,0.05),(0.15,0.15),(0.30,0.30),(0.50,0.50),(0.80,0.80),]

def compile(sources,executable,libs):
	result = "gcc -w -std=c99"

	for	source in sources:
		result += ' ' + source

	result += ' -o' + executable

	for lib in libs:
		result += ' -l'+lib

	print os.system(result)

compile(["UdpEchoServer.c",],"udpserver",["pthread"])
compile(["TcpMidServer.c","UdpSend.c",],"midserver",["pthread"])
compile(["TcpEchoClient.c",],"tcpclient",["pthread"])

def run_udp_serv():
	os.system("./udpserver")
	print "udp server started"


# run udpserver
udp_serv_thr = threading.Thread(name="udpserv",target=run_udp_serv)
udp_serv_thr.setDaemon(True)
udp_serv_thr.start()

#open file for writing
f = open("statistics.txt","w")

times = TOTALDATASIZE * 1024 * 1024 / BUFFSIZE

listlen = len(LOSDUPRATES)
for i in range(0,listlen):
	f.write("Sample " + str(i+1) + ':\n')
	loserate = LOSDUPRATES[i][0]
	duprate = LOSDUPRATES[i][1]

	f.write("Lose Rate: " + str(loserate) + '\tDup Rate: ' + str(duprate) + '\n')
	f.write("Data Size: " + str(TOTALDATASIZE) + 'MegaBytes\n')

	#run midserver
	def run_mid_serv(loserate,duprate):
		os.system("./midserver " + str(loserate) + ' ' + str(duprate)+ '' + str(20002+i))

	mid_serv_thr = threading.Thread(name="midserv",target=run_mid_serv,args=(loserate,duprate))
	mid_serv_thr.setDaemon(True)
	mid_serv_thr.start()

	#run client
	tick1 = time.time()
	os.system("./tcpclient " + str(times) + (20002+i))
	tick2 = time.time()
	duration = tick2 - tick1
	
	f.write("Duration: " + str(duration) + '\n')
	f.write("---------------------------------\n")

f.close()
