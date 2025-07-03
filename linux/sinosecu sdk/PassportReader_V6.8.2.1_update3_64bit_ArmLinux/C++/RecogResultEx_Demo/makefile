#CC := /home/nowcoder/tools/gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
#CXX := /home/nowcoder/tools/gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++
src=$(wildcard ./*.cpp)
objs=$(patsubst %.cpp,%.o,$(src))
target=../../lib/RecogResultEx_Test
$(target):$(objs) $(lib)
	$(CXX) -g $(objs)  -ldl -o $(target) 

%.o:%.cpp 
	$(CXX) -g -c $< $(include) -o $@

.PHONY:clean
clean:
	rm $(objs) -f
