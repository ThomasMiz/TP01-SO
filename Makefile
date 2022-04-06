all: app vista

app:
	cd src; make app

vista:
	cd src; make vista

clean:
	cd src; make clean


.PHONY: all app vista clean