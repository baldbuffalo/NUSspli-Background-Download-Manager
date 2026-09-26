# NUSspli Background Download Manager

A Wii U Aroma/WUPS plugin that keeps **official, unmodified NUSspli** as the foreground downloader and hands an interrupted NUSspli download to the Wii U's native Download Management only after NUSspli exits.

## Runtime flow

1. NUSspli is launched. The plugin does nothing to downloads.
2. The plugin registers a FunctionPatcher hook for NUSspli's internal `downloadTitle` function.
3. When NUSspli actually starts a title download, the hook records the title ID, version, destination device and install-after-download flag.
4. The original NUSspli `downloadTitle` function is called normally. There is **no second downloader**.
5. If NUSspli finishes that download normally, the plugin removes that task from its queue.
6. If NUSspli is exited while the download is still active, the task remains queued.
7. `ON_APPLICATION_ENDS()` detects NUSspli's real application close and calls the native NIM handoff.
8. NIM is asked to create the native background title-package task.

## Important

- NUSspli itself is not modified.
- The hook is installed against the official NUSspli RPX by title ID and the `downloadTitle` symbol.
- The plugin requires the Wii U FunctionPatcherModule/libfunctionpatcher, which is part of the modern Wii U plugin/module stack.
- Native NIM task registration is still hardware-tested code: the export names are verified from WUT, but the exact retail `TitlePackageTaskConfig` semantics are the part the Wii U needs to validate.

## Build

Requires devkitPro Wii U development packages, WUPS, WUT, WUMS and libfunctionpatcher.

```sh
make
```

The output is:

```
NUSspliBackgroundDownloadManager.wps
```

Install the `.wps` in the Aroma plugin directory.

## First hardware test

1. Boot Aroma normally.
2. Install this plugin.
3. Launch the official NUSspli.
4. Start one normal download.
5. Let it run for a few seconds so the `downloadTitle` hook definitely executes.
6. Press HOME.
7. Confirm NUSspli shows its own **A: yes / B: no** exit prompt.
8. Press A and let NUSspli return to the Wii U Menu.
9. Open Download Management.
10. Check whether the interrupted NUSspli title appears as a native background download.

If it does not appear, send the Wii U log output containing the `[NUSBG]` lines. The most useful lines are:

- `NUSspli downloadTitle hook registered`
- `Queued active download`
- `Native task registered`
- `MakeTitlePackageTaskConfig failed`
- `RegisterTitlePackageTask failed`

