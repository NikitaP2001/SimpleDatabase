TARGET=$(BINPATH)/client.exe

CPP_SRC = $(wildcard *.cxx)
OBJECTS += $(CPP_SRC:.cxx=.o)

LDLIBS += -lws2_32

%.o: %.cxx
	$(info CC $<)
	$(CC) $(CCFLAGS) -o $@ $<

all: CCFLAGS += $(DBG_CCFLAGS)
all: $(TARGET)

release: CCFLAGS += $(RLS_CCFLAGS)
release: LDFLAGS += $(RLS_LDFLAGS)
release: $(TARGET)

$(TARGET): $(OBJECTS)
	$(info LD $@)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(subst /,\,$(OBJECTS))
