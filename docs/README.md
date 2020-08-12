## Development

requirements:
- jinja2

optional requirements:
- watchdog

You can watch changes in the filesystem using the `watchdog` python package. 
It provides a cli tool `watchmedo` that wil execute a script when the specified files change.
Following example reruns the generator on every change of a python, html or json file.
```shell script
watchmedo shell-command \
    --patterns="*.py;*.html;*.json" \
    --ignore-patterns="catalog_generated.html" \
    --recursive \
    --command='python generator.py' \     
    .
```
