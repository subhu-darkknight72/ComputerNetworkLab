iitkgp:
	sudo gcc -o tr tr.c
	sudo ./tr iitkgp.ac.in

local:
	sudo gcc -o tr tr.c
	./tr localhost

default:
	traceroute iitkgp.ac.in

push:
	git add -A
	git commit -m "GG <3, always"
	git push

clean:
	sudo rm -f *.o localhost tr
