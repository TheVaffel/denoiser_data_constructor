diffcal: main.cpp diffcal.cpp util.cpp run_vmaf.cpp
	g++ -o $@ $^ -lOpenImageIO -Wall -pthread -lvmaf -g

diffcal.a: diffcal.cpp util.cpp run_vmaf.cpp
	g++ -shared -o $@ $^ -Wall -g -fpic
