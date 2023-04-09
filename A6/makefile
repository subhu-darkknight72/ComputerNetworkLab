iitkgp:
	sudo gcc -o pgti pingnetinfo.c
	sudo ./pgti iitkgp.ac.in 5 2

local:
	sudo gcc -o pgti pingnetinfo.c
	sudo ./pgti localhost

moodle:
	sudo gcc -o pgti pingnetinfo.c
	sudo ./pgti kgpmoodlenew.iitkgp.ac.in

csemoodle:
	sudo gcc -o pgti pingnetinfo.c
	sudo ./pgti moodlecse.iitkgp.ac.in

gitu:
	gcc -o ass ass6.c 
	sudo ./ass iitkgp.ac.in

default:
	traceroute iitkgp.ac.in

push:
	git add -A
	git commit -m "GG <3, always"
	git push

clean:
	sudo rm -f *.o localhost pgti ass
