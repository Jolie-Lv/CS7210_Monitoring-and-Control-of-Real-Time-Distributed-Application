CLEAN = make clean

all:
	$(MAKE) -C PodController
	$(MAKE) -C DRFM
	$(MAKE) -C Sensor
	$(MAKE) -C Jammer
clean:
	$(CLEAN) -C DRFM
	$(CLEAN) -C Jammer
	$(CLEAN) -C PodController
	$(CLEAN) -C Sensor
	rm -f main
run:
	./PodController/PodController