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
  description_path: description.md  # Markdown file, converted to Steam BBCode
  description: Workshop item description  # plain text fallback if description_path not exist
  visibility: 2
  preview_path: preview.png
  content_folder: "./example/"
  tags: [ tag1, tag2 ]
  required_publishedfield_ids: [ 123456789, 987654321 ]
  required_app_ids: [ 12345, 67890 ]
```

## YAML properties description
| Name | Value type | Description | Example |
| --- | --- | --- | --- |
| app_id | integer | Game's application ID | `480` |
| publishedfield_id | integer | Workshop item's published file ID, set to `0` for a newly uploaded item | `2147483647` |
| title | string | Title of the workshop item | `Workshop item title` |
| description_path | string (optional) | Relative or absolute path to a Markdown file, converted to Steam BBCode on upload. Takes priority over `description` if both are set | `description.md` |
| description | string (optional) | Plain text description of the workshop item. Used if `description_path` is not set | `Workshop item description` |
| visibility | integer(0/1/2/3) | Visibility of the workshop item (public/friends only/private/unlisted) | `2` |
| tags | list of string | Tags of the workshop item | `[ tag1, tag2 ]` |
| preview_path | string | Relative or absolute path of the preview image | `preview.png` |
| content_folder | string | Relative or absolute path of the workshop item folder | `./example` |
| required_publishedfield_ids | list of integer (optional) | Required Workshop item dependencies by published file ID. If present, replaces all existing dependencies. | `[ 123456789, 987654321 ]` |
| required_app_ids | list of integer (optional) | Required DLC/app dependencies by App ID. If present, replaces all existing app dependencies. | `[ 12345, 67890 ]` |

