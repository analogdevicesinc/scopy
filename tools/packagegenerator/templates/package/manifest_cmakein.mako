{
    "id": "${config['id']}",
    "title": "${config['title']}",
    "version": "@PROJECT_VERSION@",
    "description": "${config['description']}",
    "license": "${config['license']}",
    "author": "${config['author']}",
    "download_link": "",
    "zip_checksum": "",
    "scopy_compatibility": ["@CMAKE_PROJECT_VERSION@"],
    "category": [${', '.join(f'"{x}"' for x in config['category'])}]
  }
  