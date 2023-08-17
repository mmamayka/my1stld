
BUILD_DIR := build
JOBS ?= 4

.PHONY: test
test: build
	make -C $(BUILD_DIR) -j $(JOBS) test

.PHONY: build
build:
	cmake -B $(BUILD_DIR) -S .
	make -C $(BUILD_DIR) -j $(JOBS)
