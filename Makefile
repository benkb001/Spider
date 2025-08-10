spider: spider.cpp
	g++ spider.cpp -o spider -lcurl

clean: 
	rm spider *.o *.out

run: spider
	./spider