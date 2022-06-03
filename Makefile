CC = gcc

#### Check for memory leaks (at exit) ####
# $ leaks --list --atExit -- ./bin/program

#### Check for leaks (at runtime) ####
# $ export MallocStackLogging=1
# Set breakpoint in debugger
# Run debugger
# $ ps aux | grep "bin/program"
# $ leaks <proccess_id>
# Step though debugger and check for leaks each step
# $ export MallocStackLogging=0

CFLAGS = -g -Wall -Werror -I include/
LDFLAGS = -lcurl -lcjson

SRC = ./src
OBJ = ./obj
HEADER = ./include
BIN_DIR = ./bin
SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
HDRS = $(wildcard $(HEADER)/*.h)
BIN = $(addprefix $(BIN_DIR)/, program)

all: $(BIN)

$(OBJ):
	mkdir -p $(BIN_DIR)
	mkdir -p $@

$(BIN): $(OBJS) $(OBJ)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.c $(OBJ) 
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	$(RM) -r $(OBJ)
	$(RM) -r $(BIN_DIR)
