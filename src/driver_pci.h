/*
 * J. Kent Wirant
 * Started: July 12, 2021
 * Updated: May 29, 2023
 * PCI Driver Header
 */

#ifndef DRIVER_PCI_H
#define DRIVER_PCI_H

#include <stdint.h>
#include <stdbool.h>

#define PCI_HDR_VENDOR_ID							0x00
#define PCI_HDR_DEVICE_ID							0x02
#define PCI_HDR_COMMAND								0x04
#define PCI_HDR_STATUS								0x06
#define PCI_HDR_CLASS_CODE							0x0B
#define PCI_HDR_SUBCLASS							0x0A
#define PCI_HDR_PROG_IF								0x09
#define PCI_HDR_REVISION_ID							0x08
#define PCI_HDR_BIST								0x0F
#define PCI_HDR_HEADER_TYPE							0x0E
#define PCI_HDR_LATENCY_TIMER						0x0D
#define PCI_HDR_CACHE_LINE_SIZE						0x0C

#define PCI_HDR0_BAR0								0x10
#define PCI_HDR0_BAR1								0x14
#define PCI_HDR0_BAR2								0x18
#define PCI_HDR0_BAR3								0x1C
#define PCI_HDR0_BAR4								0x20
#define PCI_HDR0_BAR5								0x24
#define PCI_HDR0_CARDBUS_CIS_PTR					0x28
#define PCI_HDR0_SUBSYSTEM_DEVIDE_ID				0x2E
#define PCI_HDR0_SUBSYSTEM_VENDOR_ID				0x2C
#define PCI_HDR0_EXPANSION_ROM_BASE_ADDR			0x30
#define PCI_HDR0_CAPABILITIES_PTR					0x34
#define PCI_HDR0_INTERRUPT_LINE						0x3C
#define PCI_HDR0_INTERRUPT_PIN						0x3D
#define PCI_HDR0_MAX_LATENCY						0x3F
#define PCI_HDR0_MIN_GRANT							0x3E

#define PCI_HDR1_BAR0								0x10
#define PCI_HDR1_BAR1								0x14
#define PCI_HDR1_SECONDARY_LATENCY_TIMER			0x1B
#define PCI_HDR1_SUBORDINATE_BUS_NUMBER				0x1A
#define PCI_HDR1_SECONDARY_BUS_NUMBER				0x19
#define PCI_HDR1_PRIMARY_BUS_NUMBER					0x18
#define PCI_HDR1_SECONDARY_STATUS					0x1E
#define PCI_HDR1_IO_LIMIT_LOWER						0x1D
#define PCI_HDR1_IO_BASE_LOWER						0x1C
#define PCI_HDR1_MEMORY_LIMIT						0x22
#define PCI_HDR1_MEMORY_BASE						0x20
#define PCI_HDR1_PREFETCHABLE_MEMORY_LIMIT_LOWER	0x26
#define PCI_HDR1_PREFETCHABLE_MEMORY_BASE_LOWER		0x24
#define PCI_HDR1_PREFETCHABLE_MEMORY_LIMIT_UPPER	0x2C
#define PCI_HDR1_PREFETCHABLE_MEMORY_BASE_UPPER		0x28
#define PCI_HDR1_IO_LIMIT_UPPER						0x32
#define PCI_HDR1_IO_BASE_UPPER						0x30
#define PCI_HDR1_CAPABILITY_PTR						0x34
#define PCI_HDR1_EXPANSION_ROM_BASE_ADDR			0x38
#define PCI_HDR1_BRIDGE_CONTROL						0x3E
#define PCI_HDR1_INTERRUPT_PIN						0x3D
#define PCI_HDR1_INTERRUPT_LINE						0x3C

