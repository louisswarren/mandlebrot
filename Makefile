.PHONY: default
default: out.png

.PHONY: test
test: test.md5 out.pbm
	md5sum -c $<

.PHONY: test-save
test-save: out.pbm
	md5sum $^ > test.md5

out.png: out.pbm
	convert $< $@

out.pbm: mandlebrot
	./$< > $@

mandlebrot: mandlebrot.o
mandlebrot.o: mandlebrot.c
