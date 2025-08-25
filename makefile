all:
	make -C tool/code-cpp/
	cp tool/code-cpp/a.out .
clean:
	rm -f *.ppm
