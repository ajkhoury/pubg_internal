from idautils import *
from idc import *
from idaapi import *

#for ea in Segments():
#	if addr >= SegStart(ea) and addr < SegEnd(ea):
#		print '%x-%x' % (SegStart(ea),SegEnd(ea))

#
# !!WARNING!! MAKE SURE TO DEFINE THE TEXT AND DATA SEGMENTS FIRST!
#
textseg = idaapi.get_segm_by_name('.text')
dataseg = idaapi.get_segm_by_name('.data')
if not dataseg or not textseg:
	print "MAKE SURE TO DEFINE THE DATA AND TEXT SEGMENTS FIRST!!"
	exit(-3)

ea = dataseg.start_ea
print "%s: start=0x%x  end=0x%x" % (get_segm_name(dataseg), dataseg.start_ea, dataseg.end_ea)

# This should only take about a minute to complete on an i7 8700K 
while ea < dataseg.end_ea and ea != BADADDR:
	addr = get_qword(ea)
	if addr >= textseg.start_ea and addr < textseg.end_ea or addr >= dataseg.start_ea and addr < dataseg.end_ea:
		#print "  0x%x => 0x%x" % (ea, addr) # this should be kept commented out so it doesnt cause a perf hit
		del_items(ea, DELIT_SIMPLE, 8)
		create_data(ea, FF_QWORD, 8, ida_idaapi.BADADDR)
		op_offset(ea, 0, REF_OFF64)
		add_func(addr)
	ea += 8

print "Done.\n"
