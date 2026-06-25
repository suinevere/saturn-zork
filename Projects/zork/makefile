JO_COMPILE_WITH_VIDEO_MODULE = 0
JO_COMPILE_WITH_BACKUP_MODULE = 0
JO_COMPILE_WITH_TGA_MODULE = 1
JO_COMPILE_WITH_AUDIO_MODULE = 0
JO_COMPILE_WITH_3D_MODULE = 0
JO_COMPILE_WITH_PSEUDO_MODE7_MODULE = 0
JO_COMPILE_WITH_EFFECTS_MODULE = 0
JO_COMPILE_WITH_KEYBOARD_SUPPORT = 1
JO_DEBUG = 1
SRCS=main.c
JO_ENGINE_SRC_DIR=../../jo_engine
COMPILER_DIR=../../Compiler

# Zork game translation units. The _*.cpp files are produced by the host
# generators (see Makefile.zork) and are committed to the repo. They contain
# plain C code in .cpp files, so they are compiled as C (-x c) for the Saturn.
# After editing game.cxx / parser.cxx / villains.cxx / descriptions.cxx,
# regenerate them on a PC with: make -f Makefile.zork _enum.h _text.cpp _vocab.cpp
ZORK_OBJS = utility.o _text.o _vocab.o _game.o _parser.o _villains.o

include $(COMPILER_DIR)/COMMON/jo_engine_makefile

# Add the Zork units to the link recipe (OBJS) AND to game.elf's prerequisites.
# The prerequisite list of game.elf was already expanded inside the include, so
# appending to OBJS alone would not cause the units to be built -- the extra
# "$(TARGET): $(ZORK_OBJS)" rule adds them as prerequisites so make builds them.
OBJS += $(ZORK_OBJS)
$(TARGET): $(ZORK_OBJS)
_CCFLAGS += -DCOMPILE_SATURN

# Compile the Zork .cpp units as C using the same flags as the engine .c files.
# A static pattern rule (scoped to ZORK_OBJS) is used so it is unambiguous
# alongside the engine's built-in .c.o suffix rule.
$(ZORK_OBJS): %.o: %.cpp
	$(CC) -x c $< $(DFLAGS) $(CCFLAGS) $(_CCFLAGS) -o $@
