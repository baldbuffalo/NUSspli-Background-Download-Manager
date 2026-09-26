# NUSspli Background Download Manager

Wii U Aroma/WUPS plugin that queues download metadata while NUSspli is running and hands the queued work to the Wii U's native Download Management path only after NUSspli actually ends.

## Runtime design

1. Starting NUSspli does nothing by itself.
2. While NUSspli is running, the plugin can accept queued task records.
3. The plugin never registers a native Download Management task while NUSspli is still running.
4. NUSspli's normal downloader remains responsible for its foreground download.
5. When WUPS reports that the NUSspli application has actually ended, the plugin calls the native NIM handoff.
6. Failed native registrations remain queued instead of being silently discarded.

## Important integration detail

The current public NUSspli source keeps its download queue and `downloadTitle()` implementation inside the NUSspli RPX. WUPS's normal named replacement mechanism is for exported RPL functions, so the plugin does not pretend that a private NUSspli C function can be replaced just by naming it.

A small versioned wire bridge is included under `include/nusspli_bg_bridge.h` and `patches/nusspli_bg_publish.c`. That bridge is the intended producer side if/when NUSspli is built with the publisher call. The plugin side is already independent of NUSspli's downloader and only consumes queued metadata.

## Current native handoff

`src/nim_handoff.cpp` resolves the retail `nn_nim.rpl` exports at runtime and attempts to create a native title-package task using the background-install-policy config factory and `RegisterTitlePackageTask`.

The exact retail ABI/config semantics still need to be validated on-console. The code logs failures with the `[NUSBG]` prefix.

## Files

- `src/main.cpp` — NUSspli lifecycle detection and handoff trigger.
- `src/queue.cpp` — in-memory task queue.
- `src/nusspli_queue.cpp` — SD bridge consumer.
- `src/nusspli_hook.cpp` — lightweight polling hook used while the NUSspli RPX is active.
- `src/nim_handoff.cpp` — native NIM handoff.
- `include/nusspli_bg_bridge.h` — producer/consumer wire format.
- `patches/nusspli_bg_publish.c` — producer helper for a NUSspli build.

## First Wii U test

1. Build the `.wps` with the Wii U devkit/WUPS toolchain.
2. Put the plugin in `sd:/wiiu/plugins/`.
3. Boot Aroma/WUPS and launch NUSspli normally.
4. Confirm that launching NUSspli alone does not create a Download Management task.
5. If a bridge task is available, confirm it remains queued while NUSspli is open.
6. Exit NUSspli using its own A-confirmed exit flow.
7. Check the Wii U Download Management UI and the console log for `[NUSBG]`.
8. If native registration fails, keep the exact `[NUSBG]` error code; that is the key value needed to fix the NIM adapter.

## Status

The lifecycle/queue/plugin side is implemented. The two parts that still require real-console validation are the NUSspli producer connection and the exact retail NIM task-config ABI.
