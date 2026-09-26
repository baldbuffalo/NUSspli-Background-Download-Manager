# NUSspli Background Download Manager

A Wii U Aroma/WUPS plugin that queues actual NUSspli download selections and hands them to the Wii U's native NIM/Download Management system only after NUSspli exits.

NUSspli is an open-source Wii U application whose normal downloader runs directly against Nintendo's update servers. urlNUSspli source repositoryhttps://github.com/V10lator/NUSspli

## Runtime flow

1. NUSspli launches: no queue entry is created.
2. NUSspli enters an actual title download: a tiny NUSspli-side publisher writes a task record.
3. The plugin notices that record while NUSspli is running and moves it into an in-memory queue.
4. NUSspli's normal downloader continues unchanged.
5. The plugin does **not** register an NIM task while NUSspli is open.
6. NUSspli exits: `ON_APPLICATION_ENDS()` runs.
7. The plugin calls `nn_nim.rpl` and registers each queued title-package task.
8. Native Download Management owns the task after handoff.

WUPS supports replacing SDK/RPL functions for a selected target process, which is why the plugin uses a ROOT_RPX `OSYieldThread` hook for lightweight queue polling instead of trying to patch NUSspli's private downloader by a guessed instruction address. urlWUPS function-hook documentationhttps://wiiu-env.github.io/WiiUPluginSystem/dev_plugin_hooks

## Files

- `src/main.cpp` — NUSspli lifecycle detection and final handoff.
- `src/nusspli_queue.cpp` — consumes task records written by the NUSspli integration.
- `src/nusspli_hook.cpp` — ROOT_RPX polling hook.
- `src/nim_handoff.cpp` — native NIM task registration.
- `include/nusspli_bg_bridge.h` — task-record wire format.
- `patches/nusspli_bg_publish.c` — source file to add to a NUSspli build.
- `docs/` — implementation notes.

## NUSspli integration

The plugin repository contains the publisher implementation, but **NUSspli itself must be rebuilt with that publisher and one call at the start of its existing `downloadTitle()` function**.

The call should pass:

- task ID
- `tmd->tid`
- `tmd->title_version`
- `toUSB`
- `inst`

The publisher uses a temporary file followed by rename, so the plugin never intentionally consumes a partially written record.

NUSspli's current downloader signature and TMD fields are present in its public source. urlNUSspli downloader sourcehttps://github.com/V10lator/NUSspli/blob/master/src/downloader.c urlNUSspli TMD definitionshttps://github.com/V10lator/NUSspli/blob/master/include/tmd.h

## Native NIM side

The project resolves:

- `MakeTitlePackageTaskConfigAutoUsingBgInstallPolicy`
- `RegisterTitlePackageTask`

WUT exposes those NIM symbols in its Wii U SDK definitions. urlWUT repositoryhttps://github.com/devkitPro/wut

The 0x18-byte task configuration layout follows the public Cemu reverse-engineering implementation. Cemu documents the device fields as part of the task configuration, but its implementation is not the retail Wii U implementation, so the real-console behavior is explicitly a test point.

## What is still experimentally unverified

These are the exact things to check on the real Wii U:

1. **NIM function ABI:** the exported symbol names are confirmed, but the exact retail return/error behavior of the dynamically resolved calls has not been verified on a real console.
2. **TitleType enum:** the adapter currently supplies the neutral value `0`. If NIM rejects the task, this is the first value to investigate.
3. **Destination device:** NUSspli's `toUSB` flag is carried through the queue, but the retail meaning of the two config device bytes needs real-console verification.
4. **Ticket/content discovery:** the registration currently gives NIM no explicit content-ID array. NIM may obtain the package metadata itself, or the retail implementation may require more information.
5. **NUSspli polling:** the `OSYieldThread` hook is deliberately conservative, but filesystem access from that hook needs real-console testing.
6. **Exit timing:** if `ON_APPLICATION_ENDS` occurs too early for the native registration call on a particular Aroma/WUPS build, the handoff may need to move to the application-closed status hook.
7. **Multiple tasks:** the queue supports 32 records, but real NIM duplicate-task behavior needs testing.

## Safe first test

Use a title/homebrew item you are authorized to download. First verify that simply launching and exiting NUSspli produces **no NUSBG task-registration log**. Then test one real download and watch the debug log for:

```
[NUSBG] Native task registered for ...
```

If NIM rejects it, keep the error code. That code is the most useful information for correcting the remaining ABI/configuration uncertainty.

## Build

Requires the Wii U devkitPro toolchain with WUT/WUPS/WUMS.

```
make
```

The resulting `.wps` belongs in the Aroma plugin directory on the SD card.
