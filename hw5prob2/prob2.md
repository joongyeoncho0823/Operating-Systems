### 2A
Does the scheme described in the lecture notes, with the use of the hardware PTE "A" bits and
the PFRA algorithm, achieve true LRU behavior? If not, does it come close and how?

It doesn't achieve LRU behavior, since it doesn't use a time field. It instead uses an ACCESSED bit to determine whether pages have been active between time intervals, making it impossible to determine which page was actually last used, though we have a few candidates. When physical RAM is scarce, the PFRA attempts to steal pages from ongoing processes that are already in memory (not free) and return them to the free pool. It uses the accessed bit of the PTE to determine which pages are being actively used. It scans the pages in the physical RAM to see if it has been accessed, and the accessed bit is cleared. Another bitwise flag stores the previous vaue of accessed bit as well. Using these two bits, it determines whether the page has been accessed between the last two scans. If it has not, then this page is moved to the tail of an "inactive" list. 
The PFRA also moves the pages off the "inactive" list" and reclaim them to be "free" in a FIFO order, thus approximating LRU.

### 2B
In your own words, what does the radix-64 tree of the address_space data structure do? What question is
the kernel asking when it consults such a data structure, and what answer is it providing? During paging-in, when does the kernel need to consult a radix-64 tree structure?

The kernel is asking whether the data for a given offset in a file is already in memory. If so, it can simply point to the existing page frame. The radix 64 tree tells the kernel whether
given offsets of a file (page-aligned offset) is in memory, and the page frame containing that image. It is used to handle page faults quickly when it has already been paged in memory.

### 2C
When handling a page fault, where the faulting virtual address is on the user side of the user/kernel line, which
data structure(s) is consulted to determine if a SIGSEGV should be delivered? Which fields of that struct(s) in
particular are examined?

A SIGSEGV is raised under these conditions:
- The attempted virtual address points to the kernel address.
- It consults the vm_area_struct to determine if the page is part of the stack region, using the vm_flags field, specifically checking if the VM_GROWDOWN bit of vm_flags is 1. If the VM_GROWSDOWN bit is 1, 
it then checks the vm_start field to determine if the start address of the region is higher than the page address, confirming it is a valid region. If it is not a valid region, then it raises a SIGSEGV.
- It consults the vm_area_struct vm_flags fields to determine whether the attempted R/W/X access type is permissible. If not, it raises SIGSEGV.

### 2D
During page frame reclamation (PFRA), a particular page frame is identified for reclamation based on inactivity.
The mapping field of the struct page indicates that the page frame is assocated with a file-backed region of
virtual memory and furthermore mapcount is 1 meaning this page frame is not mapped to any other virtual page
except the one in question. What determines whether the page frame must be written back to disk before being
placed on the free list?

Since the mapcount is only 1, we just need to check that one PTE to determine if the dirty bit is set.
If is not set, then it can be placed on the free list. If it is set, then the changes must be written back to disk
before being placed on the free list.


