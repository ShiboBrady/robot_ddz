TARGET = robot_server

CC = g++

LIB = -lprotobuf\
      -levent

OBJC = $(subst .cpp,.o,$(wildcard *.cpp)) 

.PHONY: clean

$(TARGET):$(OBJC)
	$(CC) -o $@ $^ $(LIB)

%.o:%.cpp
	$(CC) -c -o $@ $<

clean:
	rm *.o $(TARGET)
