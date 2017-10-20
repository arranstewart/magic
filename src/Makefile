# Edit the first few lines of this file to suit your setup.
# You *must* set MAGLIB to the appropriate data directory.

# Where the data files live (must be readable to the MaGIC user)

MAGLIB		= /FullPathnameOfMyDataDirectory   !!! EDIT !!!

# Where the binaries live

BIN		= /FullPathnameOfMyBin             !!! EDIT !!!

# Set the optimization level to O3 (includes inlining of functions) for
# performance, or g for debugging.

OPTIMIZATION	= -O3
# OPTIMIZATION	= -g -Wall

#
# Uncomment WARNINGS to see warning messages during compilation.

# WARNINGS	= -w

# If you have times.h (NB _not_ time.h) on your system leave the next 
# item alone.  If you don't, comment it out.

HASTIMES	= -DHASTIMES

##########################################################################
# If you have no gcc or if you wish to compile MaGIC with another        #
# compiler, change the next line to suit yourself.                       #
##########################################################################

CC		= gcc

##########################################################################
# You should not have to change the stuff below                          #
##########################################################################

CFLAGS		= $(OPTIMIZATION) $(HASTIMES) $(WARNINGS) -DDATA_DIR=\"$(MAGLIB)/\"
MOBJECTS        =  MaGIC.o axioms.o dialog.o getjob.o wffs.o mp_parse.o isom.o logic_set.o logic_test.o logic_pretest.o setup.o logic_io.o axiom_tests.o
RMOBJECTS	= RM.o RM_input.o RM_output.o
UTES		= u2p u2pic onegen one_plus_t_gen u2tex sub_irr
MORPHS 		= image embedded homomorphic nt_homomorphic
POSTPROCS	= $(UTES) $(MORPHS)
UTOBJECTS	= u2p.o u2pic.o onegen.o one_plus_t_gen.o u2tex.o sub_irr.o
MORPHOBJECTS	= image.o embedded.o homomorphic.o nt_homomorphic.o hmi.o


all: magic utilities

magic: $(MOBJECTS) vntr.o
	$(CC) -o magic $(MOBJECTS) vntr.o $(LOCLIB)

utilities: $(POSTPROCS)

u2p: u2p.o $(RMOBJECTS)
	$(CC) -o u2p u2p.o $(RMOBJECTS)

u2pic: u2pic.o $(RMOBJECTS)
	$(CC) -o u2pic u2pic.o $(RMOBJECTS)

onegen: onegen.o $(RMOBJECTS)
	$(CC) -o onegen onegen.o $(RMOBJECTS)

one_plus_t_gen: one_plus_t_gen.o $(RMOBJECTS)
	$(CC) -o one_plus_t_gen one_plus_t_gen.o $(RMOBJECTS)

u2tex: u2tex.o $(RMOBJECTS)
	$(CC) -o u2tex u2tex.o $(RMOBJECTS)

sub_irr: sub_irr.o $(RMOBJECTS)
	$(CC) -o sub_irr sub_irr.o $(RMOBJECTS)

image: image.o hmi.o $(RMOBJECTS)
	$(CC) -o image image.o hmi.o $(RMOBJECTS)

embedded: embedded.o hmi.o $(RMOBJECTS)
	$(CC) -o embedded embedded.o hmi.o $(RMOBJECTS)

homomorphic: homomorphic.o hmi.o $(RMOBJECTS)
	$(CC) -o homomorphic homomorphic.o hmi.o $(RMOBJECTS)

nt_homomorphic: nt_homomorphic.o hmi.o $(RMOBJECTS)
	$(CC) -o nt_homomorphic nt_homomorphic.o hmi.o $(RMOBJECTS)

install: all
	mkdir -p $(BIN)
	mv magic $(POSTPROCS) $(BIN)
	mkdir -p $(MAGLIB)
	cp ../dat/*.* $(MAGLIB)

clean:
	rm -f *.o core $(POSTPROCS) magic

$(MOBJECTS): MaGIC.h vntr.h Mproto.h Mdef.h Mglob.h Mtypes.h axioms.h
vntr.o: vntr.h
$(RMOBJECTS) $(UTOBJECTS) $(MORPHOBJECTS): RM.h
$(MORPHOBJECTS): hmi.h
