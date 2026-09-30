# DLMS/COSEM — Association Flow

## Problem
Trace a normal HDLC-based DLMS/COSEM association:

SNRM → UA → AARQ → AARE → application service → release/disconnect.

For each stage, identify its layer, purpose, important negotiated/authentication information, and typical failure symptoms.


## Scored contract clarification

State the chosen transport and relevant protocol edition/profile. Do not assume every deployment uses HDLC or the same authentication mode. Produce a layer map, transaction trace and failure matrix; label source gaps rather than inventing fields.

Timebox: agree before starting (default 30 minutes coding or 20 minutes oral/design). Status: Unseen. Original source mapping is recorded in career/MIGRATION.md.
