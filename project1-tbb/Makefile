
all: seq pipeline_serial pipeline_with_parallels parallel

seq:
	g++ -std=c++1y -O3 seq.cpp -o seq `pkg-config --cflags --libs opencv` -ltbb

pipeline_serial:
	g++ -std=c++1y -O3 pipeline_serial.cpp -o pipeline_serial `pkg-config --cflags --libs opencv` -ltbb

pipeline_with_parallels:
	g++ -std=c++1y -O3 pipeline_with_parallels.cpp -o pipeline_with_parallels `pkg-config --cflags --libs opencv` -ltbb

parallel:
	 g++ -std=c++1y -O3 parallel.cpp -o parallel `pkg-config --cflags --libs opencv` -ltbb

clean:
	rm -rf seq pipeline_serial pipeline_with_parallels parallel
