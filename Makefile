NAME = Verlet

CFlags += -std=c99
CFlags += -Wall
CFlags += -Wextra
CFlags += -O2
CFlags += -DPlatform_DESKTOP

LIB = -lraylib -lgdi32 -lwinmm
# CFlags += -mwindows

build:
	gcc -o $(NAME) src/*.c $(LIB) $(CFlags)


RAYLIB_WEB = C:/c_libs/raylib/src/web/libraylib.a
RAYLIB = C:/c_libs/raylib/src/

# have to run emcmdpromt.bat inside emsdk first to set environment variables
web:
	emcc -o web/$(NAME).html src/*.c -Os -Wall $(RAYLIB_WEB) -I. -I$(RAYLIB) -L. -L$(RAYLIB) -s USE_GLFW=3 -s ASYNCIFY --shell-file $(RAYLIB)/minshell.html -DPLATFORM_WEB
