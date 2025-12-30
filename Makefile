CC = cl
LD = link
SRC_DIR   = src
INC_DIR   = includes
BUILD_DIR = build
OUT_DIR   = bin
LIB_DIR   = libs
TARGET = disabler.dll
MINHOOK_LIB = libMinHook-x64-v140-md.lib
CFLAGS  = /nologo /W4 /O2 /MD /EHsc /I$(INC_DIR)
LDFLAGS = /NOLOGO /DLL /OUT:$(OUT_DIR)\$(TARGET) /LIBPATH:$(LIB_DIR) $(MINHOOK_LIB)

SRCS = \
	$(SRC_DIR)\hook.c \
	$(SRC_DIR)\main.c

OBJS = \
	$(BUILD_DIR)\hook.obj \
	$(BUILD_DIR)\main.obj

all: dirs $(OUT_DIR)\$(TARGET)
dirs:
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	if not exist $(OUT_DIR) mkdir $(OUT_DIR)

{$(SRC_DIR)}.c{$(BUILD_DIR)}.obj:
	$(CC) $(CFLAGS) /Fo$@ /c $<

$(OUT_DIR)\$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS)

clean:
	if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	if exist $(OUT_DIR) rmdir /s /q $(OUT_DIR)
re: clean all
