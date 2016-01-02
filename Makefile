CPPFLAGS = -O3 -DNDEBUG # -Wall -Wextra -Wshadow -Wconversion
# CPPFLAGS = -g # -Wall -Wextra -Wshadow -Wconversion
INCLUDES = -I.
PREFIX ?= /usr/lib
MACOS_SRC = jack.cpp macwrapper.cpp

all:
	clang++ -std=c++14 -fno-builtin-malloc -arch x86_64 -pipe -g $(CPPFLAGS) $(INCLUDES) -D_REENTRANT=1 -compatibility_version 1 -current_version 1 -dynamiclib $(MACOS_SRC) -o libjack.dylib -ldl -lpthread
	clang++ -DTESTRUN -std=c++14 -arch x86_64 -fno-builtin-malloc -pipe $(CPPFLAGS) -DNDEBUG $(INCLUDES) -D_REENTRANT=1 jack.cpp -o testjack -ldl -lpthread
