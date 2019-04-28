from flask import Flask, render_template, send_from_directory, request, make_response
import requests
import imghdr

from errors import InvalidImage
from net.main import process_image

app = Flask(__name__, static_folder='../client/build', template_folder='../client/build')


@app.errorhandler(InvalidImage)
def handle_invalid_image(error):
    return error.to_response()


@app.route('/<path:path>')
def static_route(path):
    return send_from_directory(app.static_folder, path)


@app.route('/')
def root():
    return render_template('index.html')


@app.route('/upload', methods=['POST'])
def upload():
    if request.content_type != 'image/jpeg':
        raise InvalidImage('Invalid content-type. Need image/jpeg')
    if not imghdr.test_jpeg(request.data, None):
        raise InvalidImage()
    try:
        result = process_image(request.data)
        data = ','.join([str(e) for e in result[0]])
        image = requests.post('http://localhost:8001/neighbour', data=data, headers={'content-type': 'text/plain'})
        response = make_response(image.content)
        response.headers.set('Content-Type', 'image/jpeg')
    except Exception as error:
        print(error)
        return 'Internal server error', 500

    return response


app.run(host='0.0.0.0', port=8000, debug=None)
