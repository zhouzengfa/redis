
TARGET=test
#LIB=-lhiredis
LIB= ../hiredis/libhiredis.a
LIB += ../jsoncpp/libjsoncpp.a

INCLUDE= -I ../hiredis/
INCLUDE += -I ../jsoncpp/json/

#LIB_DIR= -L ../hiredis/
SRCFILE=$(wildcard *.cpp)

all:
	#@g++ $(SRCFILE) $(INCLUDE) -o $(TARGET) $(LIB_DIR) $(LIB)
	@g++ -g $(SRCFILE) $(INCLUDE) -o $(TARGET) $(LIB)
.PHONY:clean
clean:
	@rm -rf $(TARGET)
