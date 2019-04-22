CC      ?= gcc
CXX     ?= g++
CFLAGS  ?= -g -O3 -Wall -std=c++0x -pthread
LIBS    = -lm -lpthread
LDFLAGS ?= -g

OBJECTS = 	BPTC19696.o Conf.o GPS.o TCPSocket.o DTMF.o APRSWriter.o APRSWriterThread.o CRC.o \
			DelayBuffer.cpp DMRLookup.o DMREMB.o DMREmbeddedData.o APRSReader.o \
			DMRFullLC.o DMRNetwork.o DMRLC.o DMRSlotType.o DMRData.o Golay2087.o Golay24128.o \
			Hamming.o Log.o ModeConv.o Mutex.o QR1676.o Reflectors.o RS129.o StopWatch.o Sync.o \
			SHA256.o Thread.o Timer.o UDPSocket.o Utils.o WiresX.o YSFConvolution.o YSFFICH.o \
			YSFNetwork.o YSF2DMR.o YSFPayload.o Storage.o

all:		YSF2DMR

YSF2DMR:	$(OBJECTS)
		$(CXX) $(OBJECTS) $(CFLAGS) $(LIBS) -o YSF2DMR

%.o: %.cpp
		$(CXX) $(CFLAGS) -c -o $@ $<

clean:
		$(RM) YSF2DMR *.o *.d *.bak *~
install:
		mv /usr/local/bin/YSF2DMR /usr/local/bin/YSF2DMR.first
		cp ./YSF2DMR /usr/local/bin/YSF2DMR
		apt-get install dos2unix
		dos2unix ./contrib/*
		cp ./contrib/tg_generate.py /usr/local/bin
		cp ./contrib/config_clean.zip /usr/local/bin
		cp ./contrib/configure.php /var/www/dashboard/admin
		cp ./contrib/spanish_es.php /var/www/dashboard/lang
		cp ./contrib/TGList.txt /usr/local/etc
		cp ./contrib/updateDMRIDs.sh /usr/local/bin
		cp ./contrib/updateTGList.sh /usr/local/bin
		cp ./contrib/ysf2dmr /etc
		cp ./contrib/rc.local /etc
		cp ./contrib/crontab /etc
		cp ./contrib/beacon.amb /usr/local/etc
		mv /usr/local/bin/MMDVMHost /usr/local/bin/MMDVMHost.first
		cp ./contrib/MMDVMHost /usr/local/bin
