SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
CC = gcc
DFLAGS = -DGHOST_MYSQL
OFLAGS = -O3 -g
LFLAGS = -L. -L./utils/bncsutil/src/bncsutil/ -L./utils/StormLib/stormlib/  -lbncsutil -lpthread -ldl -lz -lgmp -lStorm -lmysqlclient_r -lboost_date_time -lboost_thread -lboost_system -lboost_filesystem
CFLAGS = -std=c++11

ifeq ($(SYSTEM),Darwin)
DFLAGS += -D__APPLE__
OFLAGS += -flat_namespace
else
LFLAGS += -lrt
endif

ifeq ($(SYSTEM),FreeBSD)
DFLAGS += -D__FREEBSD__
endif

ifeq ($(SYSTEM),SunOS)
DFLAGS += -D__SOLARIS__
LFLAGS += -lresolv -lsocket -lnsl
endif

CFLAGS += $(OFLAGS) $(DFLAGS) -I. -I./utils/bncsutil/src/ -I./utils/StormLib/

ifeq ($(SYSTEM),Darwin)
CFLAGS += -I../database/mysql/include/
endif

OBJS = main.o log.o config.o utils/bncsutilinterface.o bnet/bnet.o bnet/bnetprotocol.o bnls/bnlsclient.o bnls/bnlsprotocol.o network/commandpacket.o network/utils.o utils/crc32.o game/game.o game/game_base.o game/gameplayer.o game/gameprotocol.o game/gameslot.o garena/gcbiprotocol.o ohbot.o database/ghostdb.o database/ghostdbmysql.o gproxy/gpsprotocol.o language.o map.o replay/packed.o replay/replay.o replay/savegame.o utils/sha1.o network/socket.o stats/stats.o stats/statsdota.o stats/statsw3mmd.o utils/util.o
COBJS =
PROGS = ./ohs++

all: $(OBJS) $(COBJS) $(PROGS)

./ohs++: $(OBJS) $(COBJS)
	$(C++) -o ./ghost++ $(OBJS) $(COBJS) $(LFLAGS)

clean:
	rm -f $(OBJS) $(COBJS) $(PROGS)

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

$(COBJS): %.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

./ohs++: $(OBJS) $(COBJS)

all: $(PROGS)

utils/bncsutilinterface.o: ohbot.h includes.h utils/util.h utils/bncsutilinterface.h
bnet/bnet.o: ohbot.h includes.h utils/util.h config.h language.h network/socket.h network/commandpacket.h database/ghostdb.h utils/bncsutilinterface.h bnls/bnlsclient.h bnet/bnetprotocol.h bnet/bnet.h map.h replay/packed.h replay/savegame.h replay/replay.h game/gameprotocol.h game/game_base.h
bnet/bnetprotocol.o: ohbot.h includes.h utils/util.h bnet/bnetprotocol.h
bnls/bnlsclient.o: ohbot.h includes.h utils/util.h network/socket.h network/commandpacket.h bnls/bnlsprotocol.h bnls/bnlsclient.h
bnls/bnlsprotocol.o: ohbot.h includes.h utils/util.h bnls/bnlsprotocol.h
network/commandpacket.o: ohbot.h includes.h network/commandpacket.h
network/utils.o: ohbot.h utils/util.h network/socket.h
config.o: includes.h config.h
log.o: log.h
utils/crc32.o: ohbot.h includes.h utils/crc32.h
game/game.o: ohbot.h includes.h utils/util.h config.h language.h network/socket.h database/ghostdb.h bnet/bnet.h map.h replay/packed.h replay/savegame.h game/gameplayer.h game/gameprotocol.h game/game_base.h game/game.h stats/stats.h stats/statsdota.h stats/statsw3mmd.h
game/game_base.o: ohbot.h includes.h utils/util.h config.h language.h network/socket.h database/ghostdb.h bnet/bnet.h map.h replay/packed.h replay/savegame.h replay/replay.h game/gameplayer.h game/gameprotocol.h game/game_base.h utils/next_combination.h
game/gameplayer.o: ohbot.h includes.h utils/util.h language.h network/socket.h network/commandpacket.h bnet/bnet.h map.h game/gameplayer.h game/gameprotocol.h gproxy/gpsprotocol.h game/game_base.h garena/gcbiprotocol.h
game/gameprotocol.o: ohbot.h includes.h utils/util.h utils/crc32.h game/gameplayer.h game/gameprotocol.h game/game_base.h
game/gameslot.o: ohbot.h includes.h game/gameslot.h
garena/gcbiprotocol.o: garena/gcbiprotocol.h ohbot.h utils/util.h
main.o: includes.h ohbot.h utils/util.h
ghost.o: includes.h ohbot.h utils/util.h utils/crc32.h utils/sha1.h config.h language.h network/socket.h database/ghostdb.h database/ghostdbmysql.h bnet/bnet.h map.h replay/packed.h replay/savegame.h game/gameplayer.h game/gameprotocol.h gproxy/gpsprotocol.h network/utils.h game/game_base.h game/game.h garena/gcbiprotocol.h
database/ghostdb.o: ohbot.h includes.h utils/util.h config.h database/ghostdb.h
database/ghostdbmysql.o: ohbot.h includes.h utils/util.h config.h database/ghostdb.h database/ghostdbmysql.h
gproxy/gpsprotocol.o: ohbot.h utils/util.h gproxy/gpsprotocol.h
language.o: ohbot.h includes.h config.h language.h
map.o: ohbot.h includes.h utils/util.h utils/crc32.h utils/sha1.h config.h map.h
replay/packed.o: ohbot.h includes.h utils/util.h utils/crc32.h replay/packed.h
replay/replay.o: ohbot.h includes.h utils/util.h replay/packed.h replay/replay.h game/gameprotocol.h
replay/savegame.o: ohbot.h includes.h utils/util.h replay/packed.h replay/savegame.h
utils/sha1.o: utils/sha1.h
network/socket.o: ohbot.h includes.h utils/util.h network/socket.h
stats/stats.o: ohbot.h includes.h stats/stats.h
stats/statsdota.o: ohbot.h includes.h utils/util.h database/ghostdb.h game/gameplayer.h game/gameprotocol.h game/game_base.h stats/stats.h stats/statsdota.h
stats/statsw3mmd.o: ohbot.h includes.h utils/util.h database/ghostdb.h game/gameprotocol.h game/game_base.h stats/stats.h stats/statsw3mmd.h
utils/util.o: ohbot.h includes.h utils/util.h
