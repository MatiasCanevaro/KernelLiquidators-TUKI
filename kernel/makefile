include settings.mk

################################################################################

filename = $(1).out

define compile_bin
gcc $(CFLAGS) -o "$@" $^ $(IDIRS:%=-I%) $(LIBDIRS:%=-L%) $(RUNDIRS:%=-Wl,-rpath,%) $(LIBS:%=-l%)
endef

define compile_objs
gcc $(CFLAGS) -c -o "$@" $< $(IDIRS:%=-I%)
endef

################################################################################

# Check if tests folder exists
ifneq ($(wildcard tests/*),)
TESTS_ENABLED=1
endif

# Set prerrequisites
SRCS_C += $(shell find src/ -iname "*.c")
SRCS_H += $(shell find include/ -iname "*.h")
DEPS = $(foreach SHL,$(SHARED_LIBPATHS),$(SHL:%=%/bin/lib$(notdir $(SHL)).so)) \
	$(foreach STL,$(STATIC_LIBPATHS),$(STL:%=%/bin/lib$(notdir $(STL)).a))

# Set test prerrequisites
ifeq ($(TESTS_ENABLED),1)
TESTS_C += $(shell find tests/ -iname "*.c")
TESTS_H += $(shell find tests/ -iname "*.h")
endif

# Set header paths to (-I)nclude
IDIRS += $(addsuffix /include,$(SHARED_LIBPATHS) $(STATIC_LIBPATHS) .)

# Set library paths to (-L)ook
LIBDIRS = $(addsuffix /bin,$(SHARED_LIBPATHS) $(STATIC_LIBPATHS))

# Set shared library paths to be found in runtime (-rpath)
RUNDIRS = $(SHARED_LIBPATHS:%=$(shell cd . && pwd)/%/bin)

# Set intermediate objects
OBJS = $(patsubst src/%.c,obj/%.o,$(SRCS_C))

# Set test intermediate objects
ifeq ($(TESTS_ENABLED),1)
TEST_OBJS = $(TESTS_C) $(filter-out $(TEST_EXCLUDE), $(SRCS_C))
endif

# Set binary targets
BIN = bin/$(call filename,$(shell cd . && pwd | xargs basename))

# Set test binary targets
ifeq ($(TESTS_ENABLED),1)
TEST = bin/$(shell cd . && pwd | xargs basename)_tests.out
endif

.PHONY: all
all: CFLAGS = $(CDEBUG)
all: $(BIN) $(TEST)

.PHONY: release
release: CFLAGS = $(CRELEASE)
release: clean $(BIN) $(TEST)

.PHONY: clean
clean:
	-rm -rfv obj bin

.PHONY: watch
watch:
	@test $(shell which entr) || entr
	while sleep 0.1; do \
		find src/ include/ | entr -d make all --no-print-directory; \
	done

$(BIN): $(OBJS) | $(dir $(BIN))
	$(call compile_bin)

obj/%.o: src/%.c $(SRCS_H) $(DEPS) | $(dir $(OBJS))
	$(call compile_objs)

ifeq ($(TESTS_ENABLED),1)
$(TEST): $(TEST_OBJS) | $(dir $(TEST))
	gcc $(CFLAGS) -o "$@" $^ $(IDIRS:%=-I%) $(LIBDIRS:%=-L%) $(RUNDIRS:%=-Wl,-rpath,%) $(LIBS:%=-l%) -lcspecs
endif

.SECONDEXPANSION:
$(DEPS): $$(shell find $$(patsubst %bin/,%src/,$$(dir $$@)) -iname "*.c") \
	$$(shell find $$(patsubst %bin/,%include/,$$(dir $$@)) -iname "*.h")
	make --no-print-directory -C $(patsubst %bin/,%,$(dir $@))

$(sort $(dir $(BIN) $(OBJS))):
	mkdir -pv $@


################################################################################
.PHONY: run
run: $(BIN)
	./$(BIN) $(ARGS)

.PHONY: start
start: $(BIN)
	valgrind --tool=none ./$(BIN) $(ARGS)

.PHONY: daemon
daemon:
	@test $(shell which entr) || entr
	while sleep 0.1; do \
		find src/ include | entr -d make start --no-print-directory; \
	done

.PHONY: memcheck
memcheck: $(BIN)
	valgrind --leak-check=full $(MEMCHECK_FLAGS) ./$(BIN) $(ARGS)

.PHONY: helgrind
helgrind: $(BIN)
	valgrind --tool=helgrind $(HELGRIND_FLAGS) ./$(BIN) $(ARGS)


################################################################################

ifeq ($(TESTS_ENABLED),1)
.PHONY: test
stest: all
	valgrind --tool=none ./$(TEST)

.PHONY: test-daemon
test-daemon:
	@test $(shell which entr) || entr
	while sleep 0.1; do \
		find tests/ src/ | entr -d make test --no-print-directory; \
	done

.PHONY: test-memcheck
test-memcheck: all
	valgrind --leak-check=full $(MEMCHECK_FLAGS) ./$(TEST)

.PHONY: test-helgrind
test-helgrind: all
	valgrind --tool=helgrind $(HELGRIND_FLAGS) ./$(TEST)
endif
