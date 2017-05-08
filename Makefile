CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g -pthread
LDFLAGS=-g
LDLIBS=-ludev
TARGET=k81x-fkeys

SRCS=k81x.cpp k81x-fkeys.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
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

install: $(TARGET)
	install -m 0755 $(TARGET) /usr/local/bin

include .depend
