{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "addon.c" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}