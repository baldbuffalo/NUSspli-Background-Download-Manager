# Wii U NIM research notes

## Confirmed

- Wii U system software exposes the nn::nim subsystem.
- Cemu contains an implementation/emulation of parts of nn::nim, including title-package task concepts.
- OSGetTitleID is available for identifying the currently running title.

## Not yet treated as confirmed

The project does not yet have a verified real-console call sequence for:

- creating a title-package download task;
- supplying TMD/ticket/content information;
- selecting the destination device;
- starting or resuming the task;
- transferring an in-progress NUSspli download into native Download Management.

Cemu is useful for understanding structures and semantics, but its IOSU-facing implementation must not be copied blindly as a Wii U ABI.

## Rule

Do not add guessed ioctl IDs, guessed structure layouts, or guessed exported symbol signatures to production code.
