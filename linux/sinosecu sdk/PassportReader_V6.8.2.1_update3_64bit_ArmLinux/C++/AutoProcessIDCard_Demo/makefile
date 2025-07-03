EXECUTABLE :=../../lib/AutoProcessIDCard_Test
#CFLAGS := -g -Wall -O
CFLAGS := -O2
CXXFLAGS := $(CFLAGS)

SOURCE := $(wildcard ./*.c) $(wildcard ./*.cpp)
OBJS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
DEPS := $(patsubst %.o,%.d,$(OBJS))
MISSING_DEPS := $(filter-out $(wildcard $(DEPS)),$(DEPS))
MISSING_DEPS_SOURCES := $(wildcard $(patsubst %.d,%.c,$(MISSING_DEPS)) \
$(patsubst %.d,%.cpp,$(MISSING_DEPS)))
CPPFLAGS +=-MD
.PHONY : everything deps objs clean veryclean rebuild dllLib
everything : $(EXECUTABLE)
deps : $(DEPS)
objs : $(OBJS)
clean :
	$(RM) ./*.o
	$(RM) ./*.d
veryclean: clean
	@$(RM) $(EXECUTABLE)
rebuild: veryclean everything
ifneq ($(MISSING_DEPS),)
$(MISSING_DEPS) :
	@$(RM) $(patsubst %.d,%.o,$@)
endif
-include $(DEPS)
$(EXECUTABLE) : $(OBJS) $(LIBS)
	g++ -o $(EXECUTABLE) $(OBJS) $(LIBS) -lpthread -lrt -ldl -lstdc++
