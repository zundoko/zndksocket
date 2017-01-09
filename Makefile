#
# @file     Makefile
# @brief    Makefile for socket porgramming test
#
# @note     https://github.com/zundoko/zndksocket
#
# @date     2017-01-09
# @authr    zundoko
#

PRJNAME =   zndksocket

TARGETS = $(PRJNAME)_server $(PRJNAME)_client
SRCS    = $(TARGETS:%=%.c) lan_msg.c
OBJS    = $(SRCS:%.c=%.o)
DEPEND  =   Makefile.depend

CC      = $(CROSS_COMPILE)gcc
INC     =  -I.
CFLAGS  = $(INC) -c -Wall -Werror
LDFLAGS = 
LIBS    = -lrt

.PHONY: all
all: $(TARGETS)

.PHONY: $(TARGETS)
$(TARGETS): %:%.o lan_msg.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $<

.PHONY: clean
clean:
	-rm -rf $(OBJS) $(TARGETS) *~

.PHONY: depend
depend:
	-rm -rf $(DEPEND)
	$(CC) -MM -MG $(CFLAGS) $(SRCS) > $(DEPEND)

-include  $(DEPEND)

# end
