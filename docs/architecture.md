# Architecture

## Runtime contract

The plugin is split into three layers:

1. NUSspli integration reports an actual download task.
2. The queue stores tasks without starting native Download Management.
3. Native handoff runs only after NUSspli exits.

NUSspli continues using its existing downloader while it is open. The plugin does not create a second downloader.

## Native handoff

The Wii U native path is tied to Nintendo's title/package management stack rather than being a generic URL downloader.

This repository therefore does not invent ioctl numbers, NIM request structures, or function signatures. Those must be verified against Wii U reverse-engineering work before being called.

## Next milestone

- Identify the exact NIM entry points used by official Download Management/eShop.
- Determine the minimum task metadata required.
- Map a NUSspli NUS download to a native title-package task.
- Add the NUSspli integration layer.
- Test one title end-to-end.
- Add multi-task handoff and failure recovery.
