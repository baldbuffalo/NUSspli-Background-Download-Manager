# Wii U NIM research notes

## Confirmed from public Wii U references

- Cafe OS contains `nn_nim.rpl`; WiiUBrew describes it as the title-installation library.
- Aroma is a persistent plugin environment and WUPS is its plugin system.
- Wii U system software has a dedicated NIM component rather than treating official title downloads as a generic application-level HTTP download.

## Current implementation boundary

The plugin now identifies the NUSspli application by its title ID and invokes the handoff stage only when that application ends.

The actual NIM task-creation call is still isolated behind `HandoffQueuedDownloads()`.

## Why the NIM call is not guessed

A working native handoff requires the exact real-console ABI:

- exported `nn::nim` function names/signatures;
- task/config structure layout;
- IOSU request/command IDs where applicable;
- TMD/ticket/content association;
- destination device selection;
- task state/start semantics.

Cemu or other compatibility-layer implementations can reveal concepts, but they are not automatically the Wii U's callable ABI.

## Next implementation step

Replace the handoff stub with verified Wii U nn_nim calls and add the NUSspli-side producer that calls `QueueDownload()` for every real download task.
