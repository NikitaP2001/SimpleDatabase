SHELL=cmd.exe

BINPATH = $(realpath .)
OBJ_DIR = $(realpath .)
SRC_DIR = $(realpath .)
INC_DIR = $(realpath .)
TEST_DIR = $(realpath ./tests)

TARGET=$(BINPATH)/test.exe

CPP_SRC =  $(wildcard $(SRC_DIR)/*.cxx)
ASM_SRC += $(wildcard *.asm)
OBJECTS = $(subst $(SRC_DIR)/,$(OBJ_DIR)/,$(CPP_SRC:.cxx=.o))
OBJECTS += $(ASM_SRC:.asm=.o)
TEST_SRC = $(wildcard $(TEST_DIR)/*.cxx)

# expect gnu c++
CC=@c++
AS=@ml64.exe 2>NUL
LD=@c++
RM=@-del /q 2>NUL

CCFLAGS = -c -I $(INC_DIR)
ACFLAGS = /c /Cp /I B:\masm32\include64 
DBG_CCFLAGS = -DDEBUG -g
TESTLIB = $(LDLIBS)
TESTLIB += -lgtest -lgtest_main
ALDLIBS = /LIBPATH:B:\masm32\lib64\
RLS_CCFLAGS = -s -fdata-sections -ffunction-sections -O3
RLS_LDFLAGS = -Wl,--gc-sections,-s

export

%.o: %.cxx
	@echo CC $<
	$(CC) $(CCFLAGS) -o $@ $<

%.o: %.asm
	$(AS) $(ACFLAGS) $< /Fo $@

all: CCFLAGS += $(DBG_CCFLAGS)
all:
	$(MAKE) -C cli
	$(MAKE) -C server

release: CCFLAGS += $(RLS_CCFLAGS)
release: LDFLAGS += $(RLS_LDFLAGS)
release:
	$(MAKE) -C cli release
	$(MAKE) -C server release

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
	$(MAKE) -C server clean
	$(MAKE) -C cli clean