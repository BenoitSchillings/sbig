	CC = g++ 
	CFLAGS += -D_MAC -Wdeprecated-writable-strings  -O3  -fvectorize   -fslp-vectorize-aggressive
	OPENCV = -lopencv_contrib -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_legacy   -I/usr/local/include/opencv
	TINY = tinyxmlerror.cpp.o tinystr.cpp.o tinyxmlparser.cpp.o tinyxml.cpp.o
	MISC = util.cpp.o

all: take find findguide focus 

take: take.cpp cam.cpp cam.h util.cpp  cam.cpp.o tinystr.cpp.o tinyxmlparser.cpp.o tinyxml.cpp.o tinyxmlerror.cpp.o
	$(CC)  -c take.cpp -o take.cpp.o $(CFLAGS) $(OPENCV)
	$(CC)  -c cam.cpp -o cam.cpp.o $(CFLAGS) $(OPENCV)	
	$(CC)  -c util.cpp -o util.cpp.o $(CFLAGS) $(OPENCV)
	
	$(CC)    -DNDEBUG -Wl,-search_paths_first -Wl,-headerpad_max_install_names  take.cpp.o cam.cpp.o $(TINY) $(MISC) -o take -framework Cocoa  -framework SBIGUdrv -lpthread $(OPENCV) -lm

cam.cpp.o: cam.cpp cam.h
	$(CC)  -c cam.cpp -o cam.cpp.o $(CFLAGS) $(OPENCV)


tinystr.cpp.o: ./tiny/tinystr.cpp
	$(CC)  -c ./tiny/tinystr.cpp -o tinystr.cpp.o $(CFLAGS) $(OPENCV)

tinyxml.cpp.o: ./tiny/tinyxml.cpp
	$(CC)  -c ./tiny/tinyxml.cpp -o tinyxml.cpp.o $(CFLAGS) $(OPENCV)

tinyxmlparser.cpp.o: ./tiny/tinyxmlparser.cpp
	$(CC)  -c ./tiny/tinyxmlparser.cpp -o tinyxmlparser.cpp.o $(CFLAGS) $(OPENCV)

tinyxmlerror.cpp.o: ./tiny/tinyxmlerror.cpp
	$(CC)  -c ./tiny/tinyxmlerror.cpp -o tinyxmlerror.cpp.o $(CFLAGS) $(OPENCV)


find: find.cpp cam.cpp cam.h util.cpp cam.cpp.o tinystr.cpp.o tinyxmlparser.cpp.o tinyxml.cpp.o tinyxmlerror.cpp.o
	$(CC)  -c find.cpp -o find.cpp.o $(CFLAGS) $(OPENCV)
	$(CC)  -c util.cpp -o util.cpp.o $(CFLAGS) $(OPENCV)

	$(CC)    -DNDEBUG -Wl,-search_paths_first -Wl,-headerpad_max_install_names  find.cpp.o cam.cpp.o $(TINY)  $(MISC)  -o find  -framework Cocoa  -framework SBIGUdrv -lpthread $(OPENCV) -lm


findguide: findguide.cpp cam.cpp cam.h util.cpp  cam.cpp.o tinystr.cpp.o tinyxmlparser.cpp.o tinyxml.cpp.o tinyxmlerror.cpp.o
	$(CC)  -c findguide.cpp -o findguide.cpp.o $(CFLAGS) $(OPENCV)
	$(CC)  -c cam.cpp -o cam.cpp.o $(CFLAGS) $(OPENCV)
	$(CC)  -c util.cpp -o util.cpp.o $(CFLAGS) $(OPENCV)

	$(CC)    -DNDEBUG -Wl,-search_paths_first -Wl,-headerpad_max_install_names  findguide.cpp.o cam.cpp.o $(TINY)  $(MISC)  -o findguide  -framework Cocoa  -framework SBIGUdrv -lpthread $(OPENCV) -lm


focus: focus.cpp cam.cpp cam.h util.cpp   cam.cpp.o tinystr.cpp.o tinyxmlparser.cpp.o tinyxml.cpp.o tinyxmlerror.cpp.o
	$(CC)  -c focus.cpp -o focus.cpp.o $(CFLAGS) $(OPENCV)
	$(CC)  -c cam.cpp -o cam.cpp.o $(CFLAGS) $(OPENCV)
	$(CC)  -c util.cpp -o util.cpp.o $(CFLAGS) $(OPENCV)

	$(CC)    -DNDEBUG -Wl,-search_paths_first -Wl,-headerpad_max_install_names  focus.cpp.o cam.cpp.o $(TINY)  $(MISC)  -o focus  -framework Cocoa  -framework SBIGUdrv -lpthread $(OPENCV) -lm



clean:
	-rm findguide find focus take *.cpp.o 
	-rm -rf *.dSYM
