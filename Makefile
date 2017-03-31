all:
	g++ ip-checker.cpp -o ip-checker
	chmod +x ip-checker

install:
	sudo cp ./ip-checker /usr/sbin/ip-checker
	
	@if [ -f /etc/crontab ] ; \
	then \
		echo '0,15,30,45 * * * * $(shell whoami) /usr/sbin/ip-checker -r' | sudo tee -a /etc/crontab ; \
	fi;

run: ip-checker
	./ip-checker

clean:
	rm -f a.out
	rm -f ip-checker
