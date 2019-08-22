
#makefile ͨ���ļ���ʹ��ǰ��Ҫ��ʼ�����²���
#1.�汾 ����ֵΪrelease or debug
#	VER=release

#2.Ŀ�� ��̬����ִ���ļ�
#	BIN := 
#	LIB := 

#3.��������� sln�ļ����ڵ�·��
#	ROOT := ../../

#4.ͷ�ļ�Ŀ¼
#	INCLUDE_DIRS +=

#5.��Ҫ�Ŀ⼰���ڵ�Ŀ¼
#	LIBRARY_DIRS +=
#	LIBRARY_NAMES += 
# mem=tcmalloc

#6.��
#	PREPROCESSOR_MACROS += 

PREPROCESSOR_MACROS += 

SRCDIRS := . 
INCLUDE_DIRS += $(SRCDIRS) 

LIBRARY_DIRS += $(ROOT)libs

LIBRARY_NAMES += pthread

ADDITIONAL_LINKER_INPUTS +=

SRCEXTS   :=.cpp .cc .cxx .c
CPPFLAGS  += -ggdb -Wall -Werror -ffunction-sections -Wno-write-strings -std=c++17
CFLAGS    := -ggdb -Wall -Werror

# -fno-exceptions -Wno-unused-variable -mcmodel=medium

IGNORE_FLAGS := -Wno-sign-compare -fpermissive -Wno-unused-function -Wno-class-memaccess
CPPFLAGS += $(IGNORE_FLAGS)

ifeq ($(VER), debug)
  CPPFLAGS += -g -O0
  CFLAGS   += -g -O0
else
  #CPPFLAGS += -g -O0 -DNDEBUG -static
  #CFLAGS   += -g -O0 -DNDEBUG -static
  CPPFLAGS += -g -O0 -static
  CFLAGS   += -g -O0 -static
endif

ifdef LIB
	LIBRARY := lib$(LIB).a
endif

ifdef BIN
	ifeq ($(VER), debug)
		APP := $(BIN)D
	else
		APP := $(BIN)
	endif
endif

CXXFLAGS  := 
LDFLAGS   :=

ifeq ($(mem), tcmalloc)
LDFLAGS   += -ltcmalloc
endif
LDFLAGS   +=

CXXFLAGS += $(addprefix -D,$(PREPROCESSOR_MACROS))
CXXFLAGS += $(addprefix -I,$(INCLUDE_DIRS))

LDFLAGS += $(addprefix -L,$(LIBRARY_DIRS))
LDFLAGS += $(addprefix -l,$(LIBRARY_NAMES))
LDFLAGS += $(ADDITIONAL_LINKER_INPUTS)

##=============================================================================

CC      = gcc
CXX     = g++
RM      = rm -f
AR		= ar rusv

##=============================================================================
SHELL   = /bin/sh
SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
OBJS    = $(foreach x,$(SRCEXTS),$(sort $(patsubst %$(x),%.o,$(filter %$(x),$(SOURCES)))))
DEPS    = $(patsubst %.o,%.d,$(OBJS))

define gen_cxx_dep
set -e; rm -f $@; \
$(CC) -MM $(CPPFLAGS) $(CXXFLAGS) $< >$@.; \
sed 's,\($(*F)\)\.o[ :]*,$(*D)/\1.o $@ : ,g' < $@. > $@; \
rm -f $@.
endef

define gen_cc_dep
set -e; rm -f $@; \
$(CC) -MM $(CFLAGS) $(CXXFLAGS) $< >$@.; \
sed 's,\($(*F)\)\.o[ :]*,$(*D)/\1.o $@ : ,g' < $@. > $@; \
rm -f $@.
endef

.PHONY : all clean cleanall rebuild

all : $(APP) $(LIBRARY)

%.d : %.cpp
	@echo "gen dep++++++++++++++++"$<
	@$(gen_cxx_dep)
%.d : %.cc
	@echo "gen dep++++++++++++++++"$<
	@$(gen_cxx_dep)
%.d : %.cxx
	@echo "gen dep++++++++++++++++"$<
	@$(gen_cxx_dep)
%.d : %.c
	@echo "gen dep++++++++++++++++"$<
	@$(gen_cc_dep)

%.o : %.cpp
	@echo "compile obj++++++++++++++++"$<
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
%.o : %.cc
	@echo "compile obj++++++++++++++++"$<
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
%.o : %.cxx
	@echo "compile obj++++++++++++++++"$<
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
%.o : %.c
	@echo "compile obj++++++++++++++++"$<
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@
	
# Rules for producing the executable.

#----------------------------------------------
$(APP): $(OBJS)
	@echo "progrom++++++++++++++++"$@
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) 
	cp $@ ../bin/$@

$(LIBRARY): $(OBJS)
	@echo "ar library++++++"$@
	$(AR) $@ $(OBJS)
	mkdir -p $(ROOT)libs
	rm $(ROOT)libs/$@ -rf
#	mkdir lib
	cp $@ $(ROOT)libs/$@

-include $(DEPS)

rebuild: cleanall all

clean :	
	@$(RM) $(OBJS) $(DEPS)
	
cleanall: clean
	@$(RM) $(APP) $(LIBRARY)

