.PHONY: all
all: main_c main_objc_arc main_objc_noarc arc_app noarc_app

main.m: main.c
	cp $< $@

main_c: main.c
	clang -framework Cocoa -framework OpenGL -o $@ $<

main_objc_arc: main.m
	clang -framework Cocoa -framework OpenGL -fobjc-arc -o $@ $<

main_objc_noarc: main.m
	clang -framework Cocoa -framework OpenGL -fno-objc-arc -o $@ $<

arc_app:	main_objc_arc
	./appify -s main_objc_arc -n SimpleCApp_arc

noarc_app:	main_objc_noarc
	./appify -s main_objc_noarc -n SimpleCApp_noarc

