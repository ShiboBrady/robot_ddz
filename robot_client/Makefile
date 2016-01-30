TARGET = robot_client

TARGET_FILE = bin

SOURCE_FILE = src

INCLUDE_FILE = -I./include\
			   -I./protocal/include
			   #-I./tools/include

OBJ_FILE = obj

CC = g++

LIB = -lprotobuf\
      -levent\
      -llog4cplus -lpthread\
	  -lprotocal
	  #-ltools

LIB_PATH = -L./protocal/bin
		   #-L./tools/bin

STARD=-std=c++0x

VPATH = $(SOURCE_FILE)

OBJC = $(subst .cpp,.o, $(addprefix $(OBJ_FILE), $(subst $(SOURCE_FILE), , $(wildcard $(SOURCE_FILE)/*.cpp))))

MAKE_DIR := $(shell mkdir -p $(OBJ_FILE) $(TARGET_FILE))

.PHONY: all clean cleanall show

all : submake $(TARGET_FILE)/$(TARGET)

submake:
	make -C ./protocal/
	#make -C ./tools/

$(TARGET_FILE)/$(TARGET):$(OBJC)
	$(CC) -g -o $@ $(INCLUDE_FILE) $^ $(LIB_PATH) $(LIB)
	cp -rf ./protocal/bin/* $(TARGET_FILE)
	#cp -rf ./tools/bin/* $(TARGET_FILE)

$(OBJ_FILE)/%.o:%.cpp
	$(CC) -g -c $(INCLUDE_FILE) -o $@ $< ${STARD}

clean:
	rm -rf $(OBJ_FILE) $(TARGET_FILE)

cleanall:
	make -C ./protocal/ clean
	#make -C ./tools/ clean
	rm -rf $(OBJ_FILE) $(TARGET_FILE)

show:
	@echo $(OBJC)
