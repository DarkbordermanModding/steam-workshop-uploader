# Steam workshop uploader

Create and update Steam Workshop item at ease.

## Usage

1. Create a YAML file(ex: `metadata.yml`) with the same format as [Example YAML file](#example-yaml-file).

2. Run the uploader
```bash
uploader.exe metadata.yml
```

## Development

1. Compile uploader (currently only available in windows 64-bits)
```bash
g++ -std=c++11 uploader.cpp -L./lib/win64 -lsteam_api64 -o uploader.exe
```

### Prerequisite

| Name | Version |
| --- | --- |
| choco | 1.3.1 |
| yq(installed by choco) | 4.28.1 |
| MinGW g++ 64-bits | 12.2.0 |

To test the uploader, use same step in #Usage

## Example YAML file
```yml
workshop:
  app_id: 480
  publishedfield_id: 2147483647
  title: Workshop item title
  description: Workshop item description
  visibility: 2
  preview_path: preview.png
  content_folder: "./example/"
  tags: [ tag1, tag2 ]
```

## YAML properties description
| Name | Value type | description | Example |
| --- | --- | --- | --- |
| app_id | integer | Game's application id | `480` |
| publishfield_id | integer | Workshop's application id, set to `0` for newly uploaded item | `2147483647` |
| title | string | Title of workshop item | `Workshop item title` |
| description | string | Description of workshop item | `Workshop item description` |
| visibility | integer(0/1/2/3) | Visibility of workshop item (public/friends/private/public but hidden from search) | `2` |
| tags | list of string | Tags of workshop item | `[ tag1, tag2 ]` |
| preview_path | string | Relative/Absolute path of preview image, will be used as first/thumbnail image | `preview.png` |
| content_folder | string | Relative/Absolute path of workshop item folder | `./example` |

