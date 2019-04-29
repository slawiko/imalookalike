import requests
from flask import current_app as app

import sys


def resolve(url):
    if app.config['ENV'] == 'development':
        return 'http://localhost:8001/{}'.format(url)
    elif app.config['ENV'] == 'production':
        return 'http://index:8000/{}'.format(url)
    else:
        raise RuntimeError('Unknown mode')


def neighbour(embedding):
    data = ','.join([str(e) for e in embedding])
    url = resolve('neighbour')
    print(url, file=sys.stdout)

    try:
        response = requests.post(url, data=data, headers={'content-type': 'text/plain'})
        if response.status_code != requests.codes.ok:
            print('Not ok index response status: {}'.format(response.content), file=sys.stderr)
            return
        return response.content
    except Exception as e:
        print('Error happened during request to index: {}'.format(e), file=sys.stderr)
        return


def health():
    url = resolve('health')
    try:
        response = requests.get(url)
    except Exception as e:
        print(e)

    return response
