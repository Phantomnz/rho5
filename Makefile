# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall

# Executable name
EXE = dashboard.exe

# ImGui paths (relative to project root)
IMGUI_DIR = imgui
BACKENDS  = $(IMGUI_DIR)/backends

# GLFW path
GLFW_DIR = glfw

# Includes
CXXFLAGS += \
    -I$(IMGUI_DIR) \
    -I$(BACKENDS) \
    -I$(GLFW_DIR)/include

# Libraries to link
# uses the local precompiled MinGW GLFW libs
LDFLAGS = \
    -L$(GLFW_DIR)/lib-mingw-w64 \
    -lglfw3 -lopengl32 -lgdi32 -limm32

# Source files
SOURCES = \
    main.cpp \
    SerialPort.cpp \
    $(IMGUI_DIR)/imgui.cpp \
    $(IMGUI_DIR)/imgui_draw.cpp \
    $(IMGUI_DIR)/imgui_tables.cpp \
    $(IMGUI_DIR)/imgui_widgets.cpp \
    $(IMGUI_DIR)/imgui_demo.cpp \
    $(BACKENDS)/imgui_impl_glfw.cpp \
    $(BACKENDS)/imgui_impl_opengl3.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default rule
all: $(EXE)

# Link final executable
$(EXE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXE) $(LDFLAGS)

# Compile each cpp to object
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(EXE) $(OBJECT)
