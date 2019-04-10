from flask import Flask, render_template, send_from_directory

app = Flask(__name__, static_folder='../client/build', template_folder='../client/build')

@app.route('/<path:path>')
def static_route(path):
  return send_from_directory(app.static_folder, path)

@app.route('/')
def root():
  return render_template('index.html')

app.run(host='0.0.0.0', port=8000, debug=None)
