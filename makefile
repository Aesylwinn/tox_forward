CC=g++
CFLAGS= -g -Wall --std=c++11 
LFLAGS= -ltoxcore -lsodium

EXEC=tox_forwarder

OBJDIR=obj
SRCDIR=src
SRCS=main toxwrapper intermediary

OBJS=$(patsubst %, $(OBJDIR)/%.o, $(SRCS))
DEPS=$(patsubst %, $(OBJDIR)/%.d, $(SRCS))

$(EXEC): $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(EXEC)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@


.PHONY: clean doc test
clean:
	$(RM) $(OBJS) $(DEPS) $(EXEC)

doc:
	doxygen doxyfile

test:


-include $(DEPS)
