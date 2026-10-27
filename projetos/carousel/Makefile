APPLICATION = carousel
VART_DIR = ../vart
CPPFLAGS += -I..
CXXFLAGS += -Wall -DVART_OGL -DIL_LIB -std=c++11
LDLIBS += -lGL -lglut -lGLU -lIL

CORE = point4d color light texture material boundingbox memoryobj \
       sgpath snlocator scenenode graphicobj sphere cylinder mesh \
       transform bezier modifier dof joint uniaxialjoint biaxialjoint \
       polyaxialjoint camera meshobject arrow picknamelocator scene file
CONTRIB = mousecontrol viewerglutogl
OBJECTS = $(addprefix build/,$(addsuffix .o,$(CORE) $(CONTRIB))) build/main.o

.PHONY: all clean
all: $(APPLICATION)

$(APPLICATION): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

build:
	mkdir -p build

build/main.o: main.cpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

build/%.o: $(VART_DIR)/source/%.cpp $(VART_DIR)/%.h | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

build/%.o: $(VART_DIR)/contrib/source/%.cpp $(VART_DIR)/contrib/%.h | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(OBJECTS:.o=.d)

clean:
	rm -f build/*.o build/*.d $(APPLICATION)
