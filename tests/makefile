all:
	g++ -std=c++17 -Wl,--gc-sections -fexceptions -ftrivial-auto-var-init=zero -g -O2 crash-linux.cpp -o crash-linux -I../third_party/include -L/usr/lib -ldw -ffunction-sections -fdata-sections -gdwarf-3 -O2 -m64 -march=native -mtune=native -fstack-protector-strong -ffunction-sections -fdata-sections -g -mavx -mavx2 -march=native -mtune=native
clean:
	rm crash-linux
