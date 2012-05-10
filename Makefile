#  #	
CXXFLAGS = -Wall -O3 -I.
CXX=g++
CCFLAGS= -Wall -O3 -I.
CC=g++
LDFLAGS= $(CXXFLAGS) -L.
TARGETS= swad_ircd

INSTALL_PATH?=/usr/local
INSTALL=install -c

all: $(TARGETS)

swad_ircd : swad_ircd.o sockets_cpp.o text_protocol.o sirc.o sirc_message.o sirc_handles.o sirc_channel.o

install: 
	$(INSTALL) $(TARGETS) $(INSTALL_PATH)/bin
	$(INSTALL) init.d/swad_ircd /etc/init.d/
clean:
	rm -f *.o $(TARGETS)
