CC = gcc
CFLAGS = -g
DEP_OPT = -MM -MP
SRC_DIR = src
BUILD_DIR = .build
SOURCES = $(foreach d, $(SRC_DIR), $(wildcard $(addprefix $(d)/*, .c)))
DEPS = $(subst $(SRC_DIR), $(BUILD_DIR), $(SOURCES:.c=.d))
OBJS = $(subst $(SRC_DIR), $(BUILD_DIR), $(SOURCES:.c=.o))
PROGRAM = tictactoe

# Delete the default suffixes
.SUFFIXES:

$(PROGRAM): $(OBJS)
	@$(CC) $? -o $@

.build:
	@mkdir .build

# Generate dependence files
$(BUILD_DIR)/%.d: $(SRC_DIR)/%.c $(BUILD_DIR)
	@$(CC) $(DEP_OPT) $< | sed -E 's;^(.*)\.o:;$(BUILD_DIR)/\1.o:;' > $@

# Generate objects files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

.PHONY: clean build try run

clean:
	@rm -rf .build
	@rm $(PROGRAM)

build: $(PROGRAM)

run: build
	@./$(PROGRAM)

try:
	$(info $(OBJS))

