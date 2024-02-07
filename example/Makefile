CC=g++
CFLAGS=-I/home/rapit/Fast-DDS/install/fastrtps/include -I/usr/include/modbus
LDFLAGS=-L/home/rapit/Fast-DDS/install/fastrtps/lib   -lfastcdr -lfastrtps -lmodbus

PLCDriverApp: main.cpp ResponsePublisher.cpp StatePublisher.cpp QuerySubscriber.cpp
	$(CC) $(CFLAGS) -o PLCDriverApp main.cpp ResponsePublisher.cpp StatePublisher.cpp QuerySubscriber.cpp $(LDFLAGS)

clean:
	rm PLCDriverApp
