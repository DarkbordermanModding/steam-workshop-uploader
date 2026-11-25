# steam-workshop-uploader

Creates and updates a Steam Workshop mod item from a YAML file. Part of an infrastructure-as-code setup for the user's Steam content: the YAML is the desired state, and each run makes Steam match it, removals included.

Related tools in `D:\git\darkbordermanmodding\`:
- `steam-collection-uploader`: collections. It copied `lib/utilities.hpp`, `lib/md_to_steam.hpp` and the SDK from here, so fixes to those may need copying over.
- `steam-artwork-uploader`: proof-of-concept tests only.

## Design rules
- Optional list fields (`tags`, `additional_previews`, `required_publishedfield_ids`, `required_app_ids`): present replaces everything, `[]` clears, omitted leaves Steam unchanged. Each one has a `sync_*` flag in `Metadata`.
- A mod doesn't list its collections. Collection membership is owned by `steam-collection-uploader`.
- Root key is `steam_workshop`; the old `workshop` key still works, but a file can't use both.

## Layout
- `uploader.cpp`: main flow: parse → `SteamAPI_Init` → y/n prompt → create if `publishedfield_id` is 0 → upload → localizations → sync dependencies.
- `lib/parser.hpp`: `Metadata`, reads the YAML with `yq`. Can't be copied, because `tags` points into its own strings.
- `lib/steamworkshop.hpp`: `Workshop`, the Steam calls and callbacks. Status fields: 0 pending, 1 done, -1 failed.
- `lib/md_to_steam.hpp`: Markdown → Steam BBCode for `description_path`.
- `lib/utilities.hpp`: `exec` (runs yq), string split, path helpers. Windows only.

## Build and test
```bash
g++ -std=c++11 uploader.cpp -L./lib/win64 -lsteam_api64 -o uploader.exe
g++ -std=c++11 tests/test_md_to_steam.cpp -o tests/test_md_to_steam.exe && tests/test_md_to_steam.exe
g++ -std=c++11 tests/test_parser.cpp -o tests/test_parser.exe && tests/test_parser.exe
```
- Tests need no Steam. `test_parser` needs `yq` on PATH and must run from the repo root (fixtures are in `tests/fixtures/`).
- Claude runs the tests. The user runs real uploads against Steam (Steam client logged in, y/n prompt). Run from the repo root: it writes and deletes `steam_appid.txt` in the current folder.
- Keep test YAMLs and images for real uploads out of git.

## Workflow
- Past features went through `feat/*` branches and PRs; small fixes have been committed straight to `master` when the user asked.
