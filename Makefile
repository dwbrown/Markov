OBJECTS = markov.o cmd_line.o driver.o instr.o misc.o tagged_char.o \
          work.o work_data.o work_status.o
TARGET  = markov
CC      = g++
DEBUG   = -g
CCFLAGS = -Wall -c $(DEBUG)
LFLAGS  = -Wall $(DEBUG)

markov : $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o markov

markov.o : misc.h cmd_line.h instr.h driver.h work_status.h
	$(CC) $(CCFLAGS) markov.cpp

cmd_line.o : cmd_line.cpp cmd_line.h misc.h
	$(CC) $(CCFLAGS) cmd_line.cpp

driver.o : driver.cpp driver.h misc.h cmd_line.h tagged_char.h instr.h \
           work_status.h work.h
	$(CC) $(CCFLAGS) driver.cpp

instr.o : instr.cpp instr.h tagged_char.h misc.h
	$(CC) $(CCFLAGS) instr.cpp

misc.o : misc.cpp misc.h
	$(CC) $(CCFLAGS) misc.cpp

tagged_char.o : tagged_char.cpp tagged_char.h misc.h
	$(CC) $(CCFLAGS) tagged_char.cpp

work.o : work.cpp work.h tagged_char.h work_status.h instr.h
	$(CC) $(CCFLAGS) work.cpp

work_data.o : work_data.cpp work_data.h tagged_char.h misc.h
	$(CC) $(CCFLAGS) work_data.cpp

work_status.o : work_status.h misc.h
	$(CC) $(CCFLAGS) work_status.cpp

clean:
	\rm -f $(OBJECTS) markov

