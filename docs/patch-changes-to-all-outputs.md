# Plan: Move Program Changes to Patch Level

## Overview

Program changes move from `Connection` (one per connection) to `Patch`
(one per output instrument/channel pair, optional). A new lightweight
`InstrumentProgramChange` struct holds the data. Multiple entries for
the same output instrument on different MIDI channels are allowed. The
patch sends all its program changes when it starts, before activating
connections.

The output instrument picker in the patch editor shows **all** available
outputs from `DeviceManager`, not just those referenced by the patch's
connections.

---

## Step 1 — New `InstrumentProgramChange` Struct

**New files:** `Source/km/instrument_program_change.h/.cpp`

```cpp
struct InstrumentProgramChange {
    Output::Ptr output;
    int channel;    // 1–16; program changes always need a specific channel
    int bank_msb;   // UNDEFINED if not set
    int bank_lsb;   // UNDEFINED if not set
    int prog;       // UNDEFINED if not set

    void send();
};
```

`send()` sends CC 0 (MSB), CC 32 (LSB), and program change on
`output`/`channel` for any field that is not `UNDEFINED`.

Add both files to the `.jucer` project.

---

## Step 2 — Modify `Patch`

**`patch.h`:**
- Add `#include "instrument_program_change.h"`
- Add `Array<InstrumentProgramChange> _instrument_program_changes;`
- Add `instrument_program_changes()`,
  `add_instrument_program_change(InstrumentProgramChange)`,
  `remove_instrument_program_change(int index)`

**`patch.cpp`:** In `start()`, before the connections loop, call
`send()` on each entry in `_instrument_program_changes`.

---

## Step 3 — Strip Program Changes from `Connection`

**`connection.h`** — remove:
- `_prog` field
- `program_bank_msb/lsb/prog()` getters
- `set_program_bank_msb/lsb/prog()` declarations
- `program_change_can_be_sent()` and `program_change_send_channel()`
  declarations

**`connection.cpp`** — remove:
- Program change sending block in `start()` (lines ~41–56)
- All three setter bodies (lines ~94–113)
- `program_change_can_be_sent()` and `program_change_send_channel()`
  bodies (lines ~165–169)

---

## Step 4 — Update `Storage`

**Load:** Remove the four lines reading `"program"` / `"bank_msb"` /
`"bank_lsb"` from connection JSON. After building a patch, read an
`"instrument_program_changes"` array from the patch JSON; for each
element look up the output by name via `DeviceManager`, read channel
and optional bank/prog fields, construct an `InstrumentProgramChange`,
add it to the patch.

**Save:** Remove the `prog` block from connection saving. When saving a
patch, write an `"instrument_program_changes"` array; for each IPC
write `{ "output": name, "channel": ch }` plus any non-`UNDEFINED`
bank/prog fields.

**Migration:** When loading a connection that still carries a
`"program"` key, synthesize an `InstrumentProgramChange` from that
connection's output and `output_chan` (skip if `output_chan ==
ALL_CHANNELS`). Add it to the parent patch, deduplicating by
output+channel so multiple connections to the same instrument don't
produce duplicate program changes.

---

## Step 5 — Update `ConnectionEditor`

Remove entirely:
- `_prog_label`, `_msb_label`, `_msb`, `_lsb_label`, `_lsb`,
  `_prog_field_label`, `_prog` field declarations
- `init_prog()` method
- `update_enabled_states()` method (it existed solely for the program
  change fields)
- `actionListenerCallback` body (it only called `update_enabled_states()`)
- Program change row from `layout()` and its contribution to
  `CONTENT_HEIGHT`
- Program change validation and `_conn->set_program_*` calls in
  `apply()`

---

## Step 6 — Update `PatchEditor`

Add a `TableListBox` with columns:

| Output | Chan | Bank MSB | Bank LSB | Prog |
|--------|------|----------|----------|------|
| combo  | combo| text     | text     | text |

The Output combo is populated from
`KeyMaster_instance()->device_manager().outputs()` — all available
outputs — not filtered to outputs referenced by this patch's
connections.

- **Add** button: appends a new row with the first available output,
  channel 1, all bank/prog fields `UNDEFINED`
- **Remove** button: deletes the selected row
- Empty text fields mean `UNDEFINED` (same convention as the old
  `ConnectionEditor` program change fields)

Expand `CONTENT_HEIGHT` and `width()` constants to accommodate the
table, header row, and button row.

### Duplicate Check

In `apply()`, before writing back to the patch, scan
`_instrument_program_changes` for any two entries sharing the same
output identifier **and** the same channel. Collect all duplicates,
then show a single non-blocking `AlertWindow` warning that lists each
conflicting output-name + channel pair. Proceed with applying anyway —
the second entry in the array will win at runtime (it sends last),
which is almost certainly a user mistake rather than intentional.

```cpp
for (int i = 0; i < ipcs.size() - 1; ++i)
    for (int j = i + 1; j < ipcs.size(); ++j)
        if (ipcs[i].output->identifier() == ipcs[j].output->identifier()
                && ipcs[i].channel == ipcs[j].channel)
            // collect duplicate description for warning message
```

---

## Affected Files

| File | Change |
|------|--------|
| `Source/km/instrument_program_change.h/.cpp` | **New** |
| `Source/km/patch.h/.cpp` | Add IPC array + send on start |
| `Source/km/connection.h/.cpp` | Remove all program change code |
| `Source/km/storage.h/.cpp` | Repoint serialization + migration |
| `Source/gui/connection_editor.h/.cpp` | Remove program change UI |
| `Source/gui/patch_editor.h/.cpp` | Add IPC table + duplicate warning |
| `*.jucer` | Register new source files |
