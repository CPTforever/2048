CC      = clang++
CFLAGS  = -Wall -Wextra -Wpedantic -Wextra
LFLAGS  =
TARGETS = 2048  
MODULES = 
OBJECTS = $(patsubst %,%.o,$(MODULES))
TOBJECTS= $(patsubst %,%.o,$(TARGETS))      # target objects

.PHONY: all debug clean tidy

all: $(TARGETS)

debug: CFLAGS += -g
debug: $(TARGETS)

$(TARGETS): %: %.o $(OBJECTS)
	$(CC) -o $@ $^ $(LFLAGS) $(CFLAGS)

%.o: %.cc %.h
	$(CC) -c $< $(CFLAGS)

$(TOBJECTS): %.o: %.cc
	$(CC) -c $< $(CFLAGS)

clean: tidy
	$(RM) $(TARGETS)

tidy:
	$(RM) $(OBJECTS) $(patsubst %,%.o,$(TARGETS))
