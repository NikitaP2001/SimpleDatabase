SHELL=cmd.exe

TARGET=test.exe

RC_DIR = ./
OBJ_DIR = ./
INC_DIR = $(realpath .)
TEST_DIR = $(realpath ./tests)

CPP_SRC = $(wildcard *.cxx)
ASM_SRC += $(wildcard *.asm)
OBJECTS = $(CPP_SRC:.cxx=.o)
OBJECTS += $(ASM_SRC:.asm=.o)
TEST_SRC = $(wildcard $(TEST_DIR)/*.cxx)

# expect gnu c++
CC=@c++
AS=@ml64.exe 2>NUL
LD=@c++
RM=@-del /q 2>NUL

CCFLAGS = -c
ACFLAGS = /c /Cp /I B:\masm32\include64 
DBG_CCFLAGS = -DDEBUG -g
LDLIBS=
TESTLIB = $(LDLIBS)
TESTLIB += -lgtest -lgtest_main
ALDLIBS = /LIBPATH:B:\masm32\lib64\
RLS_CCFLAGS = -s -fdata-sections -ffunction-sections -O3
RLS_LDFLAGS = -Wl,--gc-sections,-s

%.o: %.cxx
	@echo CC $<
	$(CC) $(CCFLAGS) -o $@ $<

%.o: %.asm
	$(AS) $(ACFLAGS) $< /Fo $@

all: CCFLAGS += $(DBG_CCFLAGS)
all: $(TARGET) 

release: CCFLAGS += $(RLS_CCFLAGS)
release: LDFLAGS += $(RLS_LDFLAGS)
release: $(TARGET)


$(TARGET): $(OBJECTS)
	@echo LD $@
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)


TEST := $(TEST_SRC:.cxx=.exe)
TEST_OBJ += $(filter-out main.o, $(OBJECTS))

.PHONY: test
test: $(TEST_OBJ) $(TEST_SRC)
	@echo LD $(TEST)
	$(LD) $(LDFLAGS) -I $(INC_DIR) -o $(TEST) $^ $(TESTLIB)

runtest: $(TEST)
	$(foreach test,$^,$(test) ;)

.PHONY: clean
clean:
	$(RM) $(OBJECTS)	
	$(RM) $(subst /,\,$(TEST))