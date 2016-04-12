##############################################################################
# Copyright (c) 2011 Nathanael Hübbe                                         #
# All rights reserved.                                                       #
#                                                                            #
# Redistribution and use in source and binary forms, with or without         #
# modification, are permitted provided that the following conditions         #
# are met:                                                                   #
#                                                                            #
# 1. Redistributions of source code must retain the above copyright          #
#    notice, this list of conditions and the following disclaimer.           #
# 2. Redistributions in binary form must reproduce the above copyright       #
#    notice, this list of conditions and the following disclaimer in the     #
#    documentation and/or other materials provided with the distribution.    #
#                                                                            #
# THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND    #
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE      #
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE #
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE    #
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL #
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS    #
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)      #
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT #
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY  #
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF     #
# SUCH DAMAGE.                                                               #
##############################################################################

#Basic selection of generation modes.
#See the comment before the main() function for a description of all the modes and their parameters.
data_files :=
# data_files += data_0_1_0_0_0.nc
# data_files += data_0_1_0_0.5_-1.nc
# data_files += data_0_1_0_1_-1.nc
# data_files += data_0_1_0_0_0.nc

# data_files += data_0_1_1_0.9_-1.nc
# data_files += data_0_1_1_0_0.nc
# data_files += data_11_1_0.5__.nc
# data_files += data_22_1_0__.nc
# data_files += data_40____.nc
# data_files += data_41_1_100__.nc

#Some more modes.
# data_files += data_0_1_0_0.9_2.nc
# data_files += data_0_1_2_0.9_2.nc
# data_files += data_0_1_0_0.9_-1.nc
data_files += data_11_1_0.nc
data_files += data_11_1_0.25.nc
data_files += data_11_1_0.5.nc
data_files += data_11_1_0.75.nc
data_files += data_11_1_1.nc

file_to_params = $(subst _, ,$(subst data_,,$(subst .nc,,$(1))))

run: $(data_files)

#Uncomment to generate some nice little povray graphics to characterize the datasets
run: $(addsuffix .png,$(data_files))

$(addsuffix .png,$(data_files)):%.png: %.pov
	povray +O$*.png +W1000 +H750 +D +Q9 +A0.1 +R3 +UA $*.pov

$(addsuffix .pov,$(data_files)):%.pov: %
	@true	#No reciept here, it's made as a byproduct of the .nc file.

$(data_files):%: testFileGen
	./testFileGen $* $(call file_to_params,$*)
#	cdo settaxis,1970-01-01,00:00:00,1days -setgrid,r257x257 $* $*.temp
#	rm $*
#	mv $*.temp $*
# Uncomment the previous three lines to fake some metadata to the output files (requires the cdo tool).

build: testFileGen

testFileGen: main.c
	gcc -g -O3 -Wall -o testFileGen -L/opt/netcdf/4.1.1-hdf5/lib/ main.c -lnetcdf -lm

clean:
	rm -f $(data_files) $(addsuffix .pov,$(data_files)) $(addsuffix .png,$(data_files)) testFileGen
