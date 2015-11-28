TARGET = robot_client

CC = g++

LIB = -lprotobuf\
      -levent\
	  -llog4cplus -lpthread

OBJC = $(subst .cpp,.o,$(wildcard *.cpp)) 

.PHONY: clean

$(TARGET):$(OBJC)
	$(CC) -o $@ $^ $(LIB)

%.o:%.cpp
	$(CC) -c -o $@ $<

clean:
	rm *.o $(TARGET)
