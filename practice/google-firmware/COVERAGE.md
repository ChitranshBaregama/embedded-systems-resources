# Coverage audit and gap policy

The existing 1,500 prompts remain the canonical numbered bank; catalog.csv was extracted from their actual text. The migration source's topic counts were a proposed allocation, not evidence that 1,500 corresponding files existed. Do not replace or renumber the existing bank to make those counts appear true.

| Required domain | Existing coverage route | Remaining assessment work |
| --- | --- | --- |
| C, pointers, memory, UB, bits | Bank tiers 1–3; solved interview core | Refine one-line contracts before scoring; test overflow and lifetime |
| Data structures and algorithms | Bank tiers 2, 3 and 5 | Timed mixed patterns and explanation; see DSA track |
| Startup/linker, MCU, GPIO, interrupts | Architecture references; bank hardware sections | Board-specific evidence and reset/interrupt traces |
| Timers, ADC, DMA, RTC | Bank drivers; ADC/RTC handbooks | DMA and clock-tree dedicated references remain gaps; use exact device manuals |
| UART, SPI, I2C, CAN | Peripheral references, portable parsers, CAN code | Fault-injection and bus-recovery assessment |
| FreeRTOS and concurrency | Bank scheduling/synchronization; Q0005–Q0007 | Generic RTOS exercises are not proof of FreeRTOS API proficiency |
| Debugging and testing | Bank fault/diagnostic sections; runnable examples | Evidence-driven investigations and target traces |
| Networking and protocols | Networking reference; bank protocol stacks | Packet analysis and malformed-stream work |
| Linux and kernel/driver fundamentals | Supplemental systems-design/interview track | Explicit Linux labs needed; not claimed complete in the C bank |
| C++ | Supplemental oral/design track | Ownership/RAII and concurrent lifetime labs needed |
| Bootloaders and firmware update | Bank OTA; flash reference; project 2 | Interrupted update and rollback evidence |
| Embedded security | Bank security; security repository references | Threat model and nonce persistence review |
| Firmware system design | Bank senior tier; system-design track | Timed tradeoff review and portfolio defense |

This is a qualitative coverage map, not a claim of equal depth in every domain. Weekly review may add targeted supplemental Q IDs without inflating the 1,500-bank completion count. Linux, C++, FreeRTOS-specific and board-level gaps remain visible until actual attempts and artifacts close them.
