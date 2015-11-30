TARGET = robot_client

TARGET_FILE = bin

SOURCE_FILE = src

INCLUDE_FILE = include

OBJ_FILE = obj

CC = g++

LIB = -lprotobuf\
      -levent\
	  -llog4cplus -lpthread

VPATH = $(SOURCE_FILE)

OBJC = $(subst .cpp,.o, $(addprefix $(OBJ_FILE), $(subst $(SOURCE_FILE), , $(wildcard $(SOURCE_FILE)/*.cpp))))

MAKE_DIR := $(shell mkdir -p $(OBJ_FILE) $(TARGET_FILE))

.PHONY: clean show

$(TARGET_FILE)/$(TARGET):$(OBJC)
	$(CC) -o $@ -I$(INCLUDE_FILE) $^ $(LIB)

$(OBJ_FILE)/%.o:%.cpp
	$(CC) -c -I$(INCLUDE_FILE) -o $@ $<

clean:
	rm -rf $(OBJ_FILE) $(TARGET_FILE)

show:
	@echo $(OBJC)
