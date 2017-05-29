.SUFFIXEX: .cpp .o

CC = g++

SRCS = qqserver.cpp\
		pub.cpp

OBJS = $(SRCS: .cpp=.o)

EXEC = qqserverd


all:$(OBJS)
	$(CC) -o $(EXEC) $(OBJS) 

	@echo '^^^^^^^^^^^^^^^^^^^^^OK^^^^^^^^^^'
	
.cpp .o:
	$(CC) -Wall -g -o $@ -c $<
clean:
	rm -f $(OBJS)
	rm -f core*
