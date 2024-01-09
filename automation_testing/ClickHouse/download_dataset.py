import os
import yaml
import requests
import lzma

config_file= 'ch_config.yaml'
sql_file = 'ch_sql.yaml'
def main():
    with open(config_file, "r") as f:
        config = yaml.safe_load(f)

    dataset_url = config['dataset_url']
    dataset_file_name = config['dataset_file_name']
    dataset_csv_name = config['dataset_csv_name']

    ## download data
    full_url = dataset_url + dataset_file_name
    print('Starting the dataset download')

    with requests.get(full_url, stream=True) as r:
        with open(dataset_file_name , 'wb') as f:
            f.write(r.content)
    print(f'Downloaded {dataset_file_name}')

    print('Extracting the dataset from archive')
    with lzma.open(dataset_file_name) as f, open(dataset_csv_name, 'wb') as fout:
        file_content = f.read()
        fout.write(file_content)
    print(f'Extracted {dataset_csv_name}')

if __name__ == "__main__":
    main()
