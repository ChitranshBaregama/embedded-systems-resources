# Security

The substantial security work in this account lives in its own repository:

### [DLMS/COSEM Security Manual →](https://github.com/ChitranshBaregama/EncryptionAlgorithm-)

Seven volumes on the DLMS/COSEM security stack (IEC 62056), built from the
Green Book 8th edition:

- Foundations and association security — HLS/LLS, the authentication mechanisms
- Symmetric core and wire format — AES-GCM/GMAC, byte-level analysis
- Keys, PKI and key agreement — GUEK/GBEK/GAK/KEK, ECDSA/ECDH, X.509 profiles
- General ciphering and packet analysis
- Embedded firmware implementation patterns
- Debugging, attack analysis, labs, and a capstone

Test vectors were verified independently in Python, and nine errors in the
source material are documented in the errata.

---

## In this repository

Firmware security material that belongs here rather than there is not written
yet. Planned, in priority order:

| Topic | Why it matters |
| :--- | :--- |
| Secure boot and chain of trust | ROM → bootloader → application, and where the root of trust actually lives |
| Firmware update security | Signature verification, rollback protection, the A/B slot argument |
| Debug port lockdown | JTAG/SWD disable, readout protection levels, and how each is bypassed |
| Fault injection | Voltage and clock glitching against a boot check, and countermeasures |
| Side channels | Timing and power analysis against an embedded AES implementation |
| Memory protection | MPU configuration, stack guard regions, `TrustZone-M` |

Note that `../architecture/memory-systems.md` already covers TrustZone memory
architecture, IOMMU/SMMU isolation, and memory side channels at part LII–LIII.

See [`../ROADMAP.md`](../ROADMAP.md).
