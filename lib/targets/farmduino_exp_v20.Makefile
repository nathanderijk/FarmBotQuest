TARGET_farmduino_exp_v20_BUILD_DIR := $(BUILD_DIR)/farmduino_exp_v20
TARGET_farmduino_exp_v20_HEX := $(BIN_DIR)/farmduino_exp_v20.hex

TARGET_farmduino_exp_v20_OBJ := $(patsubst $(FBARDUINO_FIRMWARE_SRC_DIR)/%,$(TARGET_farmduino_exp_v20_BUILD_DIR)/%,$(CXX_OBJ))

$(TARGET_farmduino_exp_v20_HEX): $(TARGET_farmduino_exp_v20_BUILD_DIR) $(TARGET_farmduino_exp_v20_BUILD_DIR)/farmduino_exp_v20.eep $(TARGET_farmduino_exp_v20_BUILD_DIR)/farmduino_exp_v20.elf
	$(OBJ_COPY) -O ihex -R .eeprom  $(TARGET_farmduino_exp_v20_BUILD_DIR)/farmduino_exp_v20.elf $@

$(TARGET_farmduino_exp_v20_BUILD_DIR)/farmduino_exp_v20.eep: $(TARGET_farmduino_exp_v20_BUILD_DIR)/farmduino_exp_v20.elf
	$(OBJ_COPY) -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0  $< $@

$(TARGET_farmduino_exp_v20_BUILD_DIR)/farmduino_exp_v20.elf: $(TARGET_farmduino_exp_v20_OBJ)
	$(CC) -w -Os -g -flto -fuse-linker-plugin -Wl,--gc-sections,--relax -mmcu=atmega2560 -o $@ $(TARGET_farmduino_exp_v20_OBJ) $(DEPS_OBJ) $(DEP_CORE_LDFLAGS)

$(TARGET_farmduino_exp_v20_BUILD_DIR)/%.o: $(FBARDUINO_FIRMWARE_SRC_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) -DFARMBOT_BOARD_ID=2 $(DEPS_CFLAGS) $< -o $@

$(TARGET_farmduino_exp_v20_BUILD_DIR):
	$(MKDIR_P) $(TARGET_farmduino_exp_v20_BUILD_DIR)

target_farmduino_exp_v20: $(TARGET_farmduino_exp_v20_HEX)

target_farmduino_exp_v20_clean:
	$(RM) -r $(TARGET_farmduino_exp_v20_BUILD_DIR)
	$(RM) $(TARGET_farmduino_exp_v20_HEX)
