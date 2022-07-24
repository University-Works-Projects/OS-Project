## LEGENDA:
	UP = User-Processes
	SL = Support Level

# Inctroduction

### Ambiente per l'esecuzione degli UP
- Ogni Uproc è eseguito suo kuseg, con un unico ASID.
- Ogni Uproc ha il suo printer e terminale.
	
	[section 6.2-pops]
	
	UP will run in user-mode, with interrupts enabled.
### Il SL fornisce exceptions handlers per:
- TLB management exceptions [section 4.4]
- non-TLB exceptions: handler for positive syscalls and all program trap exceptions [Section 4.6]

	These two exceptions handler will run in kernel-mode with interrupts enabled.

# 4.1 Address Translation: The OS Perspective
	How the uMPS3 hardware supports address translation? [Chapter 6-pops] & [Figure 6.9-pops]	
- Every logical address for which translation is called for triggers a hardware search of the TLB seeking a matching TLB entry.
	- If no matching entry is found a TLI-Refil event is triggered.

- SL's TLB-Refil event handler:
	- This function will locate the Page Table entry in some SL data structure
	- Wrtie it into the TLB (TLBWR or TLBWI [section 6.4] & [section 4.5.2])
	- Return control (LDST) to the Current Process

	Once a matchin TLB entry is found, marked it valid

# 4.2 A U-proc's Logical Address Space and Backing Store

## 4.2.1 A U-proc's Page Table

## 4.2.2 A U-proc's Backing Store

## 4.3 The TLB-Refill event handler
### Technical point

# 4.4 Paging in Pandos

## 4.4.1 The Swap Pool
### Important point
### Technical point

## 4.4.2 Tha Pager
### Important point