#define PCI_HDR2_CARDBUS_BASE_ADDR					0x10
#define PCI_HDR2_CAPABILITIES_OFFSET				0x14
#define PCI_HDR2_SECONDARY_STATUS					0x16
#define PCI_HDR2_PCI_BUS_NUMBER						0x18
#define PCI_HDR2_CARDBUS_BUS_NUMBER					0x19
#define PCI_HDR2_SUBORDINATE_BUS_NUMBER				0x1A
#define PCI_HDR2_CARDBUS_LATENCY_TIMER				0x1B
#define PCI_HDR2_MEMORY_BASE_ADDR0					0x1C
#define PCI_HDR2_MEMORY_LIMIT0						0x20
#define PCI_HDR2_MEMORY_BASE_ADDR1					0x24
#define PCI_HDR2_MEMORY_LIMIT1						0x28
#define PCI_HDR2_IO_BASE_ADDR0						0x2C
#define PCI_HDR2_IO_LIMIT0							0x30
#define PCI_HDR2_IO_BASE_ADDR1						0x34
#define PCI_HDR2_IO_LIMIT1							0x38
#define PCI_HDR2_INTERRUPT_LINE						0x3C
#define PCI_HDR2_INTERRUPT_PIN						0x3D
#define PCI_HDR2_BRIDGE_CONTROL						0x3E
#define PCI_HDR2_SUBSYSTEM_DEVICE_ID				0x40
#define PCI_HDR2_SUBSYSTEM_VENDOR_ID				0x42
#define PCI_HDR2_PC_CARD_BASE_ADDR					0x44

#define PCI_REG_CFIG_ADDR_ENABLE					31
#define PCI_REG_CFIG_ADDR_BUS						16
#define PCI_REG_CFIG_ADDR_DEVICE					11
#define PCI_REG_CFIG_ADDR_FUNCTION					 8
#define PCI_REG_CFIG_ADDR_OFFSET					 0

#define PCI_REG_HEADER_TYPE					 		 0
#define PCI_REG_HEADER_TYPE_MULTI_FUNCTION	 		 7

#define PCI_REG_CFIG_ADDR							0x0CF8
#define PCI_REG_CFIG_DATA							0x0CFC

struct PCI_TABLE {
	uint16_t vendorId;
	uint16_t deviceId;
	uint16_t command;
	uint16_t status;
	uint8_t revisionId;
	uint8_t progIf;
	uint8_t subclass;
	uint8_t classCode;
	uint8_t cacheLineSize;
	uint8_t latencyTimer;
	uint8_t headerType;
	uint8_t bist;
	
	union {
		struct PCI_HDR0 {
			uint32_t bar0;
			uint32_t bar1;
			uint32_t bar2;
			uint32_t bar3;
			uint32_t bar4;
			uint32_t bar5;
			uint32_t cardbusCisPtr;
			uint16_t subsystemVendorId;
			uint16_t subsystemDeviceId;
			uint32_t expansionRomBaseAddr;
			uint8_t  capabilitiesPtr;
			uint8_t reserved[7]; //reserved
			uint8_t interruptLine;
			uint8_t interruptPin;
			uint8_t minGrant;
			uint8_t maxLatency;
		} hdr0;
		
		struct PCI_HDR1 {
			uint32_t bar0;
			uint32_t bar1;
			uint8_t primaryBusNumber;
			uint8_t secondaryBusNumber;
			uint8_t subordinateBusNumber;
			uint8_t secondaryLatencyTimer;
			uint8_t ioBaseLower;
			uint8_t ioLimitLower;
			uint16_t secondaryStatus;
			uint16_t memoryBase;
			uint16_t memoryLimit;
			uint16_t prefetchableMemoryBaseLower;
			uint16_t prefetchableMemoryLimitLower;
			uint32_t prefetchableMemoryBaseUpper;
			uint32_t prefetchableMemoryLimitUpper;
			uint16_t ioBaseUpper;
			uint16_t ioLimitUpper;
			uint8_t capabilityPtr;
			uint32_t epxansionRomBaseAddr;
			uint8_t interruptLine;
			uint8_t interruptPin;
			uint16_t bridgeControl;
		} hdr1;
		
		struct PCI_HDR2 {
			uint8_t placeholder;
		} hdr2;
		
		uint8_t deviceSpecific[192]; //ensures that total struct size is 256 bytes
	};
};

void pciReadTable(uint8_t bus, uint8_t device, uint8_t function, struct PCI_TABLE *table);

uint32_t pciConfigReadInt32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint16_t pciConfigReadInt16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint8_t pciConfigReadInt8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

bool pciDeviceExists(uint8_t bus, uint8_t device, uint8_t function);

uint16_t pciEnumerate(uint16_t *functionList, uint16_t max);

#endif
