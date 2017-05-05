CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g -pthread
LDFLAGS=-g
LDLIBS=-ludev
TARGET=k81x_fkeys

SRCS=k81x.cpp k81x_fkeys.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: $(TARGET)

k81x_fkeys: $(OBJS)
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS) 

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) *~ .depend
	$(RM) $(TARGET)

include .depend
