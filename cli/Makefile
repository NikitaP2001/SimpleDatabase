SHELL=cmd.exe

TARGET=$(BINPATH)/cli.exe

CPP_SRC = $(wildcard *.cxx)
OBJECTS += $(CPP_SRC:.cxx=.o)


%.o: %.cxx
	@echo CC $<
	$(CC) $(CCFLAGS) -o $@ $<

all: CCFLAGS += $(DBG_CCFLAGS)
all: $(TARGET)

release: CCFLAGS += $(RLS_CCFLAGS)
release: LDFLAGS += $(RLS_LDFLAGS)
release: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo LD $@
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(subst /,\,$(OBJECTS))