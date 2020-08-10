"""
Script to generate the OBUS catalog from JSON files.
"""
import json


def catalog_json_item_to_html(name: str, config: dict) -> str:
    # Make the section and title
    res = f'<section>\n<h3>{name}</h3>\n'

    # Add the item image and containers
    res += '<div class="module-info">\n<div class="img-container">\n<div class="square-img">\n<div class="aspect-img-wrapper">\n'
    res += f'<img src="{config["image"]}"'
    res += f'/>\n</div>\n</div>\n<p class="caption">{name}</p>\n</div>\n'

    # Add features
    res += '<div class="module-intro">\n<h5>Features</h5>\n<ul>\n'
    for feat in config['features']:
        res += f'<li>{feat}</li>\n'

    # Add descriptions
    res += '</ul>\n<h5>Description</h5>\n'
    res += f'<p>{config["description"]}</p>\n'

    # Add applications
    res += '<h5>Applications</h5>\n<ul>\n'
    for appl in config['applications']:
        res += f'<li>{appl}</li>\n'
    res += '</ul>\n</div>\n</div>\n'

    # Add electrical specs
    res += '<div class="module-specs">\n<h4>electrical specs</h4>\n<div class="table-container">\n<table>'
    res += '<tr>\n<th rowspan="2" class="bt-bord">Characteristic</th>\n<th colspan="3">Limits</th>\n<th rowspan="2" class="bt-bord">unit</th>\n</tr>\n'
    res += '<tr>\n<th class="bt-bord">min</th>\n<th class="bt-bord">typ</th>\n<th class="bt-bord">max</th>\n</tr>\n'
    for spec in config['el-specs'].keys():
        res += f'<tr>\n<td>{spec}</td>\n'
        specs = config['el-specs'][spec]
        res += f'<td>{specs["min"]}</td>\n<td>{specs["typ"]}</td>\n<td>{specs["max"]}</td>\n<td>{specs["unit"]}</td>\n</tr>\n'

    # Add application note
    res += '</table>\n</div>\n<h4>Application note</h4>\n'
    res += f'<p>{config["app-note"]}</p>\n</div>\n'

    # Add document links
    res += '<div class="module-documents">\n<h4>documents</h4>\n<ul>\n'
    for doctype, doclink in config['documents'].items():
        res += f'<li>\n<a href="{doclink}">{doctype}</a>\n</li>\n'
    res += '</ul>\n</div>\n'

    # Add order codes
    res += '<div class="order-codes">\n<h4>order codes</h4>\n<div class="table-container">\n<table>\n'
    res += '<tr>\n<th class="bt-bord">Description</th>\n<th class="bt-bord">code</th>\n</tr>\n'
    for code, desc in config['order-codes'].items():
        res += f'<tr>\n<td>{desc}</td>\n<td>{code}</td>\n</tr>\n'

    res += '</table>\n</div>\n</div>\n'

    # Add clsing tag
    res += '</section>'
    return res

def json_to_multi_html(filename:str):
    res = ''
    with open(filename, 'r') as in_file:
        cat_items = json.load(in_file)
        for cat_item, cat_item_config in cat_items.items():
            res += catalog_json_item_to_html(cat_item, cat_item_config)
    return res

if __name__ == "__main__":
    print(json_to_multi_html('controllers.json'))
    print(json_to_multi_html('inputs.json'))
    print(json_to_multi_html('outputs.json'))
    print(json_to_multi_html('panics.json'))

    
