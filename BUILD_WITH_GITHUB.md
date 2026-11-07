# Build the Windows VST3 in GitHub (no compiler installation on your PC)

This produces a real 64-bit Windows `.vst3` plugin bundle using GitHub's Windows build runner. It is not precompiled in the source ZIP.

1. Sign in to https://github.com (a free account is sufficient for public repositories).
2. Create a new **public** repository named `AcidLine303` and select **Create repository**. Do not tick the initial README option.
3. On the empty repository page, click **uploading an existing file**. Extract the supplied ZIP first and upload the **contents** of the `AcidLine303` folder, including `.github/workflows/build-windows-vst3.yml`. Maintain that folder structure: the `CMakeLists.txt` must be in the repository root. Click **Commit changes**.
4. Open the repository **Actions** tab. Select **Build Windows VST3** in the left sidebar and click **Run workflow** (or open the workflow run automatically started when files were committed).
5. After the run succeeds (green check), open it and download the **AcidLine303-Windows-VST3** artifact at the bottom. Extract that download ZIP.
6. Copy the entire `AcidLine 303.vst3` folder into `C:\Program Files\Common Files\VST3\`. You may need administrator approval.
7. In Ableton Live: Settings/Preferences > Plug-Ins > enable **Use VST3 Plug-In System Folders**; rescan plugins; load **AcidLine 303** on a MIDI track.

If the workflow fails, open the red run, select **build**, expand the failing step, and copy its error text into ChatGPT. The first version has not yet been validated by a Windows compiler and may need a code change.

Notes: The repo is public so that GitHub Actions can run using typical free-account allowances. The source is intended as independent original code, not official Roland software. For private repositories, GitHub Action minute allowances may differ. JUCE is fetched from its official repository when configuring the build, and is subject to its licensing conditions.
