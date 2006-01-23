all: dbgp.html gdb.html

gdb.html: gdb.txt
	rst2html.py gdb.txt > gdb.html

dbgp.html: dbgp.txt
	rst2html.py dbgp.txt > dbgp.html

clean:
	rm -rf dbgp.html gdb.html
