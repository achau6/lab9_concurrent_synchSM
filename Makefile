# University of California, Riverside
# CS120B Makefile
MMCU=atmega1284p
FREQ=8000000

PATHB=build/
PATHO=$(PATHB)objects/
PATHS=source/
PATHT=test/
PATHH=header/
PATHR=$(PATHB)results/

SOURCES=$(wildcard $(PATHS)*.c)
OBJS=$(patsubst $(PATHS)%,$(PATHO)%,$(SOURCES:.c=.o))

CLEAN=rm -rf
# Simulator
SIMAVRDIR=/usr/csshare/pkgs/simavr/
SIMAVR=simavr

# Compiler
AVR=avr-gcc
DEBUGFLAGS=-g -O0
SIMFLAGS=-D_SIMULATE_
# Place the section past the end of reachable memory
MMCUSECTION=-Wl,--undefined=_mmcu,--section-start=.mmcu=910000 
FLAGS=-Wall -mmcu=$(MMCU) $(MMCUSECTION)
INCLUDES=-I./$(PATHH) -I$(SIMAVRDIR)
OBJCOPY=avr-objcopy
OBJFLAGS=-j .text -j .data -O ihex
# Debugger
GDB=/usr/csshare/pkgs/avr-gdb-9.2-with-python/gdb/gdb
# GDB Testing
WHILELINENO=$$(grep -n -m 1 "while\s*(1)" source/main.c | cut -f1 -d:)
GDBCOMMANDS=$(PATHT)commands.gdb
GDBSCRIPT=$(PATHT)tests.gdb
INITGDBDEBUGGER=$(PATHT)initDebugger.gdb
GDBTESTING=-batch -x $(GDBCOMMANDS) -x $(INITGDBDEBUGGER) -x $(GDBSCRIPT)
GDBDEBUGGING=-x $(GDBCOMMANDS) -x $(INITGDBDEBUGGER)
# Python testing
PYTESTRUNNER=$(PATHT)testRunner.py
PYTESTS=$(PATHT)tests.py
PYTESTCMD=runTests
PYTESTING=-batch -x $(PYTESTS) -x $(PYTESTRUNNER) -ex $(PYTESTCMD)
PYDEBUGGING=-x $(PYTESTS) -x $(PYTESTRUNNER)
# Programmer
PROGRAM=avrdude
PROGRAMMER=atmelice_isp
MEMORY=flash
#mem:op:file where op is (w)rite, (r)ead, (v)erify; performed in the order specified
VERBOSITY=#-v -v ... etc. to have more verbose output. Normal output is usually enough unless something goes wrong. 
ADDVERBOSITY=$(info Add additional -v to the \"avrdude -c ...\" command above for more details on the error)
HIGH=hfuse
HDEFAULT=0x99
H120=0xD9
HTXT=hfuse.txt
JTAGEN=0x40
LOW=lfuse
LDEFAULT=0x42
L120=0xC2
HEX=h
RAW=m

.PHONY: defaultFuses verifyFuses fuses disableJTAG clean test program debug pytest pydebug
all: $(PATHB)main.hex

verifyFuses: 
	$(PROGRAM) -c $(PROGRAMMER) -p $(MMCU) $(VERBOSITY) -U $(HIGH):r:-:$(HEX) -U $(LOW):r:-:$(HEX)

defaultFuses: 
	$(PROGRAM) -c $(PROGRAMMER) -p $(MMCU) $(VERBOSITY) -U $(HIGH):w:$(HDEFAULT):$(RAW) -U $(LOW):w:$(LDEFAULT):$(RAW)

fuses: 
	$(PROGRAM) -c $(PROGRAMMER) -p $(MMCU) $(VERBOSITY) -U $(HIGH):w:$(H120):$(RAW) -U $(LOW):w:$(L120):$(RAW)

# Disable just the JTAG fuse
disableJTAG: 
	$(PROGRAM) -c $(PROGRAMMER) -p $(MMCU) $(VERBOSITY) -U $(HIGH):r:$(HTXT):$(HEX)
	@read -n 4 c < $(HTXT);echo -n "0x" > $(HTXT);echo "obase=16; $$(($$c | $(JTAGEN)))" | bc >> $(HTXT)
	$(PROGRAM) -c $(PROGRAMMER) -p $(MMCU) $(VERBOSITY) -U $(HIGH):w:$(HTXT):$(HEX) 
	@rm -f $(HTXT)

program: $(PATHB)main.hex
	$(PROGRAM) -c $(PROGRAMMER) -p $(MMCU) $(VERBOSITY) -U $(MEMORY):w:$<

debug: $(PATHO)main.elf
	@sed -i "s/break main.c:.*/break main.c:$(WHILELINENO)/" $(INITDEBUGGER)
	$(SIMAVR) -g -mmcu=$(MMCU) -f $(FREQ) $< &
	-$(GDB) $(GDBDEBUGGING)
	@pkill simavr

test: $(PATHO)main.elf
	@sed -i "s/break main.c:.*/break main.c:$(WHILELINENO)/" $(INITDEBUGGER)
	$(SIMAVR) -g -mmcu=$(MMCU) -f $(FREQ) $< &
	-$(GDB) $(GDBTESTING)
	@pkill simavr

pytest: $(PATHO)main.elf
	$(SIMAVR) -g -mmcu=$(MMCU) -f $(FREQ) $< &
	-$(GDB) -se=$< $(PYTESTING)
	@pkill simavr

pydebug: $(PATHO)main.elf
	$(SIMAVR) -g -mmcu=$(MMCU) -f $(FREQ) $< &
	-$(GDB) -se=$< $(PYDEBUGGING)
	@pkill simavr

$(PATHB)main.hex: $(PATHO)main.elf
	@$(OBJCOPY) $(OBJFLAGS) $< $@

$(PATHO)main.elf: $(OBJS)
	@$(AVR) $(DEBUGFLAGS) $(SIMFLAGS) $(FLAGS) $(INCLUDES) -o $@ $^

$(PATHO)%.o: $(PATHS)%.c
	@$(AVR) $(DEBUGFLAGS) $(SIMFLAGS) $(FLAGS) $(INCLUDES) -c -o $@ $<

clean:
	-$(CLEAN) $(PATHO)*.o $(PATHO)*.elf $(PATHB)*.hex
	-$(CLEAN) $(PATHR)*.vcd
	-@pkill simavr
