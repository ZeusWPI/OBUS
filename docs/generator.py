"""
Script to generate the OBUS catalog from JSON files.
"""
import json

from jinja2 import Environment, select_autoescape, FileSystemLoader

env = Environment(
    loader=FileSystemLoader('templates'),
    autoescape=select_autoescape(['html', 'xml'])
)


def catalog_json_item_to_html(name: str, config: dict) -> str:
    return env.get_template('module.html').render(name=name, config=config)


def json_to_multi_html(filename: str, id: str, title: str):
    with open(filename, 'r') as in_file:
        items = ''
        cat_items = json.load(in_file)
        for cat_item, cat_item_config in cat_items.items():
            items += catalog_json_item_to_html(cat_item, cat_item_config)
        return env.get_template('module_group.html').render(id=id, title=title, items=items)


if __name__ == "__main__":
    print("Generating catalog")

    template = env.get_template('main.html')

    # Articles

    articles = ""
    articles += json_to_multi_html(filename='controllers.json', id="controllers", title="Controllers")
    articles += json_to_multi_html(filename='inputs.json', id="input-modules", title="Input modules")
    articles += json_to_multi_html(filename='outputs.json', id="output-modules", title="Output modules")
    articles += json_to_multi_html(filename='panics.json', id="panic-modules", title="Panic modules")

    new_html = template.render(articles=articles)

    # print(catalog)

    with open('catalog_generated.html', 'w') as test_file:
        test_file.write(new_html)
