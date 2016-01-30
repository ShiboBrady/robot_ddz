TARGET = libtools.so

TARGET_FILE = bin

SOURCE_FILE = src

INCLUDE_FILE = include

OBJ_FILE = obj

CC = g++

LIB = -lprotobuf\
      -levent\
      -llog4cplus -lpthread

STARD=-std=c++0x

VPATH = $(SOURCE_FILE)

OBJC = $(subst .cpp,.o, $(addprefix $(OBJ_FILE), $(subst $(SOURCE_FILE), , $(wildcard $(SOURCE_FILE)/*.cpp))))

MAKE_DIR := $(shell mkdir -p $(OBJ_FILE) $(TARGET_FILE))

.PHONY: clean show

$(TARGET_FILE)/$(TARGET):$(OBJC)
	$(CC) -g -shared -o $@ -I$(INCLUDE_FILE) $^ $(LIB)

$(OBJ_FILE)/%.o:%.cpp
	$(CC) -g -fPIC -c -I$(INCLUDE_FILE) -o $@ $< ${STARD}

clean:
	rm -rf $(OBJ_FILE) $(TARGET_FILE)

show:
	@echo $(OBJC)
