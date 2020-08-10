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
    catalog = '<!DOCTYPE html>\n<html lang="en-gb">\n<head>\n'
    # Add head stuffs
    catalog += '<meta charset="UTF-8" />\n<meta name="viewport" content="width=device-width, initial-scale=1.0" />\n<title>OBUS Catalog</title>\n'
    catalog += '<link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.8.1/css/all.css" />\n'
    catalog += '<link rel="stylesheet" href="https://meyerweb.com/eric/tools/css/reset/reset.css" />\n'
    catalog += '<link rel="stylesheet" href="assets/css/main.css" />\n'
    catalog += '</head>\n'

    # Add scroll to top button
    catalog += '<body>\n<div class="back-to-top" onclick="window.scrollTo(0,0);">\n<div class="button">\n'
    catalog += '<i class="fas fa-arrow-up"></i>\n</div>\n</div>\n'

    # Add header
    catalog += '<header>\n<div class="hrd-left">\n<div class="hdr-img-container">\n<img src="assets/img/logo.svg" alt="Zeus logo" />\n'
    catalog += '\n</div>\n</div>\n<div class="hdr-center">\n<p class="hdr-title">O.B.U.S. Catalog</p>\n'
    catalog += '</div>\n<div class="hdr-right">\n<p class="hdr-p">August 2020</p>\n</div>\n</header>\n'

    # Add page title
    catalog += '<main>\n<h1>OBUS catalog</h1>\n<div class="img-container">\n<img src="assets/img/shell.jpeg" />\n</div>\n'

    # Add introduction and overview


    catalog += '<article>\n<div class="anchor" id="controllers"></div>\n<h2>Controllers</h2>\n'
    catalog += json_to_multi_html('controllers.json') + '\n</article>\n'
    
    catalog += '<article>\n<div class="anchor" id="input-modules"></div>\n<h2>Input modules</h2>\n'
    catalog += json_to_multi_html('inputs.json') + '\n</article>\n'
    
    catalog += '<article>\n<div class="anchor" id="output-modules"></div>\n<h2>Output modules</h2>\n'
    catalog += json_to_multi_html('outputs.json') + '\n</article>\n'
    
    catalog += '<article>\n<div class="anchor" id="panic-modules"></div>\n<h2>Panic modules</h2>\n'
    catalog += json_to_multi_html('panics.json') + '\n</article>\n'

    # Add footer and closing tags
    catalog += '</main>\n<footer>\n<div class="hdr-left">\n<p>[Catalog is still a PoC]</p>\n'
    catalog += '</div>\n<div class="hdr-center">\n<p>This is a game; not actual bombmaking...</p>\n'
    catalog += '</div>\n<div class="hdr-right">\n<p>\nAll images in this catalog are for reference only, actual products may vary.\n'
    catalog += '</p>\n</div>\n</footer>\n</body>\n</html>'

    # print(catalog)

    with open('catalog_test.html', 'w') as test_file:
        test_file.write(catalog)
    
