LD=g++
CXX=g++
CXXFLAGS=-g -Wall -O2 -DBIG_JOINS=1 -fno-strict-aliasing -DNDEBUG
LDFLAGS=
LIBS=-lbsd -L/usr/lib/x86_64-linux-gnu -lcurl -lm
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

.PHONY: test check install sim clean
all: $(TARGET) $(TARGETD)
$(TARGET): $(OBJECTS)
	$(LD) -o $@ $(LDFLAGS) $(OBJECTS)

$(TARGETD): $(OBJECTSD)
	$(LD) -o $@ $(LDFLAGS) $(OBJECTSD) $(LIBS)
	sudo chown root:root $(TARGETD)
	sudo chmod 4755 $(TARGETD)

bin/%.o: src/%.c src/*.h
	$(CXX) -o $@ $(CXXFLAGS) -c $<

test: all
	sudo chown judge:judge $(TARGET)
	sudo chmod 4755 $(TARGET)
	sudo setcap cap_sys_chroot+ep $(TARGET)
	sudo chown judge:judge test/
	g++ -o test/data/1405/spj test/data/1405/spj.cc
	cd test && sudo su judge -c ./unitTest.sh

check: all
	-cd src && ../bin/cpplint.py --linelength=100 --extensions=c,h \
	--filter=-whitespace/braces,-build/include \
	judge.c judge.h judged.c judged.h judge_core.h log.h misc.h syscalls.h
	-cppcheck src/judge.c  src/judged.c

install: all
	sudo cp $(TARGET) /usr/local/bin/powerjudge
	sudo chown judge:judge /usr/local/bin/powerjudge
	sudo chmod 4755 /usr/local/bin/powerjudge
	sudo cp $(TARGETD) /usr/local/bin/powerjudged
	sudo chown root:root /usr/local/bin/powerjudged
	sudo chmod 4755 /usr/local/bin/powerjudged
	sudo setcap cap_sys_chroot+ep /usr/local/bin/powerjudge

# powerjudge
# must chown judge:judge
# must chmod 4755, so it will run as judge:judge
# must setcap cap_sys_chroot+ep, so chroot() can work
#
# powerjudged
# must chown root:root
# must chmod 4755
# must run with user judge

sim:
	cd sim && make fresh && make exes
	chmod a+x sim/sim*
	sudo cp sim/sim_c.exe /usr/local/bin/sim_c
	sudo cp sim/sim_java.exe /usr/local/bin/sim_java
	sudo cp sim/sim_pasc.exe /usr/local/bin/sim_pas
	sudo cp sim.sh /usr/local/bin
	sudo chmod a+x /usr/local/bin/sim.sh
	sudo rm -f /usr/local/bin/sim_cc
	sudo ln -s /usr/local/bin/sim_c /usr/local/bin/sim_cc

clean:
	sudo rm -f bin/judge.o bin/powerjudge bin/judged.o bin/powerjudged
	sudo rm -rf test/temp
