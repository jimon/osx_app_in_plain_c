.PHONY: all
all: main_c main_objc_arc main_objc_noarc

main.m: main.c
	cp $< $@

main_c: main.c
	clang -framework Cocoa -framework OpenGL -o $@ $<

main_objc_arc: main.m
	clang -framework Cocoa -framework OpenGL -fobjc-arc -o $@ $<

main_objc_noarc: main.m
	clang -framework Cocoa -framework OpenGL -fno-objc-arc -o $@ $<
