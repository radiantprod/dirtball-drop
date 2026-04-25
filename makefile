##makefile

CXX = g++

CXXFLAGS = -Wall -Wextra -g3 -Og \
-fno-optimize-sibling-calls -fno-ipa-icf -fno-omit-frame-pointer -fno-common \
-fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined \
-fsanitize=shift -fsanitize=shift-exponent -fsanitize=shift-base -fsanitize=integer-divide-by-zero \
-fsanitize=unreachable -fsanitize=vla-bound -fsanitize=null -fsanitize=return \
-fsanitize=signed-integer-overflow -fsanitize=bounds -fsanitize=bounds-strict -fsanitize=alignment \
-fsanitize=object-size -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow \
-fsanitize=bool -fsanitize=enum -fsanitize=vptr -fsanitize=pointer-overflow -fsanitize=builtin \
-fno-sanitize-recover=all -fsanitize-address-use-after-scope -fstack-protector-all

SRC = dirtball.cpp
OBJ = $(SRC:.cpp=.o)

TARGET = dirtball

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(CXXFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
