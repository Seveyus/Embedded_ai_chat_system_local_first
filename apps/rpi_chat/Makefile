# Chemins
CXX = /home/yoann/buildroot/output/host/bin/aarch64-linux-g++
SYSROOT = /home/yoann/buildroot/output/staging

# Drapeaux
CXXFLAGS = -Wall -I./src --sysroot=$(SYSROOT)
LDFLAGS = -lmosquitto -lsqlite3

# Fichiers
SRCS = src/main.cpp src/message.cpp src/storage.cpp src/mqtt_client.cpp
TARGET = build/chat_client

all: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -rf build/

install: $(TARGET)
	mkdir -p ~/buildroot/board/rpi4/overlay/usr/bin/
	cp $(TARGET) ~/buildroot/board/rpi4/overlay/usr/bin/
	@echo "Installé dans l'overlay avec succès !"
