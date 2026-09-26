# NUSspli Background Download Manager

An Aroma plugin for Wii U designed to bridge NUSspli downloads to the Wii U's native Download Management system when NUSspli exits.

## Intended behavior

1. Starting NUSspli by itself does nothing.
2. The plugin watches for an actual NUSspli download.
3. When NUSspli starts a download, NUSspli sends the download metadata to this plugin.
4. The plugin stores the download information but does **not** start the Wii U Download Manager yet.
5. NUSspli continues using its normal downloader while it remains open.
6. When NUSspli exits, the plugin detects that NUSspli has closed.
7. The plugin submits the stored download information to the Wii U's native Download Management service.
8. The Wii U then owns and continues the download independently of NUSspli.

## Important design requirement

The plugin must not create a second active downloader while NUSspli is still running. The plugin is a queue/handoff layer, not a replacement HTTP downloader.

## Project status

Initial repository setup. The next implementation step is identifying the exact Wii U native Download Management/NIM interface used by the system, then building the minimal Aroma plugin around that interface.

## Planned structure

- `src/` — Aroma/WUPS plugin implementation
- `include/` — public/internal headers
- `libs/` — any required Wii U support code
- `docs/` — reverse-engineering notes and protocol/API documentation
- `.github/workflows/` — build workflow

## NUSspli integration

NUSspli will eventually need a small integration point that sends an actual download task to the plugin. No message is sent merely because NUSspli was launched.

The integration should carry enough information for the native Download Management task to reproduce the requested download.

## Handoff state

The plugin should track each task separately:

```
IDLE
  ↓
NUSspli starts an actual download
  ↓
QUEUED
  ↓
NUSspli remains open → wait
  ↓
NUSspli exits
  ↓
HANDOFF
  ↓
Native Download Management
```

## Goal

Make NUSspli downloads continue through the Wii U's official Download Management after the user closes NUSspli, without changing NUSspli's normal download behavior while it is open.
