CXX = g++

# Путь к заголовочным файлам
INCLUDES = -I./jsoncpp/include

# Путь к библиотекам
LIBDIR = -L./jsoncpp/build/lib

# Библиотеки для линковки
LIBS = -ljsoncpp

SRCS = main.cpp

TARGET = simulation

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(SRCS) $(INCLUDES) $(LIBDIR) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)
.PHONY: all clean