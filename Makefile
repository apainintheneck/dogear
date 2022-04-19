#
# **************************************************************
# *                Simple C++ Makefile Template                *
# *                                                            *
# * Author: Arash Partow (2003)                                *
# * URL: http://www.partow.net/programming/makefile/index.html *
# *                                                            *
# * Copyright notice:                                          *
# * Free use of this C++ Makefile template is permitted under  *
# * the guidelines and in accordance with the the MIT License  *
# * http://www.opensource.org/licenses/MIT                     *
# *                                                            *
# **************************************************************
#

# Modified version of the above mentioned makefile template.

CXX      := -g++
CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror -std=c++17
LDFLAGS  := -L/usr/lib -lstdc++ -lm
TARGET   := dogear
BIN_DIR  := bin
BIN 	 := $(BIN_DIR)/$(TARGET)
SRC 	 := dogear/dogear.cpp

.PHONY: all release clean test info install

all: $(BIN)

$(BIN): $(SRC)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN) $(LDFLAGS)

release: CXXFLAGS += -O2
release: all

clean:
	@rm -vrf $(BIN_DIR)

test: all
	bundle install && bundle exec rspec

info:
	@echo "[*] Binary:	    ${BIN}     "
	@echo "[*] Sources:     ${SRC}    "

install: release
	@echo "[*] To install move the binary into your path:"
	@echo "[*]    cp $(BIN) [PATH]"
	@echo "[*]"
	@echo "[*] If you are using Bash or Zsh as your default shell,"
	@echo "[*] add the contents of flipto.sh to your profile:"
	@echo "[*]    cat flipto.sh >> [PROFILE]"
	@echo "[*] If you are using Fish as your default shell,"
	@echo "[*] add the contents of flipto.fish to your functions folder:"
	@echo "[*]    cp flipto.fish ~/.config/fish/functions/flipto.fish"