diffcal: main.cpp libdiffcal.a
	g++ -o $@ $< -Wall -pthread -g -lvmaf -lOpenImageIO -L . -l diffcal -lvmaf -O3

libdiffcal.a: diffcal.cpp util.cpp run_vmaf.cpp
	g++ -shared -o $@ $^ -Wall -g -fpic -O3
