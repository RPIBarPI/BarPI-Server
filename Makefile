.SILENT: clean
# List of files
C_SRCS		= error.cpp main.cpp socket.cpp sql.cpp user.cpp utilities.cpp version.cpp
C_OBJS		= error.o main.o socket.o sql.o user.o utilities.o version.o
C_HEADERS	= error.h main.h socket.h sql.h user.h utilities.h version.h
OBJS = $(C_OBJS)

# Desired name of the executable
EXE		= BarPI

# Compiler and loader commands and flags
CC		= g++
CC_FLAGS	= -g -c
LD_FLAGS	= -g
LIBS		= -pthread -lmysqlclient
FILEPATH	= ./BarPI

# The first, and hence default, target is the executable
$(EXE): $(OBJS)
	@echo "Linking object modules..."
	$(CC) $(LD_FLAGS) $(OBJS) $(LIBS) -o $(EXE)

# Force recompilation of C objects if headers change
${C_OBJS}: ${C_HEADERS}

# Compile .c files to .o files
%.o: %.c
	@echo "Compiling..."
	$(CC) $(CC_FLAGS) $<

# Clean up the directory
clean:
	@echo "Cleaning project directory."
	rm -f *.o *~ $(EXE)
run:
	./$(EXE)
valgrind:
	valgrind --leak-check=full --show-reachable=yes ./$(EXE)
debug:
	gdb ./$(EXE)