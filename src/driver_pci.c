/*
 * J. Kent Wirant
 * Started: July 12, 2021
 * Updated: May 29, 2023
 * PCI Driver Implementation
 */

#include "driver_pci.h"
#include "x86_util.h"

void pciReadTable(uint8_t bus, uint8_t device, uint8_t function, struct PCI_TABLE *table) {
	for(int i = 0; i < 64; i++) {
		((uint32_t *) table)[i] = pciConfigReadInt32(bus, device, function, 0);
	}
}

uint32_t pciConfigReadInt32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
	uint32_t ldevice = device & 0x1F; //device is 5 bits
	uint32_t lfunction = function & 0x07; //function is 3 bits
	uint32_t loffset = offset & ~(0x03); //lower two bits of offset should be 0
	uint32_t address = (1UL << 31 | bus << 16 | device << 11 | function << 8 | offset);
	x86_outd((uint16_t) PCI_REG_CFIG_ADDR, address);
	return x86_ind((uint16_t) PCI_REG_CFIG_DATA);
}

uint16_t pciConfigReadInt16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
	return pciConfigReadInt32(bus, device, function, offset) >> (8 * (offset & 2)) & 0xFFFF;
}

uint8_t pciConfigReadInt8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
	return pciConfigReadInt32(bus, device, function, offset) >> (8 * (offset & 3)) & 0xFF;
}

bool pciDeviceExists(uint8_t bus, uint8_t device, uint8_t function) {
	return pciConfigReadInt16(bus, device, function, PCI_HDR_VENDOR_ID) != 0xFFFF;
}

uint16_t pciEnumerate(uint16_t *functionList, uint16_t max) {
    uint16_t count = 0;

    for(int bus = 0; bus < 256; bus++) {
        for(int dev = 0; dev < 32; dev++) {
            if(pciDeviceExists(bus, dev, 0)) {
                functionList[count] = bus << 8 | dev << 3;
                count++;
                if(count >= max) return count;
            
                if((pciConfigReadInt8(bus, dev, 0, PCI_HDR_HEADER_TYPE) & 0x80) != 0) {
                    for(int fn = 1; fn < 8; fn++) {
                        if(pciDeviceExists(bus, dev, fn)) {
                            functionList[count] = bus << 8 | dev << 3 | fn;
                            count++;
                            if(count >= max) return count;
                        }
                    }
                }
            }
        }
    }

    return count;
}
