all:
	g++ ip-checker.cpp -o ip-checker
	chmod +x ip-checker

run: ip-checker
	./ip-checker

clean:
	rm -f a.out
	rm -f ip-checker
