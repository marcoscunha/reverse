# Copyright (c) ENS Lyon/LIP      2004-2007
#               INSA Lyon/CITI    2004
#               INRIA             2004
#               INP Grenoble/Tima 2008
# 					Nicolas Fournel   <nicolas.fournel@imag.fr>
# 					Antoine Fraboulet <antoine.fraboulet@insa-lyon.fr>
# 
# This software is a computer program whose purpose is to help users to 
# produce and compute etrace format execution traces.
# 
# This software is governed by the CeCILL license under French law and
# abiding by the rules of distribution of free software.  You can  use, 
# modify and/ or redistribute the software under the terms of the CeCILL
# license as circulated by CEA, CNRS and INRIA at the following URL
# "http://www.cecill.info". 
# 
# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability. 
# 
# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or 
# data to be ensured and,  more generally, to use and operate it in the 
# same conditions as regards security. 
# 
# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL license and that you accept its terms.


CC        = gcc
LD        = gcc
AR        = ar

CPPFLAGS  = -I$(LIBDWARF_BASE) -I../debughelperlib/include -Iinclude
#CFLAGS    = -Wall -Wextra -Werror -O3 -c -g -fPIC
CFLAGS    = -Wall -Wextra -Werror -c -g -fPIC
LDFLAGS   =
ARFLAGS   = cru

SRCDIR    = src
OBJDIR    = obj

ifndef $(NO_DEP)
DEP_CFLAGS = -MP -MMD -MF $(OBJDIR)/$*.d
CFLAGS    += $(DEP_CFLAGS)
endif


OBJECTS   = $(OBJDIR)/callstack.o $(OBJDIR)/ctxt_stack.o

LIB       = libcstack.a

quiet-command = $(if $(VERB),$1,$(if $(2),@echo $2 && $1, @$1))

all : $(OBJDIR) $(LIB)

.PHONY  : clean

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(call quiet-command, $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<, "  CC     $<")

ifndef $(NO_DEP)
-include $(CFILES:%.c=$(DEPDIR)/%.d)
endif

$(LIB) : $(OBJECTS)
	$(call quiet-command, $(AR) $(ARFLAGS) $@ $^, "  AR     $@")

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean   :
	$(call quiet-command, rm -f $(OBJECTS) $(LIB), "  CLEAN    ")

distclean: clean
	$(call quiet-command, rm -fr $(OBJDIR), "  CLEAN    dist")
