# Someone will thank me later for this.
# Find and replace TMC2130Stepper with the name of your dependency.

DEP_TMC2130Stepper_BUILD_DIR := $(FBARDUINO_FIRMWARE_LIB_BUILD_DIR)/TMC2130Stepper
DEP_TMC2130Stepper := $(DEP_TMC2130Stepper_BUILD_DIR)/TMC2130Stepper.a
DEP_TMC2130Stepper_SRC_DIR := /home/connor/Arduino/libraries/TMC2130Stepper/src

DEP_TMC2130Stepper_CFLAGS := \
	-I$(DEP_TMC2130Stepper_SRC_DIR)

DEP_TMC2130Stepper_CFLAGS := -I$(DEP_TMC2130Stepper_SRC_DIR)
DEP_TMC2130Stepper_LDFLAGS := $(DEP_TMC2130Stepper_BUILD_DIR)/TMC2130Stepper.a -L$(DEP_TMC2130Stepper_BUILD_DIR) -lm

DEP_TMC2130Stepper_ASM_SRC := $(call rwildcard, $(DEP_TMC2130Stepper_SRC_DIR), *.S)
DEP_TMC2130Stepper_ASM_OBJ := $(DEP_TMC2130Stepper_ASM_SRC:.S=.o)

DEP_TMC2130Stepper_C_SRC   := $(call rwildcard, $(DEP_TMC2130Stepper_SRC_DIR), *.c)
DEP_TMC2130Stepper_C_OBJ   := $(DEP_TMC2130Stepper_C_SRC:.c=.o)

DEP_TMC2130Stepper_CXX_SRC := $(call rwildcard, $(DEP_TMC2130Stepper_SRC_DIR), *.cpp)
DEP_TMC2130Stepper_CXX_OBJ := $(DEP_TMC2130Stepper_CXX_SRC:.cpp=.o)

DEP_TMC2130Stepper_ALL_OBJ := $(DEP_TMC2130Stepper_ASM_OBJ) $(DEP_TMC2130Stepper_C_SRC) $(DEP_TMC2130Stepper_CXX_OBJ)

DEP_TMC2130Stepper_SRC := $(DEP_SERVO_ASM_SRC) $(DEP_SERVO_C_SRC) $(CXX_SRC)
DEP_TMC2130Stepper_OBJ := $(patsubst $(DEP_TMC2130Stepper_SRC_DIR)/%,$(DEP_TMC2130Stepper_BUILD_DIR)/%,$(DEP_TMC2130Stepper_ALL_OBJ))

DEP_TMC2130Stepper_DIRS := $(sort $(dir $(DEP_TMC2130Stepper_OBJ)))

ARDUINO_DEP_TMC2130Stepper_CXX_FLAGS_P := $(DEP_CORE_CXX_FLAGS_P) $(DEP_TMC2130Stepper_CFLAGS)

$(DEP_TMC2130Stepper): $(DEP_CORE) $(DEP_TMC2130Stepper_BUILD_DIR) $(DEP_TMC2130Stepper_OBJ)
	$(AR) rcs $(DEP_TMC2130Stepper) $(DEP_TMC2130Stepper_OBJ)

$(DEP_TMC2130Stepper_BUILD_DIR)/%.o: $(DEP_TMC2130Stepper_SRC_DIR)/%.cpp
	$(CXX) $(ARDUINO_DEP_TMC2130Stepper_CXX_FLAGS_P) $< -o $@

$(DEP_TMC2130Stepper_BUILD_DIR):
	$(MKDIR_P) $(DEP_TMC2130Stepper_DIRS)

dep_TMC2130Stepper: $(DEP_TMC2130Stepper)

dep_TMC2130Stepper_clean:
	$(RM) $(DEP_TMC2130Stepper_OBJ)
	$(RM) $(DEP_TMC2130Stepper)
