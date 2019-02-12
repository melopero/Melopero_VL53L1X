.PHONY: clean lib

SRC = api
LIB = libvl53l1_api.so
LDFLAGS :=
CFLAGS := -Iapi/core -Iapi/platform -Iapi/python-interface 

SRC_FILES := $(wildcard $(SRC)/*/*.c)
OBJ_FILES := $(patsubst $(SRC)/*/*.c,$(SRC)/*/%.o,$(SRC_FILES))


$(LIB): $(OBJ_FILES)
	$(CC) $(LDFLAGS) $(CFLAGS) -fPIC -shared -o $@ $^

$(SRC)/%.o: $(SRC)/%.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c -o $@ $<

