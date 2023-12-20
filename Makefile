make:
	gcc -g -o dns dns.c -Wall

test:
# Executes code then the tests
	gcc -g -o dns dns.c -Wall
	python3 dns_test.py 