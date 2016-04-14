PROJ=radio_waves
TLC=tlc
PART=xc3s500e-4-fg320

BIN=/opt/Xilinx/10.1/ISE/bin/lin
XST=$(BIN)/xst
NGD=$(BIN)/ngdbuild
MAP=$(BIN)/map
PAR=$(BIN)/par
TRC=$(BIN)/trce
BIT=$(BIN)/bitgen
PRG=$(BIN)/promgen
SIM=$(BIN)/fuse
ISW=$(BIN)/isimwave

BITOPTS="		\
-w			\
-g DebugBitstream:No    \
-g Binary:yes		\
-b			\
-g IEEE1532:Yes		\
-g CRC:Enable		\
-g ConfigRate:1		\
-g ProgPin:PullUp	\
-g DonePin:PullUp	\
-g TckPin:PullUp	\
-g TdiPin:PullUp	\
-g TdoPin:PullUp	\
-g TmsPin:PullUp	\
-g UnusedPin:PullDown	\
-g UserID:0xFFFFFFFF	\
-g DCMShutdown:Disable	\
-g DONE_cycle:4		\
-g GTS_cycle:5		\
-g GWE_cycle:6		\
-g LCK_cycle:NoWait	\
-g Security:None	\
-g DonePipe:No		\
-g DriveDone:No		\
"
CLKBIT="-g StartUpClk:JtagClk"
CLKPRM="-g StartUpClk:CClk"

WAVFILE=radio_waves.wav

SRC_DIR=/home/iocoder/school/networks/project/radio_waves/src
LIB_DIR=lib
UPL_DIR=_upl
XST_DIR=_xst
NGD_DIR=_ngd
MAP_DIR=_map
PAR_DIR=_par
TRC_DIR=_trc
BIT_DIR=_bit
SIM_DIR=_sim
WORK_DIR=work

PKG_FILES=$(shell find $(SRC_DIR)   | grep .vhd | grep _pkg)
VHD_FILES=$(shell find $(SRC_DIR)   | grep -v _sim | grep -v _pkg | grep .vhd)
TMP_FILES=$(shell echo $(VHD_FILES) | sed 's/ /\\\\n/g')
NVC_FILES=$(shell echo -e $(TMP_FILES) | grep -v clkgen.vhd)
TST_FILES=$(shell find $(SRC_DIR)   | grep .vhd | grep _sim)
UCF_FILE=$(SRC_DIR)/$(PROJ).ucf

CC = gcc
INCDIR = /usr/local/include/digilent/adept
LIBDIR = /usr/local/lib64/digilent/adept
CFLAGS = -O0 -I $(INCDIR) -L $(LIBDIR) -ldmgr -ldepp
CFILES = $(SRC_DIR)/upload.c
BIST   = $(SRC_DIR)/nexys2bist.bit

$(BIT_DIR)/final.bit: $(PKG_FILES) $(VHD_FILES) $(UCF_FILE) Makefile
	## make _xst working directory
	mkdir -p $(XST_DIR)
	## generate .prj file
	rm -f $(XST_DIR)/$(PROJ).prj
	for i in $(PKG_FILES) $(VHD_FILES); do \
		echo vhdl work \"$$i\" >> $(XST_DIR)/$(PROJ).prj; \
	done;
	echo nosort >> $(XST_DIR)/$(PROJ).prj;
	## generate .xst script
	rm -f $(XST_DIR)/$(PROJ).xst
	echo "run -ifn $(PROJ).prj -p $(PART) -ofn $(PROJ).ngc \
              -ent $(TLC) -top $(TLC)" >> $(XST_DIR)/$(PROJ).xst
	## synthesize
	cd $(XST_DIR) && $(XST) -ifn $(PROJ).xst
	## make translate working directory
	mkdir -p $(NGD_DIR)
	## translate
	cd $(NGD_DIR) && $(NGD) -dd _ngo -nt timestamp -p $(PART) \
	   -uc $(UCF_FILE) ../$(XST_DIR)/$(PROJ).ngc $(PROJ).ngd
	## make mapping working directory
	mkdir -p $(MAP_DIR)
	## map
	cd $(MAP_DIR) && $(MAP) -p $(PART) \
	   ../$(NGD_DIR)/$(PROJ).ngd -o $(PROJ).ncd
	## make place & route working directory
	mkdir -p $(PAR_DIR)
	## place and route:
	cd $(PAR_DIR) && $(PAR) -w -t 1 ../$(MAP_DIR)/$(PROJ).ncd \
	   $(PROJ).ncd ../$(MAP_DIR)/$(PROJ).pcf
	## make post-par static timing working directory
	mkdir -p $(TRC_DIR)
	## tarce:
	cd $(TRC_DIR) && $(TRC) -e 3 -s 4 -xml $(PROJ) \
	   ../$(PAR_DIR)/$(PROJ).ncd -o $(PROJ).twr \
	   ../$(MAP_DIR)/$(PROJ).pcf -ucf $(UCF_FILE)
	## make genbit working directory
	mkdir -p $(BIT_DIR)
	## generate genbit command line file
	echo $(BITOPTS) $(CLKBIT) > $(BIT_DIR)/$(PROJ).ut
	## generate programming file:
	cd $(BIT_DIR) && $(BIT) -f $(PROJ).ut ../$(PAR_DIR)/$(PROJ).ncd \
	   final.bit

