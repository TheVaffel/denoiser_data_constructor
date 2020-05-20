diffcal: main.cpp libdiffcal.a
	g++ -o $@ $< -lOpenImageIO -Wall -pthread -g -l diffcal -L . -lvmaf

libdiffcal.a: diffcal.cpp util.cpp run_vmaf.cpp
	g++ -shared -o $@ $^ -Wall -g -fpic -lvmaf
