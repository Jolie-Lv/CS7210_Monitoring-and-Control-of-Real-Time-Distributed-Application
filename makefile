CLEAN = make clean

all:
	$(MAKE) -C DRFM
	$(MAKE) -C Jammer
	$(MAKE) -C PodController
	$(MAKE) -C Sensor
clean:
	$(CLEAN) -C DRFM
	$(CLEAN) -C Jammer
	$(CLEAN) -C PodController
	$(CLEAN) -C Sensor
	rm -f main
run:
	./PodController/PodController