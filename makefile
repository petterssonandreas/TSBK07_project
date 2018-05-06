# set this variable to the director in which you saved the common files
commondir = ./common/

TSBK07-files-c = $(commondir)GL_utilities.c $(commondir)VectorUtils3.c $(commondir)loadobj.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c simplefont.c
project-files-c = main.c frustum.c keyboard.c heightMap.c skybox.c draw.c initFunctions.c
project-files-h = defines.h

includeflags = -I$(commondir) -I../common/Linux
links = -lXt -lX11 -lGL -lm


all : snowsim

snowsim : $(project-files-c) $(project-files-h) $(TSBK07-files-c)
	gcc -Wall -o snowsim $(includeflags) -DGL_GLEXT_PROTOTYPES $(project-files-c) $(project-files-h) $(TSBK07-files-c) $(links)


clean :
	rm snowsim
