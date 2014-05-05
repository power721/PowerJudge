LD=g++
CXX=g++
CXXFLAGS=-g -Wall -O2
LDFLAGS=
TARGET=bin/powerjudge
OBJECTS=bin/judge.o

ifdef LOG_LEVEL
	CXXFLAGS+= -DLOG_LEVEL=${LOG_LEVEL}
endif

ifdef FAST_JUDGE
	CXXFLAGS+= -DFAST_JUDGE
endif

all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(LD) $(OBJECTS) -o $@ $(LDFLAGS)
	sudo chown root:root $(TARGET)
	sudo chmod 4755 $(TARGET)

bin/%.o: src/%.c src/*.h
	$(CXX) $< -c -o $@ $(CXXFLAGS)

.PHONY: test check install clean
test:
	g++ -o test/data/1405/spj test/data/1405/spj.cc
	chmod a+x test/unitTest.sh
	cd test && ./unitTest.sh

check:
	-cd src && ./cpplint.py --linelength=100 --extensions=c,h \
	--filter=-whitespace/braces,-build/include \
	judge.c judge.h judge_core.h log.h misc.h syscalls.h
	-cppcheck src/judge.c

install:
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod 4755 /usr/local/bin/powerjudge

clean:
	sudo rm -f bin/judge.o bin/powerjudge
	sudo rm -rf test/temp
