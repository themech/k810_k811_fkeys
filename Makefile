CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g -pthread
LDFLAGS=-g
LDLIBS=-ludev
TARGET=k81x-fkeys

SRCS=src/k81x.cpp src/k81x-fkeys.cpp
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
	install -D -m 0755 $(TARGET) $(DESTDIR)/opt/k81x/$(TARGET)
	install -D -m 0755 contrib/k81x.sh $(DESTDIR)/opt/k81x/k81x.sh
	install -D -m 0644 contrib/00-k81x.rules $(DESTDIR)/etc/udev/rules.d/00-k81x.rules

include .depend
