SRC_DIR = ./src
INC_DIR = ./include

MODULES = ./modules

CFLAGS = -Wall -I$(INC_DIR) \
               -I$(MODULES)/vector \
               -I$(MODULES)/map
CC = gcc

OBJS = $(SRC_DIR)/ipli.o \
       $(SRC_DIR)/scanner.o \
       $(SRC_DIR)/parser.o \
       $(SRC_DIR)/interpreter.o \
       $(SRC_DIR)/expr.o \
       $(SRC_DIR)/stmt.o \
       $(MODULES)/vector/vector.o \
       $(MODULES)/map/map.o

EXEC = ipli

# The @ character is used to silence make's output

$(EXEC): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)
	@rm -f $(OBJS)

.SILENT: $(OBJS) # Silence implicit rule output
.PHONY: clean

clean:
	@echo "Cleaning up ..."
	@rm -f $(OBJS) $(EXEC)
