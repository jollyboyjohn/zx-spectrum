This is a quick guide to formatting a disk via a simple 8-bit IDE ZX Spectrum
interface.

Tools needed on PC:
- fuse-utils -- https://sourceforge.net/projects/fuse-emulator/
- 3e - http://www.zxprojects.com/images/stories/3e_card_manager/3e.zip
- hdfconv (my fork of 3e's hdf2hdf256 to write images back to a CF card)
- 3e-osx.patch - just in case you want to compile 3e on OSX

The 3e program in my opinion is the best method on *NIX to get files onto a CF
card.

### SPECTRUM ###

# Format the first disk (0) for 8 x 16MB partitions. Partitions are 16MB max.
# One partition is created for the filesystem management called 'PLUSIDEDOS'.
# A 256 MB card will only hold 128MB of data, so 8 x 16 MB partitions is ideal.
FORMAT TO 0,9

# Get disk geometry and check if the PLUSIDEDOS partition exists
CAT TAB

# Create a partition - max size is 16MB
NEW DATA "PartName",16

# Assign a partition to 
MOVE "C:" IN "PartName" ASN
LOAD "C:" ASN

### Linux ###

# Create an empty 128 byte HDF header with the correct geometry
createhdf -c -s -v1.0 695 15 48 empty.hdf
dd if=empty.hdf of=hdfheader.bin bs=128 count=1
rm empty.hdf

# Read the existing CF to a raw file
dd if=/dev/sdd of=raw.img bs=512

# Add the HDF header to the raw file
cat hdfheader.bin raw.img > imagefat.hdf

# Convert MSB+LSB to just LSB
hdfconv imagefat.hdf imageslim.hdf

# Check to see if it is working with 3e
3e imageslim.hdf showptable

# If working, clean up 
rm imagefat.hdf imagefat.img

# Upload a file
3e imageslim.hdf put elite.tap PartName:elite.tap 
3e imageslim.hdf dir PartName

# Convert back from LSB to MSB+LSB
hdfconv -8 imageslim.hdf imagefat.hdf 

# Write back to CF, skipping HDF header
dd ibs=128 obs=512 skip=1 if=imagefat.hdf of=/dev/sdd
