.PHONY: all clean rebuild

all: munakas

standalone: c/cs.c c/process.c c/bridge.c
	gcc -o standalone c/main.c c/cs.c c/process.c c/bridge.c -I. -lm

libmunakas.so: c/cs.c c/process.c c/bridge.c
	gcc -fPIC -shared -o libmunakas.so c/cs.c c/process.c c/bridge.c -I. -lm

munakas: libmunakas.so main.go reader.go
	go build -o munakas main.go reader.go

clean:
	rm -f libmunakas.so munakas standalone
	rm -rf /tmp/go-build/%

rebuild: clean all