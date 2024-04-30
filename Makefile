TARGET_EXEC ?= jobExecutorServer

BUILD_DIR ?= build
SRC_DIRS ?= .

CC := $(shell \
	for compiler in cc gcc clang; \
	do \
		if command -v $$compiler; then \
			break;\
		fi \
	done \
)

WARNINGS := -Wall -Wextra
DEBUG := # -fsanitize=thread

OPTIMIZE := -O2
# If the code is underperforming, try recompiling it with some of these flags:
# -falign-functions=32 -falign-loops=32 -march=native -mtune=native

LFLAGS := # -Wl,--gc-sections
LIBS := -lm

SRCS := $(shell find $(SRC_DIRS) -name *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP

$(TARGET_EXEC): $(OBJS) Makefile
	$(CC) $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(LFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)

# c source
$(BUILD_DIR)/%.c.o: %.c Makefile
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(WARNINGS) $(DEBUG) $(OPTIMIZE) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR) $(TARGET_EXEC)

-include $(DEPS)

MKDIR_P ?= mkdir -p
