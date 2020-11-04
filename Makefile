# Makefile for astrolograph

CXX = g++-8
#CXX = clang++

SRCDIR         = ./src
INCDIR         = ./inc
LIBDIR         = ./libs
BUILDDIR       = ./build

DATE_SRCDIR    = ${LIBDIR}/date/src
DATE_INCDIR    = ${LIBDIR}/date/inc
DATE_TZDIR    = .

IMGUI_SRCDIR   = ${LIBDIR}/imgui/src
IMGUI_INCDIR   = ${LIBDIR}/imgui/include


EXEC    =  astrolograph
SOURCES =  main.cpp
SOURCES += $(wildcard ${SRCDIR}/*.cpp)
SOURCES += ${IMGUI_SRCDIR}/examples/imgui_impl_glfw.cpp \
           ${IMGUI_SRCDIR}/imgui_impl_opengl3.cpp
SOURCES += ${IMGUI_SRCDIR}/imgui.cpp \
           ${IMGUI_SRCDIR}/imgui_demo.cpp \
           ${IMGUI_SRCDIR}/imgui_draw.cpp \
           ${IMGUI_SRCDIR}/imgui_widgets.cpp \
           ${IMGUI_SRCDIR}/ImGuiFileBrowser.cpp
SOURCES += ${DATE_SRCDIR}/tz.cpp

OBJS = $(addprefix ${BUILDDIR}/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
UNAME_S := $(shell uname -s)

CXXFLAGS =  -I${INCDIR} -I${LIBDIR} -I${IMGUI_INCDIR} -I${DATE_INCDIR} -I${IMGUI_INCDIR}/examples -I/usr/include/swe
# CXXFLAGS += -std=gnu++17 -lstdc++fs -O3 # RELEASE
CXXFLAGS += -std=gnu++17 -lstdc++fs -O0 -g # DEBUG
LIBS     =

# dependency check
DEPS := $(addprefix ${BUILDDIR}/, $(addsuffix .d, $(basename $(notdir $(SOURCES)))))

##---------------------------------------------------------------------
## OPENGL LOADER
##---------------------------------------------------------------------

## Using OpenGL loader: glew
## (This assumes a system-wide installation)

CXXFLAGS += -DIMGUI_IMPL_OPENGL_LOADER_GLEW                               # imgui
CXXFLAGS += -DHAS_REMOTE_API=0 -DAUTO_DOWNLOAD=0 -DINSTALL=${DATE_TZDIR}  # date/tz
LIBS += -lGLEW -lstdc++fs -lcurl


##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) # LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += -lGL -lswe `pkg-config --static --libs glfw3`

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) # APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += -L/usr/local/lib -L/opt/local/lib
	LIBS += -lglfw3 -lswe

	CXXFLAGS += -I/usr/local/include -I/opt/local/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lglfw3 -lgdi32 -lopengl32 -limm32 -lswe -lpthread -lole32

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

all: prep $(EXEC)
	@echo Build complete for $(ECHO_MESSAGE)

clean:
	rm -f $(EXEC) $(OBJS)
	rm -rf ${BUILDDIR}

prep:
	mkdir -p ${BUILDDIR}

.PHONY: all clean


$(EXEC): $(OBJS)
	$(CXX) -MMD -MP -o $@ $^ $(CXXFLAGS) $(LIBS)

-include ${DEPS}

${BUILDDIR}/%.o:%.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

${BUILDDIR}/%.o:${SRCDIR}/%.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

${BUILDDIR}/%.o:${IMGUI_SRCDIR}/%.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<
${BUILDDIR}/%.o:${IMGUI_SRCDIR}/examples/%.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

${BUILDDIR}/%.o:${DATE_SRCDIR}/%.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

