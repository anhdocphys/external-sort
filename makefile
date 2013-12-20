PROG = main
all: $(PROG)
%: %.cpp
		g++ -o $@ $< 
clean:
		rm $(PROG)