isim:
	## make fuse and isim working directory
	mkdir -p $(SIM_DIR)
	## generate .prj file
	rm -f $(SIM_DIR)/$(PROJ).prj
	for i in $(PKG_FILES) $(VHD_FILES) $(TST_FILES); do \
		echo vhdl work \"$$i\" >> $(SIM_DIR)/$(PROJ).prj; \
	done;
	## run fuse
	cd $(SIM_DIR) && $(SIM) --prj $(PROJ).prj -t $(TB) -o $(TB).exe
	## generate simcmd TCL batch file:
	rm -rf $(SIM_DIR)/simcmd
	echo "ntrace select -o on -m / -l this" >> $(SIM_DIR)/simcmd
	echo "ntrace start" >> $(SIM_DIR)/simcmd
	echo "isimrun 1000000 ns" >> $(SIM_DIR)/simcmd
	echo "quit" >> $(SIM_DIR)/simcmd
	## run simulation
	cd $(SIM_DIR) && XILINX=/opt/Xilinx/10.1/ISE ./$(TB).exe -tclbatch \
	   simcmd -wavefile $(TB).xwv
	## show results
	cd $(SIM_DIR) && $(ISW) $(TB).xwv

nvcsim:
	nvc -L $(LIB_DIR) -a $(PKG_FILES) $(TST_FILES) $(NVC_FILES)
	nvc -e tlc_sim
	nvc -r --load=../vhdsim/vhdsim.so --wave=waveform.gw tlc_sim

timing:
	LD_LIBRARY_PATH=/opt/Xilinx/10.1/ISE/lib/lin \
	XILINX=/opt/Xilinx/10.1/ISE \
	/opt/Xilinx/10.1/ISE/bin/lin/_timingan_old -ngd \
	/home/Files/Projects/graduation/build/hardware/_ngd/mipscomp.ngd \
	/home/Files/Projects/graduation/build/hardware/_par/mipscomp.ncd \
	/home/Files/Projects/graduation/build/hardware/_map/mipscomp.pcf \
	/home/Files/Projects/graduation/build/hardware/_trc/mipscomp.twr

prog: $(BIT_DIR)/final.bit
	## make _upl working directory
	mkdir -p $(UPL_DIR)
	## convert wav file to raw
	sox $(SRC_DIR)/$(WAVFILE) -r 8k -b 8 -L -c 1 -e unsigned \
	    --norm $(UPL_DIR)/music.raw gain -h +30 dither
	## compile upload.c
	$(CC) -o $(UPL_DIR)/upload $(CFLAGS) $(CFILES)
	## upload nexys2bist.bit to fpga
	djtgcfg enum
	djtgcfg init -d Nexys2
	djtgcfg prog -d Nexys2 -i 0 -f $(BIST)
	echo idle for 3 seconds...
	sleep 1
	## put wav file
	## $(UPL_DIR)/upload erase
	## $(UPL_DIR)/upload $(UPL_DIR)/music.raw 0x000000 0x200000
	## now upload our design itself to fpga
	djtgcfg enum
	djtgcfg init -d Nexys2
	djtgcfg prog -d Nexys2 -i 0 -f $(BIT_DIR)/final.bit

prog_perm: $(BIT_DIR)/final.bit
	## generate genbit command line file
	echo $(BITOPTS) $(CLKPRM) > bit/$(PROJ)_perm.ut
	## generate programming file for PROM:
	cd bit && $(BIT) -f $(PROJ)_perm.ut ../par/$(PROJ).ncd final_perm.bit
	## generate PROM FILE
	cd bit && $(PRG) -w -p mcs -c FF -o final -u 0 final_perm.bit -x xcf04s
	## program the PROM
	djtgcfg enum
	djtgcfg init -d Nexys2
	djtgcfg prog -d Nexys2 -i 1 -f $(BIT_DIR)/final.mcs
	## delay
	sleep 5
	## program the FPGA
	djtgcfg enum
	djtgcfg init -d Nexys2
	djtgcfg prog -d Nexys2 -i 0 -f $(BIT_DIR)/final.bit
	## inform the user of what they should do.
	@echo The configuration ROM and the FPGA have been programmed.
	@echo Please make sure that the switches on your FPGA board are
	@echo correctly configured to automatically program the FPGA from the
	@echo platform PROM whenever the board is powered up.

clean-local:
	rm -rf $(XST_DIR) $(NGD_DIR) $(MAP_DIR) $(PAR_DIR) \
	       $(TRC_DIR) $(BIT_DIR) $(SIM_DIR) $(WORK_DIR)
