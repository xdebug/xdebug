all: dbgp.html gdb.html

gdb.html: gdb.txt
	/dat/install/docutils-0.3.3/tools/html.py gdb.txt > gdb.html

dbgp.html: dbgp.txt
	/dat/install/docutils-0.3.3/tools/html.py dbgp.txt > dbgp.html

clean:
	rm -rf dbgp.html gdb.html
