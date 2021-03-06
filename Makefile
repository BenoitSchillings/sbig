	CC = g++ 
	CFLAGS += -D_MAC -Wdeprecated-writable-strings  -O3  -fvectorize   -fslp-vectorize-aggressive
	OPENCV = -lopencv_contrib -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_legacy   -I/usr/local/include/opencv
	TINY = tinyxmlerror.cpp.o tinystr.cpp.o tinyxmlparser.cpp.o tinyxml.cpp.o
	MISC = util.cpp.o

all: take find findguide focus filter flat cal dark

take: take.cpp cam.cpp cam.h util.cpp  cam.cpp.o tinystr.cpp.o tinyxmlparser.cpp.o tinyxml.cpp.o tinyxmlerror.cpp.o
	$(CC)  -c take.cpp -o take.cpp.o $(CFLAGS) $(OPENCV)
	$(CC)  -c cam.cpp -o cam.cpp.o $(CFLAGS) $(OPENCV)	
	$(CC)  -c util.cpp -o util.cpp.o $(CFLAGS) $(OPENCV)
	
	$(CC)    -DNDEBUG -Wl,-search_paths_first -Wl,-headerpad_max_install_names  take.cpp.o cam.cpp.o $(TINY) $(MISC) -o take -framework Cocoa  -framework SBIGUdrv -lpthread $(OPENCV) -lm


cal: cal.cpp cam.cpp cam.h util.cpp  cam.cpp.o tinystr.cpp.o tinyxmlparser.cpp.o tinyxml.cpp.o tinyxmlerror.cpp.o
	$(CC)  -c cal.cpp -o cal.cpp.o $(CFLAGS) $(OPENCV)
	$(CC)  -c cam.cpp -o cam.cpp.o $(CFLAGS) $(OPENCV)	
	$(CC)  -c util.cpp -o util.cpp.o $(CFLAGS) $(OPENCV)
	
	$(CC)    -DNDEBUG -Wl,-search_paths_first -Wl,-headerpad_max_install_names  cal.cpp.o cam.cpp.o $(TINY) $(MISC) -o cal -framework Cocoa  -framework SBIGUdrv -lpthread $(OPENCV) -lm

flat: flat.cpp cam.cpp cam.h util.cpp  cam.cpp.o tinystr.cpp.o tinyxmlparser.cpp.o tinyxml.cpp.o tinyxmlerror.cpp.o
	$(CC)  -c flat.cpp -o flat.cpp.o $(CFLAGS) $(OPENCV)
	$(CC)  -c cam.cpp -o cam.cpp.o $(CFLAGS) $(OPENCV)	
	$(CC)  -c util.cpp -o util.cpp.o $(CFLAGS) $(OPENCV)
	
	$(CC)    -DNDEBUG -Wl,-search_paths_first -Wl,-headerpad_max_install_names  flat.cpp.o cam.cpp.o $(TINY) $(MISC) -o flat -framework Cocoa  -framework SBIGUdrv -lpthread $(OPENCV) -lm

dark: dark.cpp cam.cpp cam.h util.cpp  cam.cpp.o tinystr.cpp.o tinyxmlparser.cpp.o tinyxml.cpp.o tinyxmlerror.cpp.o
	$(CC)  -c dark.cpp -o dark.cpp.o $(CFLAGS) $(OPENCV)
	$(CC)  -c cam.cpp -o cam.cpp.o $(CFLAGS) $(OPENCV)	
	$(CC)  -c util.cpp -o util.cpp.o $(CFLAGS) $(OPENCV)
	
	$(CC)    -DNDEBUG -Wl,-search_paths_first -Wl,-headerpad_max_install_names  dark.cpp.o cam.cpp.o $(TINY) $(MISC) -o dark -framework Cocoa  -framework SBIGUdrv -lpthread $(OPENCV) -lm



cam.cpp.o: cam.cpp cam.h
	$(CC)  -c cam.cpp -o cam.cpp.o $(CFLAGS) $(OPENCV)

util.cpp.o: util.cpp util.h
	$(CC)  -c util.cpp -o util.cpp.o $(CFLAGS) $(OPENCV)


tinystr.cpp.o: ./tiny/tinystr.cpp
	$(CC)  -c ./tiny/tinystr.cpp -o tinystr.cpp.o $(CFLAGS) $(OPENCV)

tinyxml.cpp.o: ./tiny/tinyxml.cpp
	$(CC)  -c ./tiny/tinyxml.cpp -o tinyxml.cpp.o $(CFLAGS) $(OPENCV)

tinyxmlparser.cpp.o: ./tiny/tinyxmlparser.cpp
	$(CC)  -c ./tiny/tinyxmlparser.cpp -o tinyxmlparser.cpp.o $(CFLAGS) $(OPENCV)

tinyxmlerror.cpp.o: ./tiny/tinyxmlerror.cpp
	$(CC)  -c ./tiny/tinyxmlerror.cpp -o tinyxmlerror.cpp.o $(CFLAGS) $(OPENCV)



filter: filter.cpp util.cpp.o  tinyxml.cpp.o tinyxmlerror.cpp.o
	$(CC)  -c filter.cpp -o filter.cpp.o $(CFLAGS) $(OPENCV)
	$(CC)    -DNDEBUG -Wl,-search_paths_first -Wl,-headerpad_max_install_names  filter.cpp.o $(TINY)  $(MISC)  -o filter  -framework Cocoa  -framework SBIGUdrv -lpthread $(OPENCV) -lm

find: find.cpp cam.cpp cam.h util.cpp.o cam.cpp.o tinystr.cpp.o tinyxmlparser.cpp.o tinyxml.cpp.o tinyxmlerror.cpp.o
	$(CC)  -c find.cpp -o find.cpp.o $(CFLAGS) $(OPENCV)
	$(CC)    -DNDEBUG -Wl,-search_paths_first -Wl,-headerpad_max_install_names  find.cpp.o cam.cpp.o $(TINY)  $(MISC)  -o find  -framework Cocoa  -framework SBIGUdrv -lpthread $(OPENCV) -lm


findguide: findguide.cpp cam.cpp.o cam.h util.cpp.o  cam.cpp.o tinystr.cpp.o tinyxmlparser.cpp.o tinyxml.cpp.o tinyxmlerror.cpp.o
	$(CC)  -c findguide.cpp -o findguide.cpp.o $(CFLAGS) $(OPENCV)
	$(CC)    -DNDEBUG -Wl,-search_paths_first -Wl,-headerpad_max_install_names  findguide.cpp.o cam.cpp.o $(TINY)  $(MISC)  -o findguide  -framework Cocoa  -framework SBIGUdrv -lpthread $(OPENCV) -lm


focus: focus.cpp cam.cpp.o cam.h util.cpp.o   cam.cpp.o tinystr.cpp.o tinyxmlparser.cpp.o tinyxml.cpp.o tinyxmlerror.cpp.o
	$(CC)  -c focus.cpp -o focus.cpp.o $(CFLAGS) $(OPENCV)
	$(CC)    -DNDEBUG -Wl,-search_paths_first -Wl,-headerpad_max_install_names  focus.cpp.o cam.cpp.o $(TINY)  $(MISC)  -o focus  -framework Cocoa  -framework SBIGUdrv -lpthread $(OPENCV) -lm



clean:
	-rm findguide find focus take *.cpp.o filter flat
	-rm -rf *.dSYM
