LD=g++
CXX=g++
CXXFLAGS=-g -Wall -O2
LDFLAGS=
LIBS=-lbsd
TARGET=bin/powerjudge
OBJECTS=bin/judge.o
TARGETD=bin/powerjudged
OBJECTSD=bin/judged.o

ifdef LOG_LEVEL
	CXXFLAGS+= -DLOG_LEVEL=${LOG_LEVEL}
endif

ifdef FAST_JUDGE
	CXXFLAGS+= -DFAST_JUDGE
endif

all: $(TARGET) $(TARGETD)
$(TARGET): $(OBJECTS)
	$(LD) -o $@ $(LDFLAGS) $(OBJECTS)
	sudo chown root:root $(TARGET)
	sudo chmod 4755 $(TARGET)

$(TARGETD): $(OBJECTSD)
	$(LD) -o $@ $(LDFLAGS) $(OBJECTSD) $(LIBS)
	sudo chown root:root $(TARGETD)
	sudo chmod 4755 $(TARGETD)

bin/%.o: src/%.c src/*.h
	$(CXX) -o $@ $(CXXFLAGS) -c $<

.PHONY: test check install clean
test:
	g++ -o test/data/1405/spj test/data/1405/spj.cc
	chmod a+x test/unitTest.sh
	cd test && ./unitTest.sh

check:
	-cd src && ../bin/cpplint.py --linelength=100 --extensions=c,h \
	--filter=-whitespace/braces,-build/include \
	judge.c judge.h judged.c judged.h judge_core.h log.h misc.h syscalls.h
	-cppcheck src/judge.c  src/judged.c

install:
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod 4755 /usr/local/bin/powerjudge
	sudo pkill -f /usr/local/bin/powerjudged
	sudo cp $(TARGETD) /usr/local/bin/
	sudo chmod 4755 /usr/local/bin/powerjudged
	sudo cp -p judge.properties /home/judge/
	/usr/local/bin/powerjudged

clean:
	sudo rm -f bin/judge.o bin/powerjudge bin/judged.o bin/powerjudged
	sudo rm -rf test/temp
