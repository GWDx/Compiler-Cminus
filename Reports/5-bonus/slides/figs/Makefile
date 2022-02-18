src = $(wildcard mmd/*.mmd)
target = $(patsubst mmd/%.mmd, png/%.png, ${src})

all: ${target}

png/%.png : mmd/%.mmd
	mmdc -s 4 -i $< -o $@
