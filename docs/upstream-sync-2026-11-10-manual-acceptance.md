# Manual acceptance checklist

Use the Release build from this synchronization branch. Technical tests do not replace these checks. Record PASS/FAIL and attach screenshots or the output video for failures.

- [ ] Switch Russian → English → Russian; confirm readable Cyrillic, stable windows/selection and no clipped controls at 100%, 150% and 200% scale.
- [ ] Browser: refresh in gallery/table view, filter Cyrillic names, preserve selection, change visibility, replace and delete a resource. Check failure messages with the server unavailable.
- [ ] Upload a small test simulation privately with «Оставить личным», then publicly with «Поделиться с сообществом». Confirm visibility with another account, the confirmation text and «Показать в браузере». Public uploads may publish a Discord link; use disposable test content.
- [ ] Open «Инструменты > Запись видео», add/reorder/edit waypoints and play/stop the camera flight. Save/load a flight through a Cyrillic filename and check invalid-file errors.
- [ ] Record a short MP4, open it in a video player and check frame size/rate, motion, time-lapse, Cyrillic labels and absence of the GUI. Stop early and verify the file still plays. Repeat while minimized. This verifies real UI capture/encoding, not a mocked state.
- [ ] During recording, verify console mode cannot activate. After recording, verify it can activate and return to the GUI normally.
- [ ] Toggle ray tracing and shadow strength in a representative world; inspect performance, shadows, help text and saved/reloaded parameters.
- [ ] Read the updated Russian recording/file/privacy help and follow its links. Confirm real server counts and privacy behavior when CLI reporting is enabled/disabled.

Do not merge to main until user acceptance is recorded. Existing server-side/client-side resource-name character restrictions have not been broadened by this update; report unexpected rejection of Cyrillic names separately.
