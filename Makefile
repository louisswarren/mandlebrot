CFLAGS += -std=c99 -fopenmp -Ofast $(WARNINGS)
LDFLAGS += -fopenmp

.PHONY: default
default: out.png test

.PHONY: test
test: test.md5 out.pbm
	md5sum -c $<

.PHONY: test-save
test-save: out.pbm
	md5sum $^ > test.md5

.PHONY: watch
watch: mandlebrot
	./$< z | mpv --scale=oversample -

out.png: out.pbm
	convert $< $@

out.pbm: mandlebrot
	time ./$< > $@

mandlebrot: mandlebrot.o
mandlebrot.o: mandlebrot.c
