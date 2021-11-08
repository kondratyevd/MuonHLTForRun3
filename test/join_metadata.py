import json
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("path", help="path where metadata files are located")
args = parser.parse_args()

print(args.path)
metadata = {}
success = True
for choice in ['barrel', 'endcap']:
    try:
        with open(args.path+'/metadata_'+choice+'.json', 'rb') as handle:
            metadata.update(json.load(handle))
    except:
        success = False
        print('metadata_'+choice+'.json not found in '+args.path+', aborting...')

if success:
    with open(args.path+'/metadata.json', 'w') as outfile:
        json.dump(metadata, outfile, indent=4)
    print('Combined metadata files and saved to '+args.path+'/metadata.json')