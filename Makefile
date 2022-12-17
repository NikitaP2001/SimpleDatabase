BINPATH = $(realpath .)
OBJ_DIR = $(realpath .)
SRC_DIR = $(realpath .)
INC_DIR = $(realpath .)
TEST_DIR = $(realpath ./tests)

TARGET=$(BINPATH)/test.exe

CPP_SRC =  $(wildcard $(SRC_DIR)/*.cxx)
OBJECTS = $(subst $(SRC_DIR)/,$(OBJ_DIR)/,$(CPP_SRC:.cxx=.o))
TEST_SRC = $(wildcard $(TEST_DIR)/*.cxx)

# expect gnu c++
CC=@c++
LD=@c++
RM=@-del /q 2>NUL

CCFLAGS = -c -I $(INC_DIR) --std=c++20
DBG_CCFLAGS = -DDEBUG -g
TESTLIB = $(LDLIBS)
TESTLIB += -lgtest -lgtest_main -static
RLS_CCFLAGS = -s -fdata-sections -ffunction-sections -O3
RLS_LDFLAGS = -Wl,--gc-sections,-s

export

%.o: %.cxx
	$(info CC $<)
	$(CC) $(CCFLAGS) -o $@ $<

all: CCFLAGS += $(DBG_CCFLAGS)
all:
	@$(MAKE) --no-print-directory -C cli
	@$(MAKE) --no-print-directory -C server
	@$(MAKE) --no-print-directory -C client

release: CCFLAGS += $(RLS_CCFLAGS)
release: LDFLAGS += $(RLS_LDFLAGS)
release:
	@$(MAKE) --no-print-directory -C cli release
	@$(MAKE) --no-print-directory -C server release
	@$(MAKE) --no-print-directory -C client release

TEST := $(TEST_SRC:.cxx=.exe)
TEST_OBJ += $(filter-out main.o, $(OBJECTS))

.PHONY: test
test: $(TEST_OBJ) $(TEST_SRC)
	$(info LD $(TEST))
	$(LD) $(LDFLAGS) -I $(INC_DIR) -o $(TEST) $^ $(TESTLIB)

runtest: $(TEST)
	$(foreach test,$^,$(test) ;)

.PHONY: clean
clean:
	$(RM) $(subst /,\,$(OBJECTS))
	@$(RM) $(subst /,\,$(TEST))
	@$(MAKE) --no-print-directory -C server clean
	@$(MAKE) --no-print-directory -C cli clean
	@$(MAKE) --no-print-directory -C client clean
