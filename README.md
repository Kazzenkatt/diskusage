DiskUsage2
==========

Fork of [DiskUsage](https://github.com/IvanVolosyuk/diskusage) / [WhiredPlanck's fork](https://github.com/WhiredPlanck/diskusage), modernized for Android 16.

DiskUsage provides a way to find files and directories on storage that consume a lot of space.
It displays a diagram where directories are shown proportional to their size, with several levels of subdirectories visible. Users can zoom in to inspect specific directory content.
The purpose is to find and clean up space hogs — it is not a general purpose file manager.

Screenshot for an ancient version:<br>
<img src="extra/screenshot.png">

YouTube video: https://www.youtube.com/watch?v=TIiCQfWdtVg

## Changes from upstream

- **Renamed to DiskUsage2** — installs side-by-side with the original (different applicationId)
- **Adaptive icon** with light gray background to distinguish from the original
- **Build system modernized**
  - Gradle 8.12, Android Gradle Plugin 8.9.1
  - compileSdk / targetSdk / minSdk = 36 (Android 16)
  - Java 21, Kotlin 2.0 (jvmTarget 21)
  - Migrated native build from ndk-build (Android.mk) to CMake
  - 16K page size alignment for native libraries (Android 16 requirement)
  - Modern Gradle DSL: `plugins {}` block, `dependencyResolutionManagement`
  - Removed Jetifier (unnecessary)
- **Dependencies updated** to latest stable versions (core-ktx 1.18.0, coroutines 1.10.2, recyclerview 1.4.0, appcompat 1.7.1, constraintlayout 2.2.1)
- **Removed splitties dependency** — replaced with a minimal local utility
- **Manifest cleaned up** — removed deprecated attributes, added `android:exported`, removed legacy storage permissions
- **Native scanner as JNI library** — `scan.c` converted from a standalone executable to a JNI library called in-process, restoring native scan speed on Android 16
- **Native scanner fallback** — if the JNI scanner fails, the app falls back to the built-in Java scanner (slower but functional)
- **Fixed permission flow** — always requests "All files access" before scanning
- **Fixed mount point detection** — no longer hardcodes the package name when deriving storage root paths

## Building

Requires:
- JDK 21
- Android SDK with platform 36, build-tools 35, NDK 26.x, CMake 3.22+

```bash
./gradlew assembleDebug
```

APK output: `app/build/outputs/apk/debug/diskusage2-v5.0-alpha1-debug.apk`

## Permissions

The app requires two special permissions:

### All Files Access (MANAGE_EXTERNAL_STORAGE)
Required to scan internal storage. On first launch, the app opens the system settings page where you grant this. Without it, the scan will return very limited results.

### Usage Access (PACKAGE_USAGE_STATS)
Required to list installed apps and their storage usage. The app shows a dialog on first launch pointing you to the system settings toggle.

## Restricted Settings (sideloaded APKs)

Android 13+ restricts sideloaded apps from accessing sensitive permissions like Usage Access. If you install the APK via `adb install` or a file manager, the Usage Access toggle will be grayed out with a message about "restricted settings".

**Workarounds:**

1. **Remove the restriction manually:** In the Usage Access settings, tap the grayed-out toggle for DiskUsage2 — this triggers a popup about restricted settings. Dismiss it, then go to App Info for DiskUsage2. You should now see a red "Restricted settings" / "Beschränkung entfernen" banner. Tap it, confirm with PIN/fingerprint, then go back to Usage Access and enable the toggle.

2. **Install with a trusted installer flag:**
   ```bash
   adb install -i "com.android.vending" diskusage2-v5.0-alpha1-debug.apk
   ```
   This tells Android the app was installed by a trusted source, avoiding the restriction entirely.

3. **Install via F-Droid or another store** — apps from recognized installers are not restricted.

Note: The "All files access" permission is not affected by this restriction — only Usage Access is.
