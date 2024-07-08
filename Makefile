driver:	driver.o myMalloc.o myMalloc-helper.o
	gcc -o driver driver.o myMalloc.o myMalloc-helper.o -lpthread

driver.o:	driver.c myMalloc.h
	gcc -c driver.c

myMalloc.o:	myMalloc.c myMalloc.h myMalloc-helper.h
	gcc -c myMalloc.c

myMalloc-helper.o:	myMalloc-helper.c myMalloc-helper.h
	gcc -c myMalloc-helper.c

clean:
	rm -f *.o driver
