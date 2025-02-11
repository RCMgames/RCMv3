board='ESP32'
# copy json files from the folder inside boardData specified by the first argument to the docs/ds/presets folder so that only the preset files compatible with the MCU are copied

import os
import shutil

source_dir = os.path.abspath(os.path.join(os.getcwd(),'RCMv3/boardData',board,'presets'))
destination_dir = os.path.abspath(os.path.join(os.getcwd(),'docs/ds/presets'))

# Delete the destination directory
if os.path.exists(destination_dir) and os.path.isdir(destination_dir):
    shutil.rmtree(destination_dir)
if not os.path.exists(source_dir):
    os.mkdir(source_dir)
shutil.copytree(source_dir, destination_dir)

import os
import json

directory_description = {"presets":{}}

def describe_directory(directory, dir_description):
    for item in sorted(os.listdir(directory), reverse=True): # reversed so that RCM boards go above Alfredo
        item_path = os.path.join(directory, item)
        if os.path.isdir(item_path):
            dir_description[item] = []
            describe_directory(item_path, dir_description[item])
        else:
            dir_description.append(item)
            #minimize json in files
            with open(item_path, 'r') as f:
                data = json.load(f)
            with open(item_path, 'w') as f:
                f.write(json.dumps(data, separators=(',', ':'), indent=None))


describe_directory(destination_dir, directory_description["presets"])

with open(os.path.join(destination_dir, 'presets.json'), 'w') as f:
    f.write(json.dumps(directory_description, separators=(',', ':'), indent=None))

import time
with open(os.path.join(destination_dir, 'build_info.json'), 'w') as f:
    f.write(json.dumps({'build_time':time.strftime('%Y-%m-%dT%H:%M:%S%z')}))