#  #	
CXXFLAGS = -Wall -O3 -I.
CXX=g++
CCFLAGS= -Wall -O3 -I.
CC=g++
LDFLAGS= $(CXXFLAGS) -L.
TARGETS= swad_ircd

all: $(TARGETS)

swad_ircd : swad_ircd.o sockets_cpp.o text_protocol.o sirc.o sirc_message.o sirc_handles.o sirc_channel.o

clean:
	rm -f *.o $(TARGETS)
