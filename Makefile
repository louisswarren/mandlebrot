WARNINGS  += -pedantic -pedantic-errors -Wno-overlength-strings
WARNINGS  += -fmax-errors=2
WARNINGS  += -Wall -Wextra -Wdouble-promotion -Wformat=2
WARNINGS  += -Wformat-signedness -Wformat-truncation=2 -Wformat-overflow=2
WARNINGS  += -Wnull-dereference -Winit-self -Wuninitialized
WARNINGS  += -Wimplicit-fallthrough=4 -Wstack-protector -Wmissing-include-dirs
WARNINGS  += -Wshift-overflow=2 -Wswitch-default -Wswitch-enum
WARNINGS  += -Wunused-parameter -Wunused-const-variable=2 -Wstrict-overflow=5
WARNINGS  += -Wstringop-overflow=4 -Wstringop-truncation -Walloc-zero -Walloca
WARNINGS  += -Warray-bounds=2 -Wattribute-alias=2 -Wlogical-op
WARNINGS  += -Wduplicated-branches -Wduplicated-cond -Wtrampolines -Wfloat-equal
WARNINGS  += -Wunsafe-loop-optimizations -Wshadow
WARNINGS  += -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion
WARNINGS  += -Wpacked -Wdangling-else -Wno-parentheses -Wsign-conversion
WARNINGS  += -Wdate-time -Wjump-misses-init -Wreturn-local-addr -Wno-pointer-sign
WARNINGS  += -Wstrict-prototypes
WARNINGS  += -Wnormalized=nfkc -Wredundant-decls
WARNINGS  += -Wnested-externs -Wno-missing-field-initializers -fanalyzer
WARNINGS  += -Wunused
WARNINGS  += #-Wvla -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations

CFLAGS += -std=c99 -fopenmp -Ofast -Werror $(WARNINGS)
LDFLAGS += -lm -fopenmp

.PHONY: default
default: out.png test

.PHONY: test
test: test.md5 out.ppm
	md5sum -c $<

.PHONY: test-save
test-save: out.ppm
	md5sum $^ > test.md5

.PHONY: watch
watch: mandlebrot
	./$< z | mpv --scale=oversample -

out.png: out.ppm
	convert $< $@

out.ppm: mandlebrot
	time ./$< > $@

mandlebrot: mandlebrot.o
mandlebrot.o: mandlebrot.c
